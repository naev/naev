--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 3">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 2</done>
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
local minerva = require "common.minerva"
local portrait = require 'portrait'
local vn = require 'vn'
local love_shaders = require "love_shaders"
local fmt = require "format"
local lmisn = require "lmisn"

local reward_amount = minerva.rewards.pirate3

local harper_image = portrait.getFullPath( portrait.get() ) -- TODO replace?
local harper_bribe_big = 1e6
local harper_bribe_sml = 1e5
local harper_bribe_tkn = 1000

local spa_portrait = minerva.terminal.portrait
local spa_description = _("Present your winning ticket at the terminal to enter the Minerva Station Spa.")

local mainsys = system.get("Limbo")
-- Mission states:
--  nil: mission not accepted yet
--    0: try to find out how to plant listening device
--    1. hear about chicken spa event
--    2. enter chicken spa event
--    3. obtain spa event ticket from harper
--    4. do chicken spa event
--    5. planted listening device
-- These two will be done in another mission
--    6. kidnap spy and take to torture ship
--    7. defend torture ship
mem.misn_state = nil
local harper -- Non-persistent state

function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   misn.setDesc( _("Someone wants you to find a Dvaered spy that appears to be located at Minerva Station.") )
   misn.setReward( _("Cold hard credits") )
   misn.setTitle( _("Finding the Dvaered Spy") )

   -- Clear variable just in case
   var.pop("harper_ticket")
end


function accept ()
   approach_pir()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   hook.enter("enter")
   hook.load("generate_npc")
   hook.land("generate_npc")
   generate_npc()
end

function generate_npc ()
   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_pir", minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
      if mem.misn_state == 4 then
         mem.npc_spa = misn.npcAdd( "approach_spa", _("Minerva Station Spa"), spa_portrait, spa_description )
      end
   end
end

function approach_spa ()
   hook.safe( "start_spa" )
end
function start_spa ()
   naev.eventStart( "Chicken Rendezvous" )
   if player.evtDone( "Chicken Rendezvous" ) then
      mem.misn_state = 5
      misn.npcRm( mem.npc_spa )
   end
end


function approach_pir ()
   if mem.misn_state==0 and player.evtDone("Spa Propaganda") then
      mem.misn_state = 1
   end

   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.vn_pirate() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if mem.misn_state==nil then
      -- Not accepted
      vn.na(_("You approach the sketchy individual who seems to be calling your attention yet once again."))
      pir(_([["Hello again, we have another job for you. Our previous actions have led us to believe that there are several Dvaered and Za'lek spies deeply infiltrated into the station infrastructure. Would you be up to the challenge of helping us get rid of them?"]]))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function ()
         mem.misn_state=0

         misn.accept()
         misn.osdCreate( _("Minerva Moles"),
               {_("Plant a listening device in a VIP room.") } )

         local minsta = spob.get("Minerva Station")
         misn.markerAdd( minsta )

         minerva.log.pirate(_("You accepted another job from the sketchy individual to uncover moles at Minerva Station.") )
      end )
      pir(_([["Glad to have you onboard again. From the few intercepted Dvaered and Za'lek communications we were able to decode, it seems like we might have some moles at Minerva Station. They are probably really deep so it won't be an easy task to drive them out."]]))
      pir(_([["That's where this comes in to place."
She takes out a metallic object from her pocket and show it to you. You don't know what to make of it.
"Ain't she a beauty?"]]))
      pir(_([["This is some high-tech shit that we got from some geeks. It's a latest gen signal capturing device, and should be able to bypass most jammers. However, we're going to need you to plant it in a VIP room or some place where we might catch the mole."]]))
      pir(_([["The main issue we have right now is that VIP rooms and such are not easy to access, so we're going to have to keep our eyes open and see if we can spot an opportunity to plant the device."]]))
      pir(_([[She hands you the signal capturing device and briefly explains how it works.
"Take the device and see if you can find a chance to place it. I'll be at the spaceport bar if you figure anything out."]]))
   else
      -- Accepted.
      vn.na(_("You approach the sketchy character you have become familiarized with."))
   end

   if mem.misn_state == 5 then
      vn.na(_("You explain to them that you were able to successfully plant the device in the Minerva Spa."))
      pir(_([["Great job! I knew you had it in you. Catching the mole now should be just a matter of time. After we collect some data and process them, we can see how to catch the asshole. Meet me again at the bar in a bit, I have a feeling we will catch them soon."]]))
      vn.na(fmt.reward(reward_amount))
      vn.func( function ()
         player.pay( reward_amount )
         minerva.log.pirate(_("You planted a listening device in the Minerva Spa to catch a mole and were rewarded for your actions.") )
      end )
      vn.sfxVictory()
      vn.done()
      vn.run()

      misn.finish(true)
   end

   vn.label("menu_msg")
   pir(_([["Is there anything you would like to know?"]]))
   vn.menu( function ()
      local opts = {
         {_("Ask about the job"), "job"},
         {_("Ask about Minerva Station"), "station"},
         {_("Leave"), "leave"},
      }
      if mem.misn_state < 3 and player.evtDone("Spa Propaganda") then
         if var.peek("minerva_spa_ticket")==nil then
            table.insert( opts, 1, {_("Ask them about the spa"), "spa" } )
         else
            table.insert( opts, 1, {_("Show them the spa ticket"), "spaticket" } )
         end
      end

      if mem.misn_state == 3 and mem.harper_gotticket then
         table.insert( opts, 1, {_("Show them the winning ticket"), "trueticket" } )
      end
      return opts
   end )

   vn.label("job")
   pir(_([["From the few intercepted Dvaered and Za'lek communications we were able to decode, it seems like we might have some moles at Minerva Station. They are probably really deep so it won't be an easy task to drive them out."]]))
   pir(_([["The high-tech latest gen signal capturing device I gave should be able to bypass most jammers. However, you're going to have to plant it in a VIP room or in some place where we might be able to catch the mole."]]))
   if mem.misn_state==0 then
      pir(_([["The main issue we have right now is that VIP rooms and such are not easy to access, so we're going to have to keep our eyes open and see if we can spot an opportunity to plant the device."]]))
      pir(_([["You never know when we'll get a chance, just take your time doing stuff near Minerva station and I'm sure you'll find an opening."]]))
   elseif mem.misn_state==1 then
      pir(_([["The Spa sounds like a perfect place to set up the signal capturing device. Nobody will suspect a thing! You should buy a ticket to the Spa and see if we can get lucky. If not, we may have to take other measures to ensure success."]]))
   elseif mem.misn_state==3 then
      if not mem.harper_gotticket then
         pir(_([["I can't believe we didn't win a ticket to the Spa. However, it seems like this guy called Harper Bowdown managed to get it instead."]]))
         pir(_([["I need you to go pay this guy a visit. See if you can 'encourage' them to give the ticket to you. Everyone has a price at Minerva Station."]]))
      else
         vn.jump("trueticket")
      end
   elseif mem.misn_state==4 then
      pir(_([["You got the ticket to the Minerva Spa, so all you have to do now is go in, plant the listening device, and enjoy the thermal waters."]]))
   end
   vn.jump("menu_msg")

   vn.label("station")
   pir(_([["Isn't this place great? They managed to set up an incredibly successful business model here. The way the Empire basically turns a blind eye to everything that goes on here is just incredible! Makes you want to root for their success you know?"]]))
   pir(_([["The issue is that even though the set-up is great, all the Dvaered and Za'lek bickering is just messing it all up. It would be a real shame if things went tits up and either the Dvaered or Za'lek were able to take over this wonderful place."]]))
   pir(_([["So me and my investors thought to ourselves, what could we do to ensure the success of such a wonderful place? This led to that and here we are."]]))
   vn.jump("menu_msg")

   vn.label("spa")
   pir(_([["Ah, so you heard the news too? The Spa sounds like a perfect place to set up the signal capturing device. Nobody will suspect a thing! You should buy a ticket and see if we can get lucky. If not, we may have to take other measures to ensure success."
She winks at you.]]))
   vn.jump("menu_msg")

   vn.label("spaticket")
   pir(_("Our chances of getting into the Spa can't be that bad, can they?"))
   local t = minerva.vn_terminal()
   vn.appear( t )
   t(_([[Suddenly, the terminals around you blast a fanfare and begin to talk on the loudspeakers.
"WE ARE HAPPY TO ANNOUNCE THE WINNER OF THE FABULOUS SPA WITH CYBORG CHICKEN EVENT!"
The terminal pauses for emphasis.]]))
   t(_([["THE WINNER IS HARPER BOWDOIN! PLEASE COME TO YOUR NEAREST TERMINAL TO COLLECT YOUR PRIZE."]]))
   vn.disappear( t )
   pir(_([["Shit! I thought we had that rigged. Damn it. Give me one second."
She starts frantically typing into her portable holo-deck. It makes weird beeps and noises.]]))
   pir(_([["OK, so we aren't so bad off. It seems like the winner was doing some space tourism around the system. Not like there is anything to see here."]]))
   pir(_([["So change of plans, I need you to go pay this guy a visit. See if you can 'encourage' them to give the ticket to you. Everyone has a price at Minerva Station."]]))
   vn.func( function ()
      misn.osdCreate( _("Minerva Moles"),
         {_("Get Harper Bowdoin's ticket in Limbo.")},
         {_("Plant a listening device in a VIP room.") } )
      mem.misn_state = 3
      minerva.log.pirate(_("You did not obtain the winning ticket of the Minerva Spa event and were tasked with obtaining it from a so called Harper Bowdoin.") )
   end )
   vn.jump("menu_msg")

   vn.label("trueticket")
   vn.na(_("You show them the winning ticket you took from Harper Bowdoin."))
   pir(_([["Great job out there. It's like taking candy from a baby."
She beams you a smile.
"Now go enjoy yourself at the Spa and don't forget to plant the listening device!"]]))
   vn.func( function ()
      mem.misn_state = 4
      misn.osdCreate( _("Minerva Moles"),
         {_("Plant a listening device in the Spa.") } )
      mem.npc_spa = misn.npcAdd( "approach_spa", _("Minerva Station Spa"), spa_portrait, spa_description )
      minerva.log.pirate(_("You obtained the winning ticket to enter the Minerva Spa.") )
   end )
   vn.jump("menu_msg")

   vn.label("leave")
   if mem.misn_state==0 then
      vn.na(_("You take your leave and ponder where you should start looking. The casino seems to be your best bet."))
   else
      vn.na(_("You take your leave."))
   end
   vn.run()
end


function enter ()
   if mem.misn_state==3 and not mem.harper_nospawn and system.cur()==system.get("Limbo") then
      -- Don't stop spawns, but claimed in case something else stops spawns
      -- TODO maybe add Minerva patrols that aggro ta make it a bit harder?
      -- Spawn Harper Bowdoin and stuff
      local pos = spob.get("Minerva Station"):pos() + vec2.newP( 5000, rnd.angle() )

      local fharper = faction.dynAdd( nil, "Harper Bowdoin" )
      harper = pilot.add( "Quicksilver", fharper, pos, _("Harper"), {ai="civilian"} )
      local aimem = harper:memory()
      aimem.loiter = math.huge -- Should make them loiter forever
      aimem.distress = false -- Don't distress or everyone aggros
      harper:setHilight(true)
      harper:setVisplayer(true)
      harper:setNoLand(true)
      harper:setNoJump(true)
      hook.pilot( harper, "attacked", "harper_gotattacked" )
      hook.pilot( harper, "death", "harper_death" )
      hook.pilot( harper, "board", "harper_board" )
      hook.pilot( harper, "hail", "harper_hail" )
      hook.pilot( harper, "land", "harper_land" )

      -- Clear variables to be kind to the player
      mem.harper_almostdied = false
      mem.harper_talked = false
      mem.harper_attacked = false
   end
end


function harper_death ()
   if not mem.harper_gotticket then
      lmisn.fail(_("You were supposed to get the ticket, but you blew up Harper's ship!"))
   end
end


function harper_land ()
   -- Case harper lands with the ticket, i.e., ran away
   if not mem.harper_gotticket then
      lmisn.fail(_("You were supposed to get the ticket but Harper got away!"))
   end
end


function harper_board ()
   if not mem.harper_gotticket then
      mem.harper_gotticket = true
      mem.harper_nospawn = true
      var.push("harper_ticket","stole")

      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_("You violently enter Harper's ship and plunder the ticket which you were after, while Harper spends his time cowering in the corner of the cockpit."))
      vn.run()
   end
end


local function harper_neardeath()
   if not harper:exists() then
      return false
   end
   local a, s, stress, disabled = harper:health()
   if disabled then
      return true
   end
   return (a+s-stress < 80)
end


function harper_gotattacked( _plt, attacker )
   if attacker and attacker:withPlayer() then
      if not mem.harper_attacked then
         -- Run to land at the station
         harper:setNoLand(false)
         harper:control()
         harper:land( spob.get("Minerva Station") )
         mem.harper_attacked = true
      end

      if harper_neardeath() then
         mem.harper_almostdied = true
         harper:comm( _("Stop shooting! Maybe we can reach an agreement!") )
         harper:hailPlayer()
      end
   end
end


function harper_hail ()

   local function harper_hologram ()
      return vn.newCharacter( _("Harper"),
            { image=harper_image, shader=love_shaders.hologram() } )
   end

   local function payhim( amount )
      if player.credits() < amount then
         return fmt.f(_("Pay him {credits} (#rnot enough!#0)"), {credits=fmt.credits(amount)})
      else
         return fmt.f(_("Pay him {credits}"), {credits=fmt.credits(amount)})
      end
   end

   local function offer( amount )
      if player.credits() < amount then
         return fmt.f(_([[Offer {credits} (#rnot enough!#0)]]), {credits=fmt.credits(amount)})
      else
         return fmt.f(_([[Offer {credits}]]), {credits=fmt.credits(amount)})
      end
   end

   local function _harper_done ()
      mem.harper_gotticket = true
      mem.harper_nospawn = true
      -- He goes land now
      harper:setNoLand(false)
      harper:control()
      harper:land( spob.get("Minerva Station") )
   end

   if mem.harper_almostdied then
      vn.clear()
      vn.scene()
      local h = harper_hologram()
      vn.transition("electric")
      vn.na(_("You see Harper's hologram appear into view. He looks paler than usual."))
      h(fmt.f(_([["I have had a change of mind."
He gulps.
"How about I give you the ticket for a mere {credits}?]]), {credits=fmt.credits(harper_bribe_sml)}))
      vn.menu( {
         { payhim(harper_bribe_sml), "pay" },
         { _("Threaten him"), "threaten" },
      } )

      vn.label("pay")
      vn.func( function ()
         if player.credits() < harper_bribe_sml then
            vn.jump("broke")
         end
      end )
      h(_([["Actually, swimming with a chicken sounds like a bit of a drag. I think I might be allergic to them anyway."
He coughs nervously.]]))
      vn.sfxMoney()
      vn.func( function ()
         _harper_done()
         player.pay( -harper_bribe_sml )
         harper:credits( harper_bribe_sml )
         var.push("harper_ticket","credits")
      end )
      vn.na(_("You wire him the money and he gives you the digital code that represents the ticket. Looks like you are set."))
      vn.done("electric")

      vn.label("threaten")
      h(_([[He laughs nervously.
"You have to be kidding right? You wouldn't kill me in cold blood would you? I have a family waiting for me back home.]]))
      vn.menu( {
         { payhim(harper_bribe_sml), "pay" },
         { _("Aim your weapons at him"), "threaten2" },
      } )

      vn.label("threaten2")
      h(_([["I have a daughter! She is turning 5 soon. You wouldn't be so cold-hearted to leave her an orphan would you?"
He is sweating profusely.]]))
      vn.menu( {
         { payhim(harper_bribe_sml), "pay" },
         { _("Prime your weapon systems"), "threaten3" },
      } )

      vn.label("threaten3")
      h(_([["Ah, damn this! I've still got things worth living for. Here, just take the damn thing."]]))
      vn.na(_("He gives you the digital code that represents the ticket. Looks like you are set."))
      vn.func( function ()
         _harper_done()
         var.push("harper_ticket","free")
      end )
      vn.done("electric")

      vn.label("broke")
      h(_([["Wait do you mean you don't have the money?"
He looks around nervously.
"Ah, damn this. I've still got things worth living for. Here, just take the damn thing."]]))
      vn.na(_("He gives you the digital code that represents the ticket. Looks like you are set."))
      vn.func( function ()
         _harper_done()
         var.push("harper_ticket","free")
      end )
      vn.done("electric")
      vn.run()

      player.commClose()
      return
   end

   if mem.harper_attacked then
      harper:comm( _("Get away from me!") )
      player.commClose()
      return
   end

   if mem.harper_talked then
      vn.clear()
      vn.scene()
      local h = harper_hologram()
      vn.transition("electric")
      vn.na(_("An impatient man appears into view."))
      h(_([["You again? I don't have time to deal with you."]]))
      vn.na(_("The comm goes silent…"))
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
He looks triumphant as he boasts.]]))
   vn.menu( {
      {_([["About the ticket…"]]), "ticket" },
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
      mem.harper_attacked = true
   end )
   vn.done("electric")

   vn.label("haggle")
   h(_([["I don't know. This is very valuable you know? What would you trade for it?"]]))
   vn.menu( function ()
      local opts = {
         { offer(harper_bribe_big), "money_big" },
         { offer(harper_bribe_sml), "money_sml" },
         {_("End transmission"), "leave" },
      }
      if minerva.tokens_get() > harper_bribe_tkn then
         table.insert( opts, 1,
            { fmt.f(_([[Offer {bribe} (have {tokens})]]), {
               bribe = minerva.tokens_str( harper_bribe_tkn ),
               tokens = minerva.tokens_str( minerva.tokens_get() ) }),
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
   h(_([["Actually, swimming with a chicken sounds like a bit of a drag. I think I might be allergic to them anyway."]]))
   vn.sfxMoney()
   vn.func( function ()
      _harper_done()
      player.pay( -harper_bribe_big )
      harper:credits( harper_bribe_big ) -- Player can theoretically board to loot it back
      var.push("harper_ticket","credits")
   end )
   vn.na(_("You wire him the money and he gives you the digital code that represents the ticket. Looks like you are set."))
   vn.jump("leave")

   vn.label("money_tkn")
   h(_([["Actually, swimming with a chicken sounds like a bit of a drag. I think I might be allergic to them anyway."]]))
   vn.sfxMoney()
   vn.func( function ()
      _harper_done()
      minerva.tokens_pay( -harper_bribe_tkn )
      var.push("harper_ticket","tokens")
   end )
   vn.na(_("You wire him the tokens and he gives you the digital code that represents the ticket. Looks like you are set."))

   vn.label("money_sml")
   vn.func( function ()
      if player.credits() < harper_bribe_sml then
         vn.jump("broke")
      end
   end )
   h(fmt.f(_([["{credits} only? I spent more than that gambling just to get this ticket."]]), {credits=fmt.credits(harper_bribe_sml)}))
   vn.menu( {
      { offer(harper_bribe_big), "money_big" },
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

   mem.harper_talked = true

   player.commClose()
end
