--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Seek And Destroy">
 <avail>
  <priority>4</priority>
  <cond>player.numOutfit("Mercenary License") &gt; 0</cond>
  <chance>875</chance>
  <location>Computer</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Za'lek</faction>
 </avail>
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

local trigger_ambush, spawn_advisor, space_clue, next_sys -- Forward-declared functions
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
quotes.dono[2] = _("This person is also looking for {plt}, but doesn't seem to know anything you don't.")
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
quotes.dono[13] = _([["I used to work with {plt}. We haven't seen each other since they stole my favorite ship, though."]])

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
comms.ambush[7] = _("You were not supposed to get on the trail of {plt}!")

quotes.pay    = {}
quotes.pay[1] = _("An officer hands you your pay.")
quotes.pay[2] = _("No one will miss this outlaw pilot! The bounty has been deposited into your account.")

osd_msg    = {__save=true}  -- 3-part OSD: Search a system, do the deed, get paid.

function create ()
   paying_faction = planet.cur():faction()

   -- Choose the target faction among Pirate and FLF
   adm_factions = {faction.get("Pirate"), faction.get("FLF")}
   local fact = {}
   for i, j in ipairs(adm_factions) do
      if paying_faction:areEnemies(j) then
         fact[#fact+1] = j
      end
   end
   target_faction = fact[rnd.rnd(1,#fact)]

   if target_faction == nil then
      misn.finish( false )
   end

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 5,
      function(s)
         local p = s:presences()[target_faction:nameRaw()]
         return p ~= nil and p > 0
      end )

   -- Create the table of system the player will visit now (to claim)
   nbsys = rnd.rnd( 5, 9 ) -- Total number of available systems (in case the player misses the target first time)
   pisys = rnd.rnd( 2, 4 ) -- System where the target will be
   mysys = {}

   if #systems <= nbsys then
      -- Not enough systems
      misn.finish( false )
   end

   mysys[1] = systems[ rnd.rnd( 1, #systems ) ]

   -- There will probably be lot of failure in this loop.
   -- Just increase the mission probability to compensate.
   for i = 2, nbsys do
      thesys = systems[ rnd.rnd( 1, #systems ) ]
      -- Don't re-use the previous system
      if thesys == mysys[i-1] then
         misn.finish( false )
      end
      mysys[i] = thesys
   end

   if not misn.claim(mysys) then
      misn.finish(false)
   end

   aname = nil
   bname = nil
   local ships
   if target_faction == faction.get("FLF") then
      name = pilotname.generic()
      ships = {"Lancelot", "Vendetta", "Pacifier"}
      aship = "Pacifier"
      bship = "Lancelot"
   else -- default Pirate
      name = pilotname.pirate()
      ships = {"Pirate Shark", "Pirate Vendetta", "Pirate Admonisher"}
      aship = "Pirate Phalanx"
      bship = "Hyena"
      bname = _("Pirate Hyena")
   end

   tgtship = ships[rnd.rnd(1,#ships)]
   credits = 1e6 + rnd.rnd()*500e3
   cursys = 1

   -- Set mission details
   misn.setTitle( fmt.f( _("Seek And Destroy Mission, starting in {sys}"), {sys=mysys[1]} ) )
   misn.setDesc( fmt.f( _("The {target_faction} pilot known as {plt} is wanted dead or alive by {paying_faction} authorities. He was last seen in the {sys} system."),
         {target_faction=target_faction, plt=name, paying_faction=paying_faction, sys=mysys[1]} ) )
   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( mysys[1], "computer" )

   -- Store the table
   mysys["__save"] = true
end

-- Test if an element is in a list
local function elt_inlist( elt, list )
   for i, elti in ipairs(list) do
      if elti == elt then
         return true
      end
   end
   return false
end

function accept ()
   misn.accept()

   stage = 0
   increment = false
   last_sys = system.cur()
   tk.msg( _("Find and Kill a pilot"), fmt.f( _("{plt} is a notorious {target_faction} pilot who is wanted by the authorities, dead or alive. Any citizen who can find and neutralize {plt} by any means necessary will be given {credits} as a reward. {paying_faction} authorities have lost track of this pilot in the {sys} system. It is very likely that the target is no longer there, but this system may be a good place to start an investigation."), {plt=name, target_faction=target_faction, credits=fmt.credits(credits), paying_faction=paying_faction, sys=mysys[1]} ) )
   jumphook = hook.enter( "enter" )
   hailhook = hook.hail( "hail" )
   landhook = hook.land( "land" )

   osd_msg[1] = fmt.f( _("Fly to the {sys} system and search for clues"), {sys=mysys[1]} )
   osd_msg[2] = fmt.f( _("Kill {plt}"), {plt=name} )
   osd_msg[3] = fmt.f( _("Land in {paying_faction} territory to collect your bounty"), {paying_faction=paying_faction} )
   misn.osdCreate( _("Seek and Destroy"), osd_msg )
end

function enter ()
   hailed = {}

   -- Increment the target if needed
   if increment then
      increment = false
      cursys = cursys + 1
   end

   if stage <= 2 and system.cur() == mysys[cursys] then
      -- This system will contain the pirate
      -- cursys > pisys means the player has failed once (or more).
      if cursys == pisys or (cursys > pisys and rnd.rnd() > .5) then
         stage = 2
      end

      if stage == 0 then  -- Clue system
         if not var.peek("got_advice") then -- A bounty hunter who explains how it works
            var.push( "got_advice", true )
            spawn_advisor ()
         end
         if cursys > 1 and rnd.rnd() < .5 then
            trigger_ambush() -- An ambush
         end
      elseif stage == 2 then  -- Target system
         misn.osdActive( 2 )

         -- Get the position of the target
         local jp  = jump.get(system.cur(), last_sys)
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

         target_ship = pilot.add( tgtship, target_faction, pos )
         target_ship:rename( name )
         target_ship:setHilight( true )
         target_ship:setVisplayer()
         target_ship:setHostile()

         death_hook = hook.pilot( target_ship, "death", "target_death" )
         pir_jump_hook = hook.pilot( target_ship, "jump", "target_flee" )
         pir_land_hook = hook.pilot( target_ship, "land", "target_land" )
         jumpout = hook.jumpout( "player_flee" )

         -- If the target is weaker, runaway
         local pstat = player.pilot():stats()
         local tstat = target_ship:stats()
         if ( (1.1*(pstat.armour + pstat.shield)) > (tstat.armour + tstat.shield) ) then
            target_ship:control()
            target_ship:runaway(player.pilot())
         end
      end
   end
   last_sys = system.cur()
end

-- Enemies wait for the player
function trigger_ambush()
   local jp     = jump.get(system.cur(), last_sys)
   local x, y
   ambush = {}

   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[1] = pilot.add( aship, target_faction, pos, aname )
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[2] = pilot.add( bship, target_faction, pos, bname )
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[3] = pilot.add( bship, target_faction, pos, bname )

   ambush[1]:setHostile()
   ambush[2]:setHostile()
   ambush[3]:setHostile()
   ambush[1]:control()
   ambush[2]:control()
   ambush[3]:control()
   ambush[1]:attack(player.pilot())
   ambush[2]:attack(player.pilot())
   ambush[3]:attack(player.pilot())

   msg = hook.timer( 1.0, "ambust_msg" )
end

-- Enemies explain that they are ambushing the player
function ambust_msg()
   for i = 1, 3 do
      ambush[i]:comm( fmt.f( comms.ambush[rnd.rnd(1,#comms.ambush)], {plt=name} ) )
   end
end

function spawn_advisor ()
   local jp     = jump.get(system.cur(), last_sys)
   local x = 4000 * rnd.rnd() - 2000
   local y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)

   advisor = pilot.add( "Lancelot", "Mercenary", pos, nil, {ai="baddie_norun"} )
   hailie = hook.timer( 2.0, "hailme" )

   hailed[#hailed+1] = advisor
end

function hailme()
    advisor:hailPlayer()
    hailie2 = hook.pilot(advisor, "hail", "hail_ad")
end

function hail_ad()
   hook.rm(hailie)
   hook.rm(hailie2)
   tk.msg( _("You're looking for someone"), _([["Hi there", says the pilot. "You seem to be lost." As you explain that you're looking for an outlaw pilot and have no idea where to find your target, the pilot laughs. "So, you've taken a Seek and Destroy job, but you have no idea how it works. Well, there are two ways to get information on an outlaw: first way is to land on a planet and ask questions at the bar. The second way is to ask pilots in space. By the way, pilots of the same faction of your target are most likely to have information, but won't give it easily. Good luck with your task!"]]) ) -- Give advice to the player
end

-- Player hails a ship for info
function hail( target )
   if target:leader() == player.pilot() then
      -- Don't want the player hailing their own escorts.
      return
   end

   if system.cur() == mysys[cursys] and stage == 0 and not elt_inlist( target, hailed ) then
      hailed[#hailed+1] = target -- A pilot can be hailed only once

      if cursys+1 >= nbsys then -- No more claimed system : need to finish the mission
         tk.msg( _("Your track is cold"), fmt.f( quotes.cold[rnd.rnd(1,#quotes.cold)], {plt=name} ) )
         misn.finish(false)
      else

         -- If hailed pilot is enemy to the target, there is less chance he knows
         if target_faction:areEnemies( target:faction() ) then
            know = (rnd.rnd() > .9)
         else
            know = (rnd.rnd() > .3)
         end

         -- If hailed pilot is enemy to the player, there is less chance he tells
         if target:hostile() then
            tells = (rnd.rnd() > .95)
         else
            tells = (rnd.rnd() > .5)
         end

         if not know then -- NPC does not know the target
            tk.msg( _("No clue"), fmt.f( quotes.dono[rnd.rnd(1,#quotes.dono)], {plt=name} ) )
         elseif tells then
            tk.msg( _("I know the pilot you're looking for"), fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=name, sys=mysys[cursys+1]} ) )
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
   if target:hostile() then -- Pilot doesn't like you
      choice = tk.choice(_("I won't tell you"), quotes.noinfo[rnd.rnd(1,#quotes.noinfo)], _("Give up"), _("Threaten the pilot")) -- TODO maybe: add the possibility to pay
      if choice == 1 then
         -- End of function
      else -- Threaten the pilot
         if isScared( target ) and rnd.rnd() < .5 then
            tk.msg( _("You're intimidating!"), fmt.f( quotes.scared[rnd.rnd(1,#quotes.scared)], {plt=name, sys=mysys[cursys+1]} ) )
            next_sys()
            target:control()
            target:runaway(player.pilot())
         else
            tk.msg( _("Not impressed"), fmt.f( quotes.not_scared[rnd.rnd(1,#quotes.not_scared)], {plt=name, sys=mysys[cursys+1]} ) )
            target:comm(comms.not_scared[rnd.rnd(1,#comms.not_scared)])

            -- Clean the previous hook if it exists
            if attack then
               hook.rm(attack)
            end
            attack = hook.pilot( target, "attacked", "clue_attacked" )
         end
      end

   else -- Pilot wants payment

      price = (5 + 5*rnd.rnd()) * 1e3
      choice = tk.choice(
         _("How much money do you have?"),
         fmt.f( quotes.money[rnd.rnd(1,#quotes.money)], {plt=name, credits=fmt.credits(price)} ),
         _("Pay the sum"), _("Give up"), _("Threaten the pilot")
      )

      if choice == 1 then
         if player.credits() >= price then
            player.pay(-price)
            tk.msg( _("I know the pilot you're looking for"), fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=name, sys=mysys[cursys+1]} ) )
            next_sys()
            target:setHostile( false )
            target:comm(comms.thank[rnd.rnd(1,#comms.thank)])
         else
            tk.msg( _("Not enough money"), _("You don't have enough money.") )
         end
      elseif choice == 2 then
         -- End of function
      else -- Threaten the pilot

         -- Everybody except the pirates takes offence if you threaten them
         if not target:faction() == faction.get("Pirate") then
            faction.modPlayerSingle( target:faction(), -1 )
         end

         if isScared (target) then
            tk.msg( _("You're intimidating!"), fmt.f( quotes.scared[rnd.rnd(1,#quotes.scared)], {plt=name, sys=mysys[cursys+1]} ) )
            next_sys()
            target:control()
            target:runaway(player.pilot())
         else
            tk.msg( _("Not impressed"), fmt.f( quotes.not_scared[rnd.rnd(1,#quotes.not_scared)], {plt=name, sys=mysys[cursys+1]} ) )
            target:comm(comms.not_scared[rnd.rnd(1,#comms.not_scared)])

            -- Clean the previous hook if it exists
            if attack then
               hook.rm(attack)
            end
            attack = hook.pilot( target, "attacked", "clue_attacked" )
         end
      end
   end

end

-- Player attacks an informant who has refused to give info
function clue_attacked( p, attacker )
   -- Target was hit sufficiently to get more talkative
   if (attacker == player.pilot() or attacker:leader() == player.pilot())
         and p:health() < 100 then
      p:control()
      p:runaway(player.pilot())
      tk.msg( _("You're intimidating!"), fmt.f( quotes.scared[rnd.rnd(1,#quotes.scared)], {plt=name, sys=mysys[cursys+1]} ) )
      next_sys()
      hook.rm(attack)
   end
end

-- Spawn NPCs at bar, that give info
function land()
   -- Player flees from combat
   if stage == 2 then
      player_flee()

   -- Player seek for a clue
   elseif system.cur() == mysys[cursys] and stage == 0 then
      if rnd.rnd() < .3 then -- NPC does not know the target
         know = 0
      elseif rnd.rnd() < .5 then -- NPC wants money
         know = 1
         price = (5 + 5*rnd.rnd()) * 1e3
      else -- NPC tells the clue
         know = 2
      end
      mynpc = misn.npcAdd("clue_bar", _("Shifty Person"), portrait.get("Pirate"), _("This person might be an outlaw, a pirate, or even worse, a bounty hunter. You normally wouldn't want to get close to this kind of person, but they may be a useful source of information."))

   -- Player wants to be paid
   elseif planet.cur():faction() == paying_faction and stage == 4 then
      tk.msg( _("Good work, pilot!"), quotes.pay[rnd.rnd(1,#quotes.pay)] )
      player.pay( credits )
      paying_faction:modPlayerSingle( rnd.rnd(1,2) )
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.finish( true )
   end
end

-- The player ask for clues in the bar
function clue_bar()
   if cursys+1 >= nbsys then -- No more claimed system : need to finish the mission
      tk.msg( _("Your track is cold"), fmt.f( quotes.cold[rnd.rnd(1,#quotes.cold)], {plt=name} ) )
      misn.finish(false)
   else
      if know == 0 then -- NPC does not know the target
         tk.msg( _("No clue"), fmt.f( quotes.dono[rnd.rnd(1,#quotes.dono)], {plt=name} ) )
      elseif know == 1 then -- NPC wants money
         choice = tk.choice(
            _("How much money do you have?"),
            fmt.f( quotes.money[rnd.rnd(1,#quotes.money)], {plt=name, credits=fmt.credits(price)} ),
            _("Pay the sum"), _("Give up")
         )

         if choice == 1 then
            if player.credits() >= price then
               player.pay(-price)
               tk.msg( _("I know the pilot you're looking for"), fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=name, sys=mysys[cursys+1]} ) )
               next_sys()
            else
               tk.msg( _("Not enough money"), _("You don't have enough money.") )
            end
         else
            -- End of function
         end

      else -- NPC tells the clue
         tk.msg( _("I know the pilot you're looking for"), fmt.f( quotes.clue[rnd.rnd(1,#quotes.clue)], {plt=name, sys=mysys[cursys+1]} ) )
         next_sys()
      end

   end
   misn.npcRm(mynpc)
end

function next_sys ()
   misn.markerMove (marker, mysys[cursys+1])
   osd_msg[1] = fmt.f( _("Fly to the {sys} system and search for clues"), {sys=mysys[cursys+1]} )
   misn.osdCreate( _("Seek and Destroy"), osd_msg )
   increment = true
end

function player_flee ()
   tk.msg( _("You're not going to kill anybody like that"), fmt.f( _("You had a chance to neutralize {plt}, and you wasted it! Now you have to start all over. Maybe some other pilots in {sys} know where your target is going."), {plt=name, sys=system.cur()} ) )
   stage = 0
   misn.osdActive( 1 )

   hook.rm(death_hook)
   hook.rm(pir_jump_hook)
   hook.rm(pir_land_hook)
   hook.rm(jumpout)
end

function target_flee ()
   -- Target ran away. Unfortunately, we cannot continue the mission
   -- on the other side because the system has not been claimed...
   tk.msg( _("Target ran away"), fmt.f( _("That was close, but unfortunately, {plt} ran away. Maybe some other pilots in this system know where your target is heading."), {plt=name} ) )
   pilot.toggleSpawn(true)
   stage = 0
   misn.osdActive( 1 )

   hook.rm(death_hook)
   hook.rm(pir_jump_hook)
   hook.rm(pir_land_hook)
   hook.rm(jumpout)
end

function target_death ()
   stage = 4
   hook.rm(death_hook)
   hook.rm(pir_jump_hook)
   hook.rm(pir_land_hook)
   hook.rm(jumpout)

   misn.osdActive( 3 )
   misn.markerRm (marker)
   pilot.toggleSpawn(true)
end
