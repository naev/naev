--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Escort">
 <unique />
 <priority>2</priority>
 <chance>100</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <done>Destroy the FLF base!</done>
 <cond>system.get("Tarsus"):jumpDist() &lt; 4 and not (spob.cur():services().shipyard == nil)</cond>
 <notes>
  <campaign>Frontier Invasion</campaign>
  <requires name="The FLF is dead"/>
 </notes>
</mission>
--]]
--[[
-- Dvaered Escort
-- This is the first mission of the Frontier War Dvaered campaign.
-- The player has to escort a representative of Dvaered high Command who meets warlords.

   Stages :
   0) Way to First system
   1) Land at first system
   2) Way to second system
   3) Jumpout to Awowa
   4) Land on fleepla
   5) Way to third system, battle with Hamelsen & such
   8) Land at last system
--]]
require "proximity"
local fw = require "common.frontier_war"
local lmisn = require "lmisn"
local fmt = require "format"
local pir = require "common.pirate"
local ai_setup = require "ai.core.setup"

-- Mission constants
local destpla1, destsys1 = spob.getS("Ginni")
local destpla2, destsys2 = spob.getS(fw.wlrd_planet)
local destpla3, destsys3 = spob.getS("Laarss")
local fleepla, fleesys = spob.getS("Odonga m1")

local ambush, hamelsen, majorTam, p, quickie, savers, warlord -- Non-persistent state
local encounterWarlord, hamelsenAmbush, spawnTam, testPlayerSpeed -- Forward-declared functions

local meet_text1 = _([[After Tam boards the Goddard, you wait for about half a period until his ship undocks from the warlord's cruiser. You then receive a message from him "Everything is right, we will now land on {pnt} in order to refuel and rest for some time."]])

function create()
   -- The mission should not appear just after the FLF destruction
   if not (var.peek("invasion_time") == nil or
           time.get() >= time.fromnumber(var.peek("invasion_time")) + time.new(0, 20, 0)) then
      misn.finish(false)
   end

   if system.cur() == destsys1 then -- We need the first target to be at least 1 jump ahead
      misn.finish(false)
   end

   if not misn.claim ( {destsys1, destsys2, destsys3} ) then
      misn.finish(false)
   end

   misn.setNPC(_("Dvaered officer"), fw.portrait_tam, _("This Dvaered senior officer could be looking for a pilot for hire. Why else would he stay at this bar?"))

   mem.previous = spob.cur()
end

function accept()
   if not tk.yesno( _("In need of a pilot"), fmt.f(_([[As you approach the officer, he hails you. "Hello, citizen {player}. I was looking for you. Of course, I know your name, you're one of the pilots who destroyed that damn FLF base in Sigur. Let me introduce myself: I am Major Tam, from Dvaered High Command, and more precisely from the Space Force Headquarters. I feel that you are a reliable pilot and the High Command could make more use of your services. That is why I propose to you now a simple escort mission. All that you need is a fast combat ship that can keep up with my Vendetta. What do you say?"]]), {player=player.name()}) ) then
      tk.msg(_("Too bad"), _([[Tam seems disappointed by your answer. "Well, then, maybe we will meet again later, who knows?"]]))
      return
   end
   tk.msg(_("Instructions"), fmt.f(_([[Tam seems satisfied with your answer. "I am going to pay a visit to three warlords, for military coordination reasons. They will be waiting for me in their respective Goddards in the systems {sys1}, {sys2} and {sys3}. I need you to stick to my Vendetta and engage any hostile who might try to intercept me."]]), {sys1=destsys1, sys2=destsys2, sys3=destsys3}))

   misn.accept()
   misn.osdCreate( _("Dvaered Escort"), {_("Escort Major Tam"), fmt.f(_("Land on {pnt}"), {pnt=destpla1})} )
   misn.setDesc(_("You agreed to escort a senior officer of the Dvaered High Command who is visiting three warlords."))
   misn.setReward(_("Dvaered never talk about money."))
   mem.mark1 = misn.markerAdd(destsys1, "low")

   mem.stage = 0
   mem.nextsys = system.cur()
   mem.tamJumped = true -- Because the player has no right to enter a system if mem.tamJumped is false

   mem.enterhook = hook.enter("enter")
   mem.landhook = hook.land("land")
   mem.loadhook = hook.load("loading")
end

function enter()
   if not (mem.tamJumped and system.cur() == mem.nextsys) then
      tk.msg(_("What are you doing here?"), _("You were supposed to escort Major Tam, weren't you?"))
      misn.finish(false)
   end

   testPlayerSpeed()

   spawnTam( mem.previous )
   mem.tamJumped = false

   if mem.stage == 0 then   -- Go to first rendezvous
      if system.cur() == destsys1 then -- Spawn the Warlord
         encounterWarlord( _("Lady Bitterfly"), destpla1 )
         hook.timer( 2.0, "meeting_msg1" )
      else
         mem.nextsys = lmisn.getNextSystem(system.cur(), destsys1)
         majorTam:control()
         majorTam:hyperspace(mem.nextsys)
         mem.jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
      end

   elseif mem.stage == 2 then  -- Travel to second rendezvous
      if system.cur() == destsys2 then -- Spawn the Baddies
         encounterWarlord( _("Lord Battleaddict"), destpla2 )
         mem.jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
         hook.timer( 2.0, "meeting_msg2" )
      else
         mem.nextsys = lmisn.getNextSystem(system.cur(), destsys2)
         majorTam:control()
         majorTam:hyperspace(mem.nextsys)
         mem.jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
      end

   elseif mem.stage == 3 then  -- Fleeing to flee planet
      hook.timer( 2.5, "explain_battle") -- Explain what happened
      majorTam:control()
      majorTam:land(fleepla)
      mem.stage = 4
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Escort"), {_("Escort Major Tam"), fmt.f(_("Land on {pnt}"), {pnt=fleepla})} )
      misn.osdActive(2)

   elseif mem.stage == 5 then  -- Travel to third rendezvous
      if system.cur() == destsys3 then -- Spawn the Warlord and Hamelsen
         hamelsenAmbush()
         encounterWarlord( _("Lord Jim"), destpla3 )
         hook.timer( 2.0, "meeting_msg3" )
      else
         mem.nextsys = lmisn.getNextSystem(system.cur(), destsys3)
         majorTam:control()
         majorTam:hyperspace(mem.nextsys)
         mem.jumpingTam = hook.pilot(majorTam, "jump", "tamJump")
      end
   end

   mem.previous = system.cur()
end

function testPlayerSpeed()
  local stats = player.pilot():stats()
  local playershipspeed = stats.speed_max
  if playershipspeed < 300 then
      tk.msg(_("Your ship is too slow"), _("Did you really expect to keep up with Major Tam with your current ship?"))
      misn.finish(false)
  end
end

function explain_battle()
   tk.msg(_("That was really close!"), fmt.f(_([[You send a message to Major Tam to ask if you are safe now. "I think so" he answers, "Lord Battleaddict's troops won't follow us if we head to {pnt} at once as the planet belongs to his deadliest enemy, Lady Pointblank." As you ask him what happened, he answers: "You know, don't let Lord Battleaddict's reaction mislead you. He is not a bad person, he is just... hem... a bit old school. He disagrees with the ideas of the new generation of generals at Dvaered High Command, and wanted to make his point clear."
   You ask Tam why the Dvaered patrol ships did not help you and he answers: "Don't expect the regular police or army to help you when you're in trouble with a warlord. Dvaered know that it is better not to be involved in warlord's affairs."]]), {pnt=fleepla}))
end

-- Messages when encountering warlords
function meeting_msg1()
   majorTam:comm( fmt.f(_("{plt} should be waiting for us in orbit around {pnt}."), {plt=_("Lady Bitterfly"), pnt=destpla1}) )
end
function meeting_msg2()
   majorTam:comm( fmt.f(_("{plt} should be waiting for us in orbit around {pnt}."), {plt=_("Lord Battleaddict"), pnt=destpla2}) )
end
function meeting_msg3()
   majorTam:comm( fmt.f(_("{plt} should be waiting for us in orbit around {pnt}."), {plt=_("Lord Jim"), pnt=destpla3}) )
end

function spawnTam( origin )
   majorTam = pilot.add( "Dvaered Vendetta", "Dvaered", origin, _("Major Tam"), {naked=true} )
   majorTam:setHilight()
   majorTam:setVisplayer()
   majorTam:setFaction( fw.fct_dhc() )

   -- TODO switch to equipopt
   majorTam:outfitAdd("S&K Light Combat Plating")
   majorTam:outfitAdd("Milspec Orion 3701 Core System")
   majorTam:outfitAdd("Tricon Zephyr II Engine")
   majorTam:outfitAdd("Unicorp Light Afterburner")
   --majorTam:outfitAdd("Solar Panel")
   majorTam:outfitAdd("Vulcan Gun",3)
   majorTam:outfitAdd("Gauss Gun",3)
   majorTam:setHealth(100,100)
   majorTam:setEnergy(100)
   majorTam:setFuel(true)
   majorTam:cargoRm( "all" )
   ai_setup.setup(majorTam)

   mem.dyingTam = hook.pilot(majorTam, "death", "tamDied")
end

function encounterWarlord( name, origin )
   pir.clearPirates(true)

   warlord = pilot.add( "Dvaered Goddard", "Dvaered", origin, name )
   warlord:control(true)
   warlord:moveto( origin:pos() + vec2.newP(rnd.rnd(1000), rnd.angle()) )

   warlord:setHilight()

   p = {}
   for i = 1, 2 do
      p[i] = pilot.add( "Dvaered Vendetta", "Dvaered", origin )
      p[i]:control(true)
      p[i]:moveto( origin:pos() + vec2.newP(rnd.rnd(1000), rnd.angle()) )
   end

   majorTam:control()
   majorTam:memory().radius = 0
   majorTam:follow(warlord, true)

   mem.proxHook = hook.timer(0.5, "proximity", {anchor = warlord, radius = 1000, funcname = "meeting_timer", focus = majorTam})
end

function tamJump()
   mem.tamJumped = true
   player.msg(fmt.f(_("Major Tam has jumped for the {sys} system."), {sys=mem.nextsys}))
end

function tamDied()
   if hamelsen ~= nil then
      hamelsen:rm() -- Because she is immortal and could kill the player
   end
   tk.msg(_("Mission failed"), _([[As you watch the final explosion of Major Tam's ship hurl the remains of what once was a proud Vendetta to the far corners of the system, you realize that you're actually contemplating one of the most bitter failures of your career. "Meh", you finally think, "I'm sure I will have another chance sooner or later."]]))
   misn.finish(false)
end

function land() -- The player is only allowed to land on special occasions
   if mem.stage == 1 then
      mem.stage = 2
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Escort"), {_("Escort Major Tam"), fmt.f(_("Land on {pnt}"), {pnt=destpla2})} )
      misn.markerRm(mem.mark1)
      mem.mark2 = misn.markerAdd(destsys2, "low")
   elseif mem.stage == 4 then
      mem.stage = 5
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Escort"), {_("Escort Major Tam"), fmt.f(_("Land on {pnt}"), {pnt=destpla3})} )
      misn.markerRm(mem.mark2)
      mem.mark3 = misn.markerAdd(destsys3, "low")
   elseif mem.stage == 8 then
      shiplog.create( "dvaered_military", _("Dvaered Military Coordination"), _("Dvaered") )
      shiplog.append( "dvaered_military", _("Major Tam, from the Space Force Headquarters of Dvaered High Command (DHC) has employed you in the framework of the military coordination. One of the Warlords he was trying to pay a visit to, Lord Battleaddict, has tried to kill him twice, with help of his second in command, Colonel Hamelsen. It looks like trying to coordinate Dvaered warlords is a really dangerous job.") )
      tk.msg(_("Thank you, citizen"), fmt.f(_([[As you land, Major Tam greets you at the spaceport. "After the losses they suffered today, I doubt those mercenaries will come after me again anytime soon. I need to report back at the Dvaer High Command station in Dvaer, and I no longer need an escort. Oh, and, err... about the payment, I am afraid there is a little setback..." You start to fear he will try to stiff you on the payment, but he continues: "I don't know why, but the High Command has not credited the payment account yet... Well do you know what we are going to do? I will give you a set of Gauss Guns worth {credits}! One always needs Gauss Guns, no?"]]), {credits=fmt.credits(fw.credits_00)}))

      -- Major Tam gives Gauss Guns instead of credits, because Major Tam is a freak.
      mem.GGprice = outfit.get("Gauss Gun"):price()
      mem.nb = math.floor(fw.credits_00/mem.GGprice+0.5)
      player.outfitAdd("Gauss Gun", mem.nb)
      misn.finish(true)
   else
      tk.msg(_("What are you doing here?"), _("You were supposed to escort Major Tam, weren't you?"))
      misn.finish(false)
   end
   --hook.rm(mem.jumpingTam)
   mem.tamJumped = true
   mem.previous = spob.cur()
   misn.npcAdd("discussWithTam", _("Major Tam"), fw.portrait_tam, _("Major Tam is a very friendly man. At least by Dvaered military standards."))
end

function loading()
   misn.npcAdd("discussWithTam", _("Major Tam"), fw.portrait_tam, _("Major Tam is a very friendly man. At least by Dvaered military standards."))
end

function meeting_timer() -- Delay the triggering of the meeting
   local pp = player.pilot()
   pp:control() -- Make sure to remove the autonav
   pp:brake()

   hook.timer(7.0, "meeting")
end

function meeting()

   player.pilot():control(false) -- Free the player

   if mem.stage == 0 then
      tk.msg(_("Everything is right"), fmt.f(meet_text1, {pnt=destpla1}))
      mem.stage = 1
      majorTam:taskClear()
      majorTam:land(destpla1)
      misn.osdActive(2)

   elseif mem.stage == 2 then

      mem.nextsys = fleesys
      tk.msg(_("They're after me!"), fmt.f(_([[Tam boards the Goddard. A few seconds later, he undocks in a hurry, while nearby fighters start to shoot at him. You receive a message "That old fool tried to kill me! Quick, we must head to {sys}! Let me jump first!"]]), {sys=mem.nextsys}))
      mem.stage = 3
      quickie = pilot.add( "Dvaered Vendetta", "Dvaered", destpla2 )
      quickie:cargoRm( "all" )
      quickie:setFaction( fw.fct_warlords() )

      majorTam:taskClear()
      majorTam:memory().careful = true
      majorTam:runaway(quickie, jump.get( system.cur(), mem.nextsys )) -- Runaway towards next system

      hook.timer( 2.0, "attackMe" ) -- A small delay to give the player a chance in case an enemy is too close

      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Escort"), {fmt.f(_("Ensure Major Tam safely jumps to {sys} and follow him"), {sys=fleesys})} )

   elseif mem.stage == 5 then
      tk.msg(_("Everything is right"), fmt.f(meet_text1, {pnt=destpla3}))
      mem.stage = 8
      majorTam:taskClear()
      majorTam:land(destpla3)
      misn.osdActive(2)
   end
end

-- Makes Battleaddict's team actually attack the player
function attackMe()
   hook.timer( 5.0, "moreBadGuys" )

   -- Change the enemies to Warlords in order to make them attack
   for i = 1,#p do
      p[i]:setFaction( fw.fct_warlords() )
      p[i]:control(false)
   end
end

-- Battleaddict's bros
function moreBadGuys()
   local buff
   local fwarlords = fw.fct_warlords()
   for i = 1, 3 do
      buff = pilot.add( "Dvaered Ancestor", "Dvaered", destpla2 )
      buff:setFaction(fwarlords)
   end
   buff = pilot.add( "Dvaered Vigilance", "Dvaered", destpla2, _("Colonel Hamelsen") )
   buff:setFaction(fwarlords)
   buff = pilot.add( "Dvaered Phalanx", "Dvaered", destpla2 )
   buff:setFaction(fwarlords)
   warlord:setFaction(fwarlords)
   warlord:control(false)
end

-- Spawn colonel Hamelsen and her mates
function hamelsenAmbush()
   local jp     = jump.get(system.cur(), mem.previous)
   local x, y, pos
   local fwarlords = fw.fct_warlords()
   ambush = {}
   for i = 1, 3 do
      x = 1000 * rnd.rnd() + 1000
      y = 1000 * rnd.rnd() + 1000
      pos = jp:pos() + vec2.new(x,y)

      ambush[i] = pilot.add( "Shark", fwarlords, pos, nil, {ai="baddie_norun"} )
      ambush[i]:setHostile()
      hook.pilot(ambush[i], "death", "ambushDied")
      hook.pilot(ambush[i], "land", "ambushDied")
      hook.pilot(ambush[i], "jump", "ambushDied")
   end

   x = 1000 * rnd.rnd() + 2000
   y = 1000 * rnd.rnd() + 2000
   pos = jp:pos() + vec2.new(x,y)
   hamelsen = pilot.add( "Shark", fwarlords, pos, _("Colonel Hamelsen"), {ai="baddie_norun", naked=true} )

   -- Nice outfits for Colonel Hamelsen (the Hellburner is her life insurance)
   -- TODO switch to equipopt
   hamelsen:outfitAdd("S&K Ultralight Combat Plating")
   hamelsen:outfitAdd("Milspec Orion 2301 Core System")
   hamelsen:outfitAdd("Tricon Zephyr Engine")
   hamelsen:outfitAdd("Hellburner")
   hamelsen:outfitAdd("Milspec Impacto-Plastic Coating")
   hamelsen:outfitAdd("Improved Stabilizer",2)
   hamelsen:outfitAdd("Gauss Gun",3)
   hamelsen:setHealth(100,100)
   hamelsen:setEnergy(100)
   hamelsen:setFuel(true)
   hamelsen:setNoDeath() -- We can't afford to loose our main baddie
   hamelsen:setNoDisable()
   ai_setup.setup(hamelsen)

   mem.attack = hook.pilot( hamelsen, "attacked", "hamelsen_attacked" )

   mem.nambush = #ambush + 1

   -- Pre-position Captain Leblanc and her mates, but as Dvaered
   savers = {}
   for i = 1, 2 do
      x = 1000 * rnd.rnd() - 2000
      y = 1000 * rnd.rnd() - 2000
      pos = jp:pos() + vec2.new(x,y)

      savers[i] = pilot.add( "Dvaered Vendetta", "Dvaered", pos )
   end
   savers[1]:rename(_("Captain Leblanc"))
   savers[1]:setNoDeath()
   savers[1]:setNoDisable()

   mem.msg = hook.timer( 4.0, "ambush_msg" )
   mem.killed_ambush = 0
end

function ambush_msg()
   tk.msg(_("Say hello to my mace rockets"), _([[As your ship decelerates to its normal speed after jumping in, you realize there are hostile ships around. An enemy Shark broadcasts the following message: "Tam, you small, fearful weakling, did you believe Lord Battleaddict would really let you live? You're doomed!"]]))
   ambush[1]:comm(_("You wanted to meet Lord Jim? How about you meet your doom instead?"))

   majorTam:control(false)
   hook.rm(mem.proxHook) -- To avoid triggering by mistake

   local fdhc = fw.fct_dhc()
   for i, pi in ipairs(savers) do
      pi:setFaction( fdhc )
   end
end

-- Hook to make Hamelsen run away
function hamelsen_attacked( )
   -- Target was hit sufficiently to run away
   local _armour, shield = hamelsen:health()
   if shield < 10 then
      hamelsen:control()
      hamelsen:memory().careful = true
      hamelsen:runaway(player.pilot(), jump.get( system.cur(), "Radix")) -- I don't want her to try to jump at closest one
      hook.rm(mem.attack)
      ambushDied() -- One less
   end
end

function ambushDied()
   mem.killed_ambush = mem.killed_ambush + 1
   if mem.killed_ambush >= mem.nambush then -- Everything back to normal: we meet Lord Jim
      majorTam:control()
      majorTam:follow(warlord, true)
      hook.timer(0.5, "proximity", {anchor = warlord, radius = 1000, funcname = "meeting_timer", focus = majorTam})
      hook.timer(3.0, "ambush_end")
   end
end

-- The end of the Ambush: a message that explains what happened
function ambush_end()
   tk.msg(_("Hostiles eliminated"), fmt.f(_([[As the remaining attackers flee, you remark that a Dvaered patrol helped you, contrary to what Tam had explained before. Then you receive the messages exchanged between Major Tam and the leader of the Dvaered squadron: "This time, I really owe you one, Captain", Tam says. "No problem, sir." the other answers, "But the most dangerous one escaped. The Shark, you know, it was Hamelsen, Battleaddict's second in command. After we heard of what the old scumbag had done to you, we put him under surveillance, and we spotted Hamelsen pursuing you with her Shark, so we followed her, pretending we're just a police squadron. You know the rest."
   Tam responds: "By the way, {player}, let me introduce you the Captain Leblanc. She belongs to the Special Operations Force (SOF), part of Dvaered High Command (DHC). I didn't tell you, but her pilots always keep an eye on me from a distance when I have to meet warlords. {player} is the private pilot I told you about, Captain." Leblanc responds: "Hello, citizen. I'm glad there are civilians like you who do their duty and serve the Dvaered Nation."]]), {player=player.name()}))
   tk.msg(_("Two attacks are one too many"), _([["Anyway," says Tam, "I am afraid this ambush is not acceptable." Leblanc responds: "True, sir. Attacking someone in one's system is a standard means of expression for a warlord, but setting an ambush here denotes a true lack of respect."
   "He will answer for this, trust me." answers Tam, "I will refer this matter to the chief. Meanwhile, I still have an appointment with Lord Jim. I just hope he will not try to make us dance as well..."]]))
end

function discussWithTam()
   -- Major Tam is not senile: he says different things at the different stops
   if mem.stage == 2 then
      tk.msg(_("Major Tam is ready"), _([[How do you do, citizen? Did you enjoy the trip so far? I'm ready for the next stop. I'll follow you when you take off.]]))
   elseif mem.stage == 5 then
      if tk.yesno(_("Major Tam is talkative today"), _([["Hello, citizen. Did you already recover from Lord Battleaddict's last trick? It reminded me of my youth, when I used to belong to a fighter squadron in Amaroq..." Do you want to encourage Tam to talk about his past?]])) then
         tk.msg(_("Major Tam before he was at the Headquarters"), _([["You know, I've not always worked at Headquarters. I started as a pilot at the DHC base on Rhaana. Oh, sorry, DHC stands for Dvaered High Command. You know, there are two kinds of Dvaered soldiers: those who directly report to DHC, like myself, and the freaks, as we call them (or the warriors, as they call themselves), the soldiers who report to local Warlords."
   "Warlords' forces can be requisitioned by DHC, but only to fight forces that threaten the integrity of the Dvaered Nation, so, in practice, they are mostly left to themselves, and make war on each other. You know, foreigners sometimes think that the internecine conflicts between warlords are pointless (I've even heard the word "stupid" once), but actually, they're the key to Dvaered philosophy. Without those wars, the Dvaered Nation would no longer exist as we know it, and we would have had to rely on totalitarianism, like the Empire, nostalgia of an idealized past, like the Frontier, oppressive technocracy like the Za'lek, or such...
   "Hey, but am I deviating from our original subject? What was it already? Oh I don't remember. Anyway, citizen, if you want to take off, I'm ready."]]))
      end
   end
end
