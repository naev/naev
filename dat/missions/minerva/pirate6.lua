--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 6">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 5</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Assassinate a Dvaered Warlord and Za'lek General.

--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local fmt = require "format"
local lmisn = require "lmisn"
local equipopt = require "equipopt"
local pilotai = require "pilotai"

local base = spob.get("Minerva Station")
local reward_amount = minerva.rewards.pirate6
local title = _("Limbo Mayhem")
local zlk_name = "TODO"
local dv_name = "TODO"

local mainsys = system.get("Limbo")
-- Mission states:
--  nil: mission not accepted yet
--    1. fly to position
--    2. mayhem starts
--    3. both targets down
mem.state = nil

function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   misn.setDesc(_("Zuri wants you do help a Za'lek General and Dvaered Warlord finish each other off."))
   misn.setReward(_("Cold hard credits"))
   misn.setTitle( title )
end

function accept ()
   approach_zuri()

   -- If not accepted, mem.state will still be nil
   if mem.state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( title, {
      _("Get to the position"),
      _("Wait for the targets"),
      _("Eliminate the targets"),
      _("Return to Minerva Station"),
   } )
   mem.mrk = misn.markerAdd( mainsys )

   minerva.log.pirate( fmt.f(_("You accepted a job from Zuri to assassinate both a Za'lek General and Dvaered Warlord at the same time in the {sys} system."), {sys=mainsys}) )

   hook.load("generate_npc")
   hook.land("land")
   hook.enter("enter")
   generate_npc()
end

function land ()
   if spob.cur()~=base then return end

   if mem.state==0 then
      generate_npc ()
      return
   elseif mem.state > 1 and mem.state < 3 then
      lmisn.fail(_("You were supposed to take out the targets!"))
      return
   end

   -- Mission completed
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   vn.na(_([[You quickly land after the chaos, take a deep breath and get off your ship to find Zuri waiting for you.]]))
   zuri(_([[""]]))

   vn.sfxVictory()
   vn.func( function () -- Rewards
      player.pay( reward_amount )
      minerva.log.pirate(_("TODO"))
      faction.modPlayerSingle("Wild Ones", 5)
   end )
   vn.na(fmt.reward(reward_amount))
   vn.run()
   misn.finish(true)
end

function generate_npc ()
   if spob.cur()~=base then return end
   misn.npcAdd( "approach_zuri", minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
end

function approach_zuri ()
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if  mem.state==nil then
      -- Not accepted
      vn.na(_([[You approach a strangely giddy Zuri calling you to her table.]]))
      zuri(_([["It looks like we finally have a nice window of opportunity! My sources indicate that a Za'lek General and Dvaered Warlord are going to be passing through the system at roughly the same time. You know what this means."
She winks at you.]]))
      vn.menu{
         {_([["Get them to fight each other?"]]), "cont01_fight"},
         {_([["Invite them for tea and settle this peacefully?"]]), "cont01_tea"},
         {_([["No idea."]]), "cont01_noidea"},
      }

      vn.label("cont01_fight")
      zuri(_([["Exactly! Two birds with... zero stones I guess? It's perfect!"]]))
      vn.jump("cont01")

      vn.label("cont01_tea")
      zuri(_([["Exactly! Wait, no!"
She shakes her head.
"We get them to fight each other! I thought we were on the same wavelength!"]]))
      vn.jump("cont01")

      vn.label("cont01_fight")
      zuri(_([["We get them to fight each other! I thought we were on the same wavelength!"]]))
      vn.jump("cont01")

      vn.label("cont01")
      zuri(_([["Tensions at Minerva Station are at an all high, if not only a Za'lek General goes down, but a Dvaered Warlord does too, it's going to really ruffle a lot of feathers! Pretty sure the Empire will have to get involved, and if we play our cards right, the Great Houses get kicked out unceremoniously!"]]))
      zuri(_([["You game for pulling the plug on the Dvaered and Za'lek oppressors? Should be a cinch!"]]))

      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () mem.state=0 end )
      zuri(fmt.f(_([["Great! Let me get you up to date with the plan. The Za'lek General {zlk} is set to do a rendezvous of the system soon as part of an inspection of the Za'lek border systems. Coincidently, the Dvaered Warlord {dv} is also going to be in the system after some training in a nearby system. We've pulled a few strings to make sure their timing in the {sys} system matches. Setting us up for a perfect operation."]]),
         {zlk=zlk_name, dv=dv_name, sys=mainsys}))
      zuri(_([["For this mission, I've enlisted the aid of a few helping hands. They should make sure that the mayhem gets started by launching a Za'lek drone to attack the Warlord, while simultaneously using mace rockets to draw the attention of the General. If all goes well we'll have a great nice firework display visible from Minerva Station!"]]))
      zuri(_([["However, the thing is, we have to make sure both the General and the Warlord go down, and that is where you come in! You have to make sure both the General and Warlord ships are destroyed. Do not let them get away, in particular, I would guess the Za'lek General will try to make a break for it, so make sure to break them before they can jump out of the system."]]))
      zuri(_([["With both the targets down, this will force the Houses to react, and the Empire will be forced to intervene. The ineptitude of both House Za'lek and House Dvaered, should make it unlikely that they will get a good resolution in their favour, and if we play our cards right, we can kick them out of the system."]]))
      zuri(_([["Remember, our helper will wait for you at the rendezvous location. Once you get in position, make sure to take out the targets! If all goes well, you should be able to make use of the chaos to achieve our goals! Return here so we can discuss what to do next."]]))
   else
      -- Accepted.
      vn.na(_("You approach Zuri."))
   end

   vn.label("menu_msg")
   zuri(_([["Is there anything you would like to know?"]]))
   vn.menu( function ()
      local opts = {
         {_("Ask about the job"), "job"},
         {_("Leave"), "leave"},
      }
      return opts
   end )

   vn.label("job")
   zuri(fmt.f(_([["You have to take out the Za'lek General and Dvaered Warlord that will be visiting the {sys} system. We've made sure that they should be in the system at roughly the same time. A helping hand will be there to start the mayhem, making them fight each other and you have to take advantage of the chaos to take out the priority targets."]]),
      {sys=mainsys}))
   zuri(_([["Remember, our helper will wait for you at the rendezvous location. Once you get in position, make sure to take out the targets! If all goes well, you should be able to make use of the chaos to achieve our goals! Return here so we can discuss what to do next."]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

local helper_npc, helper_drone
local meet_pos = vec2.new( -10700, 5500 )
function enter ()
   -- Must be goal system
   if system.cur() ~= mainsys then
      if mem.state ~= 0 then
         lmisn.fail(_("You were supposed to take out the targets!"))
      end
      return
   end

   helper_npc = pilot.add( "Vendetta", "Independent", meet_pos, _("Zuri's Helper"), {naked=true} )
   equipopt.dvaered( helper_npc )
   helper_npc:setInvincPlayer(true)
   helper_npc:setInvincible(true)
   helper_npc:setFriendly(true)
   helper_npc:setVisplayer(true)
   helper_npc:setHilight(true)
   helper_npc:control(true)
   helper_npc:brake()
   local m = helper_npc:memory()
   m.comm_greet = _([["Let's get this operation over with."]])

   helper_drone = pilot.add("Za'lek Light Drone", "Independent", meet_pos )
   helper_drone:setInvincPlayer(true)
   helper_drone:setInvincible(true)
   helper_drone:setFriendly(true)
   helper_drone:setVisplayer(true)
   helper_drone:setHilight(true)
   helper_drone:setLeader( helper_npc )

   hook.timer( 3, "check_dist" )
   hook.timer( 5, "greet_player" )
end

function greet_player ()
   helper_npc:comm(_("Hello there. Come here and we can start the operation."), true)
end

local start
function check_dist ()
   -- Player meets up near Limbo IIA
   if player.pos():dist( meet_pos ) < 1000 then
      start()
      return
   end

   hook.timer( 3, "check_dist" )
end

local general, warlord
local fzlk, fdvd
local attack_started
function start ()
   local csys = system.cur()

   -- Get rid of all pilots
   pilot.toggleSpawn(false)
   pilotai.clear()

   -- Have the helper tal to the player
   helper_npc:setHilight(false)
   helper_npc:comm( _("The targets have entered the system!"), true )
   misn.osdActive(2)
   mem.state = 1

   local fct_player = faction.player()
   local fct_zlk = faction.get("Za'lek")
   local fct_dv = faction.get("Dvaered")
   fzlk = faction.dynAdd( fct_zlk, "zlk_minerva", _("Za'lek"), {clear_allies=true, clear_enemies=true} )
   fdvd = faction.dynAdd( fct_dv, "dv_minerva", _("Dvaered"), {clear_allies=true, clear_enemies=true} )
   if fct_player:areEnemies( fct_zlk ) then
      fzlk:dynEnemy( fct_player )
   end
   if fct_player:areEnemies( fct_dv ) then
      fdvd:dynEnemy( fct_player )
   end

   -- General goes from Pultatis to Sollav
   local zl_start = jump.get( csys, "Pultatis" )
   local zl_target = jump.get( csys, "Sollav" )

   general = pilot.add( "Za'lek Mephisto", fzlk, zl_start, zlk_name )
   for k,s in ipairs{ "Za'lek Demon", "Za'lek Demon", "Za'lek Sting", "Za'lek Sting", "Za'lek Sting",
         "Za'lek Heavy Drone", "Za'lek Heavy Drone", "Za'lek Heavy Drone", "Za'lek Heavy Drone", "Za'lek Heavy Drone" } do
      local p = pilot.add( s, fzlk, zl_start )
      p:setLeader( general )
   end
   general:setHilight()
   general:control()
   general:moveto( zl_target:pos() )

   -- Warlord goes from Sollav to Provectus Nova
   local dv_start = jump.get( csys, "Sollav" )
   local dv_target = jump.get( csys, "Provectus Nova" )

   warlord = pilot.add( "Dvaered Goddard", fdvd, dv_start, dv_name )
   for k,s in ipairs{ "Dvaered Retribution", "Dvaered Vigilance", "Dvaered Vigilance",
         "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta",
         "Dvaered Ancestor", "Dvaered Ancestor", "Dvaered Ancestor" } do
      local p = pilot.add( s, fdvd, dv_start )
      p:setLeader( warlord )
   end
   warlord:setHilight()
   warlord:control()
   warlord:moveto( dv_target:pos() )

   hook.pilot( general, "exploded", "bigguy_died" )
   hook.pilot( warlord, "exploded", "bigguy_died" )
   hook.pilot( general, "attacked", "preempt_attack" )
   hook.pilot( warlord, "attacked", "preempt_attack" )
   hook.timer( 10, "check_arrival" )
   hook.timer( 8, "npc_chatter" )
end

-- NPC will chatter with the player
local chatter_state = 0
local chatter = {
   {_([["Wait here, I'll start the strike when they get close."]]), 7 },
   {_([["Haven't done this in ages."]]), 7 },
   {_([["Any time now."]]), 7 },
}
function npc_chatter ()
   chatter_state = chatter_state+1
   local s = chatter[ chatter_state ]
   if not s or not helper_npc:exists() or mem.state>2 then return end
   helper_npc:comm( s[1], true )
   if s[2] then
      hook.timer( s[2], "npc_chatter" )
   end
end

function preempt_attack( _p, attacker )
   if not attacker or not attacker:withPlayer() then
      return
   end

   attack_started = true
   mem.state = 2
   misn.osdActive(3)

   helper_npc:comm(_([["No! No! No! I'm out of here!"]]),true)
   helper_npc:control(false)
   pilotai.hyperspace( helper_npc )
end

local action_start
function check_arrival ()
   -- Must both be alive
   if not general:exists() or not warlord:exists() or attack_started then
      return
   end

   -- See if they will start to get further away in 10 seconds as criteria for attack
   local d = general:pos():dist( warlord )
   local t = 10
   local pos_g = general:pos() + general:vel() * t
   local pos_w = warlord:pos() + warlord:vel() * t
   if d > pos_g:dist( pos_w ) then
      action_start ()
      return
   end

   hook.timer( 1, "check_arrival" )
end

function action_start ()
   helper_npc:comm(_([["Go! Go! Go!"]]),true)
   helper_drone:comm(_([["TARGET ACQUIRED."]]),true)

   helper_npc:setHilight(false)
   helper_npc:attack( general )
   helper_drone:control(true)
   helper_drone:attack( warlord )

   hook.pilot( general, "attacked", "start_mayhem" )
   hook.pilot( warlord, "attacked", "start_mayhem" )
end

function start_mayhem( p, attacker )
   if attacker~=helper_npc and attacker~=helper_drone then
      return
   end

   -- They can die
   helper_drone:setInvincible(false)
   helper_npc:setInvincible(false)

   -- At least npc tries to get away
   helper_npc:control(false)
   pilotai.hyperspace( helper_npc )

   -- Say something
   if p==general then
      p:broadcast(_([["Dvaered fire? The Za'lek fear no Dvaered!"]]))
   else
      p:broadcast(_([["Petty Za'leks. Time to coat my hull in blood!"]]))
   end

   -- Have them duke it out
   pilotai.guard( general, meet_pos )
   pilotai.guard( warlord, meet_pos )

   -- Make the factions enemies
   fzlk:dynEnemy( fdvd )
end

function bigguy_died( _p )
   if not warlord:exists() and not general:exists() then
      -- Main objective is done
      mem.state = 3
      misn.osdActive(4)
      misn.markerMove( mem.mrk, base )
      player.msg("#g"..fmt.f(_([[Targets eliminated! Return to {spb}.]]),{spb=base}).."#0")
      pilot.toggleSpawn(true)

      -- Have some nasty Imperials jump in
      local j = jump.get( system.cur(), "Pultatis" )
      local boss = pilot.add( "Empire Peacemaker", "Empire", j )
      for k,s in ipairs{ "Empire Pacifier", "Empire Pacifier", "Empire Admonisher", "Empire Admonisher", "Empire Admonisher", "Empire Admonisher",
            "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot" } do
         local p = pilot.add( s, "Empire", j )
         p:setLeader( boss )
         p:setHostile(true)
      end
      boss:setHostile(true)
      -- Go to where the action is
      pilotai.guard( boss, meet_pos )

      hook.timer( 5, "boss_yell", boss )
   end
end

function boss_yell( boss )
   if not boss or not boss:exists() then return end

   boss:broadcast(_([["In the name of the Emperor, cease fighting or perish!"]]))
end
