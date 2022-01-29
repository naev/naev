--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 8">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <chance>100</chance>
  <location>Bar</location>
  <spob>Antlejos V</spob>
  <cond>require('common.antlejos').unidiffLevel() &gt;= 8</cond>
  <done>Terraforming Antlejos 7</done>
 </avail>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Go pick up a VIP ship at Griffin III and escort them to Antlejos V
--]]
local vn = require "vn"
local fmt = require "format"
local ant = require "common.antlejos"
local fleet = require "fleet"
local escort = require "escort"

local reward = ant.rewards.ant08

local retpnt, retsys = spob.getS("Antlejos V")
local mainpnt, mainsys = spob.getS("Griffin III")
local atksys1 = system.get("Yarn")
local atksys2 = system.get("Delta Polaris")

-- luacheck: globals approaching enter land escort_success miner_create miner_attacked protest (Hook functions passed by name)

function create ()
   misn.finish()
   if not misn.claim{mainsys,atksys1,atksys2} then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, _("Verner seems to be taking a break from all the terraforming and relaxing at the new spaceport bar.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_("TODO"))
   --v(fmt.f(_([["TODO"]]),
   --   {pnt=retpnt, destpnt=mainpnt, destsys=mainsys}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([["TODO"]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   local title = _("Antlejos Miner Escort")

   misn.accept()
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("TODO")),
      {pnt=mainpnt, sys=mainsys, retpnt=retpnt})
   misn.setReward( fmt.credits(reward) )
   misn.osdCreate( title, {
      fmt.f(_("Go to {pnt} ({sys} system)"),{pnt=mainpnt, sys=mainsys}),
      fmt.f(_("Escort the miner to {pnt} ({sys} system)"),{pnt=retpnt, sys=retsys}),
   })
   mem.mrk = misn.markerAdd( mainpnt )
   mem.state = 0

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==0 and spob.cur() == mainpnt then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[TODO]]))
      vn.run()

      -- Advance mission
      mem.state = 1
      misn.markerMove( mem.mrk, retpnt )
      misn.osdActive(2)

      escort.init( {"Zebra"}, {
         func_pilot_create = "miner_create",
         func_pilot_attacked = "miner_attacked",
         pilot_params = { naked = true, }, -- Don't give any weapons
      })
      escort.setDest( retpnt, "escort_success" )

   end
end

function escort_success ()
   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_([[TODO]]))
   v(_([["TODO"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.run()

   -- Done
   player.pay( reward )
   ant.log(_("TODO"))
   misn.finish(true)
end

local function fct_miner ()
   local id = "ant_miner"
   local f = faction.exists( id )
   if f then
      return f
   end
   local puaaa = ant.puaaa()
   f = faction.dynAdd( "Independent", id, _("Miner") )
   f:dynEnemy( puaaa )
   return f
end

function miner_create( p )
   p:setFaction( fct_miner() )
   p:outfitAdd( "Laser Turret MK1" )
   p:outfitAdd( "Laser Turret MK1" )
end

local last_spammed = 0
function miner_attacked( p )
   local t = naev.ticks()
   if (t-last_spammed) > 10 then
      p:comm( _("Under attack! Support requested!") )
      last_spammed = t
   end
end

local protestors = {}
local protest_lines = ant.protest_lines
local protest_id, protest_hook
function protest ()
   if protest_id == nil then
      protest_id = rnd.rnd(1,#protest_lines)
   end

   -- See surviving pilots
   local nprotestors = {}
   for _k,p in ipairs(protestors) do
      if p:exists() then
         table.insert( nprotestors, p )
      end
   end
   protestors = nprotestors
   if #protestors > 0 then
      -- Say some protest slogan
      local p = protestors[ rnd.rnd(1,#protestors) ]
      p:broadcast( protest_lines[ protest_id ], true )
      protest_id = math.fmod(protest_id, #protest_lines)+1
   end

   -- Protest again
   protest_hook = hook.timer( 15, "protest" )
end

local function spawn_protestors( pos, ships )
   local f = fleet.add( 1, ships, ant.puaaa(), pos, _("PUAAA Fighter"), {ai="baddiepos"} )
   protestors = {}
   for k,p in ipairs(f) do
      p:setHostile(true)
      table.insert( protestors, p )
   end
   if protest_hook then
      hook.rm( protest_hook )
   end
   protest_hook = hook.timer( 25, "protest" )
   return f
end

function enter ()
   if mem.state==1 and system.cur()==atksys1 then
      spawn_protestors( vec2.new( 9000, 3000 ), {"Lancelot","Lancelot","Shark","Shark"} )

   elseif mem.state==1 and system.cur()==atksys2 then
      spawn_protestors( vec2.new( -18000, 7000 ), {"Admonisher", "Ancestor", "Hyena", "Hyena"} )

   elseif mem.state==1 and system.cur()==retsys then
      local f = spawn_protestors( vec2.new(), {"Ancestor", "Ancestor", "Lancelot", "Lancelot"} )
      for k,p in ipairs(f) do
         p:changeAI( "baddiepatrol" )
         local m = p:memory()
         m.waypoints = {
            vec2.new(    0, -2000 ),
            vec2.new( -4000, 4000 ),
            vec2.new( -8000, 2000 ),
         }
      end
   end
end
