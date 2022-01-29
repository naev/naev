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
local atksys = system.get("Yarn")

-- luacheck: globals approaching enter land escort_success miner_create (Hook functions passed by name)

function create ()
   if not misn.claim{mainsys,atksys} then misn.finish() end
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

   local title = _("Miner Escort")

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
         func_ship_create = "miner_create",
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
end

local function spawn_protestors( pos, ships )
   local puaaa = ant.puaaa()
   local f = fleet.add( 1, ships, puaaa, pos, _("PUAAA Fighters"), {ai="baddiepos"} )
   for k,p in ipairs(f) do
      p:setHostile(true)
   end
   return f
end

function enter ()
   if mem.state==1 and system.cur()==atksys then
      spawn_protestors( vec2.new( 9000, 3000 ), {"Admonisher", "Hyena", "Hyena"} )

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
