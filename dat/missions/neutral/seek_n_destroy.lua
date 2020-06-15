--[[

   The player searches for an outlaw across several systems

   Stages :
   0) Next system will give a clue
   2) Next system will contain the target
   4) Target was killed

--]]

include "numstring.lua"
include "jumpdist.lua"
include "portrait.lua"
include "pilot/pirate.lua"

clue_title   = _("I know the pilot you're looking at")
clue_text    = {}
clue_text[1] = _("You ask for information about %s and the pilot tells you that this outlaw is supposed to have buisness in %s soon.")
clue_text[2] = _("%s? Of course I know that scum. I've heard he likes to hang around in %s. Good luck!")
clue_text[3] = _("%s has owed me 100k credits for dozens of cycles and never paid me back! You can probably catch him in %s.")
clue_text[4] = _("Sorry, I'm busy these days. If you're looking for %s, I would suggest going to %s and taking a look there; that's where he was last time I heard.")

dono_title   = _("No clue")
dono_text    = {}
dono_text[1] = _("This person has never heard of %s. It seems you will have to ask someone else.")
dono_text[2] = _("This person is also looking for %s, but doesn't seem to know anything you don't.")
dono_text[3] = _([["%s? Nope, I haven't seen him in many cycles at this point."]])
dono_text[4] = _([["Sorry, I have no idea where %s is."]])
dono_text[5] = _([["Oh, hell no, I stay as far away from %s as I possibly can."]])
dono_text[6] = _([["I haven't a clue where %s is."]])
dono_text[7] = _([["I don't give a damn about %s. Go away."]])
dono_text[8] = _([["%s? Don't know, don't care."]])
dono_text[9] = _("When you ask about %s, the pilot tells you to get lost and cuts communication.")
dono_text[10] = _([["I'd love to get back at %s for what he did to me, but I haven't seen him in quite some time now."]])
dono_text[11] = _([["I've not seen %s, but good luck finding him!"]])
dono_text[12] = _([["Wouldn't it be nice to get back at him? Unfortunately I haven't a clue where %s is, though."]])
dono_text[13] = _([["I used to work with %s. I haven't seen him since he stole my favorite ship, though."]])

money_title   = _("How much money do you have?")
money_text    = {}
money_text[1] = _([["%s, you say? Well, I don't offer my services for free. Pay me %s credits and I'll tell you where he is; how does that sound?"]])
money_text[2] = _([["Ah, yes, I know where %s is. I'll tell you for just %s credits. What do you say?"]])
money_text[3] = _([["%s? Of course, I know this pilot. I can tell you where he was last heading, but it'll cost you. %s credits. Deal?"]])
money_text[4] = _([["Ha ha ha! Yes, I've seen %s around! Will I tell you where? Heck no! Not unless you pay me, of course... %s credits should be sufficient."]])

IdoPay       = _("Pay the sum")
IdonnoPay    = _("Give up")
IkickYourAss = _("Treaten the pilot")

poor_title = _("Not enough money")
poor_text  = _("It seems you don't have enough money. Too bad.")

not_scared_title   = _("Not impressed")
not_scared_text    = {}
not_scared_text[1] = _([["As if the likes of you would ever try to fight me!"]])
not_scared_text[2] = _("The pilot simply sighs and cuts the connection.")
not_scared_text[3] = _([["What a lousy attempt to scare me."]])

scared_title   = _("You're intimidating!")
scared_text    = {}
scared_text[1] = _("As it becomes clear that you have no problem with blasting a ship to smithereens, the pilot tells you that %s is supposed to have buisness in %s soon.")

cold_title   = _("Your track is cold")
cold_text    = {}
cold_text[1] = _("When you ask for information about %s, they tell you that this outlaw has already been killed by someone else.")
cold_text[2] = _([["Didn't you hear? He's dead. Got blown up in an asteroid field is what I heard."]])
cold_text[3] = _([["Ha ha, you're still looking for him? You're wasting your time; he's already been taken care of."]])
cold_text[4] = _([["Ah, sorry, he's already dead. Blown to smithereens by a mercenary. I saw the scene, though! It was glorious."]])

noinfo_title   = _("I won't tell you")
noinfo_text    = {}
noinfo_text[1] = _("The pilot asks you to give them one good reason to give you that information.")
noinfo_text[2] = _([["What if I know where your target is and I don't want to tell you, eh?"]])
noinfo_text[3] = _([["Piss off! I won't tell anything to the likes of you!"]])
noinfo_text[4] = _([["And why exactly should I give you that information?"]])


advice_title = _("You're looking for someone")
advice_text  = _([["Hi there", says the pilot. "You seem to be lost." As you explain that you're looking for an outlaw pilot and have no idea where to find your target, the pilot laughs. "So, you've taken a Seek and Destroy job, but you have no idea how it works. Well, there are two ways to get information on an outlaw: first way is to land on a planet and ask questions at the bar. The second way is to ask pilots in space. By the way, pilots of the same faction of your target are most likely to have information, but won't give it easily. Good luck with your task!"]])

thank_comm    = {}
thank_comm[1] = _("It was a pleasure to deal with you!")
thank_comm[2] = _("Thank you and goodbye!")
thank_comm[3] = _("See you later!")
thank_comm[4] = _("Good luck!")

not_scared_comm    = {}
not_scared_comm[1] = _([[The pilot mocks you. "Mommy, I'm so scared!"]])
not_scared_comm[2] = _("Haw haw haw! you're ridiculous!")
not_scared_comm[3] = _("Just come at me if you dare!")
not_scared_comm[4] = _("You're so pitiful!")

ambush_comm    = {}
ambush_comm[1] = _("You want to meet %s? Well he doesn't want to meet you!")
ambush_comm[2] = _("Stop asking questions about %s!")
ambush_comm[3] = _("Why are you following %s?")
ambush_comm[4] = _("Quit following %s!")
ambush_comm[5] = _("Your quest for %s ends here!")
ambush_comm[6] = _("You ask too many questions about %s!")
ambush_comm[7] = _("You were not supposed to get on the way of %s!")


breef_title = _("Find and Kill a pilot")
breef_text = _("%s is a notorious %s pilot who is wanted by the authorities, dead or alive. Any citizen who can find and neutralize %s by any means necessary will be given %s credits as a reward. %s authorities have lost track of this pilot in the %s system. It is very likely that the target is no longer there, but this system may be a good place to start an investigation.")

flee_title = _("You're not going to kill anybody like that")
flee_text = _("You had a chance to neutralize %s, and you wasted it! Now you have to start all over. Maybe some other pilots in %s know where your target is going.")

Tflee_title = _("Target ran away")
Tflee_text = _("That was close, but unfortunately, %s ran away. Maybe some other pilots in this system know where your target is heading.")

pay_title   = _("Good work, pilot!")
pay_text    = {}
pay_text[1] = _("An officer hands you your pay.")
pay_text[2] = _("No one will miss this outlaw pilot! The bounty has been deposited into your account.")

osd_title = _("Seek and Destroy")
osd_msg    = {}
osd_msg1_r = _("Fly to the %s system and search for clues")
osd_msg[1] = " "
osd_msg[2] = _("Kill %s")
osd_msg[3] = _("Land on any %s planet and collect your bounty")
osd_msg["__save"] = true

npc_desc = _("Shifty Person")
bar_desc = _("This person might be an outlaw, a pirate, or even worse, a bounty hunter. You normally wouldn't want to get close to this kind of person, but they may be a useful source of information.")

-- Mission details
misn_title  = _("Seek And Destroy Mission, starting in %s")
misn_reward = _("%s credits")
misn_desc   = _("The %s pilot known as %s is wanted dead or alive by %s authorities. He was last seen in the %s system.")

function create ()
   paying_faction = planet.cur():faction()

   -- Choose the target faction among Pirate and FLF
   adm_factions = {faction.get("Pirate"), faction.get("FLF")}
   fact = {}
   for i, j in ipairs(adm_factions) do
      if paying_faction:areEnemies(j) then
         fact[#fact+1] = j
      end
   end
   target_faction = fact[rnd.rnd(1,#fact)]

   if target_faction == nil then
      misn.finish( false )
   end

   local systems = getsysatdistance( system.cur(), 1, 5,
      function(s)
         local p = s:presences()[target_faction:name()]
         return p ~= nil and p > 0
      end )

   -- Create the table of system the player will visit now (to claim)
   nbsys = rnd.rnd( 5, 9 ) -- Total nb of avaliable systems (in case the player misses the target first time)
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

   name = pirate_name()
   if target_faction:name() == "Pirate" then
      ships = {"Pirate Shark", "Pirate Vendetta", "Pirate Admonisher"}
      aship = "Pirate Phalanx"
      bship = "Pirate Hyena"
   else -- FLF
      ships = {"FLF Lancelot", "FLF Vendetta", "FLF Pacifier"}
      aship = "FLF Pacifier"
      bship = "FLF Lancelot"
   end

   ship = ships[rnd.rnd(1,#ships)]
   credits = 1000000 + rnd.rnd()*500000
   cursys = 1

   -- Set mission details
   misn.setTitle( misn_title:format( mysys[1]:name() ) )
   misn.setDesc( misn_desc:format( target_faction:name(), name, paying_faction:name(), mysys[1]:name() ) )
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( mysys[1], "computer" )

   -- Store the table
   mysys["__save"] = true
end

-- Test if an element is in a list
function elt_inlist( elt, list )
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
   tk.msg( breef_title, breef_text:format( name, target_faction:name(), name, numstring(credits), paying_faction:name(), mysys[1]:name() ) )
   jumphook = hook.enter( "enter" )
   hailhook = hook.hail( "hail" )
   landhook = hook.land( "land" )

   osd_msg[1] = osd_msg1_r:format( mysys[1]:name() )
   osd_msg[2] = osd_msg[2]:format( name )
   osd_msg[3] = osd_msg[3]:format( paying_faction:name() )
   misn.osdCreate( osd_title, osd_msg )
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
         if not var.peek("got_advice") then -- A bountyhunter who explains how it works
            var.push( "got_advice", true )
            spawn_advisor ()
         end
         if cursys > 1 and rnd.rnd() < .5 then
            trigger_ambush() -- An ambush
         end
      elseif stage == 2 then  -- Target system
         misn.osdActive( 2 )
         
         -- Get the position of the target
         jp  = jump.get(system.cur(), last_sys)
         if jp ~= nil then
            x = 6000 * rnd.rnd() - 3000
            y = 6000 * rnd.rnd() - 3000
            pos = jp:pos() + vec2.new(x,y)
         else
            pos = nil
         end

         -- Spawn the target
         pilot.toggleSpawn( false )
         pilot.clear()

         target_ship = pilot.add( ship, nil, pos )[1]
         target_ship:rename( name )
         target_ship:setHilight( true )
         target_ship:setVisplayer()
         target_ship:setHostile()

         death_hook = hook.pilot( target_ship, "death", "target_death" )
         pir_jump_hook = hook.pilot( target_ship, "jump", "target_flee" )
         pir_land_hook = hook.pilot( target_ship, "land", "target_land" )
         jumpout = hook.jumpout( "player_flee" )

         -- If the target is weaker, runaway
         pstat = player.pilot():stats()
         tstat = target_ship:stats()
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
   jp     = jump.get(system.cur(), last_sys)
   ambush = {}

   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[1] = pilot.add( aship, nil, pos )[1]
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[2] = pilot.add( bship, nil, pos )[1]
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)
   ambush[3] = pilot.add( bship, nil, pos )[1]

   ambush[1]:setHostile()
   ambush[2]:setHostile()
   ambush[3]:setHostile()

   ambush[1]:comm(ambush_comm[rnd.rnd(1,#ambush_comm)]:format(name))
   ambush[2]:comm(ambush_comm[rnd.rnd(1,#ambush_comm)]:format(name))
   ambush[3]:comm(ambush_comm[rnd.rnd(1,#ambush_comm)]:format(name))
end

function spawn_advisor ()
   jp     = jump.get(system.cur(), last_sys)
   x = 4000 * rnd.rnd() - 2000
   y = 4000 * rnd.rnd() - 2000
   pos = jp:pos() + vec2.new(x,y)

   advisor = pilot.addRaw( "Lancelot", "baddie_norun", pos, "Mercenary" )
   hailie = hook.timer( 2000, "hailme" )

   hailed[#hailed+1] = advisor
end

function hailme()
    advisor:hailPlayer()
    hailie2 = hook.pilot(advisor, "hail", "hail_ad")
end

function hail_ad()
   hook.rm(hailie)
   hook.rm(hailie2)
   tk.msg( advice_title, advice_text ) -- Give advice to the player
end

-- Player hails a ship for info
function hail ()
    target = player.pilot():target()

   if system.cur() == mysys[cursys] and stage == 0 and not elt_inlist( target, hailed ) then

      hailed[#hailed+1] = target -- A pilot can be hailed only once

      if cursys+1 >= nbsys then -- No more claimed system : need to finish the mission
         tk.msg( cold_title, cold_text[rnd.rnd(1,#cold_text)]:format( name ) )
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
            tk.msg( dono_title, dono_text[rnd.rnd(1,#dono_text)]:format( name ) )
         elseif tells then
            tk.msg( clue_title, clue_text[rnd.rnd(1,#clue_text)]:format( name, mysys[cursys+1]:name() ) )
            next_sys()
         else
            space_clue()
         end
      end
   end
end

-- The NPC knows the target. The player has to convince him to give info
function space_clue ()

   if target:hostile() then -- Pilot doesnt like you
      choice = tk.choice(noinfo_title, noinfo_text[rnd.rnd(1,#noinfo_text)], IdonnoPay, IkickYourAss) -- TODO maybe: add the possibility to pay
      if choice == 1 then
         -- End of function
      else -- Threaten the pilot
         if isScared (target) and rnd.rnd() < .5 then
            tk.msg( scared_title, scared_text[rnd.rnd(1,#scared_text)]:format( name, mysys[cursys+1]:name() ) )
            next_sys()
            target:control()
            target:runaway(player.pilot())
         else
            tk.msg( not_scared_title, not_scared_text[rnd.rnd(1,#not_scared_text)]:format( name, mysys[cursys+1]:name() ) )
            target:comm(not_scared_comm[rnd.rnd(1,#not_scared_comm)])

            -- Clean the previous hook if it exists
            if attack then
               hook.rm(attack)
            end
            attack = hook.pilot( target, "attacked", "clue_attacked" )
         end
      end

   else -- Pilot wants payment

      price = (5 + 5*rnd.rnd()) * 1000
      choice = tk.choice(money_title, money_text[rnd.rnd(1,#money_text)]:format(name,numstring(price)), IdoPay, IdonnoPay, IkickYourAss)

      if choice == 1 then
         if player.credits() >= price then
            player.pay(-price)
            tk.msg( clue_title, clue_text[rnd.rnd(1,#clue_text)]:format( name, mysys[cursys+1]:name() ) )
            next_sys()
            target:comm(thank_comm[rnd.rnd(1,#thank_comm)])
         else
            tk.msg( poor_title, poor_text )
         end
      elseif choice == 2 then
         -- End of function
      else -- Threaten the pilot

         -- Everybody except the pirates takes offence if you threaten them
         if not target:faction():name() == "Pirate" then
            faction.modPlayerSingle( target:faction(), -1 )
         end

         if isScared (target) then
            tk.msg( scared_title, scared_text[rnd.rnd(1,#scared_text)]:format( name, mysys[cursys+1]:name() ) )
            next_sys()
         else
            tk.msg( not_scared_title, not_scared_text[rnd.rnd(1,#not_scared_text)]:format( name, mysys[cursys+1]:name() ) )
            target:comm(not_scared_comm[rnd.rnd(1,#not_scared_comm)])

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
   if attacker == player.pilot() and p:health() < 100 then
      p:setHostile( false )
      tk.msg( scared_title, scared_text[rnd.rnd(1,#scared_text)]:format( name, mysys[cursys+1]:name() ) )
      next_sys()
      hook.rm(attack)
   end
end

-- Decides if the pilot is scared by the player
function isScared (t)
   pstat = player.pilot():stats()
   tstat = t:stats()

   -- If target is stronger, no fear
   if tstat.armour+tstat.shield > 1.1 * (pstat.armour+pstat.shield) and rnd.rnd() > .2 then
      return false
   end

   -- If target is quicker, no fear
   if tstat.speed_max > pstat.speed_max and rnd.rnd() > .2 then
      if t:hostile() then
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

-- Spawn NPCs at bar, that give info
function land ()
   -- Player flees from combat
   if stage == 2 then
      player_flee()

   -- Player seek for a clue
   elseif system.cur() == mysys[cursys] and stage == 0 then
      if rnd.rnd() < .3 then -- NPC does not know the target
         know = 0
      elseif rnd.rnd() < .5 then -- NPC wants money
         know = 1
         price = (5 + 5*rnd.rnd()) * 1000
      else -- NPC tells the clue
         know = 2
      end
      mynpc = misn.npcAdd("clue_bar", npc_desc, getPortrait("Pirate"), bar_desc)

   -- Player wants to be paid
   elseif planet.cur():faction() == paying_faction and stage == 4 then
      tk.msg( pay_title, pay_text[rnd.rnd(1,#pay_text)] )
      player.pay( credits )
      paying_faction:modPlayerSingle( rnd.rnd(1,2) )
      misn.finish( true )
   end
end

-- The player ask for clues in the bar
function clue_bar()
   if cursys+1 >= nbsys then -- No more claimed system : need to finish the mission
      tk.msg( cold_title, cold_text[rnd.rnd(1,#cold_text)]:format( name ) )
      misn.finish(false)
   else

      if know == 0 then -- NPC does not know the target
         tk.msg( dono_title, dono_text[rnd.rnd(1,#dono_text)]:format( name ) )
      elseif know == 1 then -- NPC wants money
         choice = tk.choice(money_title, money_text[rnd.rnd(1,#money_text)]:format(name,numstring(price)), IdoPay, IdonnoPay)

         if choice == 1 then
            if player.credits() >= price then
               player.pay(-price)
               tk.msg( clue_title, clue_text[rnd.rnd(1,#clue_text)]:format( name, mysys[cursys+1]:name() ) )
               next_sys()
            else
               tk.msg( poor_title, poor_text )
            end
         else
            -- End of function
         end

      else -- NPC tells the clue
         tk.msg( clue_title, clue_text[rnd.rnd(1,#clue_text)]:format( name, mysys[cursys+1]:name() ) )
         next_sys()
      end

   end
   misn.npcRm(mynpc)
end

function next_sys ()
   misn.markerMove (marker, mysys[cursys+1])
   osd_msg[1] = osd_msg1_r:format( mysys[cursys+1]:name() )
   misn.osdCreate( osd_title, osd_msg )
   increment = true
end

function player_flee ()
   tk.msg( flee_title, flee_text:format( name, system.cur():name() ) )
   stage = 0
   misn.osdActive( 1 )

   hook.rm(death_hook)
   hook.rm(pir_jump_hook)
   hook.rm(pir_land_hook)
   hook.rm(jumpout)
end

function target_flee ()
   -- Target ran ayay. Unfortunately, we cannot continue the mission
   -- on the other side because the system has not been claimed...
   tk.msg( Tflee_title, Tflee_text:format( name ) )
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
