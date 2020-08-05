--[[
-- Dvaered Escort
-- This is the first mission of the Frontier War Dvaered campaign.
-- The player has to escort a representative of Dvaered high Command who meets warlords.

   Stages :
   0) Way to First system
   1) Land at first system
   2) Way to second system
   3) Jumpout to ???
   4) Land on fleepla
   5) Way to third system, battle with Hamelsen & such
   8) Land at last system
--]]

--TODO: Set the priority and conditions of this mission
-- TODO: dialog to say: we will meet Lady Bitterfight, Lord Battleaddict, Lord Jim

require "dat/scripts/nextjump.lua"
require "proximity.lua"
require "selectiveclear.lua"

portrait_name = _("Dvaered officer")
portrait_desc = _("This Dvaered senior officer could be looking for a pilot for hire. Why else would he stay at this bar?")

npc_name = _("Major Tam")
npc_desc = _("Major Tam is a very friendly man. At least by Dvaered military standards.")

propose_title = _("In need of a pilot")
propose_text = _([[As you approach the officer, he hails you. "Hello, citizen %s. I was looking for you. Of course I know your name, you're one of the pilots who destroyed that damn FLF base in Surano. Let me introduce myself: I am Major Tam, from Dvaered High Command, and more precisely from the Space Forces Headquarters. I feel that you are a reliable pilot and the High Command could make more often use of your services. That is why I propose you for now a simple escort mission. Whar do you say?"]])

accept_title = _("Instructions")
accept_text = _([[Tam seems satisfied with your answer. "I am going to pay a visit to three warlords, for military coordination reasons. They will be waiting for me in their respective Goddards in the systems %s, %s and %s. I need you to stick to my Vendetta and engage any hostile who could try to intercept me."]])

refuse_title = _("Too bad")
refuse_text = _([[Tam seems disappointed by your answer. "Mwell, then, maybe we will meet again later, who knows?"]])

flee_title = _("What are you doing here?")
flee_text = _("You were supposed to escort Major Tam, weren't you?")

fail_title = _("Mission failed")
fail_text = _([[As you watch the final explosion of Major Tam's ship dispatching around the system the remains of what once was a proud Vendetta, you realize that you're actually contemplating one of the most bitter failures of your career. "Meh", you finally think, "I'm sure I will have an other chance sooner or later."]])

slow_title = _("Your ship is too slow")
slow_text = _("Did you really expect to catch up with Major Tam with your current ship?")

meet_title1 = _("Everything is right")
meet_text1 = _([[After Tam boards the Goddard, you wait for about half a period until his ship undocks from the warlord's cruiser. You then recieve a message from him "Everything is right, we will now land on %s in order to refuel and rest for some time."]])

meet_title2 = _("They're after me!")
meet_text2 = _([[Tam boards the Goddard. A few seconds later, he undocks in a hurry, while nearby fighters start to shoot at him. You recieve a message "That old fool tried to kill me! quick, we must head to %s! Let me jump first!"]])

discuss_title1 = _("Major Tam is ready")
discuss_text1 = _([[How do you do, citizen? Did you enjoy the trip so far? I'm ready for next stop. I'll follow you when you take off.]])

discuss_title2 = _("Major Tam is talkative today")
discuss_text2 = _([["Hello, citizen. Did you already recover from Lord Battleaddict's last joke? It recalled me of my youth, when I used to belong to a fighter's squadron in Amaroq..." Do you want to encourage Tam to talk about his past?]])

discuss_title3 = _("Major Tam before he was at the Headquarters")
discuss_text3 = _([["You know I've not always worked at the Headquarters. I started as a Lieutenant-pilot at the DHC base on Rhaana. Oh, sorry, DHC stays for Dvaered High Command. You know, there are two kinds of Dvaered soldiers: there are those who directly obey to DHC, like myself, and there are the freaks, as we call them, (or the warriors, as they call themselves) the soldiers who obey to local Warlords. 
   "Warlord's forces can be requisitioned by DHC, but only to fight a menace to the integrity of the Dvaered Nation, so in practice, they are most of the time left to themselves, and make wars one against each other. You know, foreigners sometimes think that the inner wars between warlords are pointless (I've even heard the word "stupid" once), but actually, it's the key to Dvaered philosophy. Without those wars, the Dvaered Nation would no longer exist as we know it, and we would have had to rely on totalitarism, like the Empire, nostalgia of an idealized past, like the Frontier, oppressive technocracy like the Za'lek, or such...
   "Hey, but am I deviating from our original subject? What was it already? Oh I don't remember. Anyway, citizen, if you want to take off, I'm ready."]])

ready_title = _("Major Tam is ready")
ready_text = _([["Anyway, citizen, if you want to take off, I'm ready."]])

end_title = _("Thank you, citizen")
end_text = _([[As you land, Major Tam greets you at the spaceport. "After the losses they got today, I doubt those mercenaries will come back at me before long. I need to report back at the Dvaer High Command station in Dvaer, and I don't need anymore escort. Meanwhile, please recieve those %d credits."]])

explain_title = _("That was really close!")
explain_text = _([[You send a message to Major Tam to ask if you are safe now. "I think so" he answers, "Lord Battleaddict's troops won't follow us if we head to %s at once as the planet belongs to his deadliest enemy, Lady Pointblank." As you ask to him what happened, he answers: "You know, don't let Lord Battleaddict's reaction mislead you. He is not a bad person, he is just... hem... a bit old school. He is not in par with the ideas of the new generation of generals at Dvaered High Command, and wanted to make his point clear."
   You ask Tam why the Dvaered patrol ships did not help you and he answers: "Don't expect the regular police or army to help you when you're in trouble with a warlord. Dvaered know that it is better not to be involved in warlord's affairs."]])

ambush_title = _("Say hello to my mace rockets")
ambush_text = _([[As your ship starts to recover its normal speed after jumping in, you realize that there are hostile ships around. An enemy Shark then broadcasts the following message: "Tam, you small fearful weakling, did you believe Lord Battleaddict would really let you live? You're doomed!"]])
ambush_broadcast = _("You wanted to meet Lord Jim? What about you meet your doom instead?")

saved_title1 = _("Hostiles eliminated")
saved_text1 = _([[As the remaining attackers run away, you wonder why this Dvaered patrol helped you, contrary to what Tam had explained before. Then you recieve the messages exchanged between Major Tam and the leader of the Dvaered squadron: "This time, I really owe you one, Captain Leblanc", Tam says. "No problem, sir. " the other answers "But the most dangerous one escaped. The shark, you know, it was Hamelsen, Battleaddict's second in command. After we heared of what the old monkey had done to you, we put him under surveillance and we spot Hamelsen pursuing you with her shark, so we followed her, pretending we're just a police squadron. You know the rest."
   Tam responds: "By the way, %s, let me introduce you the Capitain Leblanc, she belongs to the Special Operations Force (SOF), part of Dvaered High Command (DHC). I didn't tell you, but her pilots always keep an eye on me from a distance when I have to meet warlords. %s is the private pilot I spoke to you, Captain." Leblanc responds: "Hello, citizen. I'm glad there are civilians like you who make their duity and serve the Dvaered Nation."]])

saved_title2 = _("Two attacks are one too much")
saved_text2 = _([["Anyway," says Tam, "I am afraid this ambush is not acceptable." Leblanc responds: "True, sir. Attacking someone in one's system is a standard way of expression for a warlord, but setting an ambush here denotes a true lack of respect."
   "He will have to respond for this, trust me." answers Tam, "I will refer to the chief. Meanwhile, I still have an appointment with Lord Jim. I just hope he will not try to make us dance as well..."]])

meeting_broadcast = _("%s should be waiting for us in orbit around %s.")

jumpmsg = _("Major Tam has jumped for the %s system.")

log_text = _("The Major Tam, from the Space Forces Headquarters of Dvaered High Command (DHC) has employed you in the framework of the military coordination. One of the Warlords he was trying to pay a visit to, Lord Battleaddict, has tried to kill him twice, with help of his second in command, Colonel Hamelsen. It looks like trying to coordinate dvaered warlords is a really dangerous job.")

-- TODO : manage the osd
osd_title = _("Dvaered Escort")
osd_msg1 = _("Escort Major Tam")
osd_msg2 = _("Engage the hostiles")
osd_msg3 = _("Ensure Major Tam safely jumps to %s and follow him")
osd_msg4 = _("Land on %s")

misn_desc = _("You agreed to escort a senior officer of the Dvaered High Command who is visiting three warlords.")
misn_reward = _("Dvaered never talk about money.")

credits = 100000

function create()
   destsys1 = system.get("Tarsus")
   destsys2 = system.get("Ginger")
   destsys3 = system.get("Dvaer")

   destpla1 = planet.get("Ginni")
   destpla2 = planet.get("Buritt")
   destpla3 = planet.get("Laarss")

   fleesys = system.get("Awowa")
   fleepla = planet.get("Odonga m1")

   if not misn.claim ( {destsys1, destsys2, destsys3} ) then
      misn.finish(false)
   end

   misn.setNPC(portrait_name, "dvaered/dv_military_m3")
   misn.setDesc(portrait_desc)

   previous = planet:cur()
end

function accept()
   if not tk.yesno( propose_title, propose_text:format(player.name()) ) then
      tk.msg(refuse_title, refuse_text)
      misn.finish(false)
   end
   tk.msg(accept_title, accept_text:format(destsys1:name(), destsys2:name(), destsys3:name()))

   misn.accept()
   misn.osdCreate( osd_title, {osd_msg1, osd_msg4:format(destpla1:name())} )
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)
   misn.markerAdd(destsys1, "low")

   stage = 0
   nextsys = system.cur()
   tamJumped = true -- Because the player has no right to enter a system if tamJumped is false

   enterhook = hook.enter("enter")
   landhook = hook.land("land")
   loadhook = hook.load("loading")
end

function enter()
   if not (tamJumped and system.cur() == nextsys) then
      tk.msg(flee_title, flee_text)
      misn.finish(false)
   end

--   if boozingTam ~= nil then
--      misn.npcRm(boozingTam)
--   end

   testPlayerSpeed()

   spawnTam( previous )
   tamJumped = false

   pilot.toggleSpawn("FLF") -- TODO : It's only for testing. It can be removed once FLF is dead
   pilot.clearSelect("FLF")

--   pilot.toggleSpawn("Pirate") -- It was only for testing. I want the pirates to be there
--   pilot.clearSelect("Pirate")

   if stage == 0 then   -- Go to first rendezvous
      if system.cur() == destsys1 then -- Spawn the Warlord
         encounterWarlord( "Lady Bitterfight", destpla1 )
         hook.timer( 2000, "meeting_msg1" )
      else
         nextsys = getNextSystem(system.cur(), destsys1)
         majorTam:control()
         majorTam:hyperspace(nextsys)
         jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
      end

   elseif stage == 2 then  -- Travel to second rendezvous
      if system.cur() == destsys2 then -- Spawn the Baddies
         encounterWarlord( "Lord Battleaddict", destpla2 ) -- TODO : add Colonel Hamelsen
         jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
         hook.timer( 2000, "meeting_msg2" )
      else
         nextsys = getNextSystem(system.cur(), destsys2)
         majorTam:control()
         majorTam:hyperspace(nextsys)
         jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
      end

   elseif stage == 3 then  -- Fleeing to flee planet
      hook.timer( 2500, "explain_battle") -- Explain what happened
      majorTam:control()
      majorTam:land(fleepla)
      stage = 4
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg1, osd_msg4:format(fleepla:name())} )
      misn.osdActive(2)

   elseif stage == 5 then  -- Travel to third rendezvous
      if system.cur() == destsys3 then -- Spawn the Warlord and Hamelsen
         hamelsenAmbush()
         encounterWarlord( "Lord Jim", destpla3 )
         hook.timer( 2000, "meeting_msg3" )
      else
         nextsys = getNextSystem(system.cur(), destsys3)
         majorTam:control()
         majorTam:hyperspace(nextsys)
         jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
      end
   end

   previous = system.cur()
end

function testPlayerSpeed()
  local stats = player.pilot():stats()
  local playershipspeed = stats.speed_max
  if playershipspeed < 300 then
      tk.msg(slow_title, slow_text)
      misn.finish(false)
  end
end

function explain_battle()
   tk.msg(explain_title, explain_text:format(fleepla:name()))
end

-- Messages when encountering warlords
function meeting_msg1()
   majorTam:comm( meeting_broadcast:format("Lady Bitterfight", destpla1:name()) )
end
function meeting_msg2()
   majorTam:comm( meeting_broadcast:format("Lord Battleaddict", destpla2:name()) )
end
function meeting_msg3()
   majorTam:comm( meeting_broadcast:format("Lord Jim", destpla3:name()) )
end

function spawnTam( origin )
--   if diyingTam ~= nil then
--      hook.rm(diyingTam)
--   end

   majorTam = pilot.add("Dvaered Vendetta", nil, origin, "DHC")[1] -- TODO: Tam should be player faction
   majorTam:rename("Major Tam")
   majorTam:setHilight()
   majorTam:setVisplayer()
   majorTam:setFaction("DHC")
   -- TODO : something to remove his cargo

   majorTam:rmOutfit("all")
   majorTam:rmOutfit("cores")
   majorTam:addOutfit("S&K Light Combat Plating")
   majorTam:addOutfit("Milspec Aegis 3601 Core System")
   majorTam:addOutfit("Tricon Zephyr II Engine")
   majorTam:addOutfit("Generic Afterburner")
   majorTam:addOutfit("Shield Capacitor")
   majorTam:addOutfit("Shredder",6)
   majorTam:setHealth(100,100)
   majorTam:setEnergy(100)
   majorTam:setFuel(true)

   diyingTam = hook.pilot(majorTam, "death", "tamDied") --TODO: exploded would be better, but it makes shitty warnings
end

function encounterWarlord( name, origin ) -- TODO: ensure the warlords dont get attacked by pirates

   warlord = pilot.add("Dvaered Goddard", nil, origin)[1]
   warlord:rename( name )
   warlord:control(true)
   warlord:goto( origin:pos() + vec2.newP(rnd.rnd(1000), rnd.rnd(360)) )

   warlord:setHilight()

   p = {}
   for i = 1, 2 do
      p[i] = pilot.add("Dvaered Vendetta", nil, origin)[1]
      --p[i]:setSpeedLimit (p, 200)
      p[i]:control(true)
      p[i]:goto( origin:pos() + vec2.newP(rnd.rnd(1000), rnd.rnd(360)) )
      -- TODO maybe: control the Vendettas equipment so that they don't outrun Tam

--      p[i]:rmOutfit("cores")
--      p[i]:addOutfit("S&K Light Combat Plating")
--      p[i]:addOutfit("Milspec Aegis 3601 Core System")
--      p[i]:addOutfit("Unicorp Hawk 300 Engine")  -- Otherwise, Tam is dead

--      p[i]:setHealth(100,100)
--      p[i]:setEnergy(100)
--      p[i]:setFuel(true)
   end

   majorTam:control()
   --majorTam:goto(warlord:pos())
   majorTam:memory().radius = 0
   majorTam:follow(warlord, true)
   --hook.timer(10000, "tamGoto")
   -- TODO : add messages
   proxHook = hook.timer(500, "proximity", {anchor = warlord, radius = 1000, funcname = "meeting_timer", focus = majorTam})
end

--function tamGoto()
--   majorTam:taskClear()
--   majorTam:goto(warlord:pos()) -- Because its position has probably changed
--end

function tamJump()
   tamJumped = true
   --hook.rm(jumpingTam)
   player.msg(jumpmsg:format(nextsys:name()))
end

function tamDied()
   tk.msg(fail_title, fail_text)
   misn.finish(false)
end

--function tamLand()
--   tamJumped = true
--end

function land() -- The player is only allowed to land on special occasions
   if stage == 1 then
      stage = 2
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg1, osd_msg4:format(destpla2:name())} )
      misn.markerAdd(destsys2, "low")
   elseif stage == 4 then
      stage = 5
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg1, osd_msg4:format(destpla3:name())} )
      misn.markerAdd(destsys3, "low")
   elseif stage == 8 then
      shiplog.createLog( "frontier_war", _("Dvaered Military Coordination"), _("Dvaered") ) --TODO: create a common file for this TODO: see if it's possible to rename it afterwards
      shiplog.appendLog( "frontier_war", log_text )
      tk.msg(end_title, end_text:format(credits))
      player.pay(credits)
      misn.finish(true)
   else
      tk.msg(flee_title, flee_text)
      misn.finish(false)
   end
   --hook.rm(jumpingTam)
   tamJumped = true
   previous = planet.cur()
   boozingTam = misn.npcAdd("discussWithTam", npc_name, "dvaered/dv_military_m3", npc_desc)
end

function loading()
   boozingTam = misn.npcAdd("discussWithTam", npc_name, "dvaered/dv_military_m3", npc_desc)
end

function meeting_timer() -- Delay the triggering of the meeting
   player.pilot():control() -- Make sure to remove the autonav
   player.pilot():brake()
   player.cinematics( true )
   player.cinematics( false )

   hook.timer(7000, "meeting")
end

function meeting()

   player.pilot():control(false) -- Free the player

   if stage == 0 then
      tk.msg(meet_title1, meet_text1:format(destpla1:name()))
      stage = 1
      majorTam:taskClear()
      majorTam:land(destpla1)
      misn.osdActive(2)

   elseif stage == 2 then
      pilot.toggleSpawn("Pirate", false) -- Make sure Pirates dont get on the way
      pilot.clearSelect("Pirate")

      nextsys = fleesys
      tk.msg(meet_title2, meet_text2:format(nextsys:name()))
      stage = 3
      quickie = pilot.add("Dvaered Vendetta", nil, destpla2)[1] -- TODO: give this pilot quick outfit to force the player to dispatch him
      quickie:setFaction("Warlords")

      quickie:rmOutfit("all")
      quickie:rmOutfit("cores")
      quickie:addOutfit("S&K Light Combat Plating")
      quickie:addOutfit("Milspec Aegis 3601 Core System")
      quickie:addOutfit("Tricon Zephyr II Engine")
      quickie:addOutfit("Reactor Class I")
      quickie:addOutfit("Improved Stabilizer")
      quickie:addOutfit("Shredder",3)
      quickie:addOutfit("Vulcan Gun",3) -- TODO : use generic afterburner
      quickie:setHealth(100,100)
      quickie:setEnergy(100)
      quickie:setFuel(true)

      majorTam:control()
      majorTam:memory().careful = true
      majorTam:runaway(quickie, true) -- The nojump prevents him to land as well

      hook.timer( 15000, "tamHyperspace" ) -- At some point, he is supposed to jump

      -- Change the enemies to Warlords in order to make them attack
      for i = 1,#p do
         p[i]:setFaction("Warlords")
         p[i]:control(false)
      end

      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg3:format(fleesys:name())} )

   elseif stage == 5 then
      tk.msg(meet_title1, meet_text1:format(destpla3:name())) -- TODO maybe: different text
      stage = 8
      majorTam:taskClear()
      majorTam:land(destpla3)
      misn.osdActive(2)
   end
end

function tamHyperspace()
   majorTam:taskClear()
   majorTam:hyperspace( nextsys )
end

-- Spawn colonel Hamelsen and her mates
function hamelsenAmbush()
   pilot.toggleSpawn("Pirate", false) -- Make sure Pirates dont get on the way
   pilot.clearSelect("Pirate")

   jp     = jump.get(system.cur(), previous)
   ambush = {}
   for i = 1, 3 do
      x = 1000 * rnd.rnd() + 1000
      y = 1000 * rnd.rnd() + 1000
      pos = jp:pos() + vec2.new(x,y)

      ambush[i] = pilot.addRaw( "Hyena", "baddie_norun", pos, "Warlords" )
      ambush[i]:setHostile()
      hook.pilot(ambush[i], "death", "ambushDied") -- TODO: is there an other way for them to despawn?
      hook.pilot(ambush[i], "land", "ambushDied")
      hook.pilot(ambush[i], "jump", "ambushDied")
   end

   x = 1000 * rnd.rnd() + 1000
   y = 1000 * rnd.rnd() + 1000
   pos = jp:pos() + vec2.new(x,y)
   hamelsen = pilot.addRaw( "Shark", "baddie_norun", pos, "Warlords" )

   -- Nice outfits for Colonel Hamelsen (the Hellburner is her life insurance)
   hamelsen:rename("Colonel Hamelsen")
   hamelsen:rmOutfit("all")
   hamelsen:rmOutfit("cores")
   hamelsen:addOutfit("S&K Ultralight Combat Plating")
   hamelsen:addOutfit("Milspec Aegis 2201 Core System")
   hamelsen:addOutfit("Tricon Zephyr Engine")
   hamelsen:addOutfit("Hellburner")
   hamelsen:addOutfit("Engine Reroute",2)
   hamelsen:addOutfit("Shredder")
   hamelsen:addOutfit("Vulcan Gun",2)
   hamelsen:setHealth(100,100)
   hamelsen:setEnergy(100)
   hamelsen:setFuel(true)
   hamelsen:setNoDeath() -- We can't afford to loose our main baddie
   hamelsen:setNodisable()

--   hook.pilot(hamelsen, "death", "ambushDied")
--   hook.pilot(hamelsen, "land", "ambushDied")
--   hook.pilot(hamelsen, "jump", "ambushDied")
   attack = hook.pilot( hamelsen, "attacked", "hamelsen_attacked" )

   nambush = #ambush + 1

   -- Pre-position Captain Leblanc and her mates, but as Dvaered
   savers = {}
   for i = 1, 3 do
      x = 1000 * rnd.rnd() - 2000
      y = 1000 * rnd.rnd() - 2000
      pos = jp:pos() + vec2.new(x,y)

      savers[i] = pilot.add( "Dvaered Vendetta", nil, pos )[1]
   end
   savers[1]:rename("Capitain Leblanc")
   savers[1]:setNoDeath()
   savers[1]:setNodisable()

   msg = hook.timer( 4000, "ambush_msg" )
   killed_ambush = 0
end

function ambush_msg()
   tk.msg(ambush_title, ambush_text)
   ambush[1]:comm(ambush_broadcast)

   majorTam:control(false)
   hook.rm(proxHook) -- To avoid triggering by mistake

   for i, p in ipairs(savers) do
      p:setFaction("DHC")
   end
end

-- Hook to make Hamelsen run away
function hamelsen_attacked( )
   -- Target was hit sufficiently to run away
   local armour, shield = hamelsen:health()
   if shield < 10 then
      hamelsen:control()
      hamelsen:runaway(player.pilot(), true) -- Nojump because I don't want her to try to jump at the beginning
      hook.timer(15000, "hamelsenHyperspace")
      hook.rm(attack)
      ambushDied() -- One less
   end
end

function hamelsenHyperspace()
   hamelsen:taskClear()
   hamelsen:hyperspace( system.get("Radix") )
end

function ambushDied()
   killed_ambush = killed_ambush + 1
   if killed_ambush >= nambush then -- Everything back to normal: we meet Lord Jim
      majorTam:control()
      majorTam:follow(warlord, true)
      hook.timer(500, "proximity", {anchor = warlord, radius = 1000, funcname = "meeting_timer", focus = majorTam})
      hook.timer(3000, "ambush_end")
   end
end

-- The end of the Ambush: a message that explains what happened
function ambush_end()
   tk.msg(saved_title1, saved_text1:format(player.name(), player.name()))
   tk.msg(saved_title2, saved_text2)
end

function discussWithTam()
   -- Major Tam is not senile: he says different things at the different stops
   if stage == 2 then
      tk.msg(discuss_title1, discuss_text1)
   elseif stage == 5 then
      if tk.yesno(discuss_title2, discuss_text2) then
         tk.msg(discuss_title3, discuss_text3)
      end
   end
end

