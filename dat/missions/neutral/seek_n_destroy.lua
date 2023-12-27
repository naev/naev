--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Seek And Destroy">
 <priority>4</priority>
 <cond>
   require("misn_test").mercenary()
 </cond>
 <chance>475</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   The player searches for an outlaw across several systems

   Stages :
   0) Next system will give a clue
   2) Next system will contain the target
   4) Target was killed

--]]
local pir = require "common.pirate"
local fmt = require "format"
local portrait = require "portrait"
local pilotname = require "pilotname"
local lmisn = require "lmisn"
local vn = require "vn"
local vntk = require "vntk"
local ccomm = require "common.comm"

local trigger_ambush, spawn_advisor, space_clue, next_sys, clear_target_hook -- Forward-declared functions
local adm_factions, advisor, ambush, hailed, target_ship -- Non-persistent state

local quotes = {}
local comms = {}

quotes.clue    = {}
quotes.clue[1] = _("You ask for information about {plt} and the pilot tells you that this outlaw is supposed to have business in {sys} soon.")
quotes.clue[2] = _([["{plt}? Yes, I know that scum. I've heard they like to hang around in {sys}. Good luck!"]])
quotes.clue[3] = _([["{plt} has owed me 500K credits for dozens of cycles and never paid me back! You can probably catch that thief in {sys}."]])
quotes.clue[4] = _([["If you're looking for {plt}, I would suggest going to {sys} and taking a look there; that's where that outlaw was last time I heard."]])
quotes.clue[5] = _([["If I was looking for {plt}, I would look in the {sys} system. That's probably a good bet."]])

quotes.dono    = {}
quotes.dono[1] = _("This person has never heard of {plt}. It seems you will have to ask someone else.")
quotes.dono[2] = _("This person is also looking for {plt}, but doesn't seem to know anything more than you.")
quotes.dono[3] = _([["{plt}? Nope, I haven't seen that person in many cycles at this point."]])
quotes.dono[4] = _([["Sorry, I have no idea where {plt} is."]])
quotes.dono[5] = _([["Oh, hell no, I stay as far away from {plt} as I possibly can."]])
quotes.dono[6] = _([["I haven't a clue where {plt} is."]])
quotes.dono[7] = _([["I don't give a damn about {plt}. Go away."]])
quotes.dono[8] = _([["{plt}? Don't know, don't care."]])
quotes.dono[9] = _("When you ask about {plt}, you are promptly told to get lost.")
quotes.dono[10] = _([["I'd love to get back at {plt} for last cycle, but I haven't seen them in quite some time now."]])
quotes.dono[11] = _([["I've not seen {plt}, but good luck in your search!"]])
quotes.dono[12] = _([["Wouldn't revenge be nice? Unfortunately I haven't a clue where {plt} is, though. Sorry!"]])
quotes.dono[13] = _([["I used to work with {plt}. We haven't seen each other since they stole my favourite ship, though."]])

quotes.money    = {}
quotes.money[1] = _([["{plt}, you say? Well, I don't offer my services for free. Pay me {credits} and I'll tell you where to look; how does that sound?"]])
quotes.money[2] = _([["Ah, yes, I know where probably {plt} is. I'll tell you for just {credits}. What do you say?"]])
quotes.money[3] = _([["{plt}? Of course, I know this pilot. I can tell you where they were last heading, but it'll cost you. {credits}. Deal?"]])
quotes.money[4] = _([["Ha ha ha! Yes, I've seen {plt} around! Will I tell you where? Heck no! Not unless you pay me, of course... {credits} should be sufficient."]])
quotes.money[5] = _([["You're looking for {plt}? I tell you what: give me {credits} and I'll tell you. Otherwise, get lost!"]])

quotes.not_scared    = {}
quotes.not_scared[1] = _([["As if the likes of you would ever try to fight me!"]])
quotes.not_scared[2] = _("The pilot simply sighs and cuts the connection.")
quotes.not_scared[3] = _([["What a lousy attempt to scare me."]])

quotes.scared    = {}
quotes.scared[1] = _("As it becomes clear that you have no problem with blasting a ship to smithereens, the pilot tells you that {plt} is supposed to have business in {sys} soon.")
quotes.scared[2] = _([["OK, OK, I'll tell you! You can find {plt} in the {sys} system! Leave me alone!"]])

quotes.cold    = {}
quotes.cold[1] = _("When you ask for information about {plt}, they tell you that this outlaw has already been killed by someone else.")
quotes.cold[2] = _([["Didn't you hear? That outlaw's dead. Got blown up in an asteroid field is what I heard."]])
quotes.cold[3] = _([["Ha ha, you're still looking for that outlaw? You're wasting your time; they've already been taken care of."]])
quotes.cold[4] = _([["Ah, sorry, that target's already dead. Blown to smithereens by a mercenary. I saw the scene, though! It was glorious."]])

quotes.noinfo    = {}
quotes.noinfo[1] = _("The pilot asks you to give them one good reason to give you that information.")
quotes.noinfo[2] = _([["What if I know where your target is and I don't want to tell you, eh?"]])
quotes.noinfo[3] = _([["Piss off! I won't tell anything to the likes of you!"]])
quotes.noinfo[4] = _([["And why exactly should I give you that information?"]])
quotes.noinfo[5] = _([["And why should I help you, eh? Get lost!"]])

comms.thank    = {}
comms.thank[1] = _("Hehe, pleasure to deal with you!")
comms.thank[2] = _("Thank you and goodbye!")
comms.thank[3] = _("See ya later!")
comms.thank[4] = _("Haha, good luck!")

comms.not_scared    = {}
comms.not_scared[1] = _("Mommy, I'm so scared! Har har har!")
comms.not_scared[2] = _("Haw haw haw! You're ridiculous!")
comms.not_scared[3] = _("Just come at me if you dare!")
comms.not_scared[4] = _("You're so pitiful!")

comms.ambush    = {}
comms.ambush[1] = _("You want to meet {plt}? Well he doesn't want to meet you!")
comms.ambush[2] = _("Stop asking questions about {plt}!")
comms.ambush[3] = _("Why are you following {plt}?")
comms.ambush[4] = _("Quit following {plt}!")
comms.ambush[5] = _("Your quest for {plt} ends here!")
comms.ambush[6] = _("You ask too many questions about {plt}!")
comms.ambush[7] = _("You were not supposed to pick up the trail of {plt}!")

quotes.pay    = {}
quotes.pay[1] = _("An officer hands you your pay.")
quotes.pay[2] = _("No one will miss this outlaw pilot! The bounty has been deposited into your account.")

mem.osd_msg    = {}  -- 3-part OSD: Search a system, do the deed, get paid.

function create ()
   mem.paying_faction = spob.cur():faction()

   -- Choose the target faction among Pirate and FLF
   adm_factions = {faction.get("Pirate"), faction.get("FLF")}
   local fact = {}
   for i, j in ipairs(adm_factions) do
      if mem.paying_faction:areEnemies(j) then
         fact[#fact+1] = j
      end
   end
   mem.target_faction = fact[rnd.rnd(1,#fact)]

   if mem.target_faction == nil then
      misn.finish( false )
   end

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 5,
      function(s)
         local p = s:presences()[mem.target_faction:nameRaw()]
         return p ~= nil and p > 0
      end )

   -- Create the table of system the player will visit now (to claim)
   mem.nbsys = rnd.rnd( 5, 9 ) -- Total number of available systems (in case the player misses the target first time)
   mem.pisys = rnd.rnd( 2, 4 ) -- System where the target will be
   mem.mysys = {}

   if #systems <= mem.nbsys then
      -- Not enough systems
      misn.finish( false )
   end

   mem.mysys[1] = systems[ rnd.rnd( 1, #systems ) ]

   -- There will probably be lot of failure in this loop.
   -- Just increase the mission probability to compensate.
   for i = 2, mem.nbsys do
      mem.thesys = systems[ rnd.rnd( 1, #systems ) ]
      -- Don't re-use the previous system
      if mem.thesys == mem.mysys[i-1] then
         misn.finish( false )
      end
      mem.mysys[i] = mem.thesys
   end

   if not misn.claim(mem.mysys) then
      misn.finish(false)
   end

   local ships
   if mem.target_faction == faction.get("FLF") then
      mem.name = pilotname.generic()
      ships = {"Lancelot", "Vendetta", "Pacifier"}
      mem.aship = "Pacifier"
      mem.bship = "Lancelot"
   else -- default Pirate
      mem.name = pilotname.pirate()
      ships = {"Pirate Shark", "Pirate Vendetta", "Pirate Admonisher"}
      mem.aship = "Pirate Phalanx"
      mem.bship = "Pirate Hyena"
   end

   mem.tgtship = ships[rnd.rnd(1,#ships)]
   mem.credits = 1e6 + rnd.rnd()*500e3
   mem.cursys = 1
   -- Faction prefix
   local prefix
   if mem.paying_faction:static() then
      prefix = ""
   else
      prefix = require("common.prefix").prefix(mem.paying_faction)
   end

   -- Set mission details
   misn.setTitle(prefix..fmt.f( _("Seek And Destroy Mission, starting in {sys}"), {sys=mem.mysys[1]} ) )
   local desc = fmt.f(_([[The {target_faction} pilot known as {plt} is wanted dead or alive by {paying_faction} authorities. They were last seen in the {sys} system.

#nTarget:#0 {plt} ({shipclass}-class ship)
#nWanted:#0 Dead or Alive
#nLast Seen:#0 {sys} system]]),
         {target_faction=mem.target_faction, plt=mem.name, paying_faction=mem.paying_faction, sys=mem.mysys[1], shipclass=_(ship.get(mem.tgtship):classDisplay())} )
   if not mem.paying_faction:static() then
      desc = desc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=mem.paying_faction})
   end
   misn.setDesc(desc)
   misn.setReward( mem.credits )
   mem.marker = misn.markerAdd( mem.mysys[1], "computer" )
end

function accept ()
   misn.accept()

   mem.stage = 0
   mem.increment = false
   mem.last_sys = system.cur()

   vntk.msg( _("Find and Kill a pilot"), fmt.f( _("{plt} is a notorious {target_faction} pilot who is wanted by the authorities, dead or alive. Any citizen who can find and neutralize {plt} by any means necessary will be given {credits} as a reward. {paying_faction} authorities have lost track of this pilot in the {sys} system. It is very likely that the target is no longer there, but this system may be a good place to start an investigation."),
      {plt=mem.name, target_faction=mem.target_faction, credits=fmt.credits(mem.credits), paying_faction=mem.paying_faction, sys=mem.mysys[1]} ) )
   mem.jumphook = hook.jumpin( "enter" )
   mem.hailhook = hook.hail( "hail" )
   mem.landhook = hook.land( "land" )

   mem.osd_msg[1] = fmt.f( _("Fly to the {sys} system and search for clues"), {sys=mem.mysys[1]} )
   mem.osd_msg[2] = fmt.f( _("Kill {plt}"), {plt=mem.name} )
   mem.osd_msg[3] = fmt.f( _("Land in {paying_faction} territory to collect your bounty"), {paying_faction=mem.paying_faction} )
   misn.osdCreate( _("Seek and Destroy"), mem.osd_msg )
end

function enter ()
   hailed = {}

   -- Increment the target if needed
   if mem.increment then
      mem.increment = false
      mem.cursys = mem.cursys + 1
   end

   if mem.stage <= 2 and system.cur() == mem.mysys[mem.cursys] then
      -- This system will contain the pirate
      -- mem.cursys > mem.pisys means the player has failed once (or more).
      if mem.cursys == mem.pisys or (mem.cursys > mem.pisys and rnd.rnd() > .5) then
         mem.stage = 2
      end

      if mem.stage == 0 then  -- Clue system
         if not var.peek("got_advice") then -- A bounty hunter who explains how it works
            var.push( "got_advice", true )
            spawn_advisor ()
         end
         if mem.cursys > 1 and rnd.rnd() < .5 then
            trigger_ambush() -- An ambush
         end
      elseif mem.stage == 2 then  -- Target system
         misn.osdActive( 2 )

         -- Get the position of the target
         local jp  = jump.get(system.cur(), mem.last_sys)
         local pos
         if jp ~= nil then
            local x = 6000 * rnd.rnd() - 3000
            local y = 6000 * rnd.rnd() - 3000
            pos = jp:pos() + vec2.new(x,y)
         else
            pos = nil
         end

         -- Spawn the target
         pilot.toggleSpawn( false )
         pilot.clear()

         target_ship = pilot.add( mem.tgtship, mem.target_faction, pos, mem.name )
         target_ship:setHilight( true )
         target_ship:setVisplayer()
         target_ship:setHostile()

         mem.death_hook = hook.pilot( target_ship, "death", "target_death" )
         mem.board_hook = hook.pilot( target_ship, "board", "target_board" )
         mem.pir_jump_hook = hook.pilot( target_ship, "jump", "target_flee" )
         mem.pir_land_hook = hook.pilot( target_ship, "land", "target_flee" )
         mem.jumpout = hook.jumpout( "player_flee" )

         -- If the target is weaker, runaway
         local pstat = player.pilot():stats()
         local tstat = target_ship:stats()
         if ( (1.1*(pstat.armour + pstat.shield)) > (tstat.armour + tstat.shield) ) then
            target_ship:control()
            target_ship:runaway(player.pilot())
         end
      end
   end
   mem.last_sys = system.cur()
end

-- Enemies wait for the player
function trigger_ambush()
   local jp = jump.get( system.cur(), mem.last_sys )
   local x, y
   ambush = {}

   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   local pos = jp:pos() + vec2.new(x,y)
   ambush[1] = pilot.add( mem.aship, mem.target_faction, pos )
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[2] = pilot.add( mem.bship, mem.target_faction, pos )
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[3] = pilot.add( mem.bship, mem.target_faction, pos )

   ambush[1]:setHostile()
   ambush[2]:setHostile()
   ambush[3]:setHostile()
   ambush[1]:control()
   ambush[2]:control()
   ambush[3]:control()
   ambush[1]:attack(player.pilot())
   ambush[2]:attack(player.pilot())
   ambush[3]:attack(player.pilot())

   mem.msg = hook.timer( 1.0, "ambust_msg" )
end

-- Enemies explain that they are ambushing the player
function ambust_msg()
   for i = 1, 3 do
      ambush[i]:comm( fmt.f( comms.ambush[rnd.rnd(1,#comms.ambush)], {plt=mem.name} ) )
   end
end

function spawn_advisor ()
   local jp     = jump.get(system.cur(), mem.last_sys)
   local x = 4000 * rnd.rnd() - 2000
   local y = 4000 * rnd.rnd() - 2000
   local pos = jp:pos() + vec2.new(x,y)

   advisor = pilot.add( "Lancelot", "Mercenary", pos, nil, {ai="baddie_norun"} )
   mem.hailie = hook.timer( 2.0, "hailme" )

   hailed[#hailed+1] = advisor
end

function hailme()
    advisor:hailPlayer()
    mem.hailie2 = hook.pilot(advisor, "hail", "hail_ad")
end

function hail_ad()
   hook.rm(mem.hailie)
   hook.rm(mem.hailie2)

   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, advisor )
   vn.transition()
   p(_([["Hi there", says the pilot. "You seem to be lost." As you explain that you're looking for an outlaw pilot and have no idea where to find your target, the pilot laughs. "So, you've taken a Seek and Destroy job, but you have no idea how it works. Well, there are two ways to get information on an outlaw: first way is to land on a planet and ask questions at the bar. The second way is to ask pilots in space. By the way, pilots of the same faction as your target are most likely to have information, but won't give it easily. Good luck with your task!"]]) ) -- Give advice to the player
   vn.run()

   player.commClose()
end

-- Player hails a ship for info
function hail( target )
   if target:withPlayer() or (target:leader() == player.pilot()) then
      -- Don't want the player hailing their own escorts.
      return
   end

   if system.cur() == mem.mysys[mem.cursys] and mem.stage == 0 and not inlist( hailed, target ) then
      hailed[#hailed+1] = target -- A pilot can be hailed only once

      if mem.cursys+1 >= mem.nbsys then -- No more claimed system : need to finish the mission
         vn.clear()
         vn.scene()
         local p = ccomm.newCharacter( vn, target )
         vn.transition()
         p(fmt.f( quotes.cold[rnd.rnd(1,#quotes.cold)], {plt=mem.name}))
         vn.run()

         misn.finish(false)
      else

         -- If hailed pilot is enemy to the target, there is less chance he knows
         if mem.target_faction:areEnemies( target:faction() ) then
            mem.know = (rnd.rnd() > .9)
         else
            mem.know = (rnd.rnd() > .3)
         end

         -- If hailed pilot is enemy to the player, there is less chance he mem.tells
         if target:hostile() then
            mem.tells = (rnd.rnd() > .95)
         else
            mem.tells = (rnd.rnd() > .5)
         end

         if not mem.know then -- NPC does not know the target
            vn.clear()
            vn.scene()
            local p = ccomm.newCharacter( vn, target )
            vn.transition()
            p(fmt.f( quotes.dono[rnd.rnd(1,#quotes.dono)], {plt=mem.name}))
            vn.run()

         elseif mem.tells then
            vn.clear()
            vn.scene()
            local p = ccomm.newCharacter( vn, target )
            vn.transition()
            p(fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]}))
            vn.run()

            next_sys()
            target:setHostile( false )
         else
            space_clue( target )
         end
      end

      player.commClose()
   end
end

-- Decides if the pilot is scared by the player
local function isScared( target )
   local pstat = player.pilot():stats()
   local tstat = target:stats()

   -- If target is stronger, no fear
   if tstat.armour+tstat.shield > 1.1 * (pstat.armour+pstat.shield) and rnd.rnd() > .2 then
      return false
   end

   -- If target is quicker, no fear
   if tstat.speed_max > pstat.speed_max and rnd.rnd() > .2 then
      if target:hostile() then
         target:control()
         target:runaway(player.pilot())
      end
      return false
   end

   if rnd.rnd() > .8 then
      return false
   end

   return true
end

-- The NPC knows the target. The player has to convince him to give info
function space_clue( target )
   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, target )
   vn.transition()

   if target:hostile() then -- Pilot doesn't like you
      p(quotes.noinfo[rnd.rnd(1,#quotes.noinfo)])
      vn.menu{
         {_([[Give up]]), "giveup"},
         {_([[Threaten the pilot]]), "threaten"},
         -- TODO maybe: add the possibility to pay
      }

      vn.label("giveup")
      vn.done()

      vn.label( "threaten" )
      vn.func( function ()
         if isScared( target ) and rnd.rnd() < 0.5 then
            vn.jump( "intimidating" )
         else
            vn.jump( "notimpressed" )
         end
      end )

      vn.label( "intimidating" )
      p( fmt.f( quotes.scared[rnd.rnd(1,#quotes.scared)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]} ) )
      vn.func( function ()
         next_sys()
         target:control()
         target:runaway(player.pilot())
      end )
      vn.done()

      vn.label( "notimpressed" )
      p( fmt.f( quotes.not_scared[rnd.rnd(1,#quotes.not_scared)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]} ) )
      vn.func( function ()
         target:comm(comms.not_scared[rnd.rnd(1,#comms.not_scared)])

         -- Clean the previous hook if it exists
         if mem.attack then
            hook.rm(mem.attack)
         end
         mem.attack = hook.pilot( target, "attacked", "clue_attacked" )
      end )

   else -- Pilot wants payment

      vn.func( function ()
         mem.price = (5 + 5*rnd.rnd()) * 1e3
      end )
      p(_("How much money do you have?"))
      vn.menu( function ()
         return {
            {fmt.f(_([[Pay {amount}]]), {amount=fmt.credits(mem.price)}), "pay"},
            {_([[Give up]]), "giveup"},
            {_([[Threaten the pilot]]), "threaten"},
         }
      end )

      vn.label("broke")
      vn.na(_("You don't have enough money."))
      vn.done()

      vn.label("giveup")
      vn.done()

      vn.label("pay")
      vn.func( function ()
         if player.credits() < mem.price then
            vn.jump("broke")
            return
         end
         player.pay(-mem.price)
      end )
      p(_("I know the pilot you're looking for"), fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]}))
      vn.func( function ()
         next_sys()
         target:setHostile( false )
         target:comm(comms.thank[rnd.rnd(1,#comms.thank)])
      end )
      vn.done()

      vn.label("threaten")
      vn.func( function ()
         -- Everybody except the pirates takes offence if you threaten them
         if not pir.factionIsPirate( target:faction() ) then
            faction.modPlayerSingle( target:faction(), -1 )
         end

         if isScared(target) then
            vn.jump("scared")
         else
            vn.jump("notimpressed")
         end
      end )

      vn.label("scared")
      p(fmt.f( quotes.scared[rnd.rnd(1,#quotes.scared)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]}))
      vn.func( function ()
            next_sys()
            target:control()
            target:runaway(player.pilot())
      end )

      vn.label("notimpressed")
      p(fmt.f( quotes.not_scared[rnd.rnd(1,#quotes.not_scared)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]}))
      vn.func( function ()
            target:comm(comms.not_scared[rnd.rnd(1,#comms.not_scared)])
            -- Clean the previous hook if it exists
            if mem.attack then
               hook.rm(mem.attack)
            end
            mem.attack = hook.pilot( target, "attacked", "clue_attacked" )
      end )
   end

   vn.run()
end

-- Player attacks an informant who has refused to give info
function clue_attacked( p, attacker )
   -- Target was hit sufficiently to get more talkative
   if attacker and attacker:withPlayer() and p:health() < 100 then
      p:control()
      p:runaway(player.pilot())
      vntk.msg( _("You're intimidating!"), fmt.f( quotes.scared[rnd.rnd(1,#quotes.scared)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]} ) )
      next_sys()
      hook.rm(mem.attack)
   end
end

-- Spawn NPCs at bar, that give info
function land()
   -- Player flees from combat
   if mem.stage == 2 then
      player_flee()

   -- Player seek for a clue
   elseif system.cur() == mem.mysys[mem.cursys] and mem.stage == 0 then
      if rnd.rnd() < .3 then -- NPC does not know the target
         mem.know = 0
      elseif rnd.rnd() < .5 then -- NPC wants money
         mem.know = 1
         mem.price = (5 + 5*rnd.rnd()) * 1e3
      else -- NPC mem.tells the clue
         mem.know = 2
      end
      mem.mynpc_name = _("Shifty Person")
      mem.mynpc_portrait = portrait.get("Pirate")
      mem.mynpc = misn.npcAdd("clue_bar", mem.mynpc_name, mem.mynpc_portrait, _("This person might be an outlaw, a pirate, or even worse, a bounty hunter. You normally wouldn't want to get close to this kind of person, but they may be a useful source of information."))

   -- Player wants to be paid
   elseif spob.cur():faction() == mem.paying_faction and mem.stage == 4 then
      lmisn.sfxVictory()
      vntk.msg( _("Good work, pilot!"), quotes.pay[rnd.rnd(1,#quotes.pay)].."\n\n"..fmt.reward(mem.credits) )
      player.pay( mem.credits )
      mem.paying_faction:modPlayerSingle( rnd.rnd(1,2) )
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.finish( true )
   end
end

-- The player ask for clues in the bar
function clue_bar()
   vn.clear()
   vn.scene()
   local p = vn.newCharacter( mem.mynpc_name, {image=portrait.getFullPath(mem.mynpc_portrait)} )
   vn.transition()

   if mem.cursys+1 >= mem.nbsys then -- No more claimed system : need to finish the mission
      p(fmt.f( quotes.cold[rnd.rnd(1,#quotes.cold)] .. "\n\n#r" .. _("MISSION FAILED") .. "#0", {plt=mem.name}))
      vn.run()

      misn.finish(false)
   else
      if mem.know == 0 then -- NPC does not know the target
         p(fmt.f( quotes.dono[rnd.rnd(1,#quotes.dono)], {plt=mem.name} ) )

      elseif mem.know == 1 then -- NPC wants money
         p( fmt.f( quotes.money[rnd.rnd(1,#quotes.money)], {plt=mem.name, credits=fmt.credits(mem.price)} ) )
         vn.menu{
            {_([[Pay the sum]]), "pay"},
            {_([[Give up]]), "giveup"},
         }

         vn.label("giveup")
         vn.done()

         vn.label("pay")
         vn.func( function ()
            if player.credits() >= mem.price then
               player.pay(-mem.price)
               vn.jump("paid")
            else
               vn.jump("nomoney")
            end
         end )

         vn.label("nomoney")
         vn.na(_([[You don't have enough money.]]))
         vn.done()

         vn.label("paid")
         p(fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]}))
         vn.func( function ()
            next_sys()
         end )
         vn.done()

      else -- NPC tells the clue
         p(fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=mem.name, sys=mem.mysys[mem.cursys+1]}))
         vn.func( function ()
            next_sys()
         end )
      end
   end

   vn.run()

   misn.npcRm(mem.mynpc)
end

function next_sys ()
   misn.markerMove (mem.marker, mem.mysys[mem.cursys+1])
   mem.osd_msg[1] = fmt.f( _("Fly to the {sys} system and search for clues"), {sys=mem.mysys[mem.cursys+1]} )
   misn.osdCreate( _("Seek and Destroy"), mem.osd_msg )
   mem.increment = true
end

function player_flee ()
   vntk.msg( _("You're not going to kill anybody like that"), fmt.f( _("You had a chance to neutralize {plt}, and you wasted it! Now you have to start all over. Maybe some other pilots in {sys} know where your target is going."), {plt=mem.name, sys=system.cur()} ) )
   mem.stage = 0
   misn.osdActive( 1 )
   clear_target_hook()
end

function target_flee ()
   -- Target ran away. Unfortunately, we cannot continue the mission
   -- on the other side because the system has not been claimed...
   vntk.msg( _("Target ran away"), fmt.f( _("That was close, but unfortunately, {plt} ran away. Maybe some other pilots in this system know where your target is heading."), {plt=mem.name} ) )
   pilot.toggleSpawn(true)
   mem.stage = 0
   misn.osdActive( 1 )
   clear_target_hook()
end

function target_death ()
   mem.stage = 4
   clear_target_hook()

   misn.osdActive( 3 )
   misn.markerRm(mem.marker)
   pilot.toggleSpawn(true)
end

function target_board( p )
   mem.stage = 4
   clear_target_hook()

   vntk.msg( _("Target captured"), _("You board the ship and, after a short but intense firefight, are able to take the wanted outlaw alive. Time to hand them in to the authorities.") )
   p:disable() -- Permanently disable
   local c = commodity.new( N_("Wanted Outlaw"), N_("A wanted outlaw you captured.") )
   misn.cargoAdd( c, 0 )

   misn.osdActive( 3 )
   misn.markerRm(mem.marker)
   pilot.toggleSpawn(true)

   player.unboard()
end

function clear_target_hook ()
   hook.rm(mem.death_hook)
   hook.rm(mem.board_hook)
   hook.rm(mem.pir_jump_hook)
   hook.rm(mem.pir_land_hook)
   hook.rm(mem.jumpout)
end
