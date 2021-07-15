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
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   misn.setDesc( misn_desc )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end


function accept ()
   approach_pir()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      return
   end

   misn.accept()
   osd = misn.osdCreate( misn_title, {
      _("Find out who the mole is"),
      _("Take the mole to the interrogation facility")
   } )

   shiplog.append( logidstr, _("You accepted another job from the shady individual deal with a mole at Minerva Station.") )

   hook.load("generate_npc")
   hook.land("generate_npc")
   hook.enter("enter")
   generate_npc()
end

function generate_npc ()
   npc_pir = nil
   if planet.cur() == planet.get("Minerva Station") and misn_state < 1 then
      npc_pir = misn.npcAdd( "approach_pir", minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   end
end

function approach_pir ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.vn_pirate() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if misn_state==nil then
      -- Not accepted
      vn.na(_("You approach the sketch individual who seems to be somewhat excited."))
      pir(_([["It seems like we have finally started to get the fruits of our labour. It seems like we have found the mole, and we would like you to help us deal with them. Are you up to the task? Things might get a little… messy though."
They beam you a smile.]]))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () misn_state=0 end )
      pir(_([["Glad to have you onboard again! So we have tracked down the mole and know that they are infiltrated in the station from the intercepted messages. It appears that they are most likely working at the station. Now there are not many places to work at the station so it is likely that they are involved in the gambling facility."]]))
      pir(_([["The bad news is we don't exactly know who the moles is. However, the good news is we were able to intercept a messenger. It was after a delivery so we weren't able to capture anything very interesting. But there was a small memo we found that could be a hint."
They should you a crumpled up dirty piece of paper that has '10K 5-6-3-1' on it and hands it to you.]]))
      vn.func( function ()
         local c = misn.cargoNew( _("Crumpled Up Note"), _("This is a crumpled up note that says '10K 5-6-3-1' on it. How could this be related to the Dvaered spy on Minerva Station?") )
         misn.cargoAdd( c, 0 )
      end )
      pir(_([["We're still trying to figure exactly who they are, but that note is our best hint. Maybe it can be of use to you when looking for them. Once we get them we'll kindly escort them to an interrogation ship we have and we can try to get them to spill the beans."]]))
   else
      -- Accepted.
      vn.na(_("You approach the shady character you have become familiarized with."))
   end

   vn.label("menu_msg")
   pir(_([["Is there anything you would like to know?"]]))
   vn.menu{
      {_("Ask about the job"), "job"},
      -- TODO add some other more info text
      {_("Leave"), "leave"},
   }

   vn.label("job")
   pir(_([["How is the search going? We haven't been able to find any new leads on the mole. Our best bet is still the note I gave you that says '10K 5-6-3-1' on it. Maybe it's some sort of code for something?"]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function enter ()
   -- Must be goal system
   if system.cur() ~= system.get(mainsys) then return end
   -- Must be in mission mode (we allow the player to run away nad come back)
   if misn_state < 3 then return end
   misn_state = 2

   -- Clear pilots
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Main positions
   local pos_drone_control1 = vec2.new( -500, 2000 )
   local pos_drone_control2 = vec2.new( 18500, 9200 )
   local pos_hacking_center = vec2.new( -5000, -17000 )

   -- Define the routes
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
      pos_hacking_center,
   }
   local route4 = {
      vec2.new( -5000, 8000 ),
      vec2.new( -9000, -5000 ),
      vec2.new( 0, -15000 ),
      vec2.new( -9000, -5000 ),
   }
   local route5 = {
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
      return dc
   end

   -- Fuzzes a position a bit
   local function fuzz_pos( pos, max_offset )
      max_offset = max_offset or 100
      local p = vec2.newP(max_offset*rnd.rnd(), 360*rnd.rnd()) + pos
   end

   -- Spawn the main controllers
   drone_control1 = spawn_drone_controller( pos_drone_control1 )
   hook.pilot( drone_control1, "death", "drone_control1_dead" )
   drone_control2 = spawn_drone_controller( pos_drone_control2 )
   hook.pilot( drone_control3, "death", "drone_control2_dead" )
   hacking_center = spawn_drone_controller( pos_hacking_center )
   hacking_center:rename(_("Hacking Center"))
   hook.pilot( hacking_center, "death", "hacking_center_dead" )

   -- Groups of controlled drones
   drone_group1 = {}
   drone_group2 = {}
   all_ships = {}

   -- Main boss, isn't necessary to kill
   local pos = pos_hacking_center - vec2.new( -67, -109 )
   main_boss = pilot.add( "Za'lek Mephisto", "Za'lek", pos, nil, {naked=true, ai="guard"} )
   all_ships[1] = main_boss
   -- Be nice and give only close-range weapons
   equipopt.zalek( main_boss, {
      type_range = {
         ["Launcher"]      = { max=0 },
         ["Fighter Bay"]   = { max=0 },
      },
   } )
   local mem            = main_boss:memory()
   mem.guardpos         = pos
   mem.guarddodist      = 8e3 -- Should be enough to go far out for torpedo type weapons
   mem.guardreturndist  = 15e3
   mem.doscans          = false
   -- Give him some drone friends
   for k,v in ipairs({
         "Za'lek Heavy Drone",
         "Za'lek Bomber Drone",
         "Za'lek Light Drone",
      }) do
      local p = pilot.add( v, "Za'lek", fuzz_pos(pos), nil )
      p:setLeader( main_boss )
      table.insert( all_ships, p )
   end
   for k,v in ipairs({
         "Za'lek Heavy Drone",
         "Za'lek Heavy Drone",
         "Za'lek Bomber Drone",
         "Za'lek Bomber Drone",
         "Za'lek Light Drone",
         "Za'lek Light Drone",
      }) do
      local p = pilot.add( v, "Za'lek", fuzz_pos(pos), nil )
      p:setLeader( main_boss )
      table.insert( drone_group1, p )
      table.insert( all_ships, p )
   end
   for k,v in ipairs({
         "Za'lek Heavy Drone",
         "Za'lek Heavy Drone",
         "Za'lek Bomber Drone",
         "Za'lek Bomber Drone",
         "Za'lek Light Drone",
         "Za'lek Light Drone",
      }) do
      local p = pilot.add( v, "Za'lek", fuzz_pos(pos) )
      p:setLeader( main_boss )
      table.insert( drone_group2, p )
      table.insert( all_ships, p )
   end

   local function add_patrol_group( route, ships, group )
      local pos = route[1]
      local l
      for k, s in ipairs( ships ) do
         local p = pilot.add( s, "Za'lek", pos )
         if k==1 then
            l = p
            local mem = p:memory()
            mem.waypoints = route
         else
            p:setLeader( l )
         end
         table.insert( all_ships, p )
         table.insert( group, p )
      end
   end

   -- Now add the different patrol groups
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
   add_patrol_group( route1, small_group, drone_group1 )
   add_patrol_group( route2, small_group, drone_group1 )
   add_patrol_group( route3, small_group, drone_group2 )
   add_patrol_group( route4, large_group, drone_group1 )
   add_patrol_group( route5, large_group, drone_group2 )

   -- Tell the player to f off
   hook.timer( 5e3, "message_warn" )
   hook.timer( 25e3, "message_hostile" )
end

function message_warn ()
   hacking_center:broadcast( string.format(_("Due to military exercises, %s is under lockdown. Please evacuate the area immediately."), mainsys), true )
end

function message_hostile ()
   local pp = player.pilot()
   if pp:flags().stealth then
      return
   end
   for k,p in pairs(all_ships) do
      p:setHostile(true)
   end
end

-- Drone controllers disable their respective drones
function drone_control1_dead ()
   for k,p in ipairs(drone_group1) do
      if p:exists() then
         p:disable(true)
      end
   end
end
function drone_control2_dead ()
   for k,p in ipairs(drone_group2) do
      if p:exists() then
         p:disable(true)
      end
   end
end
function hacking_center_dead ()
   misn_state = 3
end
