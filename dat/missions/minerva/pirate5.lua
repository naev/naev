--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 5">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Minerva Station</planet>
  <done>Minerva Pirates 4</done>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Destroy Za'lek hacking station

   This mission has two ways of being done:
   1. Using brute force and killing everything.
   2. Using stealth if the player boards the drone controllers or hacking center, they can disable drones.

   There are drones patrolling the hacking center, it requires disabling two drone controllers to get there.
   There are also drones wandering around that are independent / depend on the hacking center.
--]]
local minerva = require "campaigns.minerva"
local portrait = require 'portrait'
local vn = require 'vn'
local equipopt = require 'equipopt'
require 'numstring'

logidstr = minerva.log.pirate.idstr

misn_title = _("Za'lek Hacking Center")
misn_reward = _("Cold hard credits")
misn_desc = _("Someone wants you to disable a Za'lek hacking center located near Minerva Station.")
reward_amount = 500e3

mainsys = "Gammacron"
-- Mission states:
--  nil: mission not accepted yet
--    1. fly to mainsys
--    2. mission time!
--    3. hacking center disabled
misn_state = nil


function create ()
   if not var.peek('testing') then misn.finish( false ) end
   if not misn.claim( system.get(mainsys) ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   misn.setDesc( misn_desc )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end

function accept ()
   approach_zuri()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      return
   end

   misn.accept()
   osd = misn.osdCreate( misn_title, {
      string.format( _("Go to the %s system"), mainsys ),
      _("Destroy the hacking center"),
      _("Return to Minerva Station"),
   } )
   mrk_mainsys = misn.markerAdd( system.get(mainsys) )

   shiplog.append( logidstr, string.format(_("You accepted a job from Zuri to take out a hacking center at the %s system"), mainsys) )

   hook.load("generate_npc")
   hook.land("generate_npc")
   hook.enter("enter")
   generate_npc()
end

function generate_npc ()
   npc_zuri = nil
   if planet.cur() == planet.get("Minerva Station") and misn_state < 1 then
      npc_zuri = misn.npcAdd( "approach_zuri", minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   end
end

function approach_zuri ()
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if misn_state==nil then
      -- Not accepted
      vn.na(_("You approach Zuri who seems to be motioning to you."))
      zuri(_([[""]]))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () misn_state=0 end )
      zuri(_([[""]]))
   else
      -- Accepted.
      vn.na(_("You approach Zuri."))
   end

   vn.label("menu_msg")
   zuri(_([["Is there anything you would like to know?"]]))
   vn.menu{
      {_("Ask about the job"), "job"},
      -- TODO add some other more info text
      {_("Leave"), "leave"},
   }

   vn.label("job")
   zuri(_([[""]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function enter ()
   -- Must be goal system
   if system.cur() ~= system.get(mainsys) then
      if misn_state == 2 then
         misn.osdActive(1)
         misn_state = 1
      end
      return
   end
   -- Must be in mission mode (we allow the player to run away and come back)
   if misn_state == 3 then return end
   misn_state = 2
   misn.osdActive(2)

   -- Clear pilots
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Main positions
   local pos_drone_control1 = vec2.new( -500, 2000 )
   local pos_drone_control2 = vec2.new( 18500, 9200 )
   local pos_hacking_center = vec2.new( 5000, -17000 )

   -- Define the routes
   local route0 = {
      pos_drone_control1,
      pos_drone_control2,
   }
   local route1 = {
      pos_drone_control1,
      jump.get( "Gammacron", "Daan" ):pos(),
   }
   local route2 = {
      pos_drone_control1,
      pos_hacking_center,
   }
   local route3 = {
      pos_drone_control2,
      jump.get( "Gammacron", "Daan" ):pos(),
   }
   local route4 = {
      pos_drone_control2,
      pos_hacking_center,
   }
   local route5 = {
      vec2.new( -5000, 8000 ),
      vec2.new( -9000, -5000 ),
      vec2.new( 0, -15000 ),
      vec2.new( -9000, -5000 ),
   }
   local route6 = {
      vec2.new( -4000, -7400 ),
      vec2.new( 7500, -5000 ),
      vec2.new( 15000, -7500 ),
      vec2.new( 7500, -5000 ),
   }

   -- We'll use a dynamic faction for some ships to lower faction standing damage
   local zfact = faction.dynAdd( "Za'lek", "zalek_nohit", _("Za'lek") )

   -- Controller spawning functions
   local function spawn_drone_controller( pos )
      local dc = pilot.add( "Za'lek Sting", zfact, pos, nil, {naked=true} )
      dc:rename(_("Drone Controller"))
      dc:control()
      dc:brake()
      dc:setHostile(true)
      dc:setVisplayer(true)
      dc:setHilight(true)
      -- Controllers can't detect anything
      dc:intrinsicSet( "ew_hide", -75 )
      dc:intrinsicSet( "ew_detect", -1000 )
      dc:setActiveBoard(true) -- Can board them to blow them up
      return dc
   end

   -- Groups of controlled drones
   drone_group1 = {}
   drone_group2 = {}
   all_ships = {}

   -- Fuzzes a position a bit
   local function fuzz_pos( pos, max_offset )
      max_offset = max_offset or 100
      return vec2.newP(max_offset*rnd.rnd(), 360*rnd.rnd()) + pos
   end
   local function spawn_drone( shipname, pos )
      local p = pilot.add( shipname, "Za'lek", fuzz_pos(pos) )
      -- We are nice and make the drones easier to see for this mission
      p:intrinsicSet( "ew_hide", -50 )
      table.insert( all_ships, p )
      return p
   end

   -- Spawn the main controllers
   drone_control1 = spawn_drone_controller( pos_drone_control1 )
   hook.pilot( drone_control1, "death", "drone_control1_dead" )
   hook.pilot( drone_control1, "board", "plant_explosives" )
   drone_control2 = spawn_drone_controller( pos_drone_control2 )
   hook.pilot( drone_control2, "death", "drone_control2_dead" )
   hook.pilot( drone_control2, "board", "plant_explosives" )
   hacking_center = spawn_drone_controller( pos_hacking_center )
   hacking_center:rename(_("Hacking Center"))
   hook.pilot( hacking_center, "death", "hacking_center_dead" )
   hook.pilot( hacking_center, "board", "plant_explosives" )

   local function add_patrol_group( route, ships, group, start )
      local start = start or rnd.rnd(1,#route)
      local pos = route[ start ]
      local l
      for k, s in ipairs( ships ) do
         local p = spawn_drone( s, pos )
         if k==1 then
            l = p
            local mem = p:memory()
            mem.waypoints = route
            mem.loiter = math.huge -- patrol forever
         else
            p:setLeader( l )
         end
         if group then
            table.insert( group, p )
         end
      end
   end

   local function add_guard_group( pos, ships, group )
      local l
      for k, s in ipairs( ships ) do
         local p = spawn_drone( s, pos )
         if k==1 then
            l = p
            p:changeAI("guard")
            local mem = p:memory()
            mem.guardpos = pos
            mem.guarddodist = 6e3
            mem.guardreturndist = 12e3
         else
            p:setLeader( l )
         end
         if group then
            table.insert( group, p )
         end
      end
   end

   -- Main boss, isn't necessary to kill
   local bosspos = pos_hacking_center - vec2.new( -67, -109 )
   main_boss = pilot.add( "Za'lek Mephisto", "Za'lek", bosspos, nil, {naked=true, ai="guard"} )
   all_ships[1] = main_boss
   -- Be nice and give only close-range weapons
   equipopt.zalek( main_boss, {
      type_range = {
         ["Launcher"]      = { max=0 },
         ["Fighter Bay"]   = { max=0 },
      },
   } )
   local mem            = main_boss:memory()
   mem.guardpos         = bosspos
   mem.guarddodist      = 8e3 -- Should be enough to go far out for torpedo type weapons
   mem.guardreturndist  = 15e3
   mem.doscans          = false

   -- Now add the different patrol groups
   local tiny_group = {
      "Za'lek Light Drone",
      "Za'lek Light Drone",
   }
   local small_group = {
      "Za'lek Heavy Drone",
      "Za'lek Light Drone",
      "Za'lek Light Drone",
   }
   local large_group = {
      "Za'lek Heavy Drone",
      "Za'lek Bomber Drone",
      "Za'lek Bomber Drone",
      "Za'lek Light Drone",
      "Za'lek Light Drone",
   }
   add_patrol_group( route0, small_group, nil )
   add_patrol_group( route1, small_group, drone_group1 )
   add_patrol_group( route2, small_group, drone_group1 )
   add_patrol_group( route3, small_group, drone_group2 )
   add_patrol_group( route4, small_group, drone_group2 )
   add_patrol_group( route5, small_group, drone_group2, 2 )
   add_patrol_group( route5, small_group, drone_group1, 4 )
   add_patrol_group( route6, small_group, drone_group1, 2 )
   add_patrol_group( route6, small_group, drone_group2, 4 )

   --add_guard_group( pos_drone_control1, tiny_group, drone_group1 )
   --add_guard_group( pos_drone_control2, tiny_group, drone_group2 )

   add_guard_group( fuzz_pos(bosspos,300), small_group, nil )
   add_guard_group( fuzz_pos(bosspos,300), large_group, drone_group1 )
   add_guard_group( fuzz_pos(bosspos,300), large_group, drone_group2 )

   -- Tell the player to f off
   hook.timer( 5e3,  "message_first" )
   hook.timer( 20e3, "message_warn" )
   hook.timer( 25e3, "message_hostile" )
end

function message_first ()
   hacking_center:broadcast( string.format(_("Due to military exercises, %s is under lockdown. Please evacuate the area immediately."), mainsys), true )
end

function message_warn ()
   hacking_center:broadcast( _("Unwelcome ships will be terminated. You have been warned."), true )
end

function message_hostile ()
   for k,p in pairs(all_ships) do
      p:setHostile(true)
   end
end

-- Drone controllers disable their respective drones
function plant_explosives( p )
   hook.timer( 3e3, "blowup", p )
   player.msg(_("You plant explosives on the ship."))
   player.unboard()
end
function blowup( p )
   p:setHealth( -1, -1 )
end
function drone_control1_dead ()
   for k,p in ipairs(drone_group1) do
      if p:exists() then
         p:disable()
      end
   end
   drone_control1 = nil
   drone_control_update()
end
function drone_control2_dead ()
   for k,p in ipairs(drone_group2) do
      if p:exists() then
         p:disable()
      end
   end
   drone_control2 = nil
   drone_control_update()
end
function drone_control_update ()
   if drone_control1 or drone_control2 then
      return
   end

   -- Move the boss a bit away
   if main_boss and main_boss:exists() then
      local mem = main_boss:memory()
      mem.guardpos = vec2.new( 4000, -14000 )
   end
end
function hacking_center_dead ()
   misn.markerMove( mrk_mainsys, system.get("Limbo") )
   misn.osdActive(3)
   misn_state = 3
   player.msg("#gThe hacking center has been destroyed!#0")
end
