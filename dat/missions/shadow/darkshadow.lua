--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dark Shadow">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="Shadowcomm2">Triggers</done_evt>
  <campaign>Shadow</campaign>
 </notes>
</mission>
--]]
--[[
   This is the third mission in the "shadow" series, featuring the return of SHITMAN.
--]]
local fleet = require "fleet"
local fmt = require "format"
local shadow = require "common.shadow"
local cinema = require "cinema"
local ai_setup = require "ai.core.setup"
require "proximity"

local genbu, joe, leader, leaderdest, leaderstart, seiryuu, squads -- Non-persistent state
local spawnGenbu, spawnSquads -- Forward-declared functions

-- Mission constants
local seirplanet, seirsys = spob.getS("Edergast")
local jorekplanet1, joreksys1 = spob.getS("Manis")
local jorekplanet2, joreksys2 = spob.getS("The Wringer")
local ambushsys = system.get("Herakin")
local safesys = system.get("Eiderdown")

-- Mission info stuff
local function dest_updated(pnt, sys)
   misn.osdCreate(_("Dark Shadow"), {
      fmt.f(_("Look for Jorek on {pnt} in the {sys} system"), {pnt=pnt, sys=sys}),
   })
   misn.markerMove(mem.marker, pnt)
end

function create()
   var.push("darkshadow_active", true)

   if not misn.claim ( {seirsys, joreksys2, ambushsys} ) then
      abort()
   end

   tk.msg(_("An urgent invitation"), fmt.f(_([[Suddenly, out of nowhere, one of the dormant panels in your cockpit springs to life. It shows you a face you've never seen before in your life, but you recognize the plain grey uniform as belonging to the Four Winds.
    "Hello {player}," the face says. "You must be wondering who I am and how it is I'm talking to you like this. Neither question is important. What is important is that Captain Rebina has urgent need of your services. You are to meet her on the Seiryuu, which is currently in orbit around {pnt} in the {sys} system. Please don't ask any questions now. We expect to see you as quickly as you can make your way here."
    The screen goes dead again. You decide to make a note of this in your log. Perhaps it would be a good idea to visit the Seiryuu once more, if only to find out how they got a private line to your ship!]]), {player=player.name(), pnt=seirplanet, sys=seirsys}))
   mem.firstmarker = misn.markerAdd(seirsys, "low")
   accept() -- The player automatically accepts this mission.
end

-- This is the initial phase of the mission, when it still only shows up in the mission list. No OSD, reward or markers yet.
function accept()
   misn.setReward(_("Unknown"))
   misn.setDesc(fmt.f(_([[You have been summoned to the {sys} system, where the Seiryuu is supposedly waiting for you in orbit around {pnt}.]]), {sys=seirsys, pnt=seirplanet}))
   misn.accept()
   misn.osdDestroy() -- This is here because setDesc initializes the OSD.

   mem.stage = 1

   hook.enter("enter")
end

-- This is the "real" start of the mission. Get yer mission variables here!
local function accept2()
   mem.tick = {false, false, false, false, false}
   mem.marker = misn.markerAdd(jorekplanet1, "low")
   dest_updated(jorekplanet1, joreksys1)
   misn.setDesc(_([[You have been tasked by Captain Rebina of the Four Winds to assist Jorek McArthy.]]))
   misn.setReward(_("A sum of money."))
   mem.landhook = hook.land("land")
   mem.loadhook = hook.load("spobNpcs") -- Ensure NPCs appear at loading
   mem.jumpouthook = hook.jumpout("jumpout")
end

-- Handle boarding of the Seiryuu
function seiryuuBoard()
   seiryuu:setActiveBoard(false)
   seiryuu:setHilight(false)
   player.unboard()
   if mem.stage == 1 then -- Briefing
      tk.msg(_("Disclosure"), fmt.f(_([[You make your way through the now familiar corridors of the Seiryuu. You barely notice the strange environment anymore. It seems unimportant compared to the strange events that surround your every encounter with these Four Winds.
    You step onto the bridge where Captain Rebina is waiting for you. "Welcome back, {player}," she says. "I'm pleased to see that you decided to respond to our communication. I doubt you would have come here if you weren't willing to continue to aid us. Your presence here confirms that you are a reliable partner, so I will treat you accordingly."
    The captain motions you to take a seat at what looks like a holotable in the centre of the bridge. "Before I tell you what I've called you here for, I feel I should explain to you in full who we are, what we do, and what your part in all this is." She takes a seat opposite from yours, and leans on the holotable. "As I've said before, we are the Four Winds. Our organization is a very secretive one, as you've experienced firsthand. Very few outside our ranks know of our existence, and now you're one of those few."]]), {player=player.name()}))
      tk.msg(_("Disclosure"), fmt.f(_([["The Four Winds are old, {player}. Very old indeed. The movement dates back to old Earth, before the Space Age, even. We have been with human civilization throughout the ages, at first only in the Eastern nations, later establishing a foothold worldwide. Our purpose was to guide humanity and prevent it from making mistakes it could not afford to make. We never came out in the open, we always worked behind the scenes, from the shadows. We were diplomats, scientists, journalists, politicians' spouses, sometimes even assassins. We used any means necessary to gather information and avert disaster, when we could.
    "Of course, we didn't always succeed. We couldn't prevent the nuclear strikes on Japan, though we managed to prevent several others. We foiled the sabotage attempts on several of the colony ships launched during the First Growth, but sadly failed to do so in Maelstrom's case. We failed to stop the Faction Wars, though we managed to help the Empire gain the upper hand. Our most recent failure is the Incident - we should have seen it coming, but we were completely taken by surprise."]]), {player=player.name()}))
      shadow.addLog( fmt.f(_([[Captain Rebina has further explained the organization she works for.
    "As I've said before, we are the Four Winds. Our organization is a very secretive one, as you've experienced firsthand. Very few outside our ranks know of our existence, and now you're one of those few.
    "The Four Winds are old, {player}. Very old indeed. The movement dates back to old Earth, before the Space Age, even. We have been with human civilization throughout the ages, at first only in the Eastern nations, later establishing a foothold worldwide. Our purpose was to guide humanity and prevent it from making mistakes it could not afford to make. We never came out in the open, we always worked behind the scenes, from the shadows. We were diplomats, scientists, journalists, politicians' spouses, sometimes even assassins. We used any means necessary to gather information and avert disaster, when we could.
    "Of course, we didn't always succeed. We couldn't prevent the nuclear strikes on Japan, though we managed to prevent several others. We foiled the sabotage attempts on several of the colony ships launched during the First Growth, but sadly failed to do so in Maelstrom's case. We failed to stop the Faction Wars, though we managed to help the Empire gain the upper hand. Our most recent failure is the Incident - we should have seen it coming, but we were completely taken by surprise."]]), {player=player.name()} ) )
      tk.msg(_("Disclosure"), _([[Captain Rebina sits back in her chair and heaves a sigh. "I think that may have been when things started to change. We used to be committed to our purpose, but apparently things are different now. No doubt you remember what happened to the diplomatic exchange between the Empire and the Dvaered some time ago. Well, suffice to say that increasing the tension between the two is definitely not part of our mandate. In fact, it's completely at odds with what we stand for. And that was not just an isolated incident either. Things have been happening that suggest Four Winds involvement, things that bode ill."
    She activates the holotable, and it displays four cruisers, all seemingly identical to the Seiryuu, though you notice subtle differences in the hull designs.
    "These are our flagships. Including this ship, they are the Seiryuu, Suzaku, Byakko and Genbu. I'm given to understand that these names, as well as our collective name, have their roots in ancient Asian mythology." The captain touches another control and four portraits appear, superimposed over the ships. "These are the four captains of the flagships, which by extension makes them the highest level of authority within the Four Winds. You know me. The other three are called Giornio, Zurike and Farett."]]))
      tk.msg(_("Disclosure"), fmt.f(_([["It is my belief that one or more of my fellow captains have abandoned their mission, and are misusing their resources for a different agenda. I have been unable to find out the details of Four Winds missions that I did not order myself, which is a bad sign. I am being stonewalled, and I don't like it. I want to know what's going on, {player}, and you're going to help me do it."
    The captain turns the holotable back off so she can have your undivided attention. "I have sent Jorek on a recon mission to the planet of {pnt} in the {sys} system. He hasn't reported back to me so far, and that's bad news. Jorek is a reliable agent. If he fails to meet a deadline, then it means he is tied down by factors outside of his control, or worse. I want you to find him. Your position as an outsider will help you fly below the radar of potentially hostile Four Winds operatives. You must go to {pnt} and contact Jorek if you can, or find out where he is if you can't."
    Captain Rebina stands up, a signal that this briefing is over. You are seen to your ship by a gray-uniformed crewman. You sit in your cockpit for a few hectoseconds before disengaging the docking clamp. What Captain Rebina has told you is a lot to take in. A shadowy organization that guides humanity behind the scenes? And parts of that organization going rogue? The road ahead could well be a bumpy one.]]), {player=player.name(), pnt=jorekplanet1, sys=joreksys1}))
      accept2()
      misn.markerRm(mem.firstmarker)
      mem.stage = 2
   elseif mem.stage == 6 then -- Debriefing
      tk.msg(_("A safe return"), fmt.f(_([[You find yourself back on the Seiryuu, in the company of Jorek and the Four Winds informant. The informant is escorted deeper into the ship by grey-uniformed crew members, while Jorek takes you up to the bridge for a meeting with Captain Rebina.
    "Welcome back, Jorek, {player}," Rebina greets you on your arrival. "I've already got a preliminary report on the situation, but let's have ourselves a proper debriefing. Have a seat."
    Jorek and you sit down at the holotable in the middle of the bridge, and report on the events surrounding Jorek's retrieval. When you're done, Captain Rebina calls up a schematic view of the Genbu from the holotable.
    "It would seem that Giornio and his comrades have a vested interest in keeping me away from the truth. It's a good thing you managed to get out of that ambush and bring me that informant. I do hope he'll be able to shed more light on the situation. I've got a bad premonition, a hunch that we're going to have to act soon if we're going to avert disaster, whatever that may be. I trust that you will be willing to aid us again when that time comes, {player}. We're going to need all the help we can get. For now, you will find a modest amount of credits in your account. I will be in touch when things are clearer."
    You return to your ship and undock from the Seiryuu. You reflect that you had to run for your life this time around, and by all accounts, things will only get worse with the Four Winds in the future. A lesser person might get nervous.]]), {player=player.name()}))
      player.pay( shadow.rewards.darkshadow )
      seiryuu:control()
      seiryuu:hyperspace()
      var.pop("darkshadow_active")
      shadow.addLog( _([[You found Jorek and successfully retrieved his informant on behalf of Captain Rebina. The Genbu ambushed you, but you managed to get away and dock the Seiryuu. Captain Rebina remarked on the situation.
    "It would seem that Giornio and his comrades have a vested interest in keeping me away from the truth. It's a good thing you managed to get out of that ambush and bring me that informant. I do hope he'll be able to shed more light on the situation. I've got a bad premonition, a hunch that we're going to have to act soon if we're going to avert disaster, whatever that may be."
    She said she may need your services again in the future.]]) )
      misn.finish(true)
   end
end

-- Board hook for Joe
function joeBoard()
   tk.msg(_("An extra passenger"), fmt.f(_([[You board the Four Winds vessel, and as soon as the airlock opens a nervous looking man enters your ship. He eyes you warily, but when he sees that Jorek is with you his tension fades.
    "Come on, {player}," Jorek says. "Let's not waste any more time here. We got what we came for. Now let's give these damn vultures the slip, eh?"]]), {player=player.name()}))
   local c = commodity.new(N_("Four Winds Informant"), N_("Jorek's informant."))
   misn.cargoAdd(c, 0)
   player.unboard()
   misn.markerMove(mem.marker, seirsys)
   misn.osdActive(2)
   mem.stage = 5
end

-- Jump-out hook
function jumpout()
   mem.playerlastsys = system.cur() -- Keep track of which system the player came from
   hook.rm(mem.poller)
   hook.rm(mem.spinter)
end

-- Enter hook
function enter()
   if system.cur() == seirsys then
      seiryuu = pilot.add( "Pirate Kestrel", shadow.fct_fourwinds(), vec2.new(300, 300) + seirplanet:pos(), _("Seiryuu"), {ai="trader"} )
      seiryuu:setInvincible(true)
      seiryuu:control()
      if mem.stage == 1 or mem.stage == 6 then
         seiryuu:setActiveBoard(true)
         seiryuu:setHilight(true)
         hook.pilot(seiryuu, "board", "seiryuuBoard")
      else
         seiryuu:setNoboard(true)
      end
   elseif system.cur() == joreksys2 and mem.stage == 3 then
      pilot.clear()
      pilot.toggleSpawn(false)
      spawnSquads(false)
   elseif system.cur() == joreksys2 and mem.stage == 4 then
      pilot.clear()
      pilot.toggleSpawn(false)
      player.landAllow(false, _("Landing permission denied. Our docking clamps are currently undergoing maintenance."))
      -- Meet Joe, our informant.
      joe = pilot.add( "Vendetta", shadow.fct_fourwinds(), vec2.new(-500, -4000), _("Four Winds Informant"), {ai="trader"} )
      joe:control()
      joe:setHilight(true)
      joe:setVisplayer()
      joe:setInvincible(true)
      joe:disable()
      spawnSquads(true)

      -- Make everyone visible for the cutscene
      squadVis(true)

      -- The cutscene itself
      local delay = 0
      mem.zoomspeed = 2500
      hook.timer(delay, "playerControl", true)
      delay = delay + 2.0
      hook.timer(delay, "zoomTo", joe)
      delay = delay + 4.0
      hook.timer(delay, "showText", _([[Jorek> "That's my guy. We got to board his ship and get him off before we jump."]]))
      delay = delay + 4.0
      hook.timer(delay, "zoomTo", leader[1])
      delay = delay + 1.0
      hook.timer(delay, "showText", _([[Jorek> "Watch out for those patrols though. If they spot us, they'll be all over us."]]))
      delay = delay + 2.0
      hook.timer(delay, "zoomTo", leader[2])
      delay = delay + 3.0
      hook.timer(delay, "zoomTo", leader[3])
      delay = delay + 2.0
      hook.timer(delay, "showText", _([[Jorek> "They're tougher than they look. Don't underestimate them."]]))
      delay = delay + 3.0
      hook.timer(delay, "zoomTo", leader[4])
      delay = delay + 4.0
      hook.timer(delay, "zoomTo", leader[5])
      delay = delay + 4.0
      hook.timer(delay, "zoomTo", player.pilot())
      hook.timer(delay, "playerControl", false)

      -- Hide everyone again
      delay = delay + 2.0
      hook.timer(delay, "squadVis", false)
      delay = delay + 0.001
      -- ...except the leaders.
      hook.timer(delay, "leaderVis", true)

      hook.pilot(joe, "board", "joeBoard")
      mem.poller = hook.timer(0.5, "patrolPoll")
   elseif system.cur() == ambushsys and mem.stage == 4 then
      tk.msg(_("You forgot the informant!"), fmt.f(_([[Jorek is enraged. "Dammit, {player}! I told you to pick up that informant on the way! Too late to go back now. I'll have to think of somethin' else. I'm disembarkin' at the next spaceport, don't bother taking me back to the Seiryuu."]]), {player=player.name()}))
      shadow.addLog( _([[You failed to pick up Jorek's informant. As such, he refused to allow you to take him to the Seiryuu.]]) )
      abort()
   elseif system.cur() == ambushsys and mem.stage == 5 then
      pilot.clear()
      pilot.toggleSpawn(false)
      hook.timer(0.5, "invProximity", { location = jump.pos(system.cur(), "Suna"), radius = 8000, funcname = "startAmbush" }) -- Starts an inverse proximity poll for distance from the jump point.
   elseif system.cur() == safesys and mem.stage == 5 then
      mem.stage = 6 -- stop spawning the Genbu
   elseif mem.genbuspawned and mem.stage == 5 then
      spawnGenbu(mem.playerlastsys) -- The Genbu follows you around, and will probably insta-kill you.
      continueAmbush()
   end
end

function spawnSquads(highlight)
   -- Start positions for the leaders
   leaderstart = {
      vec2.new(-2500, -1500),
      vec2.new(2500, 1000),
      vec2.new(-3500, -4500),
      vec2.new(2500, -2500),
      vec2.new(-2500, -6500),
   }

   -- Leaders will patrol between their start position and this one
   leaderdest = {
      vec2.new(2500, -1000),
      vec2.new(-500, 1500),
      vec2.new(-4500, -1500),
      vec2.new(2000, -6000),
      vec2.new(1000, -1500),
   }

   squads = {}

   -- Shorthand notation for the leader pilots
   leader = {}

   for i, start in ipairs(leaderstart) do
      squads[i] = fleet.add( 4, "Vendetta", shadow.fct_rogues(), leaderstart[i], _("Four Winds Patrol") )
      for j, k in ipairs(squads[i]) do
         hook.pilot(k, "attacked", "attacked")
         k:outfitRm("all")
         k:outfitAdd("Cheater's Laser Cannon", 6) -- Equip these fellas with unfair weaponry
         ai_setup.setup(k)
         k:setNoDisable()
      end
      squads[i][1]:control() -- Only need to control leader. Others will follow
      leader[i] = squads[i][1]
   end

   leaderVis(highlight)

   -- Kickstart the patrol sequence
   for i, j in ipairs(leader) do
      j:moveto(leaderdest[i], false)
   end

   -- Set up the rest of the patrol sequence
   for _, j in ipairs(leader) do
      hook.pilot(j, "idle", "leaderIdle")
   end
end

-- Makes the squads either visible or hides them
function squadVis(visible)
   for _, squad in ipairs(squads) do
      for _, k in ipairs(squad) do
         k:setVisplayer(visible)
      end
   end
end

-- Makes the leaders visible or hides them, also highlights them (or not)
function leaderVis(visible)
   for _, j in ipairs(leader) do
      j:setVisplayer(visible)
      j:setHilight(visible)
   end
end

-- Hook for hostile actions against a squad member
function attacked()
   for _, squad in ipairs(squads) do
      for _, k in ipairs(squad) do
         k:hookClear()
         k:control(false)
         k:setHostile()
      end
   end
end

-- Hook for the idle status of the leader of a squad.
-- Makes the squads patrol their routes.
function leaderIdle(pilot)
   for i, j in ipairs(leader) do
      if j == pilot then
         if mem.tick[i] then pilot:moveto(leaderdest[i], false)
         else pilot:moveto(leaderstart[i], false)
         end
         mem.tick[i] = not mem.tick[i]
         return
      end
   end
end

-- Check if any of the patrolling leaders can see the player, and if so intercept.
function patrolPoll()
   for j, patroller in ipairs(leader) do
      if patroller ~= nil and patroller:exists() and vec2.dist(player.pos(), patroller:pos()) < 1200 then
         patroller:broadcast(_("All pilots, we've detected McArthy on that ship! Break and intercept!"))
         attacked()
         return
      end
   end
   mem.poller = hook.timer(0.5, "patrolPoll")
end

-- Spawns the Genbu
function spawnGenbu(sys)
   genbu = pilot.add( "Pirate Kestrel", shadow.fct_fourwinds(), sys, _("Genbu") )
   genbu:outfitRm("all")
   genbu:outfitAdd("Heavy Laser Turret", 3)
   genbu:outfitAdd("Cheater's Ragnarok Beam", 3) -- You can't win. Seriously.
   ai_setup.setup(genbu)
   genbu:control()
   genbu:setHilight()
   genbu:setVisplayer()
   genbu:setNoDeath()
   genbu:setNoDisable()
   mem.genbuspawned = true
end

-- The initial ambush cutscene
function startAmbush()
   spawnGenbu(system.get("Anrique"))

   local delay = 0
   mem.zoomspeed = 4500
   hook.timer(delay, "playerControl", true)
   hook.timer(delay, "zoomTo", genbu)
   delay = delay + 5.0
   hook.timer(delay, "showMsg", {_("Ambush!"), fmt.f(_([[Suddenly, your long range sensors pick up a ship jumping in behind you. Jorek checks the telemetry beside you. Suddenly, his eyes go wide and he groans. The Four Winds informant turns pale.
    "Oh, damn it all," Jorek curses. "{player}, that's the Genbu, Giornio's flagship. I never expected him to take an interest in me personally! Damn, this is bad. Listen, if you have anything to boost our speed, now would be the time. We got to get outta here as if all hell was hot on our heels, which it kinda is! If that thing catches us, we're toast. I really mean it, you don't wanna get into a fight against her, not on your own. Get your ass movin' to Sirius space. Giornio ain't gonna risk getting into a scrap with the Sirius military, so we'll be safe once we get there. Come on, what are you waitin' for? Step on it!"]]), {player=player.name()})})
   delay = delay + 1.0
   hook.timer(delay, "zoomTo", player.pilot())
   hook.timer(delay, "playerControl", false)
   hook.timer(delay, "continueAmbush")
end

-- The continuation of the ambush, for timer purposes
function continueAmbush()
   genbu:setHostile()
   genbu:attack(player.pilot())
   mem.waves = 0
   mem.maxwaves = 5
   mem.spinter = hook.timer(5.0, "spawnInterceptors")
end

-- Spawns a wing of Lancelots that intercept the player.
function spawnInterceptors()
   local inters = fleet.add( 3, "Lancelot", shadow.fct_rogues(), genbu:pos(), _("Four Winds Lancelot") )
   for _, j in ipairs(inters) do
      j:outfitRm("all")
      j:outfitAdd("Cheater's Laser Cannon", 4) -- Equip these fellas with unfair weaponry
      j:outfitAdd("Engine Reroute", 1)
      j:outfitAdd("Improved Stabilizer", 1)
      ai_setup.setup(j)
      j:setLeader( genbu )
      j:control()
      j:attack(player.pilot())
   end
   if mem.waves < mem.maxwaves then
      mem.waves = mem.waves + 1
      mem.spinter = hook.timer(25.0, "spawnInterceptors")
   end
end

-- Land hook
function land()
   if spob.cur() == jorekplanet1 and mem.stage == 2 then
      -- Thank you player, but our SHITMAN is in another castle.
      tk.msg(_("No Jorek"), _([[You step into the bar, expecting to find Jorek McArthy sitting somewhere at a table. However, you don't see him anywhere. You decide to go for a drink to contemplate your next move. Then, you notice the barman is giving you a curious look.]]))
   end
   spobNpcs()
end

-- Get the NPCs to appear
function spobNpcs()
   if spob.cur() == jorekplanet1 and mem.stage == 2 then
      mem.barmanNPC = misn.npcAdd("barman", _("Barman"), "neutral/barman.webp", _("The barman seems to be eyeing you in particular."), 4)
   elseif spob.cur() == jorekplanet2 and mem.stage == 3 then
      mem.joreknpc = misn.npcAdd("jorek", _("Jorek"), "neutral/unique/jorek.webp", _("There he is, Jorek McArthy, the man you've been chasing across half the galaxy. What he's doing on this piece of junk is unclear."), 4)
   end
end

-- NPC hook
function barman()
   tk.msg(_("A tip from the barman"), fmt.f(_([[You meet the barman's stare. He hesitates for a moment, then speaks up.
    "Hey... Are you {player} by any chance?"
    You tell him that yes, that's you, and ask how he knows your name.
    "Well, your description was given to me by an old friend of mine. His name is Jarek. Do you know him?"
    You tell him that you don't know anyone by the name of Jarek, but you do know a man named Jorek. The barman visibly relaxes when he hears that name.
    "Ah, good. You're the real deal then. Can't be too careful in times like these, you know. Anyway, old Jorek was here, but he couldn't stay. He told me to keep an eye out for you, said you'd be coming to look for him." The barman glances around to make sure nobody is within earshot, even though the bar's music makes it difficult to overhear anyone who isn't standing right next to you. "I have a message for you. Go to the {sys} system and land on {pnt}. Jorek will be waiting for you there. But you better be ready for some trouble. I don't know what kind of trouble it is, but Jorek is never in any kind of minor trouble. Don't say I didn't warn you."
    You thank the barman, pay for your drink, and prepare to head back to your ship, wondering whether your armaments will be enough to deal with whatever trouble Jorek is in.]]), {player=player.name(), sys=joreksys2, pnt=jorekplanet2}))
   dest_updated(jorekplanet2, joreksys2)
   misn.npcRm(mem.barmanNPC)
   mem.stage = 3
end

-- NPC hook
function jorek()
   tk.msg(_("Still an unpleasant man"), fmt.f(_([["Well hello there {player}," Jorek says when you approach his table. "It's about damn time you showed up. I've been wastin' credits on this awful swill for days now."
    Not at all surprised that Jorek is still as disagreeable as the last time you encountered him, you decide to ask him to explain the situation, beginning with how he knew that it was you who would be coming for him. Jorek laughs heartily at that.
    "Ha! Of course it was going to be you. Who else would that lass Rebina send? She's tough as nails, that girl, but I know how her mind works. She's cornered, potential enemies behind every door in the organization. And you have done us a couple of favours already. In fact, you're the only one she can trust outside her own little circle of friends, and right now I'm not too sure how far she trusts those. Plus, she really has a keen nose when it comes to sniffin' out reliable people, and she knows it. Yeah, I knew she'd send you to find me."
    That answers one question. But you still don't know why Jorek hasn't been reporting in like he should have.
    "Yeah, right, about that. You know about the deal with the other branches getting too big for their britches? Good. Well, I've been lookin' into that, pokin' my nose into their business. Since I'm dealin' with my fellow Shadows here, I couldn't afford to give myself away. So that's that. But there's more."]]), {player=player.name()}))
   tk.msg(_("Still an unpleasant man"), _([["I dunno if you've seen them on your way here, but there's guys of ours hangin' around in the system. And when I say guys of ours, I mean guys of theirs, since they sure ain't our guys any more. They've been on my ass ever since I left Manis, so I think I know what they want. They want to get me and see what I know, or maybe they just want to blow me into space dust. Either way, I need you to help me get out of this rathole."
    You ask Jorek why he didn't just lie low on some world until the coast was clear, instead of coming to this sink for the dregs of intergalactic society.
    "It ain't that simple," Jorek sighs. "See, I got an inside man. A guy in their ranks who wants out. I need to get him back to the old girl so he can tell her what he knows firsthand. He's out there now, with the pack, so we need to pick him up on our way out. Now, there's two ways we can do this. We can either go in fast, grab the guy, get out fast before the wolves get us. Or we can try to fight our way through. Let me warn you though, these guys mean business, and they're not your average pirates. Unless you got a really tough ship, I recommend you run."
    Jorek sits back in his chair. "Well, there you have it. I'll fill you in on the details once we're spaceborne. Show me to your ship, buddy, and let's get rollin'. I've had enough of this damn place."]]))
   misn.npcRm(mem.joreknpc)
   local c = commodity.new(N_("Jorek"), N_("An unpleasant man."))
   misn.cargoAdd(c, 0)

   misn.osdCreate(_("Dark Shadow"), {
      _("Fetch the Four Winds informant from his ship"),
      fmt.f(_("Return Jorek and the informant to the Seiryuu in the {sys} system"), {sys=seirsys}),
   })

   mem.stage = 4
end

-- Capsule function for camera.set, for timer use
function zoomTo(target)
   camera.set(target, false, mem.zoomspeed)
end

-- Capsule function for player.msg, for timer use
function showText(text)
   player.msg(text)
end

-- Capsule function for tk.msg, for timer use
function showMsg(content)
   tk.msg(content[1], content[2])
end

-- Capsule function for player.pilot():control(), for timer use
-- Also saves the player's velocity.
local pvel
function playerControl( status )
   local pp = player.pilot()
   if status then
      cinema.on()
      pp:control(false)
      pvel = pp:vel()
      pp:setVel(vec2.new(0, 0))
   else
      cinema.off()
      pp:setVel(pvel)
   end
end

-- Poll for player proximity to a point in space. Will trigger when the player is NOT within the specified distance.
-- argument trigger: a table containing:
-- location: The target location
-- radius: The radius around the location
-- funcname: The name of the function to be called when the player is out of proximity.
function invProximity(trigger)
   if vec2.dist(player.pos(), trigger.location) >= trigger.radius then
      _G[trigger.funcname]()
   else
      hook.timer(0.5, "invProximity", trigger)
   end
end

-- Handle the unsuccessful end of the mission.
function abort()
   var.pop("darkshadow_active")
   misn.finish(false)
end
