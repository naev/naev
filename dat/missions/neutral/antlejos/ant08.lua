--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 8">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Antlejos V</spob>
 <cond>require('common.antlejos').unidiffLevel() &gt;= 8</cond>
 <done>Terraforming Antlejos 7</done>
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
local pilotai = require "pilotai"
local ai_setup = require "ai.core.setup"

local reward = ant.rewards.ant08

local retpnt, retsys = spob.getS("Antlejos V")
local mainpnt, mainsys = spob.getS("Griffin III")
local atksys1 = system.get("Yarn")
local atksys2 = system.get("Delta Polaris")


function create ()
   if not misn.claim{mainsys,atksys1,atksys2} then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, ant.verner.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_("You find Verner enjoying a horrible smelling drink."))
   v(fmt.f(_([["Hey, you seen how everything is chugging along? This beats my wildest expectations. However, we've recently run into a small setback. We're trying to strengthen the gravitational field to minimize the effect of space adaptation syndrome, however, the core seems to be much harder than expected and our current machinery is unable to cut it. Our contacts at Gordon's Exchange will loan us a Melendez Zebra that has been adapted as a core miner, but given the recent PUAAA activity, I need someone to escort it over here from {pnt} in the {sys} system. Would you be willing to do me the favour?"]]),
      {pnt=mainpnt, sys=mainsys}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(fmt.f(_([["Great! The core miner isn't really equipped with proper defences, so it's somewhat of a sitting duck. It was recently doing some operations on the Sander seafloor, but I gave them an offer they can't refuse and they're willing to come immediately if they get some protection given the recent… incidents. I've been feeding some fake data to the PUAAA to hopefully distract them a bit, but I don't think we'll be able to sneak in such a large ship, so prepare for the worst. Try to protect the core miner at all costs on the way here from {pnt}! It'll be nearly impossible to replace if lost."]]),
      {pnt=mainpnt}))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   local title = _("Antlejos Miner Escort")

   misn.accept()
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("Verner needs you to escort a core miner from {pnt} in the {sys} system to {retpnt} to help with the terraforming efforts."),
      {pnt=mainpnt, sys=mainsys, retpnt=retpnt}))
   misn.setReward(reward)
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
      vn.na(fmt.f(_([[You land and are promptly greeted by the captain of the core miner "GE Talpini". After an exchange of some pleasantries, they show you around their ship. It is in an impressive behemoth loaded with an array of fearsome looking drills. However, you can see that the heavy modifications on the ship make it much less suited for movement in space than underground or underwater. Looks like it will be mainly up to you to keep it in one piece on the way to {pnt}.]]),
         {pnt=retpnt}))
      vn.na(fmt.f(_("The captain mentions that they will not be making any stops on the way to {pnt}, so you should make sure you have enough fuel for the entire trip. Furthermore, in order to ensure the safety of the GE Talpini, you must jump and land only after the GE Talpini has done so."),
         {pnt=retpnt}))
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

-- luacheck: globals escort_success
function escort_success ()
   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_([[You land and are met with an awe-struck Verner, who seems earnestly impressive by the massive GE Talpini that dwarfs most of the spaceport.]]))
   v(_([["Damn, look at all those drills! It's much more impressive in person that what you see on the holograms. This should make the core mining look like child's play! Thanks a bunch for bringing it over here in one piece."]]))
   v(_([["Oh, by the way, in one of our routine patrols around the system, we recently found a really curious peculiarity. It seems like there is a very faint jump lane leading out of Antlejos. It doesn't seem like it goes anywhere really interesting, but you might want to take a look. I've updated your system navigation so you should be able to use it now."]]))
   v(_([["If you're still interested, we still need all the help we can get with terraforming. There are still a lot of tasks to do!"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.run()

   -- Done
   local j1, j2 = jump.get("Antlejos", "NGC-4087")
   j1:setKnown(true)
   j2:setKnown(true)
   player.pay( reward )
   ant.log(_("You helped escort the core mining ship GE Talpini to Antlejos V to continue the terraforming efforts."))
   misn.finish(true)
end

local function fct_miner ()
   local id = "ant_miner"
   local f = faction.exists( id )
   if f then
      return f
   end
   local puaaa = ant.puaaa()
   f = faction.dynAdd( "Independent", id, _("GE Talpini") )
   f:dynEnemy( puaaa )
   return f
end

function miner_salute( p )
   p:comm( fmt.f(_("I'm counting on you on the way to {pnt}!"),{pnt=retpnt}) )
end

local firstcreate = true
-- luacheck: globals miner_create
function miner_create( p )
   p:setFaction( fct_miner() )
   p:outfitAdd( "Laser Turret MK1" )
   p:outfitAdd( "Laser Turret MK1" )
   ai_setup.setup(p)
   if firstcreate then
      hook.timer( 10, "miner_salute", p )
      firstcreate = false
   end
end

local last_spammed = 0
-- luacheck: globals miner_attacked
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
   local f = fleet.add( 1, ships, ant.puaaa(), pos, _("PUAAA Protestor"), {ai="baddiepos"} )
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
         pilotai.patrol( p, {
            vec2.new(    0, -2000 ),
            vec2.new( -4000, 4000 ),
            vec2.new( -8000, 2000 ),
         } )
      end
   end
end
