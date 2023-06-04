--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 5">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 4</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Destroy Za'lek hacking station

   TODO probably should be changed from Gammacron to Fried system

   This mission has two ways of being done:
   1. Using brute force and killing everything.
   2. Using stealth if the player boards the drone controllers or hacking center, they can disable drones.

   There are drones patrolling the hacking center, it requires disabling two drone controllers to get there.
   There are also drones wandering around that are independent / depend on the hacking center.
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local equipopt = require 'equipopt'
local fmt = require "format"
local pilotai = require "pilotai"

local reward_amount = minerva.rewards.pirate5
local drone_control_update -- Forward-declared functions

local mainsys = system.get("Gammacron")
-- Mission states:
--  nil: mission not accepted yet
--    1. fly to mainsys
--    2. mission time!
--    3. hacking center disabled
mem.misn_state = nil
local all_ships, drone_control1, drone_control2, drone_group1, drone_group2, hacking_center, main_boss -- Non-persistent state


function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   misn.setDesc( _("Zuri wants you to disable a Za'lek hacking centre located near Minerva Station.") )
   misn.setReward( _("Cold hard credits") )
   misn.setTitle( _("Za'lek Hacking Centre") )
end

function accept ()
   approach_zuri()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( _("Za'lek Hacking Centre"), {
      fmt.f( _("Go to the {sys} system"), {sys=mainsys} ),
      _("Destroy the hacking centre"),
      _("Return to Minerva Station"),
   } )
   mem.mrk_mainsys = misn.markerAdd( mainsys )

   minerva.log.pirate( fmt.f(_("You accepted a job from Zuri to take out a Za'lek hacking centre at the {sys} system"), {sys=mainsys}) )
   local c = commodity.new( N_("High-Density Explosives"), N_("Explosives that can be used to detonate all sorts of critical infrastructure.") )
   misn.cargoAdd( c, 0 )

   hook.load("generate_npc")
   hook.land("generate_npc")
   hook.enter("enter")
   generate_npc()
end

function generate_npc ()
   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_zuri", minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   end
end

function approach_zuri ()
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if mem.misn_state==3 then
      vn.na(_("You approach Zuri to report your success. She listens to you intently as you explain how things went down when you took out the hacking centre."))
      zuri(_([["Great work out there. This will definitely set the Za'lek scheme back a few decaperiods. Must have been satisfying see the show of fireworks as the hacking station went down. I just wish I was able to see it, but I haven't been able to properly pilot a ship in ages. Boss just never cuts me any slack."]]))
      zuri(_([["I've wired you your reward as always."
She grins at you.
"That said, our work is still far from done. We won't stop until Minerva Station is free from Za'lek and Dvaered inference. Meet me up in the bar in a bit if you want to help."]]))
      vn.sfxVictory()
      vn.func( function () -- Rewards
         player.pay( reward_amount )
         minerva.log.pirate(_("You took down an advanced Za'lek hacking centre and got rewarded for your efforts.") )
         faction.modPlayerSingle("Wild Ones", 5)
      end )
      vn.na(fmt.reward(reward_amount))
      vn.run()
      misn.finish(true)
   elseif  mem.misn_state==nil then
      -- Not accepted
      vn.na(_("You approach Zuri who seems to be motioning to you. She seems faintly tired."))
      zuri(_([["I suppose you have a lot of questions, but first things first. Remember the suspicious Za'lek drones? Well, we were able to look into that some more, and it seems like they were part of the scouts of an advanced hacking attack on Minerva Station."]]))
      zuri(_([["Those Za'lek are always pretending to talk about science and research, and the moment you turn your back on them, they start prodding and stealing everything they can find."]]))
      zuri(_([["Long story short, we need you to take out their hacking centre nearby. This time we won't be able to use much brute force, you'll have to use more finesse. Will you help us again?"
She smiles at you.]]))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () mem.misn_state=0 end )
      zuri(fmt.f(_([["Great! We can always count on you.
So, as I was saying, it seems like the Za'lek have set up some pretty serious surveillance and hacking infrastructure in the nearby {sys} system. We sent a scout to look at it quickly and it looks like they have set up several drone controllers besides the hacking centre. The main objective is taking out the hacking centre, but it looks like it won't be possible without taking out the drone controllers too."]]), {sys=mainsys}))
      zuri(_([["The main issue is that the system is infested with Za'lek drones. Heavies, bombers, lights, you name it. While usually not a real challenge to a great pilot like you, their sheer numbers make it so that a frontal attack will only end up getting you killed. However, controlling so many drones requires infrastructure, and the Za'lek, being the lazy bums they are, are not commanding them from ships, but using the drone controllers. If you could take them out, that should incapacitate most of the drones and make taking out the hacking centre a piece of cake."]]))
      zuri(fmt.f(_([["My recommendation to you is to jump into {sys} and stay low. We've got some really good explosives we'll hook you up with that should let you take out the drone controllers or hacking stations if you can get close enough. So try to sneak past all the drones, plant the bombs on the drone controllers, and once they are out, do the same with the hacking centre. Of course, if you prefer to be old fashioned, you can take the hacking centre down with missiles, railguns, or whatever you have hand. Probably easier said than done, but I know you can do it."
She grins at you.]]), {sys=mainsys}))
      zuri(_([["That said, given the amount of drones, you should probably take a stealthy ship that is also able to take some out if you get found. They are really fast, so it's unlikely you will be able to outrun them while planting the explosives."
"Oh and about the explosives, they've already been loaded onto your ship so you don't have to worry about them."]]))
   else
      -- Accepted.
      vn.na(_("You approach Zuri."))
   end

   vn.label("menu_msg")
   zuri(_([["Is there anything you would like to know?"]]))
   vn.menu( function ()
      local opts = {
         {_("Ask about the job"), "job"},
         {_([["Are you pirates?"]]), "pirate"},
         {_("Leave"), "leave"},
      }
      return opts
   end )

   vn.label("job")
   zuri(fmt.f(_([["The job is a bit trickier than what we've done up until now, but you should be able to handle it. The main objective is to take out the Za'lek hacking centre in the {sys} system; everything else can be ignored. That said, the system is infested with drones that are being controlled by several drone controllers. You should probably take them out first if you want to have a shot at the hacking centre."]]), {sys=mainsys}))
   zuri(_([["We've loaded explosives onto your ship, so if you are able to avoid the drones and board the drone controllers or even the hacking centre, you should be able to detonate them easily. However, they should also be able to be taken down by conventional weapons. Whatever is easier for you."
She beams you a smile.
"Make sure you take a stealthy ship that can take down a couple of drones if things go wrong, and you get caught."]]))
   vn.jump("menu_msg")

   vn.label("pirate")
   zuri(_([["Pirates?"
She chuckles.
"Let me ask you one thing, do you think the decrepit Empire is good? Overtaxing and overworking the citizens to feed the ostentatious bureaucracy and aristocracy, all the while starving and exploiting the workers all over. Is this what we want?"]]))
   vn.menu{
      {_([["Yes, it brings stability."]]), "cont1"},
      {_([["No, it hampers human development."]]), "cont1"},
      {_([["How is this related to my question?"]]), "cont1"},
   }
   vn.label("cont1")
   zuri(_([["We represent those who want to stand up to this tyranny and oppression, and believe that a better universe is possible. Sure, there are those who dismissively label us as pirates or scoundrels, but that is because they represent the status quo. They do not want anyone to challenge their reign and want to continue their life of luxury."]]))
   zuri(_([["Of course, it is not that easy, as many people are complicit even against their own interests. They believe they can become part of the elite if they work hard enough, you know, the Imperial dream. However, that never happens, and they die a sad, depressed life filled with delusions of grandeur. This is not the universe we wish for humankind."]]))
   zuri(_([["It is not easy to go toe-to-toe with such a large establishment, which is why we have to work from the shadows and focus on small objectives, like Minerva Station. The Station has such potential, yet it is wasted on Za'lek and Dvaered squabbles. We wish to free the people of Minerva Station to live their full potential, even though we have to ruffle some feathers here and there."]]))
   zuri(_([["So yes, maybe we are pirates. Pirates who dream of a better universe for you and me."
Her eyes blaze with hard determination.]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function enter ()
   -- Must be goal system
   if system.cur() ~= mainsys then
      if mem.misn_state == 2 then
         misn.osdActive(1)
         mem.misn_state = 1
      end
      return
   end
   -- Must be in mission mode (we allow the player to run away and come back)
   if mem.misn_state == 3 then return end
   mem.misn_state = 2
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
   local route7 = {
      vec2.new( -7000, -7000 ),
      vec2.new( 11000, -5000 ),
      pos_hacking_center,
   }

   -- We'll use a dynamic faction for some ships to lower faction standing damage
   local zfact = faction.dynAdd( "Za'lek", "zalek_nohit", _("Za'lek") )

   -- Controller spawning functions
   local function spawn_drone_controller( pos )
      local dc = pilot.add( "Za'lek Sting", zfact, pos, _("Drone Controller"), {naked=true} )
      dc:control()
      dc:brake()
      dc:setHostile(true)
      dc:setVisplayer(true)
      dc:setHilight(true)
      -- Controllers can't detect anything
      dc:intrinsicSet( "ew_hide", -75 )
      dc:intrinsicSet( "ew_detect", -1000 )
      -- Much more bulky than normal
      dc:intrinsicSet( "shield", 500 )
      dc:intrinsicSet( "armour", 1500 )
      dc:intrinsicSet( "absorb", 30 )
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
      return vec2.newP(max_offset*rnd.rnd(), rnd.angle()) + pos
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
   hacking_center:rename(_("Hacking Centre"))
   hook.pilot( hacking_center, "death", "hacking_center_dead" )
   hook.pilot( hacking_center, "board", "plant_explosives" )

   local function add_patrol_group( route, ships, group, start )
      start = start or rnd.rnd(1,#route)
      local pos = route[ start ]
      local l
      for k, s in ipairs( ships ) do
         local p = spawn_drone( s, pos )
         if k==1 then
            l = p
            pilotai.patrol( p, route )
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
            local aimem = p:memory()
            aimem.guardpos = pos
            aimem.guarddodist = 6e3
            aimem.guardreturndist = 12e3
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
         --["Launcher"]      = { max=0 },
         ["Fighter Bay"]   = { max=0 },
      },
   } )
   local aimem            = main_boss:memory()
   aimem.guardpos         = bosspos
   aimem.guarddodist      = 8e3 -- Should be enough to go far out for torpedo type weapons
   aimem.guardreturndist  = 15e3
   aimem.doscans          = false

   -- Now add the different patrol groups
   --local tiny_group = {
   --   "Za'lek Light Drone",
   --   "Za'lek Light Drone",
   --}
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
   local sting_group = {
      "Za'lek Sting",
      "Za'lek Heavy Drone",
      "Za'lek Heavy Drone",
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
   add_patrol_group( route7, sting_group, nil )

   --add_guard_group( pos_drone_control1, tiny_group, drone_group1 )
   --add_guard_group( pos_drone_control2, tiny_group, drone_group2 )

   add_guard_group( fuzz_pos(bosspos,300), small_group, nil )
   add_guard_group( fuzz_pos(bosspos,300), large_group, drone_group1 )
   add_guard_group( fuzz_pos(bosspos,300), large_group, drone_group2 )

   -- Tell the player to f off
   hook.timer( 5.0,  "message_first" )
   hook.timer( 20.0, "message_warn" )
   hook.timer( 25.0, "message_hostile" )
end

function message_first ()
   hacking_center:broadcast( fmt.f(_("Due to military exercises, {sys} is under lockdown. Please evacuate the area immediately."), {sys=mainsys}), true )
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
   hook.timer( 3.0, "blowup", p )
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
         p:setInvisible(true)
      end
   end
   drone_control1 = nil
   drone_control_update()
end
function drone_control2_dead ()
   for k,p in ipairs(drone_group2) do
      if p:exists() then
         p:disable()
         p:setInvisible(true)
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
      local aimem = main_boss:memory()
      aimem.guardpos = vec2.new( 4000, -14000 )
   end
end
function hacking_center_dead ()
   misn.markerMove( mem.mrk_mainsys, spob.get("Minerva Station") )
   misn.osdActive(3)
   mem.misn_state = 3
   player.msg("#g".._("The hacking centre has been destroyed!").."#0")
end
