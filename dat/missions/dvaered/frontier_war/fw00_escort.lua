--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Escort">
 <unique />
 <priority>2</priority>
 <chance>100</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <done>Destroy the FLF base!</done>
 <cond>system.get("Tarsus"):jumpDist() &lt; 4</cond>
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
local vn = require "vn"
local fw = require "common.frontier_war"
local lmisn = require "lmisn"
local fmt = require "format"
local vntk = require "vntk"
local pir = require "common.pirate"
local equipopt = require "equipopt"
local love_shaders = require "love_shaders"

-- Mission constants
local destpla1, destsys1 = spob.getS("Ginni")
local destpla2, destsys2 = spob.getS(fw.wlrd_planet)
local destpla3, destsys3 = spob.getS("Laarss")
local fleepla, fleesys = spob.getS("Odonga m1")

local ambush, hamelsen, majorTam, p, quickie, savers, warlord -- Non-persistent state
local encounterWarlord, hamelsenAmbush, spawnTam, testPlayerSpeed -- Forward-declared functions

function create()
   -- The mission should not appear just after the FLF destruction
   if not (var.peek("invasion_time") == nil or
         time.cur() >= time.fromnumber(var.peek("invasion_time")) + time.new(0, 20, 0)) then
      misn.finish(false)
   end

   if system.cur() == destsys1 then -- We need the first target to be at least 1 jump ahead
      misn.finish(false)
   end

   if not misn.claim ( {destsys1, destsys2, destsys3} ) then
      misn.finish(false)
   end

   misn.setNPC(_("Dvaered officer"), fw.tam.portrait, _("This Dvaered senior officer could be looking for a pilot for hire. Why else would he stay at this bar?"))

   mem.previous = spob.cur()
end

function accept()

   vn.clear()
   vn.scene()
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam ) )
   tamVN:rename(_("Dvaered officer"))
   local doaccept = false
   vn.transition()
   
   tamVN(fmt.f(_([["Hello, citizen {player}. I was looking for you."]]), {player=player.name()}))
   vn.menu{
      {_([["Nice to meet you, citizen…"]]), "askname"},
      {_([["How do you know my name?"]]), "howname"},
      {_([["And I was not looking for you."]]), "dontcare"},
   }

   vn.label("dontcare")
   tamVN(_([["Well, then, maybe we will meet again later, who knows?"]]))
   vn.func( function () doaccept = false end )
   vn.done()

   vn.label("howname")
   tamVN(_([["Of course, I know your name, you're one of the pilots who destroyed that damn FLF base in Sigur."]]))

   vn.label("askname")
   tamVN(_([["Let me introduce myself: I am Major Tam, from Dvaered High Command, and more precisely from the Space Force Headquarters. I feel that you are a reliable pilot and the High Command could make more use of your services. That is why I propose to you now a simple escort mission. All that you need is a fast combat ship that can keep up with my Vendetta. What do you say?"]]))
   vn.menu{
      {_("I accept."), "accept"},
      {_("No."), "dontcare"},
   }
   
   vn.label("accept")
   tamVN(fmt.f(_([["I am going to pay a visit to three warlords, for military coordination reasons. They will be waiting for me in their respective Goddards in the systems {sys1}, {sys2} and {sys3}. I need you to stick to my Vendetta and engage any hostile who might try to intercept me."]]), {sys1=destsys1, sys2=destsys2, sys3=destsys3}))
   vn.func( function () doaccept = true end )
   vn.done()
   vn.run()

   -- Test acceptance
   if not doaccept then misn.finish(false) end
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
      vntk.msg(_("What are you doing here?"), _("You were supposed to escort Major Tam, weren't you?"))
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
      vntk.msg(_("Your ship is too slow"), _("Did you really expect to keep up with Major Tam with your current ship?"))
      misn.finish(false)
   end
end

function explain_battle()

   local explain  = _([["What the hell happened?"]])
   local violence = _([["Let's go back there with a pair torpedoes launchers."]])
   local help     = _([["Why didn't the patrol ships help us?"]])
   local leave    = fmt.f(_([["Copy that. Heading to {pnt}."]]),{pnt=fleepla})

   vn.clear()
   vn.scene()
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam, { shader=love_shaders.hologram() } ) )
   vn.transition()
   
   vn.na(_("Major Tam opens a communication channel with you."))
   tamVN(fmt.f(_([["That was close but we should be safe now. Lord Battleaddict's troops won't follow us if we head to {pnt} at once as the planet belongs to his deadliest enemy, Lady Pointblank."]]), {pnt=fleepla}))
   vn.menu{
      {explain, "explain"},
      {violence, "violence"},
      {help, "help"},
      {leave, "leave"},
   }
   
   vn.label("explain")
   tamVN(_([["Don't let Lord Battleaddict's reaction mislead you. He is not a bad person, he is just… hem… a bit old school. He disagrees with the ideas of the new generation of generals at Dvaered High Command, and wanted to make his point clear."]]))
   vn.menu{
      {_([["What is clear is that we should go back there with a pair torpedoes launchers."]]), "violence"},
      {help, "help"},
      {leave, "leave"},
   }
   
   vn.label("violence")
   tamVN(_([["Oh no, I am afraid this is not possible, citizen. Killing a warlord is a crime, no matter what are the circumstances… I mean for non-warlords or generals that is. Besides, I am pretty sure we don't have the material ressources to kill Battleaddict and survive after that."]]))
   vn.menu{
      {help, "help"},
      {leave, "leave"},
   }
   
   vn.label("help")
   tamVN(_([["Don't expect the regular police or army to help you when you're in trouble with a warlord. Dvaered know that it is better not to be involved in warlord's affairs."]]))
   
   vn.label("leave")
   vn.done()
   vn.run()
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
   majorTam = pilot.add( "Dvaered Vendetta", "Dvaered", origin, _("Major Tam") )
   majorTam:setHilight()
   majorTam:setVisplayer()
   majorTam:setFaction( fw.fct_dhc() )

   equipopt.dvaered( majorTam, {
      cores = {
         hull = "S&K Skirmish Plating",
         hull_secondary = "S&K Skirmish Plating",
         systems = "Milspec Orion 2301 Core System",
         systems_secondary = "Milspec Orion 2301 Core System",
         engines = "Tricon Zephyr Engine",
         engines_secondary = "Tricon Zephyr Engine",
      },
      prefer = {
         ["Gauss Gun"] = 100,
         ["Unicorp Light Afterburner"] = 100,
      },
      move = 0,
      max_same_weap = 10,
      rnd = 0, -- Consistent
   } )
   majorTam:setFuel(true)

   assert( majorTam:fuel() > 0 )

   mem.dyingTam = hook.pilot(majorTam, "death", "tamDied")
end

function encounterWarlord( name, origin )
   pir.clearPirates(true)

   warlord = pilot.add( "Dvaered Goddard", "Dvaered", origin, name )
   warlord:control(true)
   warlord:moveto( origin:pos() + vec2.newP(rnd.rnd(0,1000), rnd.angle()) )

   warlord:setHilight()

   p = {}
   for i = 1, 2 do
      p[i] = pilot.add( "Dvaered Vendetta", "Dvaered", origin )
      p[i]:control(true)
      p[i]:moveto( origin:pos() + vec2.newP(rnd.rnd(0,1000), rnd.angle()) )
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
   vntk.msg(_("Mission failed"), _([[As you watch the final explosion of Major Tam's ship hurl the remains of what once was a proud Vendetta to the far corners of the system, you realize that you're actually contemplating one of the most bitter failures of your career. "Meh", you finally think, "I'm sure I will have another chance sooner or later."]]))
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
      
      vn.clear()
      vn.scene()
      local tamVN = vn.newCharacter( fw.vn_char( fw.tam ) )
      vn.transition()
      
      vn.na(_("As you land, Major Tam greets you at the spaceport."))
      
      tamVN(_([[After the losses they suffered today, I doubt those mercenaries will come after me again anytime soon. I need to report back at the Dvaer High Command station in Dvaer, and I no longer need an escort. Oh, and, err… about the payment, I am afraid there is a little setback…]]))
      vn.menu{
         {_("Are you going to try to stiff me on the payment?"), "stiff"},
         {_("…"), "leave"},
      }
      
      vn.label("stiff")
      tamVN(_([[Oh no, don't worry, you will get paid. I represent the Dvaered High Command I'm no crook.]]))
      vn.jump("leave")
      
      vn.label("leave")
      tamVN(fmt.f(_([["I don't know why, but the High Command has not credited the payment account yet… Well do you know what we are going to do? I will give you {rew}! One always needs Gauss Guns, no?"]]), {
         rew=fmt.f("#o".._("a set of Gauss Guns worth {credits}").."#0",
            {credits=fmt.credits(fw.credits_00)})
      }))
      vn.done()
      vn.run()

      -- Major Tam gives Gauss Guns instead of credits, because Major Tam is a freak.
      mem.GGprice = outfit.get("Gauss Gun"):price()
      mem.nb = math.floor(fw.credits_00/mem.GGprice+0.5)
      player.outfitAdd("Gauss Gun", mem.nb)
      misn.finish(true)
   else
      vntk.msg(_("What are you doing here?"), _("You were supposed to escort Major Tam, weren't you?"))
      misn.finish(false)
   end
   --hook.rm(mem.jumpingTam)
   mem.tamJumped = true
   mem.previous = spob.cur()
   misn.npcAdd("discussWithTam", _("Major Tam"), fw.portrait_tam, _("Major Tam is a very friendly man. At least by Dvaered military standards."))
end

function loading()
   if mem.stage ~= 0 then -- Tam has nothing to say at stage 0
      misn.npcAdd("discussWithTam", _("Major Tam"), fw.portrait_tam, _("Major Tam is a very friendly man. At least by Dvaered military standards."))
   end
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
   
      VNproceed2land(destpla1)
      mem.stage = 1
      majorTam:taskClear()
      majorTam:land(destpla1)
      misn.osdActive(2)

   elseif mem.stage == 2 then

      mem.nextsys = fleesys
      
      vn.clear()
      vn.scene()
      local tamVN = vn.newCharacter( fw.vn_char( fw.tam, { shader=love_shaders.hologram() } ) )
      vn.transition()
      vn.na("Tam boards the Goddard. A few seconds later, he undocks in a hurry, while nearby fighters start to shoot at him. He hails you and you answer.")
      tamVN(fmt.f(_([["That old fool tried to kill me! Quick, we must head to {sys}! Let me jump first!"]]), {sys=mem.nextsys}))
      vn.done()
      vn.run()

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
      VNproceed2land(destpla3)
      mem.stage = 8
      majorTam:taskClear()
      majorTam:land(destpla3)
      misn.osdActive(2)
   end
end

function VNproceed2land( destpla )
   vn.clear()
   vn.scene()
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam, { shader=love_shaders.hologram() } ) )
   vn.transition()
   vn.na("After Tam boards the Goddard, you wait for about half a period until his ship undocks from the warlord's cruiser. You get hailed by him.")
   tamVN(fmt.f(_([["Everything is right, we will now land on {pnt} in order to refuel and rest for some time."]]), {pnt=destpla}))
   vn.done()
   vn.run()
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
      x = 1000 * rnd.rnd() + 2000
      y = 1000 * rnd.rnd() + 2000
      pos = jp:pos() + vec2.new(x,y)

      ambush[i] = pilot.add( "Shark", fwarlords, pos, nil, {ai="baddie_norun"} )
      ambush[i]:setHostile()
      hook.pilot(ambush[i], "death", "ambushDied")
      hook.pilot(ambush[i], "land", "ambushDied")
      hook.pilot(ambush[i], "jump", "ambushDied")
   end

   x = 1000 * rnd.rnd() + 3000
   y = 1000 * rnd.rnd() + 3000
   pos = jp:pos() + vec2.new(x,y)
   hamelsen = pilot.add( "Shark", fwarlords, pos, _("Colonel Hamelsen"), {ai="baddie_norun", naked=true} )

   -- Nice outfits for Colonel Hamelsen (the Hellburner is her life insurance)
   equipopt.dvaered( hamelsen, {
      cores = {
         hull = "S&K Skirmish Plating",
         systems = "Milspec Orion 2301 Core System",
         engines = "Tricon Zephyr Engine",
      },
      prefer = {
         ["Gauss Gun"] = 100,
         ["Hellburner"] = 100,
         ["Improved Stabilizer"] = 100,
      },
      outfits_add = {
         "Hellburner",
      },
      move = 0,
      max_same_weap = 3,
      max_same_stru = 2,
      rnd = 0, -- Consistent
   } )
   hamelsen:setFuel(true)
   hamelsen:setNoDeath() -- We can't afford to loose our main baddie
   hamelsen:setNoDisable()

   mem.attack = hook.pilot( hamelsen, "attacked", "hamelsen_attacked" )

   mem.nambush = #ambush + 1

   -- Pre-position Captain Leblanc and her mates, but as Dvaered
   savers = {}
   for i = 1, 2 do
      x = 1000 * rnd.rnd() - 3000
      y = 1000 * rnd.rnd() - 3000
      pos = jp:pos() + vec2.new(x,y)

      savers[i] = pilot.add( "Dvaered Vendetta", "Dvaered", pos )
   end
   savers[1]:rename(_("Captain Leblanc"))
   savers[1]:setNoDeath()
   savers[1]:setNoDisable()

   mem.msg = hook.timer( 3.0, "ambush_msg" )
   mem.killed_ambush = 0
end

function ambush_msg()
   vn.clear()
   vn.scene()
   local hamelsenVN = vn.newCharacter( fw.vn_char( fw.hamelsen, { pos="left", shader=love_shaders.hologram() } ) )
   hamelsenVN:rename(_("Ambusher"))
   local tamVN      = vn.newCharacter( fw.vn_char( fw.tam, { pos="right", shader=love_shaders.hologram() } ) )
   
   vn.transition()
   vn.na("As your ship decelerates to its normal speed after jumping in, you realize there are hostile ships around. An enemy Shark opens a communication channel with you and Major Tam.")
   hamelsenVN(_([["Tam, you small, fearful weakling, did you believe Lord Battleaddict would really let you live?"]]))
   tamVN(_([["You have no right here. You are outside of Lord Battleaddict's space! I am the guest of Lord Jim."]]))
   hamelsenVN(_([["You wanted to meet Lord Jim? How about you meet your doom instead?"]]))
   vn.done()
   vn.run()

   ambush[1]:comm(_("Say hello to my mace rockets"))

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
      hamelsen:setEnergy(100) -- To activate the afterburner
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
   vn.clear()
   vn.scene()
   local leblancVN = vn.newCharacter( fw.vn_char( fw.leblanc, { pos="left", shader=love_shaders.hologram() } ) )
   leblancVN:rename(_("Patrol Leader"))
   local tamVN     = vn.newCharacter( fw.vn_char( fw.tam, { pos="right", shader=love_shaders.hologram() } ) )

   vn.transition()
   vn.na("As the remaining attackers flee, you remark that a Dvaered patrol helped you, contrary to what Tam had explained before.")
   leblancVN( fmt.f(_([["Good day, Major Tam."]]), {player=player.name()}) )
   tamVN(_([["This time, I really owe you one, Captain"]]))
   leblancVN(_([["No problem, sir. But the most dangerous one escaped. The Shark, you know, it was Hamelsen, Battleaddict's second in command."]]))
   leblancVN(_([["After we heard of what the old scumbag had done to you, we put him under surveillance, and we spotted Hamelsen pursuing you with her Shark, so we followed her, pretending we're just a police squadron. You know the rest."]]))
   tamVN(fmt.f(_([["By the way, {player}, let me introduce you the Captain Leblanc. She belongs to the Special Operations Force (SOF), part of Dvaered High Command (DHC). I didn't tell you, but her pilots always keep an eye on me from a distance when I have to meet warlords. {player} is the private pilot I told you about, Captain."]]),{player=player.name()}))
   leblancVN(_([["Hello, citizen. I'm glad there are civilians like you who do their duty and serve the Dvaered Nation."]]))
   tamVN(_([["Anyway, I am afraid this ambush is not acceptable."]]))
   leblancVN(_([["True, sir. Attacking someone in one's system is a standard means of expression for a warlord, but setting an ambush here denotes a true lack of respect."]]))
   tamVN(_([["He will answer for this, trust me. I will refer this matter to the chief. Meanwhile, I still have an appointment with Lord Jim. I just hope he will not try to make us dance as well…"]]))
   vn.done()
   vn.run()
end

function discussWithTam()
   -- Major Tam is not senile: he says different things at the different stops
   if mem.stage == 2 then
      vn.clear()
      vn.scene()
      local tamVN = vn.newCharacter( fw.vn_char( fw.tam ) )
      vn.transition()
      tamVN(_([["How do you do, citizen? Did you enjoy the trip so far? I'm ready for the next stop. I'll follow you when you take off."]]))
      vn.done()
      vn.run()
   elseif mem.stage == 5 then
      vn.clear()
      vn.scene()
      local tamVN = vn.newCharacter( fw.vn_char( fw.tam ) )
      vn.transition()
      tamVN(_([["Hello, citizen. Did you already recover from Lord Battleaddict's last trick? It reminded me of my youth, when I used to belong to a fighter squadron in Amaroq…"]]))
      vn.menu{
         {_("Encourage Tam to talk about his past"), "tellmemore"},
         {_("Don't"), "finish"},
      }
      
      vn.label("tellmemore")
      tamVN(_([["You know, I've not always worked at Headquarters. I started as a pilot at the DHC base on Rhaana. Oh, sorry, DHC stands for Dvaered High Command. You know, there are two kinds of Dvaered soldiers: those who directly report to DHC, like myself, and the freaks, as we call them (or the warriors, as they call themselves), the soldiers who report to local Warlords."]]))
      tamVN(_([["Warlords' forces can be requisitioned by DHC, but only to fight forces that threaten the integrity of the Dvaered Nation, so, in practice, they are mostly left to themselves, and make war on each other. You know, foreigners sometimes think that the internecine conflicts between warlords are pointless (I've even heard the word "stupid" once), but actually, they're the key to Dvaered philosophy. Without those wars, the Dvaered Nation would no longer exist as we know it, and we would have had to rely on totalitarianism, like the Empire, nostalgia of an idealized past, like the Frontier, oppressive technocracy like the Za'lek, or such…"]]))
      tamVN(_([["Hey, but am I deviating from our original subject? What was it already? Oh I don't remember. Anyway, citizen, if you want to take off, I'm ready."]]))
      
      vn.label("finish")
      vn.done()
      vn.run()
   end
end
