--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Delivery">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <done>Dvaered Census 0</done>
 <cond>faction.playerStanding("Dvaered") &gt;= 20 and player.outfitNum("Mercenary License") &gt; 0 and (not diff.isApplied("flf_dead"))</cond>
 <notes>
  <tier>3</tier>
  <campaign>Dvaered Recruitment</campaign>
 </notes>
</mission>
--]]
--[[
   Dvaered Delivery
   This is the mission of the Dvaered Recruitment arc when the player starts to commit with Dvaered.
   The player has to transmit a parcel to another private pilot on Zhiru. The destination of the parcel is the Empire.
   But the other pilot is missing and the player has to do a series of tasks.
   At some point, the FLF is suspected to have abducted the agent (pretext to get the player enemy with them)

   Stages :
   0) Way to Zhiru to meet Dvaered contact agent
   1) Way to Caladan to meet Empire contact agent
   2) Going to speak to the contact agent
   3) Way to Theras and ambush on the FLF Lancelot
   4) Battle at Theras
   5) After the Battle against the Swan
   6) Kill the true killer of the lost agent
   7) Battle
   8) Way back to any Dvaered planet to get paid
--]]

local fmt      = require "format"
local portrait = require "portrait"
local vn       = require 'vn'
local vntk     = require 'vntk'
local dv       = require "common.dvaered"


-- Define the cargo commodity
local cargo_misn
local function _cargo()
   if not cargo_misn then
      cargo_misn = commodity.new( N_("Secret Cargo"), N_("A pink container with two stickers on it. First one reads 'teddy bears', and second one reads 'bio-hazard, do not open without protection'"), {} )
   end
   return cargo_misn
end

local port = portrait.getMil("Dvaered")
local agentPort = "neutral/female1.webp"

function create()
   mem.flfsys  = system.get("Theras")
   mem.flfoys  = system.get("Haleb") -- Origin of the target
   mem.duchpnt, mem.duchsys  = spob.getS("Fort Myuirr")

   if not misn.claim({mem.flfsys,mem.duchsys}) then misn.finish(false) end -- Claim

   misn.setNPC( _("Dvaered Soldier"), port, _("This soldier seems to want to talk to you.") )
end

function accept()
   misn.accept()

   local sys
   mem.spob1, sys      = spob.getS("Zhiru")
   mem.spob2, mem.sys2 = spob.getS("Caladan")
   local mass      = 4 -- Cargo mass. Ideally doable on schroedinger (it should be fairly easy)
   mem.sys_cur = system.cur()

   -- Discussion
   vn.clear()
   vn.scene()
   local sol = vn.newCharacter( _("Dvaered Soldier"), { image=portrait.getFullPath(port) } )
   local doaccept = false

   vn.transition( )
   sol(fmt.f(_([[I am glad to meet you, citizen {player}. Please let me introduce myself: I am...]]), {player=player.name()}))
   sol(_("Hem... Hehe..."))
   sol(_([[Actually, I am not authorized to tell you my rank nor my name or the service I am working for. Anyway, I am an employee of the High Command and we need you for a transport task.]]))
   sol( fmt.f(_([[It should be a relatively straightforward mission, except that you have to avoid getting scanned by patrol ships, including Dvaered patrol ships. What you need is a ship capable to discretely transport {tonnes} of fret to {pnt} in {sys}.]]), { tonnes=fmt.tonnes(mass), pnt=mem.spob1:name(), sys=sys }) )
   vn.na(_([[A mysterious soldier, a confidential identity and a secret delivery. You look at the small Dvaered insignia on your interlocutor's chest: are your ready to work with that Great House?
Red-painted ships, mace rockets and big nose drawings... This mission might be the gate to a further association with House Dvaered. Are you going to take part to this secret delivery?]]))
   vn.menu{
      {_("I will do it."), "accept"},
      {_("I'm not interested"), "decline"},
   }

   vn.label("decline")
   sol(_([[Too bad! I'll have to find someone else then.]]))
   vn.func( function () doaccept = false end )
   vn.done()

   vn.label("accept")
   sol(_([[Good choice, citizen!]]))
   sol(fmt.f(_([[So, as said, go to {pnt} in {sys}. There, you will meet our contact agent named Bony Boudica who will take the cargo in charge. I'll give you a datapad containing informations to identify her.
Oh, I almost forgot! There is one thing you are authorized to know: this cargo is transferred to the Empire as a part of the Dvaered-Empire collaboration program.]]), {pnt=mem.spob1:name(), sys=sys}))
   vn.func( function () doaccept = true end )
   vn.done()
   vn.run()

   -- Test acceptance
   if not doaccept then misn.finish(false) end

   -- Mission details
   mem.credits = 50e3
   misn.setTitle(_("Dvaered Delivery"))
   misn.setReward( mem.credits )
   misn.setDesc( fmt.f(_("You have to transfer a parcel to the Empire at {pnt} ({sys})"), {pnt=mem.spob1,sys=sys}))
   mem.misn_marker = misn.markerAdd( mem.spob1 )
   mem.misn_state = 0
   misn.osdCreate( _("Dvaered Delivery"), {
      fmt.f(_("Go to {sys} and land on {pnt}. Avoid being scanned on the way."), {sys=sys, pnt=mem.spob1} ),
   } )

   -- hooks
   hook.enter("enter")
   hook.load("loading")
   hook.land("land")
   hook.jumpout("escape")

   -- Add the cargo
   local cmisn = _cargo()
   mem.cid = misn.cargoAdd( cmisn, mass )
end

function enter()
   -- Track previour system
   mem.sys_prev = mem.sys_cur
   mem.sys_cur  = system.cur()

   -- Player transports cargo : just initialize scan hook.
   if mem.misn_state == 0 or mem.misn_state == 1 then
      if not mem.schook then
         mem.schook = hook.pilot( player.pilot(), "scanned", "scanned" )
      end

   -- Player jumps in Theras to intercept the Swan
   elseif mem.misn_state == 3 and system.cur() == mem.flfsys then
      pilot.toggleSpawn()
      pilot.clear()
      mem.misn_state = 4
      hook.timer(3.0,"spawnSwan")

   -- Player jump in Dvaered system to intercept Chilperic Duchmol
   elseif mem.misn_state == 6 and system.cur() == mem.duchsys then
      mem.misn_state = 7
      hook.timer(3.0,"spawnDuchmol")
   end
end

function land()
   escape() -- Test if player is not escaping a situation

   -- Player makes it to the first rendez-vous point.
   if mem.misn_state == 0 and spob.cur() == mem.spob1 then
      vn.clear()
      vn.scene()
      local agent = vn.newCharacter( _("Agent"), { image=portrait.getFullPath(agentPort) } )
      vn.transition( )
      vn.na(_([[When you step out from your ship, you remark a woman that was waiting for you on the dock. She matches the description that was made to you.]]))
      agent(fmt.f(_([[Glad to see that you made it to our rendezvous point, {player}. My name is Bony Boudica... I mean, it's my codename, of course.]]), {player=player.name()}))
      agent(fmt.f(_([[I am afraid we might have an issue to deal with, you and me. The other pilot, who was supposed to take your cargo did not show up.
But it is too risky to wait for them here. This means that we have to take the cargo to a safer destination, where the Imperials will be able to store it. I'll embark with you next time you take off, and you will go to {pnt} in {sys}.]]), {pnt=mem.spob2, sys=mem.sys2}))

      vn.menu{
         {_("Understood."), "yes"},
         {_("Look, lady, I have been hired to take that cargo and nothing more."), "no"},
      }

      vn.label("no")
      agent(_([[Well, pilot, I have three answers to that:
1) The cargo is of the highest importance and has to be transported to a safe destination at all costs
2) You have not been paid yet and if you want to, you'll have to do what I say
3) You will be rewarded for that]]))

      vn.label("yes")
      vn.done()
      vn.run()

      mem.misn_state = 1
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Delivery"), {
         fmt.f(_("Go to {sys} and land on {pnt}. Avoid being scanned on the way."), {sys=mem.sys2, pnt=mem.spob2} ),
      } )
      misn.markerRm( mem.misn_marker )
      mem.misn_marker = misn.markerAdd( mem.spob2 )

   -- Player makes it to the second rendez-vous point. An overly complicated discussion happens to show how messy the Dvaered-Imperial black-ops system is.
   elseif mem.misn_state == 1 and spob.cur() == mem.spob2 then
      hook.rm(mem.schook) -- Remove scan hook.

      vn.clear()
      vn.scene()
      local agent1 = vn.newCharacter( _("Intermediate"), { image=portrait.getFullPath(agentPort) } )
      local agent2 = vn.newCharacter( _("Imperial Agent"), { image=portrait.getFullPath("neutral/male12.webp") } )
      vn.transition( )
      vn.na(_([[A group is waiting at your dock. Before you proceed with the final landing procedure, Boudica looks at them with binoculars, and then confirms they are the Imperial team.
You dock at the spaceport and step out on the platform. Workers immediately take your cargo in charge, under the protection of armed cyborgs.]]))
      vn.na(_([[You observe the soldiers, looking for imperial insignias or gallons, in vain. You accidentally make eye contact with one of the cyborgs, which cause the blood in your veins to instantly freeze. Does this guy have scars INSIDE the eyes? How is it only possible?
A seemingly-unarmed man steps forward. He must be the imperial agent in charge of the operation.]]))
      agent2(_([[Hello, pilot. Hello, Boudica, nice to meet you again, how do you do?]]))
      agent1(_([[Not so bad on my side. Glad to see you too, Ak-Ak. I would just have preferred we meet in different circumstances. Any news of your pilot?]]))
      agent2(_([[Neh. Not a single clue. I'd say the safest bet is that the FLF is implied.]]))
      agent1(_([[How would they have known?]]))
      agent2(_([[Dunno. Anyways, you remember when I told ya we didn't know where "Shaky Swan" was?]]))
      agent1(_([[Yep.]]))
      agent2(_([[It was not true.]]))
      agent1(_([[Oooh. I'm sooo surprised! I totally was not expecting you to lie at all about that!]]))
      agent2(fmt.f(_([[Come on... Anyways, I think it's time to get that swan. If the FLF is implied in the disappearance, so is the swan. The good thing is that we know that he is supposed to have business in {sys} soon.]]),{sys=mem.flfsys}))
      agent1(_([[Nice to hear that from you. But why do you tell it to me? Just send the flying circus after him.]])) -- Flying Circus is the surname of the squadron of the red baron during WWI, And in game, I plan it to be the surname of an imperial black ops squadron.
      agent2(_([[Yeah, I've made a backup request to the Imperial Lair's Secretary, but the Second Archbishop of Mayhem has opposed a veto, so I only have access to the squadrons that are outside of his jurisdiction. And I finally got squadron 138...]]))
      agent1(_([[Man, I don't care of your Bishop of Myass, nor of the ID of your squadron. Just get them to intercept the swan.]]))
      agent2(_([[I can't. Squadron 138 has army transponders, and as you obviously know, the swan can identify them. I need an unrelated pilot to disable his ship before the squadron jumps in to catch him.]]))
      vn.na(_([[Both agents turn their faces towards you. You try to pretend you are not here, looking at the roof and scratching your nose, but soon Boudica breaks the silence.]]))
      agent1(fmt.f(_([[Okay, {player}. Once you are ready, just meet me at the bar. We'll put up a plan to catch the swan.]]), {player=player.name()}))
      vn.done()
      vn.run()

      misn.npcAdd("discussWithAg", _("Bony Boudica"), agentPort, _("This agent is an intermediate between the Empire and Dvaered.")) -- Add boudica at the bar
      misn.cargoRm( mem.cid ) -- Done with the cargo
      misn.markerRm( mem.misn_marker )
      mem.misn_state = 2

   -- Player meets Boudica again after having captured Shaky Swan.
   elseif mem.misn_state == 5 and spob.cur() == mem.spob2 then
      vn.clear()
      vn.scene()
      local agent = vn.newCharacter( _("Intermediate"), { image=portrait.getFullPath(agentPort) } )
      vn.na(_([[Bony Boudica was waiting for you at the dock.]]))
      agent(fmt.f(_([[Hi, {name}. Did you have fun out there with the Imperial pilots?
Meanwhile, our boy Ak-Ak did make his investigation. He got his cyborgs to break a few knees and now we know what happened to the lost pilot.]]),{name=player.name()}))
      agent(_([[Actually, Shaky Swan has nothing to do with that. Nor the FLF. The Empire pilot just accepted a random bounty hunt mission on their way to the rendezvous planet. And they miserably failed.]]))
      agent(_([[And now, the Imperials want to avenge their pilot. They found track to the pirate pilot, who is called 'The Death Dealer'. He is a clanless pirate, wanted by the Imperial police, as well as a few other faction's authorities. But he never broke the law in Dvaered space, where he is a peaceful trader under his real name: Chilperic Duchmol.]]))
      agent(fmt.f(_([[So, Duchmol is supposed to take off from {pnt} in {sys} with his Koala soon.
Of course, the Imperials could send their squadron to obliterate him, but as you probably already guessed at this point, that would lead to unneeded tensions between the Empire and House Dvaered. On the other hand, requiring support by the Dvaered Space Forces would mean more risk that Duchmol gets warned and escapes.]]),{pnt=mem.duchpnt,sys=mem.duchsys}))
      agent(_([[In conclusion, once more, we need you to take care of this.
And again, be ensured that your initial reward will be dramatically increased from its initial amount.]]))
      vn.done()
      vn.run()
      mem.misn_state = 6
      misn.setReward( fmt.f(_( "Hopefully much more than {credits}"), {credits=fmt.credits(mem.credits)}) )
      misn.setDesc( _("When will it end?") )
      misn.markerRm( mem.misn_marker )
      mem.misn_marker = misn.markerAdd( mem.duchpnt )

      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Delivery (gone wild)"), {
         fmt.f(_("Go to {sys} and kill Chilperic Duchmol."), {sys=mem.duchsys} ),
         fmt.f(_("Go back to {pnt} and (hopefully) get your reward."), {pnt=mem.spob2} ),
      } )

   -- Player gets finally paid
   elseif mem.misn_state == 8 and spob.cur() == mem.spob2 then
      local reward = 1e6

      vn.clear()
      vn.scene()
      local agent = vn.newCharacter( _("Intermediate"), { image=portrait.getFullPath(agentPort) } )
      vn.na(_([[After landing, you once again meet Bony Boudica. And hopefully now, she has a nice reward for you.]]))
      agent(fmt.f(_([[Hi, {name}. How do you do?]]),{name=player.name()}))
      vn.menu{
         {_("Very well for someone who met a guy named 'The Death Dealer'."), "well"},
         {_("Well... Unless there is another bastard I need to fight before you accept to pay me."), "meh"},
         {_("Am I going to get paid some day?"), "bad"},
      }

      vn.label("meh")
      agent(_([[Actually... No.]]))
      vn.jump("well")

      vn.label("bad")
      agent(_([[Of course. I know the mission did not go very well from your point of view, but sometimes unexpected things happen. And when you work for secret services of major powers, you have to fix unexpected things as soon as possible. Otherwise, the consequences might be catastrophic.]]))
      vn.jump("well")

      vn.label("well")
      agent(fmt.f(_([[Now that Chilperic Duchmol aka The Death Dealer is no more, your mission is over and it is my utmost privilege to reward you the sum of {credits} in name of the Dvaered High Command.]]),{credits=fmt.credits(reward)}))
      agent(_([[The fact that this mission did not go as planned is actually a rather good thing for you, you know: now you have proven to the Dvaered High Command that you are a reliable pilot. I don't know if you intend to continue working for them, but I have informations that suggest that they might offer you more work in the future.]]))

      vn.done()
      vn.run()

      -- TODO once the whole recruitment campaign is stabilized: faction.get("Dvaered"):modPlayerRaw(someQuantity)
      dv.addStandardLog( _([[You performed a delivery mission for the Dvaered-Empire collaboration. This mission did however turn out oddly and you ended up helping the Empire capture a shady FLF-friendly agent surnamed "Shaky Swan" and killing a pirate.]]) )
      player.pay(reward)
      misn.finish(true)
   end
end

-- Tests to determine if the player is running away
function escape()
   -- Player should be catching Shaky Swan
   if mem.misn_state == 4 then
      vntk.msg("", _([[Didn't you have a certain Shaky Swan to catch?
Your mission is a failure!]]))
      misn.finish(false)

   -- Player should be kill Chilperic Duchmol
   elseif mem.misn_state == 7 then
      vntk.msg("", _([[Didn't you have a certain Chilperic Duchmol to kill?
Your mission is a failure!]]))
      misn.finish(false)
   end
end

-- Player got scanned by a patrol ship: mission is a failure
function scanned()
   vntk.msg("", _("You receive the signal that a patrol ship has achieved a scan of your cargo. The pilot does not mention anything special, but it is obvious that they now know what you were transporting. In that condition, you have to jettison the cargo and to abort the mission."))
   misn.cargoRm( mem.cid )
   misn.finish(false)
end

function loading()
   if mem.misn_state == 2 and spob.cur() == mem.spob2 then
      misn.npcAdd("discussWithAg", _("Bony Boudica"), agentPort, _("This agent is an intermediate between the Empire and Dvaered.")) -- Add boudica at the bar
   end
end

-- Discussion with agent Boudica about the interception of the "Shaky Swan"
function discussWithAg()
   vn.clear()
   vn.scene()
   local agent = vn.newCharacter( _("Bony Boudica"), { image=portrait.getFullPath(agentPort) } )

   agent(fmt.f(_([[I don't know how much you did understand of our conversation on the spaceport, but we need you now to disable the ship of a pilot surnamed 'Shaky Swan'. He is supposed to be hanging around {sys} in the near future.]]),{sys=mem.flfsys}))

   vn.label("menu")
   vn.menu{
      {_("Ask for details on the operation"), "details"},
      {_("Tell her you need more information on the context"), "lore_menu"},
      {_("Protest and tell her you initially were only supposed to deliver a parcel"), "protest"},
      {_("Tell her you know what to do"), "accept"},
   }

   vn.label("details")
   agent(fmt.f(_([[According to our information, Shaky Swan is in the Frontier sector right now, flying a Lancelot, presumably under FLF transponder. He might soon enter {sys1} from the jump point to {sys2}.
Your task will be to approach, engage and disable his ship. Only afterwards, squadron 138 will enter the system and load the swan's ship into a Pacifier's hangar. After that, you will come back here to collect your payment.]]),{sys1=mem.flfsys,sys2=mem.flfoys}))
   agent(_([[Very important point: the Imperials insist that they want to catch the swan by themselves. Do not board his ship under any circumstances.]]))
   vn.jump("menu")

   vn.label("lore_menu")
   vn.na(_([["What do you want to ask Boudica?"]]))
   vn.menu{
      {_("What was actually in that box I was transporting?"), "box"},
      {_("Who are that Imperial agent and those cyborgs?"), "imperial"},
      {_("Who is 'Shaky Swan'?"), "swan"},
      {_("What is the 'Flying Circus'?"), "circus"},
      {_("Who is the Second Archbishop of Mayhem?"), "bishop"},
      {_("Now, back to my mission..."), "menu"},
   }

   vn.label("box")
   agent(_([[That were teddy bears, of course. Didn't you read the sticker on it?]]))
   vn.menu{
      {_("Oh, thanks for the information!"), "lore_menu"},
      {_("Actually, there was another sticker on the box, where it was written 'bio-hazard, do not open without protection'. So I doubt there are really teddy bears in it."), "teddy"},
   }

   vn.label("teddy")
   vn.na(_([[Boudica seems irritated by your remark.]]))
   agent(_([[Look, mate. If the High Command says there are teddy bears in the damn box, then there are teddy bears in the damn box. Period.]]))
   vn.jump("lore_menu")

   vn.label("imperial")
   agent(_([[He is cool, isn't he? His codename is Akward Akira, but most people in the business call him Ak-Ak. I am regularly in contact with him as he seems to be in charge of the Dvaered affairs at the Imperial secret services. And for the cyborgs, they are probably Imperial spacemarines which were put at the disposition of Ak-Ak.]]))
   vn.jump("lore_menu")

   vn.label("swan")
   agent(_([[Shaky Swan is a shifty individual. Really. I don't know much about them, except for some of their deeds. Some say they used to be an Imperial agent as well. But nowadays, they are more into stuff like assassinations, weapons dealing or even terrorism...]]))
   vn.na(_([[Boudica stops talking and seems to think.]]))
   agent(_([[... Actually, that are the kind of things Imperial agents do.
But Shaky Swan is also in touch with the FLF and provides them with all kinds of informations and weapons. The Dvaered counter-insurrection task-force, led by Colonel Urnus, has wanted to capture this swan for quite long now.
We have always suspected that the Imperials are protecting the swan. Anyways, now that a pilot employed by the Empire is missing, they are disposed to capture that individual. The only thing I hope is that they don't set the swan free once they have recovered their pilot.]]))
   vn.jump("lore_menu")

   vn.label("circus")
   agent(_([[Forget about the Flying Circus. Just remember you don't want to get in affairs with them.]]))
   vn.jump("lore_menu")

   vn.label("bishop")
   agent(_([[Second Archbishop of Mayhem? Who knows who this other comic is? Probably one of the numerous Imperial dignitaries that live at the Emperor's lair. You know, those are a pack of decadent weirdos who tend to veto things just because there was not enough honey on their ostrich pudding of last night's orgy.]]))
   vn.jump("lore_menu")

   vn.label("protest")
   vn.na(_([["Boudica sighs and turns her face to the roof."]]))
   agent(fmt.f(_([[Listen, {name}, would you by any chance like the Empire and House Dvaered to trigger an Universal War?]]),{name=player.name()}))
   vn.menu{
      {_("Yes"), "war"},
      {_("No"), "peace"},
      {_("I couldn't care less"), "war"},
   }

   vn.label("war")
   agent(_([[You obviously have no idea what you are talking about.
Shall you see the death squadrons taking off from Halir, loaded with all the kinds of deadly viruses only human madness can breed. Shall you see the fleet of the united Warlords silently flying towards humanity's doom before an helpless sky of horrified stars. Shall you see them hide the sun of Doranthex as would the black wings of death.
Shall you hear the frenetic lament of sirens while the sky of Antica gets torn apart by the trails of hypersonic bombs. Shall you hear the roaring of space stations, wounded to death by torpedoes, disseminating into the void the helpless bodies of their inhabitants.]]))
   agent(_([[Maybe you think that as a pilot, you won't endure starvation, contrary to the billions of poor souls who will struggle to survive the nuclear winter on their sterilized planets. Maybe you think you won't have to endure the gaze of the slowly dying victims of incendiary bombs, bio-weapons, climatic and seismic bombs...]]))
   agent(_([[But this won't be the worst! Because I've got my informations. And the Empire has weapons even more powerful. Even more abominable. Even more unmentionable and unimaginable. Some say the Incident was provoked by a such weapon. Imagine if they decide to make a new incident happen!
Then you would have to bare that monstrous responsibility on your shoulders. Just because you refused to catch that shaky bastard!]])) -- Clearly, she is not well informed about the Incident.
   vn.jump("peace")

   vn.label("peace")
   agent(_([[Because war is not that far away, you know. Galactopolitics has not been that unstable since the end of the Faction's war. Our work is to do our best to have the Empire and House Dvaered barely tolerate each other. If there were not, they would soon start a war, and House Sirius, Za'lek and Soromids would follow.
   I know you were only asked to transport a random parcel, but this pilot who disappears is serious business. We have to find who is behind that disappearance. And by the way, you can be ensured that you will be paid more than whatever initial reward was promised to you.]]))
   vn.jump("menu")

   vn.label("accept")
   vn.done()
   vn.run()

   mem.misn_marker = misn.markerAdd( mem.flfsys )
   mem.misn_state = 3
   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Delivery (not going as expected)"), {
      fmt.f(_("Go to {sys} and disable Shaky Swan. Target is supposed to come from {syso}."), {sys=mem.flfsys, syso=mem.flfoys} ),
      _("Wait for squadron 138 to jump in to arrest Shaky Swan. DO NOT BOARD THE SHIP YOURSELF!"),
      fmt.f(_("Go back to {pnt} to get your reward."), {pnt=mem.spob2} ),
   } )
   misn.setReward( fmt.f(_("Hopefully more than {credits}"), {credits=fmt.credits( mem.credits )}) )
   misn.setDesc( _("You are helping the Empire investigate the disappearance of one of their agents") )
end

-- Spawn the Shaky Swan for the player to disable them
local dhook
function spawnSwan()
   mem.swan = pilot.add("Lancelot", "FLF", mem.flfoys, _("Shaky Swan"), {ai="mercenary"})
   mem.swan:setHilight()
   mem.swan:setVisible() -- Not very elegant...
   dhook = hook.pilot(mem.swan,"disable","swanDisabled")
   hook.pilot(mem.swan,"exploded","swanExploded")
   hook.pilot(mem.swan,"board","swanBoarded")
   hook.pilot(mem.swan,"land","swanEscaped")
   hook.pilot(mem.swan,"jump","swanEscaped")
end

-- Player Has disabled Shaky Swan
function swanDisabled()
   mem.swan:disable() -- To be sure it won't recover.
   mem.swan:comm(_("What the? Damn!"))
   faction.modPlayerRaw(faction.get("FLF"), -5) -- Faction loss with the FLF.
   hook.rm(dhook)
   hook.timer(1.0,"spawnSquad")
   misn.osdActive(2)
end

-- All ways to get Shaky Swan's interception to fail
function swanExploded()
   vntk.msg("", _([[Good work, pilot! You can be proud for how well you managed to make short work of the Swan!
Wait a second... You were actually supposed to catch this pilot alive, weren't you?
Your mission is a pitiful failure.]]))
   misn.finish(false)
end
function swanBoarded( )
   vntk.msg("", _([[Once the boarding maneuver is over, you remember that Bony Boudica had explicitly requested you NOT to board the ship yourself.
Your mission is a failure.]]))
   misn.finish(false)
end
function swanEscaped()
   vntk.msg("", _([[It appears Shaky Swan has managed to escape the system. Your mission is a failure!]]))
   misn.finish(false)
end

-- Squadron 138 Boards Shaky Swan
function assetBoardsSwan( )
   mem.asset:broadcast(_("Hi, Swanie, we have to talk, you and me!"))
   mem.swan:rm()
   mem.asset:hyperspace()
   mem.misn_state = 5
   misn.osdActive(3)
   misn.markerRm( mem.misn_marker )
   mem.misn_marker = misn.markerAdd( mem.spob2 )
end

-- Spawn Squadron 138 to capture the swan
function spawnSquad()
   mem.asset = pilot.add("Empire Pacifier", "Empire", mem.flfoys, _("138-Asset"))
   local squa0 = pilot.add("Empire Lancelot", "Empire", mem.flfoys, _("138-Leader"))
   local squa1 = pilot.add("Empire Lancelot", "Empire", mem.flfoys, _("138-Wingman"))
   local squa2 = pilot.add("Empire Lancelot", "Empire", mem.flfoys, _("138-Wingman"))

   for i, j in ipairs({squa0,squa1,squa2}) do
      j:setLeader(mem.asset)
   end

   mem.asset:control()
   mem.asset:board(mem.swan)
   hook.pilot(mem.asset,"boarding","assetBoardsSwan")
   mem.asset:setHilight()

   -- Some messages to give the player something to read during the approach of the Imperials.
   hook.timer(  5.0, "message", {pilot = mem.swan, msg = _("Hey! Did you just disable my ship?")} )
   hook.timer( 10.0, "message", {pilot = mem.swan, msg = _("How disrespectful it is!")} )
   hook.timer( 15.0, "message", {pilot = mem.swan, msg = _("Hello?")} )
   hook.timer( 20.0, "message", {pilot = mem.swan, msg = _("Are you even listening?")} )
   hook.timer( 25.0, "message", {pilot = mem.swan, msg = _("Hello?")} )
   hook.timer( 30.0, "message", {pilot = mem.swan, msg = _("I'm Shaky Swan, you know? You're supposed to be scared of me!")} )
   hook.timer( 35.0, "message", {pilot = mem.swan, msg = _("Is anyone listening?")} )
   hook.timer( 40.0, "message", {pilot = mem.swan, msg = _("Wait! Wait! Are that imperial ships?")} )
   hook.timer( 45.0, "message", {pilot = mem.swan, msg = _("Hey, Imperial guys! You're not after me, right?")} )
   hook.timer( 50.0, "message", {pilot = mem.swan, msg = _("Are you?")} )
end

-- Spawn target Chilperic Duchmol
function spawnDuchmol()
   mem.duchmol = pilot.add("Koala", "Mercenary", mem.duchpnt, _("Chilperic Duchmol"))
   mem.duchmol:setHilight()
   mem.duchmol:control()
   mem.duchmol:hyperspace( mem.sys_prev ) -- Poor Chilperic Duchmol: he chose the system the player comes from!

   mem.atkhook = hook.pilot(mem.duchmol,"attacked","duchAttacked") -- Manage target running away if attacked
   hook.pilot(mem.duchmol,"exploded","duchExploded")
   hook.pilot(mem.duchmol,"land","duchEscaped")
   hook.pilot(mem.duchmol,"jump","duchEscaped")
end
function duchAttacked()
   hook.rm(mem.atkhook)
   mem.duchmol:runaway( player.pilot() )
end
function duchExploded()
   vntk.msg("", _([[The final explosion of an hostile ship is always a pleasant sight, isn't it? It is now time to report back to Bony Boudica.]]))
   misn.osdActive(2)
   mem.misn_state = 8
   misn.markerRm( mem.misn_marker )
   mem.misn_marker = misn.markerAdd( mem.spob2 )
end
function duchEscaped()
   vntk.msg("", _([[It appears Chilperic Duchmol has managed to escape the system. Your mission is a failure!]]))
   misn.finish(false)
end

-- Gets someone to make a message (useful for hook.timer)
function message( arg )
   local pilot = arg.pilot
   local msg = arg.msg
   if pilot:exists() then
      pilot:broadcast( msg )
   end
end
