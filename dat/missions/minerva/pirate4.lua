--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 4">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 3</done>
 <notes>
  <campaign>Minerva</campaign>
  <done_evt name="Chicken Rendezvous" />
 </notes>
</mission>
--]]

--[[
-- Torture the Dvaered Spy
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local love_shaders = require "love_shaders"
local fmt = require "format"
local lmisn = require "lmisn"

local reward_amount = minerva.rewards.pirate4

local mainsys     = system.get("Fried")
local dvaeredsys  = system.get("Limbo")
local piratesys   = system.get("Effetey")
local shippos     = vec2.new( 4000, 0 ) -- asteroid field center
-- Mission states:
--  nil: mission not accepted yet
--    0. have to find spy
--    1. kidnap spy and take to torture ship
--    2. defend torture ship
mem.misn_state = nil
local mainship, spawned_dvaereds, spawned_pirates -- Non-persistent state

function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   misn.setDesc( _("Someone wants you to deal with a Dvaered spy that appears to be located at Minerva Station.") )
   misn.setReward( _("Cold hard credits") )
   misn.setTitle( _("Minerva Mole") )
   -- In the case the player failed/crashed the mission ,we want to clear the
   -- change so they can repeat it without modifying their save
   var.pop("minerva_chuckaluck_change")
end

function accept ()
   approach_pir()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( _("Minerva Mole"), {
      _("Find out who the mole is"),
   } )

   mem.sysmarker = misn.markerAdd( spob.get("Minerva Station") )

   minerva.log.pirate(_("You accepted another job from the sketchy individual to deal with a mole at Minerva Station.") )

   hook.load("generate_npc")
   hook.land("generate_npc")
   hook.enter("enter")
   hook.custom("minerva_secretcode", "found_mole")
   var.push("minerva_caninputcode",true)
   generate_npc()
end

function generate_npc ()
   mem.npc_pir = nil
   if spob.cur() == spob.get("Minerva Station") and mem.misn_state < 1 then
      mem.npc_pir = misn.npcAdd( "approach_pir", minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   end
end

function approach_pir ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.vn_pirate() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if mem.misn_state==nil then
      -- Not accepted
      vn.na(_("You approach the sketchy individual who seems to be somewhat excited."))
      pir(_([["It seems like we have finally started to get the fruits of our labour. We believe we have found the mole, and would like you to help us deal with them. Are you up to the task? Things might get a little… messy though."
They beam you a smile.]]))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline her offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () mem.misn_state=0 end )
      pir(_([["Glad to have you onboard again! So we have tracked down the mole and know from intercepted messages that they are most likely working at the station. Now, there are not many places to work at the station, so it is likely that they are involved in the gambling facility."]]))
      pir(_([["The bad news is we don't exactly know who the mole is. However, the good news is we were able to intercept a messenger. It was after a delivery so we weren't able to capture anything very interesting. But there was a small memo we found that could be a hint."
The individual produces a crumpled up dirty piece of paper that has '10k¤ 5-6-3-1' on it and hands it to you.]]))
      vn.func( function ()
         local c = commodity.new( N_("Crumpled Up Note"), N_("This is a crumpled up note that says '10k¤ 5-6-3-1' on it. How could this be related to the Dvaered spy on Minerva Station?") )
         misn.cargoAdd( c, 0 )
      end )
      pir(_([["We're still trying to figure exactly who they are, but that note is our best hint. Maybe it can be of use to you when looking for them. Once we get them we'll kindly escort them to an interrogation ship we have and we can try to get them to spill the beans."]]))
   else
      -- Accepted.
      vn.na(_("You approach the sketchy character you have become familiarized with."))
   end

   vn.label("menu_msg")
   pir(_([["Is there anything you would like to know?"]]))
   vn.menu{
      {_("Ask about the job"), "job"},
      -- TODO add some other more info text
      {_("Leave"), "leave"},
   }

   vn.label("job")
   pir(_([["How is the search going? We haven't been able to find any new leads on the mole. Our best bet is still the note I gave you that says '10k¤ 5-6-3-1' on it. Maybe it's some sort of code for something?"]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function found_mole ()
   vn.clear()
   vn.scene()
   local mole = vn.newCharacter( minerva.vn_mole() )
   vn.transition()
   vn.na(_("After the chuck-a-luck dealer's shift you follow him to a back alley in the station."))
   mole(_([["I don't recognize you, are you the new messenger? Last guy got sliced up."
They make a cutting gesture from their belly up to their neck.
"Poor kid, not the best way to leave this world."]]))
   mole(_([["Hey, wait a moment. Haven't I seen you around?"]]))
   vn.na(_("While they furrow their brows, you suddenly hear a soft thud and the sound of strong electric current. You see their eyes glaze over and muscles stiffen. They quickly drop to the ground and a familiar face appears into view."))

   vn.scene()
   local pir = vn.newCharacter( minerva.vn_pirate() )
   vn.transition()
   pir(fmt.f(_([["Great job! It seems like you found our mole. However, our job is not done here. We have to get all the information we can out of him. This is not something we can do here at Minerva Station. Here, take him on your ship and bring him to the {sys} system. There should be a ship waiting for him in the middle of the asteroid field, it might be a bit hard to spot at first."]]), {sys=mainsys}))
   pir(_([["I'll be waiting on the ship with my crew. Make sure to bring him over, however, watch out for any Dvaered patrols. We don't want them to know we have taken him. We're not expecting much trouble, but bring a strong ship just in case."]]))
   vn.run()

   -- Add illegal cargo
   local c = commodity.new( N_("Dvaered Mole"), N_("An unconscious and restrained Dvaered mole. You better not let Dvaered ships find out you are carrying this individual.") )
   c:illegalto{"Dvaered"}
   mem.cargo_mole = misn.cargoAdd( c, 0 )

   -- Signal they were caught
   var.pop("minerva_caninputcode")
   naev.trigger( "minerva_molecaught" )

   -- On to next state
   mem.misn_state = 1
   misn.osdCreate( _("Minerva Mole"), {
      fmt.f(_("Take the mole to the interrogation facility at {sys}"), {sys=mainsys}),
   } )
   misn.markerMove( mem.sysmarker, mainsys )
   misn.npcRm( mem.npc_pir )
end


function enter ()
   if mem.misn_state==2 then
      var.pop("minerva_chuckaluck_change")
      lmisn.fail(_("You were supposed to protect the interrogation ship!"))
   end

   if mem.misn_state==1 and system.cur()==mainsys then
      -- Clear system
      -- TODO maybe don't clear everything, leave some dvaered and stuff around as an obstacle
      pilot.clear()
      pilot.toggleSpawn(false)

      -- Main ship player has to protect
      mainship = pilot.add( "Pirate Rhino", "Wild Ones", shippos, _("Interrogation Ship"), {ai="guard"} )
      local aimem = mainship:memory()
      aimem.aggressive = false -- only fight back
      aimem.guardpos = shippos -- Stay around origin
      aimem.guarddodist = 2500
      aimem.guardreturndist = 4000
      aimem.enemyclose = aimem.guarddodist
      mainship:setFriendly(true)
      mainship:setVisplayer(true)
      mainship:setHilight(true)
      mainship:setNoDisable(true)
      mainship:setActiveBoard(true)
      mainship:control(true)
      mainship:stealth()

      -- Some hooks
      hook.pilot( mainship, "board", "mainship_board" )
      hook.pilot( mainship, "attacked", "mainship_attacked" )
      hook.pilot( mainship, "death", "mainship_dead" )
      hook.pilot( mainship, "stealth", "mainship_stealth" )

      -- Increase state
      mem.misn_state=2
   end
end


function mainship_board ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.vn_pirate() )
   local maikki = vn.newCharacter(_("Strangely Familiar Voice"), { color = minerva.maikkiP.colour } )
   vn.transition()
   vn.na(_("You board the ship and are once again greeting by the sketchy figure you are getting used to."))
   pir(_([["I was worried. Glad you made it here in one piece!"
Some crew members escort the Dvaered mole towards the inner part of the ship. They have fairly rough and burly complexions.]]))
   pir(_([["Don't worry about the mole, we'll take care of them from now on."
She beams you a smile.
"If all goes well, we'll get the information we were looking for in the next few periods and we can all go ]]))
   maikki(_([[You faintly hear an angry voice that sounds strangely familiar.
"What the hell are you guys doing loafing around? We have work to do! I don't pay you to sit on your bums all day!"]]))
   pir(_([[They visibly wince when they hear the angry voice.
"Let us talk about your payment."]]))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.na(_([[Suddenly an alarm starts blaring throughout the ship.]]))
   -- Using boars as slang for Dvaered
   maikki(_([[The familiar and angry voice bellows in the distance.
"Zuri! We've got incoming boars closing in our or position! Take care of it!"]]))
   pir:rename(_("Zuri"))
   pir(_([["Shit, it looks like we have Dvaered company. Make sure they don't find our ship! I dunno, just shoot them or something! I'll be manning the turrets here."
She rushes off into the depths of the ship.]]))
   vn.na(_("You race to get back to your ship before the Dvaereds jump in."))
   vn.run()

   -- Set up stuff
   misn.cargoRm( mem.cargo_mole )
   mainship:setActiveBoard(false)
   mainship:control(false) -- Should fight back a bit

   -- Dvaered jump in hooks
   spawned_dvaereds = {}
   spawned_pirates = {}
   player.msg(fmt.f(_("Sensors detecting Dvaered patrol incoming from {sys}!"), {sys=dvaeredsys}))
   hook.timer(  5.0, "msg1" )
   hook.timer( 10.0, "dv_reinforcement1" )
   hook.timer( 100.0, "dv_reinforcement1" )
   hook.timer( 110.0, "msg2" )
   hook.timer( 170.0, "dv_reinforcement2" )
   hook.timer( 200.0, "msg3" )
   hook.timer( 230.0, "dv_reinforcement3" )
   hook.timer( 240.0, "msg4" )
   hook.timer( 330.0, "pir_reinforcements" )

   -- Unboard
   player.unboard()
end

function mainship_attacked ()
   if mem.attacked_spam then
      return
   end

   mainship:comm(_("We are under attack!"), true)
   mem.attacked_spam = true
   hook.timer( 6e3, "attack_spam_over" )
end
function attack_spam_over ()
   mem.attacked_spam = false
end

function mainship_stealth( _p, status )
   if status==false then
      mainship:comm(_("We have been discovered!"), true)
   end
end

function mainship_dead ()
   var.pop("minerva_chuckaluck_change")
   lmisn.fail(_("The interrogation ship was destroyed!"))
end

function pir_reinforcements ()
   -- Get ready!
   mainship:comm(_("Reinforcements are coming!"))

   -- Reinforcement spawners
   local function addpir( shipname, leader )
      local p = pilot.add( shipname, "Wild Ones", piratesys, nil, {ai="guard"} )
      p:setFriendly(true)
      p:setLeader(leader)
      local aimem = p:memory()
      aimem.guardpos = shippos -- Try to guard the Rhino
      table.insert( spawned_pirates, p )
      return p
   end

   -- Reinforcements!
   local l = addpir( "Pirate Kestrel" )
   addpir( "Pirate Admonisher", l )
   addpir( "Pirate Admonisher", l )
   addpir( "Pirate Shark", l )
   addpir( "Pirate Vendetta", l )
   addpir( "Pirate Ancestor", l )
   addpir( "Pirate Ancestor", l )

   -- Message stuff
   mainship:comm(_("Support incoming! Hurrah!"), true)
   misn.osdCreate( _("Minerva Mole"), {
      _("Clean up the Dvaered patrols"),
   } )

   -- Detect all Dvaered deaths
   hook.timer( 3.0, "heartbeat" )
end

function msg1 ()
   mainship:comm(_("Try to buy us time! We have reinforcements coming!"), true)
end
function msg2 ()
   mainship:comm(_("They keep on coming! Keep them distracted!"), true)
end
function msg3 ()
   mainship:comm(_("Just a bit more and reinforcements should arrive!"), true)
end
function msg4 ()
   mainship:comm(_("Shit, is that a Dvaered Goddard?"), true)
end

local function spawn_dvaereds( ships )
   local plts = {}
   for k,v in ipairs(ships) do
      -- We exploit the 'guard' AI to get the Dvaered to go ontop of the
      -- interrogation ship and destroy it
      local p = pilot.add( v, "Dvaered", dvaeredsys, nil, {ai="guard"} )
      p:setVisplayer(true)
      local aimem = p:memory()
      aimem.guardpos = shippos -- Go to mainship
      aimem.guarddodist = math.huge -- don't actually want to guard
      aimem.guardreturndist = math.huge
      table.insert( plts, p )
      table.insert( spawned_dvaereds, p )
   end
   -- Set leader stuff
   local l = plts[1]
   for k = 2,#plts do
      local v = plts[k]
      v:setLeader(l)
   end
   return plts
end

function dv_reinforcement1 ()
   local dvships = {
      "Dvaered Phalanx",
      "Dvaered Vendetta",
      --"Dvaered Vendetta",
   }
   spawn_dvaereds( dvships )
end

function dv_reinforcement2 ()
   local dvships = {
      "Dvaered Vigilance",
      "Dvaered Ancestor",
      --"Dvaered Ancestor",
   }
   spawn_dvaereds( dvships )
end

function dv_reinforcement3 ()
   local dvships = {
      "Dvaered Goddard",
      "Dvaered Vigilance",
      "Dvaered Vendetta",
      "Dvaered Vendetta",
   }
   spawn_dvaereds( dvships )
end

function heartbeat ()
   local left = {}
   for k,v in ipairs(spawned_dvaereds) do
      if v:exists() then
         table.insert( left, v )
      end
   end
   spawned_dvaereds = left

   -- Still left
   if #spawned_dvaereds > 0 then
      hook.timer( 3.0, "heartbeat" )
      return
   end

   -- Add a delay for the followup
   hook.timer( 7.0, "followup" )
end

function followup ()
   -- Mission should be done
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.vn_zuri{ shader=love_shaders.hologram()} )
   local maikki = vn.newCharacter(_("Strangely Familiar Voice"), { color = minerva.maikkiP.colour } )
   vn.transition("electric")
   vn.na(_("As everything settles down you receive an incoming transmission from the interrogation ship, and the individual apparently known as 'Zuri' appears on screen.."))
   pir(_([["Damn that was close. I never thought the Dvaered would be tricky enough to trail us over here. Pretty sure the damn mole had some sort of tracking device we must have missed."]]))
   pir(_([["I'm sure you have a lot of questions, however, now is not the time for answers. We can't linger here long, the Dvaered are bound to be back, and this time in larger numbers."]]))
   maikki(_([[The still familiar voice butts in.
"Zuri! Get your ass back to navigation! Wait until I tell you how that Dvaered squealed like a pig!"]]))
   pir(_([[Zuri makes a somewhat complicated face at the unwanted interruption.
"I have some business to attend to."
She gives you a tired grin.
"Anyway, I'll wire you a reward. Meet me back at Minerva Station."]]))
   vn.na(fmt.reward(reward_amount))
   vn.func( function () -- Rewards
      player.pay( reward_amount )
      minerva.log.pirate(_("You helped defend an interrogation ship from Dvaered vessels.") )
      faction.modPlayerSingle("Wild Ones", 5)
   end )
   vn.sfxVictory()
   vn.done("electric")
   vn.run()

   -- Pirates go away
   mainship:changeAI( "pirate" )
   mainship:control(false)
   for k,v in ipairs(spawned_pirates) do
      if v:exists() then
         v:changeAI( "pirate" ) -- Guard AI is bad for stuff other than guarding
         v:control(false)
         v:setLeader( mainship )
      end
   end

   -- We done here
   misn.finish(true)
end

function abort ()
   var.pop("minerva_caninputcode")
   var.pop("minerva_chuckaluck_change")
   misn.finish(false)
end
