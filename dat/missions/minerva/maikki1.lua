--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Maikki's Father 1">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>100</chance>
   <location>Bar</location>
   <planet>Minerva Station</planet>
   <cond>var.peek("minerva_altercation_probability")~=nil</cond>
  </avail>
 </mission>
--]]

--[[
-- Maikki (Maisie McPherson) asks you to find her father, the famous pilot Kex
-- McPherson. She heard rumours he was still alive and at Minerva station.
--
-- 1. Player is sent to Doeston to try to find the whereabouts.
-- 2. Guys in the bar talk about how he never came back from the Nebula,
-- 3. Player sent to explore Arandon, finds some scavengers, they run away.
-- 4. Player goes back to bar, comment how some have been trying to sell some ship scrap, apparently from Zerantix. Player told to follow scavengers.
-- 5. Scavengers show up and talk about sensors not working well, talk through broadcast. Player has to follow without getting too close (maybe needs outfits to improve sensor range in nebula?)
-- 6. They lead the player to some debris and start looting. Player gets to talk to them and convince to leave, or kill them. Learns that Za'lek have been buying the goods.
-- 7. Player finds a picture among the debris.
-- 8. Return to Maikki
--
-- Some dates for lore purposes:
--  591ish - maikki is born
--  593:3726.4663 - the incident
--  596ish - Kex disappears (maikki is 8ish)
--  598ish - Kex is found
--  603ish - game start (~15 years after incident, maikki is 18ish)
--]]
local minerva = require "minerva"
local portrait = require 'portrait'
local vn = require 'vn'
require 'numstring'

maikki_name = _("Distraught Young Woman")
maikki_description = _("You see a small young woman sitting by herself. She has a worried expression on her face.")
maikki_portrait = minerva.maikki.portrait
maikki_image = minerva.maikki.image
maikki_colour = minerva.maikki.colour

oldman_name = _("Old Man")
oldman_portrait = "old_man.png"
oldman_description = _("You see a nonchalant old man sipping on his drink with a carefree aura.")
oldman_image = "old_man.png"

scav_name = _("Scavengers")
scav_portrait = "scavenger1.png"
scav_desc = _("You see a pair of dirty looking fellows talking loudly among themselves.")
scavengera_image = "scavenger1.png"
scavengerb_image = scavengera_image
scavengera_portrait = "scavenger1.png"
scavengerb_portrait = scavengera_portrait
scavengera_colour = nil
scavengerb_colour = nil

misn_title = _("Finding Father")
misn_reward = _("???")
misn_desc = _("Maikki wants you to help her find her father.")

mainsys = "Limbo"
searchsys = "Doeston"
cutscenesys = "Arandon"
stealthsys = "Zerantix"
-- Mission states:
--  nil: mission not accepted yet
--   -1: mission started, have to talk to maikki
--    0: going to doeston
--    1: talked to old man, going to arandon
--    2: saw scavengers, go back to doeston
--    3: talked to old man again
--    4: talk to scavengers, going to zerantix
--    5: looted ship
misn_state = nil


function create ()
   if not misn.claim( {system.get(cutscenesys), system.get(stealthsys)} ) then
      misn.finish( false )
   end
   misn.setNPC( maikki_name, maikki_portrait )
   misn.setDesc( maikki_description )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end


function accept ()
   approach_maikki()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      misn.finish(false)
      return
   end

   hook.land("generate_npc")
   hook.load("generate_npc")
   hook.enter("enter")

   -- Re-add Maikki if accepted
   generate_npc()
end


function generate_npc ()
   if planet.cur() == planet.get("Cerberus") then
      npc_oldman = misn.npcAdd( "approach_oldman", oldman_name, oldman_portrait, oldman_description )
      if misn_state==3 or misn_state==4 or bribed_scavengers==true then
         npc_scavenger = misn.npcAdd( "approach_scavengers", scav_name, scav_portrait, scav_desc )
      end
   elseif planet.cur() == planet.get("Minerva Station") then
      npc_maikki = misn.npcAdd( "approach_maikki", minerva.maikki.name, minerva.maikki.portrait, minerva.maikki.description )
   end
end


function approach_maikki ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikki() )
   if misn_state==nil then
      maikki:rename( maikki_name )
   end
   vn.fadein()

   if misn_state==nil then
      -- Start mission
      vn.na(_("You approach a young woman who seems somewhat distraught. It looks like she has something important on her mind."))
      maikki(_([["You wouldn't happen to be from around here? I'm looking for someone. I was told they would be here, but I never expected this place to be so..."
She trails off.]]) )
      vn.menu( {
         { _([["spacious?"]]), "menu1done" },
         { _([["grubby?"]]), "menu1done" },
      } )
      vn.label( "menu1done" )
      maikki(_([[She frowns a bit.
"...unsubstantial. Furthermore, it is all so tacky! I thought such a famous gambling world would be much more cute!"]]))
      maikki(_([[She suddenly remembers why she came here and your eyes light up.
"You wouldn't happen to be familiar with the station? I'm looking for someone"]]))
      vn.menu( {
         { _("Offer to help her"), "accept" },
         { _("Decline to help"), "decline" },
      } )
      vn.label( "decline" )
      vn.na(_("You feel it is best to leave her alone for now and disappear into the crowds leaving her once again alone to her worries."))
      vn.done()

      vn.label( "accept" )
      vn.func( function ()
         misn.accept()
         misn_state = -1
      end )
      maikki(_([["I was told he would be here, but I've been here for ages and haven't gotten anywhere."
She gives out a heavy sigh.]]))
      maikki:rename( minerva.maikki.name )
      maikki(_([["My name is Maisie, but you can call me Maikki."]]))
      vn.menu( {
         { _("Offer her a drink (#p10 Minerva Tokens#0)"), "drink" },
         { _("Ask her who she is looking for"), "nodrink" },
      } )
      vn.label( "drink" )
      vn.func( function ()
         if minerva.tokens_get() < 10 then
            vn.jump( "notenough" )
         else
            minerva.tokens_pay( -10 )
            minerva.maikki_mood_mod( 1 )
         end
      end )
      vn.na(_("You offer her a drink. After staring intently at the drink menu, she orders a strawberry cheesecake caramel parfait with extra berries. Wait, was that even on the menu?"))
      maikki(_([["Thank you! At least the food isn't bad here!"
She starts eating the parfait, which seems to be larger than her head.]]))
      vn.jump( "nodrink" )

      vn.label( "notenough" )
      vn.na(_("You do not have enough Minerva Tokens to buy her a drink."))
      vn.jump( "menu" )

      vn.label( "nodrink" )
      maikki(_([["The truth is I am looking for my father. He disappeared when I was a little girl and I don't even remember a single memory of him."]]))

   else
      vn.na(_("You approach Maikki who seems to be enjoying a parfait."))
   end

   -- Normal chitchat
   local opts = {
      {_("Ask about her father"), "father"},
      {_("Leave"), "leave"},
   }
   if misn_state and misn_state >=4 then
      table.insert( opts, 1, {_("Show her the picture"), "showloot"} )
   end
   vn.label( "menu" )
   vn.menu( opts )

   vn.label( "father" )
   maikki(_([["I don't remember him at all since he disappeared when I was only 5 cycles old, but before my mother died, she told me he was a famous space pilot."]]))
   maikki(_([["She used to tell me stories about how he would go on all sorts of brave adventures in the nebula to recover artefacts of human history."]]))
   vn.menu( {
      { _([["He was a scavenger?"]]), "menuscholar" },
      { _([["He was a scholar?"]]), "menuscholar" },
   } )
   vn.label( "menuscholar" )
   maikki(_([["I like to think that he was a scholar, but most people would call him a scavenger."]]))
   maikki(_([["My mother died without telling me, but after her death, while going through her stuff, I found out that my father was the famous Kex McPherson!"]]))
   maikki(_([["Apparently, one day he went into the nebula with his business partner Mireia and they were never seen again. All attempts to find them failed."]]))
   maikki(_([["Most people believe they are dead, but I think he was kidnapped and is being held here. Maybe he hit his head and even forgot who he was!"]]))
   maikki(_([["I don't have a spaceship, but while I look around here, could you try to look for hints around where he went missing? I heard he was very fond of the Cerberus bar in Doeston. Maybe there is a hint there."]]))
   vn.func( function ()
      if misn_state < 0 then
         misn_state = 0
         misn_osd = misn.osdCreate( misn_title,
            { string.format(_("Look around the %s system"), _(searchsys)) } )
         misn_marker = misn.markerAdd( system.get(searchsys), "low" )
      end
   end )
   vn.jump( "menu_msg" )

   vn.label( "showloot" )
   vn.na(_("You show her the picture you found in Zerantix of her and her parents."))
   maikki(_([[As she stares deeply at the picture, her eyes tear up.]]))
   maikki(_([["I'm sorry, I shouldn't be crying. I hardly even know the man. It's just seeing us together just brings back some memories which I had thought I had forgotten."]]))
   maikki(_([["He looks so goofy in this picture, and my mother looks so happy... This is what should have been my childhood..."
She reminisces.]]))
   vn.na(_("You give a few moments to recover before explaining her what you saw in the wreck and your encounter with the scavengers."))
   maikki(_([[She dries her eyes with a handkerchief trying unsuccessfully not to smear her makeup.
"From what you tell me, it seems like it wasn't an accident..."
She looks up expectantly.]]))
   maikki(_([["If you didn't find a body I'm sure he survived! He shouldn't be a guy that dies that easily!"
She looks clearly excited at the possibility.]]))
   maikki(_([[She furrows her brows.
"The scavengers were selling to the Za'lek right? What could the Za'lek have to do with my father so far away? I always thought those creeps couldn't be up to anything good."]]))
   maikki(_([[Her face glows.
"I think I have an idea for our next steps. Meet me up here in a bit. I have to get some information first."]]))
   vn.func( function ()
      -- no reward, yet...
      vn.sfxVictory()
      mission_finish = true
   end )
   vn.fadeout()
   vn.done()

   vn.label( "menu_msg" )
   maikki(_([["Is there anything you would like to know about?"]]))
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.na(_("You take your leave to continue the search for her father."))
   vn.fadeout()
   vn.run()

   -- Can't run it in the VN or it causes an error
   if mission_finish then
      misn.finish(true)
   end
end


function approach_oldman ()
   vn.clear()
   vn.scene()
   local om = vn.newCharacter( oldman_name,
         { image=oldman_image } )
   vn.fadein()
   vn.na( _("You see an old man casually drinking at the bar. He has a sort of self-complacent bored look on his face.") )

   vn.label( "menu" )
   local opts = {
      {_("Ask about Kex McPherson"), "kex" },
      {_("Ask about Doeston"), "doeston"},
      {_("Ask about the Nebula"), "nebula"},
      {_("Leave"), "leave"},
   }
   if misn_state>=2 then
      table.insert( opts, 1, {_("Ask about scavengers you saw"), "scavengers"} )
   end
   if misn_state >=4 then
      table.insert( opts, 1, {string.format(_("Ask about %s"),_(stealthsys)), "stealthmisn"} )
   end
   if misn_state >=5 then
      table.insert( opts, 1, {_("Show him the picture"), "showloot"} )
   end
   vn.menu( opts )

   vn.label( "kex" )
   om(_([["Kex? Now that is not a name I've heard in a while."
He nods reflectively.]]))
   om(_([["Kex was a great guy. He used to hang out here before venturing into the nebula, spending his time with the useless lot of us. Shame that he went missing."]]))
   om(_([["Since they never found his ship, I like to think that he made it to the other side of the nebula, if there is one."
He takes a long swig from his drink.]]))
   om(_([["Still that doesn't stop the odd folk here and there from trying to find it, they usually don't end up much past Arandon."]]))
   vn.func( function ()
      if misn_state==0 then
         misn.markerMove( misn_marker, system.get(cutscenesys) )
         misn_state=1
      end
   end )
   vn.jump( "menu_msg" )

   vn.label( "doeston" )
   om(_([["Not much to do in Doeston. Mainly a stopping place for all them crazy folk heading into the nebula. Maybe if I were any younger I would be with them exploring, but can't with this bad knee."
He pats his left knee.]]))
   om(_([["It used to be a more popular place, but with most of the easy pickings getting scavenged out of the nebula, not many people come here after all. Especially not after the disappearance of famous scavengers like Kex and Mireia."
He muses thoughtfully.]]))
   om(_([["To better times."
He downs his drink and orders another.]]))
   vn.jump( "menu_msg" )

   vn.label( "nebula" )
   om(_([[He whistles casually.
"The nebula's a real piece of work. It's almost mesmerizing to fly through it, however, it do got quite a character. If you try to go too deep into 'er you can't easily get back. Many a soul has been lost in there."]]))
   vn.sfxEerie()
   om(_([[He goes a bit quieter and gets closer to you.
"Rumour has it that there are ghosts lurking in the depths. I've seen people come back, pale as snow, claiming they seen them."]]))
   om(_([["I believe it be the boredom getting to their heads. Likely naught but a scavenger or some debris."]]))
   vn.jump( "menu_msg" )

   vn.label( "scavengers" )
   om(_([["It's not uncommon to see scavengers around here. Although there are no where near as many as there were in Kex's time."]]))
   om(_([[He takes a long sip, gets close and whispers.
"You wouldn't suppose they found something interesting?"]]))
   om(_([["Suppose not. However, you could ask them over there. Those two look like they could be scavengers."]]))
   vn.na(_("He points to a table with two fairly drunk people."))
   vn.func( function ()
      if misn_state==2 then
         misn_state=3
         npc_scavenger = misn.npcAdd( "approach_scavengers", scav_name, scav_portrait, scav_desc )
      end
   end )
   vn.jump( "menu_msg" )

   vn.label( "stealthmisn" )
   om(string.format(_([["%s? That should be just past %s. Do you think the scavengers could have found something there?"]]), _(stealthsys), _(cutscenesys)))
   om(_([["If you plan to go, you should bring your best sensors. It's very hard to see anything due to the density of the nebula there."]]))
   vn.jump( "menu_msg" )

   vn.label( "showloot" )
   om(_([["Wow! Where did you find that picture of Kex? He looks younger than I remember him!"]]))
   vn.jump( "menu_msg" )

   vn.label( "menu_msg" )
   om( _([[He gives you a bored look as he takes a sip from his drink.
"Is there anything else you would like to know about?"]]) )
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.na(_("You take your leave."))
   vn.fadeout()
   vn.run()
end


function approach_scavengers ()
   vn.clear()
   vn.scene()
   local scavA = vn.newCharacter( _("Scavenger A"),
         { image=scavengera_image, color=scavengera_colour } )
   local scavB = vn.newCharacter( _("Scavenger B"),
         { image=scavengerb_image, color=scavengerb_colour } )
   vn.fadein()

   if bribed_scavengers==true then
      -- TODO maybe more text?
      scavB(_([["What are you looking at?"]]))
      vn.fadeout()
      vn.done()
   end

   vn.na(_("You see some scavengers at the bar. They are clearly plastered. They don't really seem to be aware of your presence."))

   if misn_state==4 then
      -- Already got mission, just give player a refresher
      scavB(_([["Aren't you drinking too much. Don't forget to fix my sensors before we leave to Zerantix tomorrow."]]))
      scavA(_([["Meee? Drinkinging tooo mich? Thatss sshtoopid."
He takes another long swig of his drink and burps.]]))
   else
      -- Blabber target to player
      scavA(_([["...and then I said to him 'while that may look like a hamster, it's got a bite like a moose!'"]]))
      vn.na(_("The scavengers hoot with laughter."))
      scavB(_([["That's a great story. Them space hamsters be wicked."]]))
      scavA(_([["About tomorrow, you sure the info is correct? Going that deep into the nebula always gives me the chills."
He shivers exaggeratedly.]]))
      scavB(_([["Yeah! I saw it with my own eyes. That shit is legit. Seems to be the wreck of a scavenger."]]))
      scavA(_([["Why didn't you haul it back then? We might not be able to find it again!"]]))
      vn.sfxBingo()
      scavB(_([["How many times do I have to tell you? My sensors were acting up, didn't want to spend too long in Zerantix with all them ghosts around."]]))
      scavA(_([["I'll take a look at fixing your sensors. Ain't nothing these brutes can't fix!"
He pats his biceps in a fairly uninspiring way.]]))
      scavB(_([["Yeah, let's do that after one more round of drinks. Got to get there before other scroungers get there."]]))
      scavA(_([["To our future success in Zerantix!"]]))
      vn.na(_("They cheer, down their drinks, and order another round. Perhaps the wreck in Zerantix is related to Kex somehow."))
      vn.func( function ()
         if misn_state==3 then
            misn_osd = misn.osdCreate( misn_title,
               { string.format(_("Follow the scavengers in the %s system"), _(stealthsys)) } )
            misn.markerMove( misn_marker, system.get(stealthsys) )
            misn_state=4
         end
      end )
   end

   vn.na(_("You take your leave without them noticing you."))
   vn.fadeout()
   vn.run()
end


function enter ()
   if system.cur() == system.get(cutscenesys) and misn_state==1 then
      -- Set up system
      pilot.clear()
      pilot.toggleSpawn(false)

      -- Cutscene with scavengerB
      -- Scavenger is flying to doeston from arandon (near the middle)
      -- cutscene
      local j = jump.get( cutscenesys, searchsys )
      local pos = j:pos() + vec2.new(3000,5000)
      local fscav = faction.dynAdd( "Independent", "Scavenger", "Scavenger" )
      pscavB = pilot.addRaw( "Vendetta", "independent", pos, "Scavenger" )
      pscavB:rename(_("Scavenger Vendetta"))
      pscavB:control()
      pscavB:brake()
      cuttimer = hook.timer( 3000, "cutscene_timer" )
      pscavB:setInvincible() -- annoying to handle the case the player kills them
      -- Timer in case the player doesn't find them in a long while
      sysmaker = nil
      cuttimeout = hook.timer( 120*1000, "cutscene_timeout" ) -- 2 minutes
   elseif system.cur() == system.get(stealthsys) and misn_state==4 then
      -- Set up system
      pilot.clear()
      pilot.toggleSpawn(false)
      -- Have to follow scavengers
      waypoints = {
         vec2.new( -15000,  8000-1200 ),
         vec2.new( -11000, 13000 ), -- 7200 dist or 36s
         vec2.new(  -9000,  7000 ), -- 6400 dist or 32s
         vec2.new(  -6000, 12000 ), -- 5800 dist or 29s
      }
      local pos = waypoints[1]
      local posA = pos + vec2.new( 100, 80 )
      local posB = pos + vec2.new( -50, -20 )
      local fscav = faction.dynAdd( "Independent", "Scavenger", "Scavenger" )
      pscavA = pilot.addRaw( "Shark", "independent", posA, "Scavenger" )
      pscavB = pilot.addRaw( "Vendetta", "independent", posB, "Scavenger" )
      for k,p in ipairs{ pscavA, pscavB } do
         p:control()
         p:setSpeedLimit( 200 )
         hook.pilot( p, "attacked", "scav_attacked" )
      end
      pscavA:face( pscavB )
      pscavB:face( pscavA )

      hook.timer( 3000, "stealthstart" )
   end
end

cutscene_msg = 0
cutscene_messages = {
   _("Sensors damaged. Requesting assistance."),
   _("S.O.S. Scavenger here, sensors damaged."),
   _("Is anybody out there? Sensors damaged, requesting assistance."),
   _("Mayday! Requesting assistance. Wait, what was that?"),
}

function cutscene_timer ()
   if not pscavB:exists() then return end
   local dist = pscavB:pos():dist( player.pos() )
   pscavB:taskClear()
   if hailhook ~= nil then
      hook.rm( hailhook )
      hailhook = nil
   end
   if (dist < 1000) then
      pscavB:face( player.pilot() )
      pscavB:hailPlayer()
      hailhook = hook.pilot( pscavB, "hail", "cutscene_hail" )
      -- Get rid of the timeout hook
      if cuttimeout then
         hook.rm( cuttimeout )
         cuttimeout = nil
      end
      if sysmarker then
         system.mrkRm( sysmarker )
         sysmarker = nil
      end
   else
      pscavB:brake()
      cutscene_msg = (cutscene_msg % #cutscene_messages)+1
      local msg = cutscene_messages[ cutscene_msg ]
      pscavB:broadcast( msg )
   end
   cuttimer = hook.timer( 3000, "cutscene_timer" )
end


function cutscene_timeout ()
   player.msg(_("#pYour ship has detected a curious signal originating from inside the system.#0"))
   sysmarker = system.mrkAdd( _("Curious Signal"), pscavB:pos() )
   cuttimeout = nil
end


function cutscene_hail ()
   local asshole = false
   vn.clear()
   vn.scene()
   local scavB = vn.newCharacter( _("Scavenger"),
         { image=portrait.hologram( scavengerb_portrait ),
         color=scavengerb_colour } )
   vn.fadein()
   vn.na(_("The comm flickers as a scavenger appears into view. He looks a bit pale."))
   scavB(_([["Thank you. I thought I was a goner. My sensors failed me at the worst time and it's impossible to see shit in this nebula."]]))
   scavB(string.format(_([["Could you tell me the way to %s? I have to get out of here as soon as possible."]]), _(searchsys)))
   vn.menu( {
      { _("Give him directions"), "help" },
      { _("Leave"), "leave" },
   } )

   vn.label("leave")
   vn.na(_("You close the comm and leave the scavenger to his fate."))
   vn.func( function () asshole = true end )
   vn.fadeout()
   vn.done()

   vn.label("help")
   scavB(_([["Thanks! I can't wait to get out of this hellhole."]]))
   scavB(_([["Don't tell me you've also come here to scavenge? I'm telling you, this place is haunted."]]))
   scavB(_([["I was told this would be easy money on the blackmarket, but this wasn't what I expected at all."]]))
   scavB(_([["Anyway, good luck scavenging."]]))
   vn.na(_("The scavenger disappears from view."))
   vn.fadeout()
   vn.run()

   -- Close comm immediately
   player.commClose()

   -- Was player an asshole?
   if asshole then
      pscavB:broadcast(_("Asshole!"))
      hook.rm( cuttimer ) -- reset timer
      cuttimer = hook.timer( 3000, "cutscene_timer" )
   else
      pscavB:broadcast(_("Thanks!"))
      pscavB:taskClear()
      pscavB:hyperspace( system.get(searchsys) )
      misn.markerMove( misn_marker, system.get(searchsys) )
      misn_state = 2
      hook.rm( cuttimer ) -- reset timer
      pilot.toggleSpawn(true)
   end
end


function stealthstart ()
   local pp = player.pilot()
   pp:control()
   pp:brake()

   -- Start the cinematics
   stealthanimation = 0
   player.cinematics( true )
   camera.set( waypoints[1], true )
   hook.timer( 1000, "stealthstartanimation" )
end


function stealthstartanimation ()
   stealthanimation = stealthanimation+1
   if stealthanimation==1 then
      pscavB:broadcast( _("Damnit! I thought I told you to fix the sensors.") , true )
      hook.timer( 4000, "stealthstartanimation" )
   elseif stealthanimation==2 then
      pscavA:broadcast( _("Hey! I fixed them, it's this damn nebula that must have broken them again."), true )
      hook.timer( 4000, "stealthstartanimation" )
   elseif stealthanimation==3 then
      pscavB:broadcast( _("Damnit. I really dislike broadcasting, but my encryption also seems damaged now."), true )
      hook.timer( 4000, "stealthstartanimation" )
   elseif stealthanimation==4 then
      pscavA:broadcast( _("Don't worry, it's not like there's anybody else here."), true )
      hook.timer( 4000, "stealthstartanimation" )
   elseif stealthanimation==5 then
      pscavB:broadcast( _("Let's just get this over with."), true )
      hook.timer( 4000, "stealthstartanimation" )
   elseif stealthanimation==6 then
      -- Back to player
      player.cinematics( false )
      camera.set( nil, true )
      player.pilot():control(false)
      stealthtarget = 0
      hook.timer( 4000, "stealthheartbeat" )
   end
end


function stealthbroadcast ()
   stealthmessages = {
      { pscavA, 4000, _("This place always gives me the creeps.") },
      { pscavB, 4000, _("C'mon, the guy said that this was a great wreck.") },
      { pscavB, 4000, _("You know the Za'lek pay premium for this sort of shit, right?") },
      { pscavA, 4000, _("Yeah, yeah...  just hope we don't see any ghosts.") },
      { pscavB, 4000, _("You don't really believe in them do you?") },
      { pscavA, 4000, _("Dude, there's really freaky shit out there.") },
      { pscavB, 4000, _("Yeah, but most of that freaky shit are Soromid bioengineered crap.") },
      { pscavA, 4000, _("Ugh, Soromids give me the creeps.") },
      { pscavB, 5000, _("I'd take them over Za'leks any day. I have no idea what Za'leks are thinking.") },
      -- 37 seconds
      { pscavA, 4000, _("Hey, did you see something?") },
      { pscavB, 4000, _("How the hell am I supposed to see anything with my sensors broken, dipshit?") },
      { pscavA, 4000, _("I already said I was sorry!") },
      { pscavB, 4000, _("This is coming out of your cut.") },
      -- 53 seconds
      { pscavA, -1, _("C'mon!") },
      -- Cut off point here, restarts at heading to waypoint[4]
      { pscavB, 3000, _("We should be there soon.") },
      { pscavA, 3000, _("I swear I'll never come to the nebula again after this...") },
      { pscavB, -1, _("That's what you said last time too!") }
   }

   stealthmsg = stealthmsg or 0
   stealthmsg = stealthmsg+1
   if stealthmsg > #stealthmessages then
      return
   end

   local msg = stealthmessages[ stealthmsg ]
   msg[1]:broadcast( msg[3] )

   if msg[2] > 0 then
      stealthbroadcasthook = hook.timer( msg[2], "stealthbroadcast" )
   end
end

function stealthheartbeat ()
   local pp = player.pilot()
   local dist= math.min ( pscavA:pos():dist(pp:pos()), pscavB:pos():dist(pp:pos()) )
   local stats = pp:stats()
   if dist < 1000 / stats.ew_hide then
      if stealthfailing==nil then
         stealthfailing = 0
         player.msg("#rYou are about to be discovered!")
      end
      stealthfailing = stealthfailing+1
      if stealthfailing > 6 then
         pscavA:broadcast( _("Run!") )
         pscavB:broadcast( _("There's definately something there! Scram!") )
         for k,p in ipairs{pscavA, pscavB} do
            p:taskClear()
            p:hyperspace( system.get( cutscenesys ) )
         end
         player.msg( _("#rYou have been detected! Stealth failed!") )
         return
      end
   elseif dist > 2000 * stats.ew_detect then
      if stealthfailing==nil then
         stealthfailing = 0
         player.msg("#rYou are about to lose track of the scavengers!!")
      end
      stealthfailing = stealthfailing+1
      if stealthfailing > 6 then
         pscavA:rm()
         pscavB:rm()
         if wreck ~= nil then
            wreck:rm()
         end
         player.msg( _("#rYou lost track of the scavengers! Stealth failed!") )
         return
      end
   else
      stealthfailing = nil
   end

   if stealthtarget==0 then
      -- Starting out
      pscavB:broadcast( _("Let's get going.") )
      pscavA:taskClear()
      pscavB:taskClear()
      pscavA:follow( pscavB )
      stealthtarget = 1
      pscavB:moveto( waypoints[stealthtarget] )
      hook.timer( 3000, "stealthbroadcast" )
   else
      -- Check if made it to next target
      local pos = waypoints[ stealthtarget ]
      dist = pscavA:pos():dist( pos )
      if dist < 500 then
         -- Finished current target, go to next
         stealthtarget = stealthtarget+1
         if stealthtarget > #waypoints then
            -- Made it to target
            for k,p in ipairs{ pscavA, pscavB } do
               p:taskClear()
               p:brake()
               p:face( wreck )
            end
            local pp = player.pilot()
            pp:control()
            pp:brake()
            player.cinematics( true )
            camera.set( waypoints[ #waypoints ], true )
            wreckscene = 0
            hook.timer( 3000, "wreckcutscene" )
            return
         else
            -- pscavA is following pscavB
            if stealthtarget==4 then
               if not stopped_once then
                  pscavB:taskClear()
                  pscavB:brake()
                  pscavB:broadcast( _("Did you see something?") )
                  stealthtarget = 3 -- Should trigger another catch
                  stopped_once = true
                  hook.timer( 5000, "stealthheartbeat" )
                  return
               else
                  pscavA:broadcast( _("It's your imagination. Let's get this over with.") )
                  pscavB:taskClear()
                  pscavB:moveto( waypoints[stealthtarget] )
                  hook.timer( 3000, "stealthbroadcast" )
               end
            else
               pscavB:taskClear()
               pscavB:moveto( waypoints[stealthtarget] )
            end
         end

         -- Spawn the wreck
         if stealthtarget==4 then
            pos = waypoints[ #waypoints ] + vec2.new(-10, -50)
            wreck = pilot.addRaw( "Rhino", "independent", pos, "Derelict" )
            wreck:rename( _("Ship Wreck") )
            wreck:disable()
            wreck:setInvincible()
            hook.pilot( wreck, "board", "board_wreck" )
         end
      else
         -- Still travelling to target
      end
   end

   hook.timer( 500, "stealthheartbeat" )
end


function wreckcutscene ()
   wreckscene = wreckscene + 1
   if wreckscene==1 then
      pscavB:broadcast( _("Looks like we made it!"), true )
   elseif wreckscene==2 then
      pscavA:broadcast( _("We came all the way for this wreck? It better be worth it..."), true )
   elseif wreckscene==3 then
      pscavB:broadcast( _("They said it was whatshisname... Xex? Vex? Some famous guy's wreck."), true )
   elseif wreckscene==4 then
      pscavA:broadcast( _("Wait, someone is there!"), true )
   elseif wreckscene==5 then
      scavengers_encounter()
      found_wreck = true
      local pp = player.pilot()
      pp:control( false )
      player.cinematics( false )
      camera.set( nil, true )
      return
   end
   hook.timer( 3000, "wreckcutscene" )
end


function scavengers_encounter ()
   local bribeamount = 500000 -- 500k credits

   vn.clear()
   vn.scene()
   local scavA = vn.newCharacter( _("Scavenger A"),
         { image=portrait.hologram(scavengera_portrait),
         color=scavengera_colour } )
   local scavB = vn.newCharacter( _("Scavenger B"),
         { image=portrait.hologram( scavengerb_portrait ),
         color=scavengerb_colour } )
   vn.fadein()

   vn.na(_("Two angry scavengers appear on your screen."))
   scavB(_([["You better beat it, punk. We are doing business here."]]))
   scavA(_([["Yeah, the Za'lek wouldn't like it if we weren't able to deliver the goods they want."]]))
   scavB(_([[He scowls at his partner before staring you down again.
"This ain't no place for people like you. Get lost or we'll leave you in a worse state than that wreck over there."
He points at the wreck nearby.]]))
   vn.menu( {
      { _([["What is that about the Za'lek?"]]), "zalek" },
      { _("Lock your weapon systems on their ships"), "locked" },
   })

   vn.label("zalek")
   scavB(_([[He glares at his partner.
"This is why I always tell you to keep your mouth shut!"]]))
   scavA(_([["Damnit, why can't shit go right for a change?"
He seems to be clutching his head. A headache perhaps?]]))
   vn.menu( {
      { _([["Look I just want to talk"]]), "trytalk" },
      { string.format(_([[Try to bribe them (#r%s#0)]]), creditstring(bribeamount)), "trybribe" },
   })

   -- TODO possibly add a pacifist option here too
   vn.label("trytalk")
   scavA(_([["What do you want to talk about asshole? This is our job."]]))
   vn.jump("stall")

   vn.label("trybribe")
   vn.func( function ()
      if player.credits() < bribeamount then
         vn.jump("poor")
      else
         player.pay( -bribeamount )
         bribed_scavengers = true
         var.push( "maikki_scavengers_alive", true )
         for k,p in ipairs{ pscavA, pscavB } do
            p:taskClear()
            p:hyperspace( system.get(cutscenesys) )
         end
      end
   end )
   vn.sfxMoney()
   vn.na(string.format(_("You wire them %s."), creditstring(bribeamount)))
   scavB(_([["I guess this isn't worth our trouble. We already got enough stuff for the Za'leks already."]]))
   scavA(_([["C'mon, let's get out of here. This place gives me the creeps. Feel like a ghost is going to pop out any minute."]]))
   scavB(_([["Next round in Doeston is on me."]]))
   vn.na(_("The scavengers disappear from your screen as you see their ships start to head back to Arandon."))
   vn.fadeout()
   vn.done()

   -- Fight to the death :D
   vn.label("poor")
   vn.na(_("You don't have enough money to bribe them and fumble with words."))
   vn.label("stall")
   scavB(_([["He's stalling for time! He must have reinforcements coming!"]]))
   vn.label("locked")
   scavA(_([["Shit man! I knew we shouldn't have come here."]]))
   scavB(_([["Shut up and follow my lead!"]]))
   vn.na(_("You detect they are powering up their weapon systems."))
   vn.func( function ()
      for k,p in ipairs{ pscavA, pscavB } do
         p:taskClear()
         p:attack( player.pilot() )
         -- TODO add angry messages
      end
   end )

   vn.fadeout()
   vn.run()
end


function scav_attacked ()
   if found_wreck then return end
   player.msg(_("MISSION FAILED! Scavenger attacked."))
   for k,p in ipairs{pscavA, pscavB} do
      if p:exists() then
         p:control( false )
      end
   end
   misn.finish( false )
end


function board_wreck ()
   local saw_bridge, saw_dormitory, saw_engineroom
   vn.clear()
   vn.scene()
   vn.fadein()
   vn.na(_("You can see clear laser burns on the hull of the wreck as you approach the ship and prepare to board. This doesn't look like it was an accident."))
   vn.na(_("You board the wreck in your space suit and begin to investigate the insides of the ship."))
   vn.label("menu")
   -- Give player the illusion of choice
   vn.menu( function ()
      local opts = {}
      if not saw_bridge then
         table.insert( opts, { _("Investigate the bridge"), "bridge" } )
      end
      if not saw_dormitory then
         table.insert( opts, { _("Investigate the dormitories"), "dormitories" } )
      end
      if not saw_engineroom then
         table.insert( opts, { _("Investigate the engine room"), "engineroom" } )
      end
      if saw_bridge and saw_dormitory and saw_engineroom then
         table.insert( opts, { _("Leave"), "leave" } )
      end
      return opts
   end )

   vn.label("bridge")
   vn.na(_("You make your way to what is left of the bridge. You can see what appears to be very old space-weathered bloodstains over most of the controls. However, there are no bodies to be seen around."))
   vn.func( function () saw_bridge = true end )
   vn.jump("menu")

   vn.label("dormitories")
   vn.na(_("The dormitories are the part of the ship that appear to have been kept in best shape, if you don't count all the damage that seems to have been done by scavengers trying to find parts to sell."))
   vn.na(_("Although there seems to be nothing of value left, a small piece of paper catches your eye. You grab what appears to be a picture of two adults and a child. The child looks very familiar."))
   vn.sfxBingo()
   vn.na(_("You turn the picture around and you see that 'Maikki 596:0928' is written in the corner. You should probably bring this back to Maikki."))
   vn.func( function () saw_dormitory = true end )
   vn.jump("menu")

   vn.label("engineroom")
   vn.na(_("The engine room seems to be the part that took most of the beating. It seems like most of it was sliced off by some powerful beam weapon. Someone really didn't want this ship getting away."))
   vn.func( function () saw_engineroom = true end )
   vn.jump("menu")

   vn.label("leave")
   vn.na(_("After your thorough investigation, you leave the wreck behind and get back into your ship."))
   vn.fadeout()
   vn.run()

   -- Move target back to origin
   misn_osd = misn.osdCreate( misn_title,
         { string.format(_("Return to %s in the %s system"), minerva.maikki.name, _(mainsys)) } )
   misn.markerMove( misn_marker, system.get(mainsys) )
   misn_state=5

   -- Clear scavengers if exist
   for k,p in ipairs{pscavA, pscavB} do
      if p:exists() then
         p:rm()
      end
   end

   -- Unboard
   player.unboard()
end
