--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Minerva Station</planet>
  <done>Minerva Pirates 2</done>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
  <provides name="Spa Propaganda" />
  <provides name="Chicken Rendezvous">Kidnapped Winner</provides>
 </notes>
</mission>
--]]

--[[
-- Find the Dvaered spy
--]]
local minerva = require "minerva"
local portrait = require 'portrait'
local vn = require 'vn'
require 'numstring'

logidstr = minerva.log.pirate.idstr

misn_title = _("Finding the Dvaered Spy")
misn_reward = _("Cold hard credits")
misn_desc = _("Someone wants you to find a Dvaered spy that appears to be located at Minerva Station.")
reward_amount = 200000 -- 200k

harper_portrait = portrait.get() -- TODO replace?
harper_bribe_big = 1e7
harper_bribe_sml = 1e6
harper_bribe_tkn = 1000

spa_name = _("Minerva Station Spa")
spa_portrait = minerva.terminal.portrait
spa_description = _("Present your winning ticket at the terminal to enter the Minerva Station Spa.")

mainsys = "Limbo"
-- Mission states:
--  nil: mission not accepted yet
--    0: try to find out how to plant listening device
--    1. hear about chicken spa event
--    2. enter chicken spa event
--    3. obtain spa event ticket from harper
--    4. do chicken spa event
--    5. planted listening device
--    6. kidnap spy and take to torture ship
--    7. defend torture ship
misn_state = nil


function create ()
   misn.finish( false ) -- Disabled for now
   if not misn.claim( system.get(mainsys) ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait )
   misn.setDesc( minerva.pirate.description )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end


function accept ()
   approach_pir()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      misn.finish(false)
      return
   end

   misn.accept()
   misn.setDesc( misn_desc )
   osd = misn.osdCreate( _("Minerva Moles"),
         {_("Plant a listening device in a VIP room.") } )

   shiplog.appendLog( logidstr, _("You accepted another job from the shady individual to uncover moles at Minerva Station.") )

   hook.enter("enter")
   hook.load("generate_npc")
   hook.land("generate_npc")
   generate_npc()
end


function generate_npc ()
   if planet.cur() == planet.get("Minerva Station") then
      npc_pir = misn.npcAdd( "approach_pir", minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
      if misn_state == 4 then
         npc_spa = misn.npcAdd( "approach_spa", spa_name, spa_portrait, spa_description )
      end
   end
end

function approach_spa ()
   hook.safe( "start_spa" )
end
function start_spa ()
   naev.eventStart( "Chicken Rendezvous" )
   if player.evtDone( "Chicken Rendezvous" ) then
      misn.npcRm( npc_spa )
      misn_state = 5
   end
end


function approach_pir ()
   if misn_state==0 and player.evtDone("Spa Propaganda") then
      misn_state = 1
   end

   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.pirate.name, {image=minerva.pirate.image} )
   vn.transition()

   if misn_state==nil then
      -- Not accepted
      vn.na(_("You approach the sketch individual who seems to be calling your attention yet once again."))
      pir(_("Hello again, we have another job for you. Our previous actions has led us to believe that there are several Dvaered and Za'lek spies deeply infiltrated into the station infrastructure. Would you be up to the challenge of helping us get rid of them?"))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () misn_state=0 end )
      pir(_([["Glad to have you onboard again. From the few intercepted Dvaered and Za'lek communications we were able to decode, it seems like we might have some moles at Minerva Station. They are probably really deep so it won't be an easy task to drive them out."]]))
      pir(_([["That's where this comes in to place."
They take out a metallic object from their pocket and show it to you. You don't know what to make of it.
"Ain't she a beauty?"]]))
      pir(_([["This is some high tech shit that we got from some geeks. It is a latest gen signal capturing device. It should be able to bypass most of the jamming, however, we're going to need you to plant it in a VIP room or some place where we might catch the mole."]]))
      pir(_([["The main issue we have now is that VIP rooms and such are not of easy access, so we're going to have to keep our eyes open and see if we can spot an opportunity."]]))
      pir(_([[They hand you the signal capturing device and explain briefly how it works.
"Take the device and see if you can find an opportunity to place it. I'll be at the spaceport bar if you figure out anything."]]))
   else
      -- Accepted.
      vn.na(_("You approach the shady character you have become familiarized with."))
   end

   if misn_state == 5 then
      vn.na(_("You explain to them that you were able to successfully plant the device in the Minerva Spa."))
      pir(_([["Great job! I knew you had it in you. Catching the mole now should be just a matter of time. After we collect some data and process them, we can see how to catch the asshole. Meet me again at the bar in a bit, I have a feeling we will catch them soon."]]))
      vn.na(string.format(_("You have received #g%s."), creditstring(reward_amount)))
      vn.func( function ()
         player.pay( reward_amount )
         shiplog.appendLog( logidstr, _("You planted a listening device in the Minerva Spa to catch a mole and were rewarded for your actions.") )
      end )
      vn.sfxVictory()
      vn.done()
   end

   vn.label("menu_msg")
   pir(_([["Is there anything you would like to know?"]]))
   vn.menu( function ()
      local opts = {
         {_("Ask about the job"), "job"},
         {_("Ask about Minerva Station"), "station"},
         {_("Leave"), "leave"},
      }
      if misn_state < 3 and player.evtDone("Spa Propaganda") then
         if var.peek("minerva_spa_ticket")==nil then
            table.insert( opts, 1, {_("Ask them about the Spa"), "spa" } ) 
         else
            table.insert( opts, 1, {_("Show them the Spa Ticket"), "spaticket" } ) 
         end
      end

      if misn_state == 3 and harper_gotticket then
         table.insert( opts, 1, {_("Show them the winning Ticket"), "trueticket" } )
      end
      return opts
   end )

   vn.label("job")
   pir(_([["From intercepted Dvaered and Za'lek communication it seems like "]]))
   vn.jump("menu_msg")

   vn.label("station")
   pir(_([["Isn't this place great? They managed to set up an incredibly successful business model here. The way the Empire basically turns an eye to everything that goes on here is just incurable. Makes you want to root for their success you no?"]]))
   pir(_([["The issue is that even though the set-up is great, all the Dvaered and Za'lek bickering is just messing it all up. It would be a real shame if things went tits up and either the Dvaered or Za'lek were able to take over this wonderful place."]]))
   pir(_([["So me and my investors thought to ourselves, what could we do to ensure the success of such a wonderful place? This led to that and here we are."]]))
   vn.jump("menu_msg")

   vn.label("spa")
   pir(_([["Ah, so you heard the news too? The spa sounds like a perfect place to set up the signal capturing device. Nobody will suspect a thing! You should buy a ticket and see if we can get lucky. If  not, we may have to take other measures to ensure success."
They wink at you.]]))
   vn.jump("menu_msg")

   vn.label("spaticket")
   pir(_("Our chances of getting into the Spa can't be that bad can they?"))
   local t = minerva.vn_terminal()
   vn.appear( t )
   t(_([[Suddenly, the terminals around you blast a fanfare and begin to talk on the loudspeakers.
"WE ARE HAPPY TO ANNOUNCE THE WINNER OF THE FABULOUS SPA WITH CYBORG CHICKEN EVENT!"
The terminal pauses for emphasis.]]))
   t(_([["THE WINNER IS HARPER BOWDOIN! PLEASE COME TO YOUR NEAREST TERMINAL TO COLLECT YOUR PRIZE."]]))
   vn.disappear( t )
   pir(_([["Shit! I thought we had that rigged. Damn it. Give me one second."
They start frantically typing into their portable holo-deck. It makes weird beeps and noises.]]))
   pir(_([["OK, so we aren't so bad off. It seems like the winner was doing some space tourism around the system. Not like there is anything to see here."]]))
   pir(_([["So change of plans, I need you to go pay this guy a visit, see if you can 'encourage' them to give the ticket to you. Everyone has a price at Minerva Station."]]))
   vn.func( function ()
      osd = misn.osdCreate( _("Minerva Moles"),
         {_("Get Harper Bowdoin's ticket in Limbo.")},
         {_("Plant a listening device in a VIP room.") } )
      misn_state = 3
      shiplog.appendLog( logidstr, _("You obtained the winning ticket to enter the Minerva Spa.") )
   end )
   vn.jump("menu_msg")

   vn.label("trueticket")
   vn.na(_("You show them the winning ticket you took from Harper Bowdoin."))
   pir(_([["Great job out there. It's like taking candy from a baby."
She beams you a smile.
"Now go enjoy yourself at the spa and don't forget to plant the listening device!"]]))
   vn.func( function ()
      misn_state = 4
      misn.osdActive(2)
      npc_spa = misn.npcAdd( "approach_spa", spa_name, spa_portrait, spa_description )
   end )
   vn.jump("menu_msg")

   vn.label("leave")
   if misn_state==0 then
      vn.na(_("You take your leave and ponder where you should start looking. The casino seems to be your best bet."))
   else
      vn.na(_("You take your leave."))
   end
   vn.run()

   misn.finish(true)
end


function enter ()
   if misn_state==3 and not harper_nospawn and system.cur()==system.get("Limbo") then
      -- Don't stop spawns
      -- TODO maybe add Minerva patrols that aggro ta make it a bit harder?
      -- Spawn Harper Bowdoin and stuff
      local pos = planet.get("Minerva Station"):pos() + vec2.newP( 5000, rnd.rnd(360) )

      local fharper = faction.dynAdd( nil, "Harper Bowdoin" )
      harper = pilot.add( "Gawain", fharper, pos, "Harper", "civilian" )
      local mem = harper:memory()
      mem.loiter = math.huge -- Should make them loiter forever
      harper:setHilight(true)
      harper:setNoLand(true)
      harper:setNoJump(true)
      hook.pilot( harper, "attacked", "harper_attacked" )
      hook.pilot( harper, "death", "harper_death" )
      hook.pilot( harper, "board", "harper_board" )
      hook.pilot( harper, "hail", "harper_hail" )
      hook.pilot( harper, "land", "harper_land" )

      -- Clear variables to be kind to the player
      harper_almostdied = false
      harper_talked = false
   end
end


function harper_death ()
   if not harper_gotticket then
      player.msg(_("#rMISSION FAILED! You were supposed to get the ticket, but you blew up Harper's ship!"))
      misn.finish(false)
   end
end


function harper_land ()
   -- Case harper lands with the ticket, i.e., ran away
   if not harper_gotticket then
      player.msg(_("#rMISSION FAILED! You were supposed to get the ticket but they got away!"))
      misn.finish(false)
   end
end


function harper_board ()
   if not harper_gotticket then
      harper_gotticket = true
      harper_nospawn = true

      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_("You violently enter Harper's ship and plunder the ticket which you were after, while Harper spends his time cowering in the corner of the cockpit."))
      vn.run()
   end
end


function harper_neardeath()
   if not harper:exists() then
      return false
   end
   local a, s, stress, disabled = harper:getHeight()
   if disabled then
      return true
   end
   return (a+s < 0.4)
end


function harper_attacked( pilot, attacker )
   if attacker == pilot.player() then
      if not harper_attacked then
         -- Run to land at the station
         harper:setNoLand(false)
         harper:control()
         harper:land( planet.get("Minerva Station") )
         harper_attacked = true
      end

      if harper_neardeath() then
         harper_almostdied = true

      end
   end
end


function harper_hail ()
   if harper_attacked then
      harper:msg( player.pilot(), _("Get away from me!") )
      player.commClose()
      return
   end

   local function harper_hologram ()
      return vn.newCharacter( _("Harper"),
            { image=portrait.hologram( harper_portrait ) } )
   end

   local function enoughcreds( amount )
      if player.credits() < amount then
         return " (#rnot enough!#0)"
      end
      return ""
   end

   local function _harper_done ()
      harper_gotticket = true
      harper_nospawn = true
      -- He goes land now
      harper:setNoLand(false)
      harper:control()
      harper:land( planet.get("Minerva Station") )
   end

   if harper_almostdied then
      vn.clear()
      vn.scene()
      local h = harper_hologram()
      vn.transition("electric")
      vn.na(_("You see Harper's hologram appear into view paler than usual."))
      h(string.format(_([["I have had a change of mind."
He gulps.
"How about I give you the ticket for a mere %s?]]), creditstring(harper_bribe_sml)))
      vn.menu( {
         { string.format(_("Pay him %s%s"),creditstring(harper_bribe_sml),enoughcreds(harper_bribe_sml)), "pay" },
         { _("Threaten him"), "threaten" },
      } )

      vn.label("pay")
      vn.func( function ()
         if player.credits() < harper_bribe_sml then
            vn.jump("broke")
         end
      end )
      h(_([["Actually, swimming in a chicken sounds like a bit of a drag. I think I might be allergic them anyway."
He coughs nervously.]]))
      vn.sfxMoney()
      vn.func( function ()
         _harper_done()
         player.pay( -harper_bribe_sml )
         harper:credits( harper_bribe_sml )
      end )
      vn.na(_("You wire him the money and he gives you the digital code that represents the ticket. Looks like you are set."))
      vn.done("electric")

      vn.label("threaten")
      h(_([[He laughs nervously.
"You have to be killing right? You wouldn't kill me in cold blood would you? I have a family waiting for me back home.]]))
      vn.menu( {
         { string.format(_("Pay him %s%s"),creditstring(harper_bribe_sml),enoughcreds(harper_bribe_sml)), "pay" },
         { _("Aim your weapons at him"), "threaten2" },
      } )

      vn.label("threaten2")
      h(_([["I have a daughter! She is turning 5 soon. You wouldn't be so cold-hearted to leave her an orphan would you?"
He is sweating profusely.]]))
      vn.menu( {
         { string.format(_("Pay him %s%s"),creditstring(harper_bribe_sml),enoughcreds(harper_bribe_sml)), "pay" },
         { _("Prime your weapon systems"), "threaten3" },
      } )

      vn.label("threaten3")
      h(_([["Ah, damn this! I've still got things worth living for. Here, just take the damn thing."]]))
      vn.na(_("He gives you the digital code that represents the ticket. Looks like you are set."))
      vn.func( function () _harper_done() end )
      vn.done("electric")

      vn.label("broke")
      h(_([["Wait do you mean you don't have the money?"
He looks around nervously.
"Ah, damn this. I've still got things worth living for. Here, just take the damn thing."]]))
      vn.na(_("He gives you the digital code that represents the ticket. Looks like you are set."))
      vn.func( function () _harper_done() end )
      vn.done("electric")
      vn.run()

      player.commClose()
      return
   end

   if harper_talked then
      vn.clear()
      vn.scene()
      local h = harper_hologram()
      vn.transition("electric")
      vn.na(_("An impatient man appears into view."))
      h(_([["You again? I don't have time to deal with you."]]))
      vn.na(_("The comm goes silent..."))
      vn.done("electric")
      vn.run()

      player.commClose()
      return
   end

   vn.clear()
   vn.scene()
   local h = harper_hologram()
   vn.transition("electric")
   vn.na(_("You hail Harper's vessel and you see him appear into view."))
   h(_([["What do you want? Can't you see I'm celebrating my luck?"]]))
   vn.menu( {
      {_([["Luck?"]]), "luck" },
      {_([["Celebrating?"]]), "luck" },
   } )
   vn.label("luck")
   h(_([["Yeah, you may have heard the news, but I'm the big winner of the Minerva hot spring event! On my first time to Minerva Station too! Maybe this will finally cure my back pains once and for all. The ticket is even made of pure gold!"
As he boasts, he looks triumphant.]]))
   vn.menu( {
      {_([["About the ticket..."]]), "ticket" },
      {_("End transmission"), "leave" },
   } )
   vn.label("ticket")
   h(_([["What about it?"]]))
   vn.menu( {
      {_([["Give it to me. Now."]]), "threaten" },
      {_([["I would like to take it off your hands."]]), "haggle"},
      {_("End transmission"), "leave" },
   } )

   vn.label("threaten")
   h(_([["What are you going to do? Shoot me?"
He scoffs at you and closes the transmission.]]))
   vn.func( function ()
      harper_attacked = true
   end )
   vn.done("electric")

   vn.label("haggle")
   h(_([["I don't know. This is very valuable you know? What would you trade for it?"]]))
   vn.menu( function ()
      local opts = {
         { string.format(_([[Offer %s%s]]), creditstring(harper_bribe_big),
            enoughcreds(harper_bribe_big)), "money_big" },
         { string.format(_([[Offer %s%s]]), creditstring(harper_bribe_sml),
            enoughcreds(harper_bribe_sml)), "money_sml" },
         {_("End transmission"), "leave" },
      }
      if minerva.tokens_get() > harper_bribe_tkn then
         table.insert( opts, 1,
            { string.format(_([[Offer %s (have %s)]]),
               minerva.tokens_str( harper_bribe_tkn ),
               minerva.tokens_str( minerva.tokens_get() ) ),
            "money_tkn" } )
      end
      return opts
   end )

   vn.label("money_big")
   vn.func( function ()
      if player.credits() < harper_bribe_big then
         vn.jump("broke")
      end
   end )
   h(_([["Actually, swimming in a chicken sounds like a bit of a drag. I think I might be allergic them anyway."]]))
   vn.sfxMoney()
   vn.func( function ()
      _harper_done()
      player.pay( -harper_bribe_big )
      harper:credits( harper_bribe_big ) -- Player can theoretically board to loot it back
   end )
   vn.na(_("You wire him the money and he gives you the digital code that represents the ticket. Looks like you are set."))
   vn.jump("leave")

   vn.label("money_tkn")
   h(_([["Actually, swimming in a chicken sounds like a bit of a drag. I think I might be allergic them anyway."]]))
   vn.sfxMoney()
   vn.func( function ()
      _harper_done()
      minerva.tokens_pay( -harper_bribe_tkn )
   end )
   vn.na(_("You wire him the tokens and he gives you the digital code that represents the ticket. Looks like you are set."))

   vn.label("money_small")
   vn.func( function ()
      if player.credits() < harper_bribe_sml then
         vn.jump("broke")
      end
   end )
   h(string.format(_([["%s only? I spent more than that gambling just to get this ticket."]]), creditstring(harper_bribe_sml)))
   vn.menu( {
      { string.format(_([[Offer %s%s]]), creditstring(harper_bribe_big),
         enoughcreds(harper_bribe_big)), "money_big" },
      {_("End transmission"), "leave" },
   } )

   vn.label("broke")
   h(_([["You trying to pull my leg or something? You don't have enough credits. Anyway, I'm busy now."
He cuts the transmission.]]))
   vn.done("electric")

   vn.label("leave")
   vn.na(_("You close the transmission channel."))
   vn.done("electric")
   vn.run()

   harper_talked = true

   player.commClose()
end


