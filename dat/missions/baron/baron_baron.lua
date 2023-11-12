--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Baron">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="Baroncomm_baron">Triggers</done_evt>
  <campaign>Baron Sauterfeldt</campaign>
 </notes>
</mission>
--]]
--[[
-- This is the first mission in the baron string.
--]]
local vn = require "vn"
local fmt = require "format"
local baron = require "common.baron"
local vni = require "vnimage"
local ccomm = require "common.comm"

local pnt, sys1 = spob.getS("Varia")
local sys2 = system.get("Ingot")

local credits = baron.rewards.baron
local pinnacle, vendetta1, vendetta2 -- Non-persistent state

local _agent1img, agent1prt = vni.pirate()
local _agent2img, agent2prt = vni.pirate()
local _agent3img, agent3prt = vni.pirate()

function create ()
   -- Can do inclusives as pilot.clear() is not called
   if not misn.claim({ sys1, sys2 }, true) then
      warn(_("Unable to claim systems that should be claimed from event!"))
      abort()
   end

   -- Mission should be accepted already from the event
   misn.accept()

   misn.setTitle(_("Baron"))
   misn.setReward(_("A tidy sum of money"))
   misn.setDesc(_("You've been hired as a courier for one Baron Sauterfeldt. Your job is to transport a holopainting from a Dvaered world to the Baron's ship."))

   misn.osdCreate(_("Baron"), {
      fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=sys1, pnt=pnt}),
      fmt.f(_("Fly to the {sys} system and dock with (board) Gauss Pinnacle"), {sys=sys2}),
   })

   mem.misn_marker = misn.markerAdd( pnt, "low" )

   mem.talked = false
   mem.stopping = false

   hook.land("land")
   hook.enter("jumpin")
   hook.takeoff("takeoff")
end

function land()
   if spob.cur() == pnt and not mem.talked then
      local npc_desc = _("These must be the 'agents' hired by this Baron Sauterfeldt. They look shifty. Why must people involved in underhanded business always look shifty?")
      mem.thief1 = misn.npcAdd("talkthieves", _("Sauterfeldt's agents"), agent1prt, npc_desc)
      mem.thief2 = misn.npcAdd("talkthieves", _("Sauterfeldt's agents"), agent2prt, npc_desc)
      mem.thief3 = misn.npcAdd("talkthieves", _("Sauterfeldt's agents"), agent3prt, npc_desc)
   end
end

function jumpin()
   if mem.talked and system.cur() == sys2 then
      local homepos = spob.get("Ulios"):pos()
      pinnacle = pilot.add( "Proteron Gauss", "Proteron", homepos + vec2.new(-400,-400), _("Pinnacle"), {ai="trader"} )
      pinnacle:setFaction("Independent")
      pinnacle:setInvincible(true)
      pinnacle:control()
      pinnacle:setHilight(true)
      pinnacle:moveto(homepos + vec2.new( 400, -400), false)
      mem.idlehook = hook.pilot(pinnacle, "idle", "idle")
      hook.pilot(pinnacle, "hail", "hail")
   end
end

function idle()
   local homepos = spob.get("Ulios"):pos()
   pinnacle:moveto(homepos + vec2.new( 400,  400), false)
   pinnacle:moveto(homepos + vec2.new(-400,  400), false)
   pinnacle:moveto(homepos + vec2.new(-400, -400), false)
   pinnacle:moveto(homepos + vec2.new( 400, -400), false)
end

function hail()
   if mem.talked then

      vn.clear()
      vn.scene()
      local plt = ccomm.newCharacter( vn, pinnacle )
      vn.transition()
      plt(_([[Your comm is answered by a communications officer on the bridge of the Pinnacle. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the Pinnacle will be brought to a halt.]]))
      vn.run()

      pinnacle:taskClear()
      pinnacle:brake()
      pinnacle:setActiveBoard(true)
      hook.pilot(pinnacle, "board", "board")
      hook.rm(mem.idlehook)
      player.commClose()
   end
end

function board()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(_([[When you arrive at your ship's airlock, the chest containing the Dvaered holopainting is already being carted onto the Pinnacle by a pair of crewmen. "You'll be wanting your reward, eh? Come along", one of them yells at you. They both chuckle and head off down the corridor.]]))
   vn.scene()
   local brn = vn.newCharacter(baron.vn_baron())
   vn.transition()
   vn.na(_([[You follow the crewmen as they push the cart through the main corridor of the ship. Soon you arrive at a door leading to a large, luxurious compartment. You can tell at a glance that these are Baron Sauterfeldt's personal quarters. The Baron himself is present. He is a large man, wearing a tailored suit that manages to make him look stately rather than pompous, a monocle, and several rings on each finger. In a word, the Baron has a taste for the extravagant.]]))
   brn(_([["Ah, my holopainting," he coos as the chest is being carried into his quarters. "At last, I've been waiting forever." The Baron does not seem to be aware of your presence at all. He continues to fuss over the holopainting even as his crewman strip away the chest and lift the frame up to the wall.]]))
   vn.na(_([[You look around his quarters. All sorts of exotic blades and other "art" works adorn his room, along with tapestries and various other holopaintings. You notice a bowl atop a velvet rug with "Fluffles" on it. Hanging above it seems to be a precariously balanced ancient blade.]]))
   vn.na(_([[The crewmen finally unpack the holopainting. You glance at the three-dimensional depiction of a Dvaered warlord, who seems to be discussing strategy with his staff. Unfortunately you don't seem to be able to appreciate Dvaered art, and you lose interest almost right away.]]))
   vn.na(_([[You cough to get the Baron's attention. He looks up, clearly displeased at the disturbance, then notices you for the first time.]]))
   brn(_([["Ah, of course," he grunts. "I suppose you must be paid for your service. Here, have some credits. Now leave me alone. I have art to admire." The Baron tosses you a couple of credit chips, and then you are once again air to him.]]))
   vn.na(_([[You are left with little choice but to return to your ship, undock, and be on your way.]]))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( credits )
   end )
   vn.na(fmt.reward(credits))
   vn.run()

   player.refuel()
   player.unboard()
   pinnacle:control(false)
   pinnacle:changeAI("flee")
   pinnacle:setHilight(false)
   pinnacle:setActiveBoard(false)
   baron.addLog( _([[You helped some selfish baron steal a Dvaered holopainting and were paid a measly sum of credits.]]) )
   misn.finish(true)
end

function talkthieves()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[The three shifty-looking patrons regard you with apprehension as you approach their table. Clearly they don't know who their contact is supposed to be. You decide to be discreet, asking them if they've ever heard of a certain Sauterfeldt. Upon hearing this, the trio visibly relaxes. They tell you that indeed they know the man you speak of, and that they have something of his in their possession. Things proceed smoothly from that point, and several hectoseconds later you are back at your ship, preparing it for takeoff while you wait for the agents to bring you your cargo.]]))
   vn.na(_([[You're halfway through your pre-flight security checks when the three appear in your docking hangar. They have a cart with them on which sits a rectangular chest as tall as a man and as long as two. Clearly this holopainting is fairly sizeable. As you watch them from your bridge's viewport, you can't help but wonder how they managed to get something that big out of a Dvaered museum unnoticed.]]))
   vn.na(_([[As it turns out, they didn't. They have only just reached the docking bridge leading into your ship when several armed Dvaered security forces come bursting into the docking hangar. They spot the three agents and immediately open fire. One of them goes down, the others hurriedly push the crate over the bridge towards your ship. Despite the drastic change in the situation, you have time to note that the Dvaered seem more interested in punishing the criminals than retrieving their possession intact.]]))
   vn.na(_([[The second agent is caught by a Dvaered bullet, and topples off the docking bridge and into the abyss below. The third manages to get the cart with the chest into your airlock before catching a round with his chest as well. As the Dvaered near your ship, you seal the airlock, fire up your engines and punch it out of the docking hangar.]]))
   vn.run()

   misn.npcRm(mem.thief1)
   misn.npcRm(mem.thief2)
   misn.npcRm(mem.thief3)

   mem.talked = true
   local c = commodity.new( N_("The Baron's holopainting"), N_("A rectangular chest containing a holopainting.") )
   mem.carg_id = misn.cargoAdd(c, 0)
   c:illegalto( "Dvaered" )

   misn.osdActive(2)
   misn.markerMove( mem.misn_marker, sys2 )

   player.takeoff()
end

function takeoff()
   if mem.talked and system.cur() == sys1 then
      hook.timer( 3, "takeoff_delay" )
   end
end

function takeoff_delay ()
   vendetta1 = pilot.add( "Dvaered Vendetta", "Dvaered", pnt, _("Dvaered Police Vendetta"), {ai="dvaered_norun"} )
   vendetta2 = pilot.add( "Dvaered Vendetta", "Dvaered", pnt, _("Dvaered Police Vendetta"), {ai="dvaered_norun"} )
   for k,v in ipairs{vendetta1, vendetta2} do
      v:setHostile(true)
   end
   vendetta1:broadcast(fmt.f(_("All troops, engage {ship_type} {ship_name}! It has broken {pnt} law!"), {ship_type=_(player.pilot():ship():baseType()), ship_name=player.ship(), pnt=pnt}), true)
end

function abort()
   misn.finish(false)
end
