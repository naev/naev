--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Judgement">
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 6</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   * Player's testimony gets called into question if killed harper or let scavengers live
   * Kex deciphers final stuff and blurts out important thing to get CEO convicted at the end, but by doing that shows off he is an intelligent being and gets sentenced to dissection by the Za'lek.
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local vne = require 'vnextras'
local fmt = require "format"
local love_shaders = require "love_shaders"
local love_audio = require 'love.audio'
local reverb_preset = require 'reverb_preset'

local zalek_image = "zalek_thug1.png"
local dvaered_image = "dvaered_thug1.png"
local zalek_colour = {1, 0.4, 0.4}
local dvaered_colour = {1, 0.7, 0.3}

local trialspb, trialsys = spob.getS("Jade Court")

local title = _("Minerva Judgement")

-- Mission states:
--  nil: mission not accepted yet
--    1. fly to Jade Court

function create()
   -- Zuri gives the mission to go to court at minerva
   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("The future of Minerva Station is at stake with deliberations being held at {spb} in the {sys} system. Zuri has aske you to take her to the courtroom to see if it is possible to influence the case."),
      {sys=trialspb, spb=trialspb}))
   misn.setReward(_("The future of Minerva Station!"))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   vn.na(_([[You find Zuri waving to you at the bar. It seems like she finished whatever she had to do.]]))
   zuri(fmt.f(_([["OK, I've got some help, they'll meet up with us on {spb}. I have no idea to expect out of all of this, but it seems like a pretty good damn chance to clear up Minerva Station if we play our cards right."]]),
      {spb=trialspb}))
   zuri(fmt.f(_([["I'll be going with you on your ship to {spb}. We don't really have much of a plan but to show up, and hope that all the data collected will be useful for us. You ready to take us to the {sys} system and do this?"]]),
      {spb=trialspb, sys=trialsys}))
   vn.menu( {
      {_("Let's go!"), "accept"},
      {_("Maybe later."), "decline"},
   } )

   vn.label("decline")
   vn.na(_([[You tell Zuri to give you a bit of time to prepare and leave her waiting at the bar.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   zuri(fmt.f(_([["Great! Take us to {spb} in the {sys} system."]]),
      {spb=trialspb, sys=trialsys}))

   vn.run()

   if not accepted then
      return
   end

   local c = commodity.new( N_("Zuri"), N_("A mysterious character that has been hiring you to 'free' Minerva Station.") )
   misn.cargoAdd( c, 0 )

   mem.state = 0

   misn.accept()
   misn.osdCreate( title, {
      fmt.f(_("Go to {spb} ({sys} system)"),{spb=trialspb, sys=trialsys}),
   } )
   mem.mrk = misn.markerAdd( trialspb )

   hook.load("land")
   hook.land("land")
   hook.enter("enter")
end

function land ()
   if spob.cur() == trialspb then
      misn.npcAdd( "trial_start", minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   end
end

function trial_start ()
--[[
- minerva_altercation_helped ("dvaered", "zalek", nil)
- maikki_gave_drink (true, nil)
- maikki_response ("yes", "no", nil)
- maikki_scavengers_alive (true, nil)
- harper_ticket ("credits", "tokens", "free", "stole" )
- strangelove_death ("unplug", "comforted", "shot", nil)
--]]
   local didtrial = false
   local zl_points = 0
   local dv_points = 0
   local pir_points = 0

   -- Main characters
   local zuri = minerva.vn_zuri()
   local kex = minerva.vn_kex()
   local maikki = minerva.vn_maikki()
   local ceo = minerva.vn_ceo()
   local judge = vn.Character.new( _("Judge Holmes"), { image="judge_holmes.webp" } )
   local zlk = vn.Character.new( _("Za'lek Lawyer"), { image=zalek_image, color=zalek_colour } )
   local dvd = vn.Character.new( _("Dvaered Representative"), { image=dvaered_image, color=dvaered_colour } )
   local scv = vn.Character.new( minerva.scavengera.name,
         { image=minerva.scavengera.image, color=minerva.scavengera.colour, pos="farleft" } )

   vn.clear()
   vn.scene()
   vn.newCharacter( zuri )
   vn.music( minerva.loops.pirate )
   vn.transition()

   vn.na(_([[You meet up with Zuri who is stretching her legs.]]))
   zuri(_([["Ready to go to court? It's my first time not going as a defendant! Although I do feel quite like one. We must ensure the best for Minerva Station."]]))
   vn.menu{
      {_([[Go to the courtroom.]]), "01_start"},
      {_([[Maybe later.]]), "01_later"},
   }

   vn.label("01_later")
   vn.na(_([[You decide to post-pone deciding the future of Minerva Station.]]))
   vn.done()

   vn.label("01_start")
   vn.func( function () didtrial = true end )
   vn.na(_([[The station seems to be crowded with the many simultaneous trials and ongoings. Watching people bustling around with unpredictable patterns is surprisingly entertaining, however, you remember your purpose and follow Zuri to the assigned courtroom.]]))
    vn.music( 'snd/music/empire1.ogg' ) -- TODO trial music
   -- TODO background?
   vn.na(_([[After getting lost in the corridors overflowing with people several times, you make it to the courtroom. It seems to be quite packed and the ambience electric, with uncertainty about how the upcoming legal drama would unfold.]]))
   zuri(_([[Zuri turns to you.
"I'll be right back, I have to prepare some things. Take a set and hold on tight! I have a feeling this trial is going to be wild."]]))
   vn.na(_([[Zuri leaves you and not having anything better to do in the clamorous room, you find an empty seat and sit down.]]))
   vn.na(_([[Without anything better to do, you begin to observe your surroundings. Looking around the public you could make out quite a few figures you know.]]))

   vn.scene()
   vn.newCharacter( ceo )
   vn.transition( "slideleft" )
   vn.na(_([[The Minerva CEO is near the front of the crowd, he seems to be quite pale and looking very well.]]))

   vn.scene()
   vn.newCharacter( zlk )
   vn.transition( "slideleft" )
   vn.na(_([[To the left is the Za'lek crowd. They seem to be intently comparing notes and discussing amongst themselves.]]))

   vn.scene()
   vn.newCharacter( dvd )
   vn.transition( "slideleft" )
   vn.na(_([[To the right are the Dvaered officials. They seem to boisterously arguing with each other.]]))

   vn.scene()
   vn.newCharacter( maikki )
   vn.transition( "slideleft" )
   vn.na(_([[Wait, what is Maikki doing here?]]))

   vn.scene()
   vn.newCharacter( kex )
   vn.transition( "slideleft" )
   vn.na(_([[Even weirder, it seems like Kex is also at the court... How did they get here?]]))

   vn.scene()
   vn.transition( "slideleft" )
   vn.na(_([[Before you can try to do anything, suddenly a hush fills the room. It seems like the show is about to start.]]))

   vn.scene()
   vn.newCharacter( judge )
   vn.transition()
   judge(_([[A judge floats up to the center of the room.
"Order! Order!"
She hits her gavel.]]))
   vn.na(_([[Except for the odd cough, silence envelopes the room.]]))
   judge(_([["We are gathered here to day to deliberate over the future of Minerva Station, which has been accused of falling into debauchery and lawlessness. These are very serious accusation that question the sovereignty of Minerva Station, which currently enjoys the privilege of independent rule."]]))
   judge(_([[According to Principle of System Sovereignty, the parties who have claims on the system have the right for equal, fair, and honest deliberations of the partake in the distribution of powers.]]))

   vn.func( function ()
      zlk.pos = "farleft"
      dvd.pos = "farright"
      ceo.pos = "farleft"
      maikki.pos = "farleft"
   end )
   judge(_([["In the case of Minerva Station, the three parties involved are House Za'lek..."]]))
   vn.appear( zlk, "slideup" )
   zlk(_([[The Za'lek lawyer does a short concise bow.
"Your honour."]]))
   vn.disappear( zlk, "slideup" )
   judge(_([["...House Dvaered..."]]))
   vn.appear( dvd, "slideup" )
   dvd(_([[The Dvaered representative does an exaggerated deep bow.
"Your honour."]]))
   vn.disappear( dvd, "slideup" )
   judge(_([["..and the CEO of Minerva Station."]]))
   vn.appear( ceo, "slideup" )
   ceo(_([[A pale feverishly sweating Minerva CEO stands up.
"Your h..."
...and promptly collapses in a puddle of their sweat.]]))
   vn.na(_([[A collective gasp is let out in the courtroom. Not the best way to start the deliberations.]]))
   vn.disappear( ceo, "slideup" )
   judge(_([[The judge clicks her tongue in annoyance.
"Passing out without filing an EJ-2891? Unheard of."]]))
   vn.appear( maikki, "slideup" )
   maikki(_([[Suddenly, Maikki stands up and raises a wad of papers.]]))
   judge(_([["Is that the EJ-2891? Go on."
The judge nods towards Maikki, giving her permission to speak.]]))
   maikki(_([["Your honour."
Maikki gives an impeccable formal bow.]]))
   maikki(_([["I would like to submit an EJ-7777 motion to substitute the Minera CEO."]]))
   judge(_([[The judge makes a curious expression and hovers down to collect the papers from Maikki. She quickly skims over it, and gives a sly grin.]]))
   judge(_([["Well it seems like the deliberations will be able to proceed today. Replacing the previous individual, we have Maisie McPherson, who will be representing the independent interests of Minerva Station. Please sit down."]]))
   vn.disappear( maikki, "slideup" )
   vn.na(_([[You see Zuri out of the corner of your eye dragging away the body of the Minerva CEO. Is that even legal?]]))
   judge(_([["All parties are present and accounted for. Let us proceed to the deliberations. Given the order of the filing, House Za'lek shall be the first to begin deliberations on the subject at hand. House Za'lek, please begin with your exposition."]]))
   vn.na(_([[You hear some flutter of papers as the process begins its ascent.]]))

   -- Reset positions
   vn.func( function ()
      zlk.pos = "center"
      dvd.pos = "center"
      maikki.pos = "center"
   end )
   vn.scene()
   vn.newCharacter( zlk )
   vn.transition( "slideright" )
   zlk(_([[The Za'lek Lawyer stands up, clears their throat and begins.
"Ladies and gentlemen, it is long known that Minerva Station is a stain on the galactic map. Despite all the excellent potential, it has been squandered away during cycles neither benefiting the Empire nor the Great Houses."]]))
   zlk(_([["Today, I stand before you as the representative of House Za'lek, armed not only with legal expertise, but intellect that has guided our House to prosperity throughout the Empire. I shall present a compelling case as to why Minerva Station rightfully belongs to House Za'lek."]]))
   local log = vne.flashbackTextStart(_("Narrator"))
   log(_([[The Za'lek Lawyer gives a meticulously prepared and organized speech regarding the plans and benefits that House Za'lek can bring to Minerva Station with a focus on the track record of House Za'lek and the potential for refocusing the station as a cultural enlightenment center to improve collaboration among the great houses.]]))
   vne.flashbackTextEnd()
   zlk(_([["...In conclusion, I am confident that your wisdom and discernment will lead you to make the just and informed decision, aligning the fate of this space station with the unmatched expertise and vision of House Za'lek."]]))

   vn.scene()
   vn.newCharacter( judge )
   vn.transition( "slideleft" )
   judge(_([["Let us continue with House Dvaered."]]))

   vn.scene()
   vn.newCharacter( dvd )
   vn.transition( "slideleft" )
   dvd(_([[The Dvaered Representative briskly stands up and begins to talk with a loud voice that echoes in the room.
"Ladies and gentlemen of the court, esteemed Judges, and respected attendees,"]]))
   dvd(_([["Today, I stand before you as the representative of House Dvaered, a house known for its strength, determination, and unwavering pursuit of victory. I come before you with passion and conviction. I will argue why the ownership of Minerva Station belongs to House Dvaered."]]))
   log = vne.flashbackTextStart(_("Narrator"))
   log(_([[The Dvaered Representative goes on about how only House Dvaered can crush weakness, disobedience, and insubordination through rigorous discipline and punishment. They argue how this has worked for many of their planets, turning them from dens of piracy, to beacons of order.]]))
   vne.flashbackTextEnd()
   dvd(_([["...In conclusion, I ask you to embrace the spirit of House Dvaered. Our strength, our desire for conquest, our loyalty, and our commitment to justice make us the natural choice for owning Minerva Station."]]))

   vn.scene()
   vn.newCharacter( judge )
   vn.transition( "slideright" )
   judge(_([["Last is the independent exposition."]]))

   vn.scene()
   vn.newCharacter( maikki )
   vn.transition( "slideleft" )
   maikki(_([[Maikki stands up to begin her exposition.
"You honour."]]))
   maikki(_([["Today, we gather here to determine the fate of a celestial jewel, a bastion of hope, and a symbol of untethered possibility—the sovereignty of Minerva Station. I stand before you as an advocate for the voice of freedom, the embodiment of autonomy, and the guardian of neutrality."]]))
   maikki(_([["We find ourselves in the midst of a clash between the titans of power and influence: the sly House Za'lek and the aggressive House Dvaered. They seek dominion over this station, driven by their thirst for control and their insatiable desire for conquest."]]))
   maikki(_([["Yet, amidst this power struggle, we must not lose sight of the value inherent in independence. Minerva Station, free from the shackles of allegiance, stands as a sanctuary away from the clutches of power-hungry houses. In its independence, Minerva Station supports trade and diplomacy throughout the Empire."]]))
   maikki(_([["To subject this station to the dominion of either House Za'lek or House Dvaered would be to shackle it, to stifle the boundless potential that has blossomed under its independent governance. Although foreign interference has created instability in the system, this is nothing that can not be solved by an independent government once free of Great House interference."]]))
   maikki(_([["The choice is clear: we must uphold the independence of this station, for the stability of the Empire, for the advancement of science and knowledge, and for the preservation of our shared ideals."]]))

   vn.scene()
   vn.newCharacter( judge )
   vn.transition( "slideright")
   judge(_([["Let us proceed with the presentation of evidence."]]))
   log = vne.flashbackTextStart(_("Narrator"))
   log(_([[First House Za'lek begins to present evidence, presenting results of economical and social growth in systems that were handed-over to the House in the original days. They also call in an expert who presents testimony and highlights the differences with House Dvaered. The Dvaered Representative raises objections, but they are overruled. Overall it is quite a scientific and dry exposition, albeit sound and well established on facts. ]]))
   log(_([[Next comes House Dvaered that presents a more emotional defense of Dvaered prosperity, instead of relying on economic indicators, they use social analysis and questionnaire results to show increased levels of relative happiness compared to Za'lek territory, where objections by the Za'lek Lawyer are overruled. Finally, they bring a Dvaered expert to testify on how House Dvaered has a curtailing effect on piracy and unwanted behaviours. All in all, as expected from House Dvaered.]]))
   log(_([[Next is Maikki's turn...]]))
   vne.flashbackTextEnd{ characters={maikki} }

   maikki(fmt.f(_([["Your honour, I do not believe House Za'lek and House Dvaered are being honest with the courtroom. They paint a picture of roses and hearts, when the reality of their proposal is uniformity and subjugation. I would like to call {playername} to the stand as my witness to set the record straight on the unscrupulous behaviour and depravity of House Za'lek and Dvaered."]]),
      {playername=player.name()}))
   vn.na(_([[There is an audible gasp among the crowd with both the Za'lek lawyer and Dvaered representative pushing for objectives that quickly get overruled.]]))
   vn.na(_([[Before you have a chance to process what happened, you are ushered to a podium with a live microphone in front of you. The Empire rule codex is thrust under your hands, and you are forced to swear an oath of truthfulness. What could Maikki be thinking?]]))

   maikki(fmt.f(_([[{playername}, you can attest for the constant bickering and fighting between House Za'lek and House Dvaered at Minerva Station. Is it not true that you were dragged into a firefight right outside Minerva Station.]]),
      {playername=player.name()}))
   vn.menu( function ()
      return rnd.permutation{
         {_([["House Za'lek provoked House Dvaered."]]), "01_dv"},
         {_([["House Dvaered attacked House Za'lek."]]), "01_zl"},
         {_([["Both houses were picking fights."]]), "01_both"},
      }
   end )

   vn.label("01_dv")
   vn.func( function ()
      dv_points = dv_points+1
   end )
   vn.jump("01_cont")

   vn.label("01_zl")
   vn.func( function ()
      zl_points = zl_points+1
   end )
   vn.jump("01_cont")

   vn.label("01_both")
   vn.func( function ()
      pir_points = pir_points+1
   end )
   vn.jump("01_cont")

   vn.label("01_cont")
   -- Set positions
   vn.func( function ()
      zlk.pos = "farleft"
      dvd.pos = "farright"
   end )
   local helped = var.peek("minerva_altercation_helped")
   if helped == "dvaered" then
      vn.appear( zlk, "slideup" )
      zlk(fmt.f(_([["Objection! {playername} sided with the Dvaered and brutishly helped destroy a House Za'lek vessel!"]]),
         {playername=player.name()}))
      maikki(_([[Maikki frowns a bit.]]))
      vn.func( function ()
         dv_points = dv_points-1
      end )
      vn.disappear( zlk, "slideup" )
   elseif helped == "zalek" then
      vn.appear( dvd, "slideup" )
      dvd(fmt.f(_([["Objection! {playername} joined the fight with House Za'lek and backstabbed the Dvaered vessel!"]]),
         {playername=player.name()}))
      maikki(_([[Maikki frowns a bit.]]))
      vn.func( function ()
         zl_points = zl_points-1
      end )
      vn.disappear( dvd, "slideup" )
   else
      maikki(_([["What is important is that such fighting creates uncertainty, and insecurity at Minerva Station."]]))
   end
   maikki(_([["Was or was not Minerva Station infiltrated by House Dvaered, who went so far to plant a mole employee in the gambling operations?"]]))
   vn.na(_([[You state the facts you remember, avoiding mentioning the fact you helped kidnap them and so forth.]]))
   maikki(_([["Your honour, this establishes that House Dvaered was actively attempting to undermine Minerva Station even though it is under independence rule!"]]))
   maikki(_([["Furthermore, {playername}, is it true or not that House Za'lek established listening post in order to capture communications near Minerva Station and thus violate the sovereignty of the independent space?"]]))
   vn.na(_([[You once again state the facts, avoiding mentioning you were the one who blew it all up.]]))
   maikki(_([["See, Your Honour, not only can we put in doubt House Dvaered's ill intentions, House Za'lek was also undermining the independent of Minerva Station!"]]))
   maikki(_([["House Dvaered and House Za'lek can not be trusted, and the only way to ensure the local prosperity is to ensure the independence of Minerva Station."
Having finished her interrogation, Maikki sits down.]]))

   vn.func( function ()
      zlk.pos = "center"
      dvd.pos = "center"
   end )
   vn.scene()
   vn.newCharacter( dvd )
   vn.transition()
   if helped=="zalek" then
      dvd(fmt.f(_([["{playername}, although we already know you have a taste for House Dvaered, let us see how trustworthy you really are."]]),
         {playername=player.name()}))
   else
      dvd(fmt.f(_([["{playername}, let us see how trustworthy you really are."]]),
         {playername=player.name()}))
   end
   local scavengers_alive = var.peek( "maikki_scavengers_alive" )
   dvd(fmt.f(_([["There have been reports of scavengers missing at the {stealthsys} system, you wouldn't know anything about that, would you?"]]),
      {stealthsys=system.get("Zerantix")}))
   vn.menu( function ()
      return rnd.permutation{
         {_([["I have no idea what you are talking about."]]), "02_met"},
         {_([["I remember blowing up scavengers there."]]), "02_blowup"},
         {_([["I met some, but nothing happened."]]), "02_met"},
      }
   end )

   vn.label("02_met")
   dvd(fmt.f(_([["Not only did {playername} meet up wit the scavengers, they blew them up, and left their remains in the Nebula!"]]),
      {playername=player.name()}))
   if scavengers_alive then
      vn.appear( scv, "slideup" )
      scv(_([["We're not dead!"]]))
      vn.disappear( scv, "slideup" )
      dvd(_([[Murmurs spread across the room as the Dvaered Representative frowns.]]))
      vn.func( function ()
         zl_points = zl_points-1
      end )
   else
      dvd(fmt.f(_([["Here is the proof in the black box we recovered from the scavenger ships. We can see that the last moments clearly log {playername}'s ship attacking them."]]),
         {playername=player.name()}))
      vn.func( function ()
         pir_points = pir_points-1
      end )
   end
   vn.jump("02")

   vn.label("02_blowup")
   dvd(_([["So you admit to destroying civilian vessels in the Nebula, does this not make you a pirate? We ought to have a trial about your infamy, rather than the future of Minerva Station!"]]))
   if scavengers_alive then
      vn.appear( scv, "slideup" )
      scv(_([["We're not dead!"]]))
      vn.disappear( scv, "slideup" )
      dvd(fmt.f(_([[The Dvaered representative looks puzzled.
"{playername} may have not destroyed the scavengers as we have thought, but this does show that they do not speak the truth."]]),
         {playername=player.name()}))
      vn.func( function ()
         pir_points = pir_points+1
      end )
   end
   vn.jump("02")

   local gave_drink = var.peek("maikki_gave_drink")
   vn.label("02")
   if gave_drink then
      dvd(_([["I would like to ask about your relationship with the Independent Counsel, Maisie McPherson. What is your relationship?"]]))
      vn.na(_([[Maikki objects, but is overruled.]]))
      vn.menu( function ()
         return rnd.permutation{
            {_([["Just met her."]]), "03_justmet"},
            {_([["We are friends."]]), "03_friends"},
            {_([["I have seen her at Minerva Station."]]), "03_justmet"},
         }
      end )

      vn.label("03_justmet")
      dvd(_([["Then I would like to ask the witness what is this."]]))
      vn.na(_([[They show a clear picture of you buying Maikki a drink on Minerva station.]]))
      vn.func( function ()
         dv_points = dv_points+1
      end )
      vn.jump("03_friends")

      vn.label("03_friends")
      dvd(_([["I would like to question the impartiality of the witness. With such a close relationship with the independent counsel, it is likely they have ulterior motivations regarding their declarations."]]))
      vn.jump("03")
   end

   vn.label("03")
   dvd(_([["I have no further questions for the witness."]]))

   vn.scene()
   vn.newCharacter( zlk )
   vn.transition()
   zlk(_([[Finally, it is House Za'lek's interrogation.]]))
   if helped=="dvaered" then
      zlk(fmt.f(_([["{playername}, although you seem to have a preference for joining House Dvaered in brutish ways against House Za'lek. Let us put that aside to analyze your personality and lack of trustworthiness."]]),
         {playername=player.name()}))
   else
      zlk(fmt.f(_([["{playername}, let us analyze your personality and lack of trustworthiness."]]),
         {playername=player.name()}))
   end

   local harper_ticket = var.peek("harper_ticket") -- "credits", "tokens", "free", "stole"
   if harper_ticket=="free" or harper_ticket=="stole" then
      zlk(_([["Do you know an individual know as Harper Bowdown?"]]))
      vn.menu( function ()
         return rnd.permutation{
            {_([["Never met them."]]), "04_nomet"},
            {_([["They gave me their winning ticket."]]), "04_gave"},
            {_([["I took their winning ticket."]]), "04_took"},
         }
      end )

      vn.label("04_nomet")
      vn.func( function ()
         pir_points = pir_points-1
      end )
      zlk(_([["Not only do we have a testimony from Harper Bowdown that you met them, but that you forcibly took their possessions, a winning ticket from a Minerva Station raffle!"]]))
      vn.jump("04")

      vn.label("04_gave")
      vn.func( function ()
         pir_points = pir_points-1
      end )
      zlk(_([["You say they gave you their winning ticket, but we have a testimony from Harper Bowdown where they say you intimidated and took the ticket by force!"]]))
      vn.jump("cont04")

      vn.label("cont04_took")
      zlk(fmt.f(_([["Let it be noted that {playername} admits to using intimidation and force te deprive a legal Imperial citizen of their possessions!"]]),
         {playername=player.name()}))
      vn.jump("cont04")
   end

   vn.label("cont04")
   local strangelove_death = var.peek("strangelove_death") -- "unplug", "comforted", "shot", nil
   zlk(fmt.f(_([[The Za'lek Lawyer looks at their notes.
"Let us see, {playername}, you wouldn't know what happened to the Za'lek Scientist Dr. Strangelove?"]]),
   {playername=player.name()}))
   vn.menu( function ()
      return rnd.permutation{
         {_([["He got what he deserved."]]), "05_deserve"},
         {_([["I killed him."]]), "05_kill"},
         {_([["He passed away."]]), "05_dead"},
      }
   end )

   vn.label("05_dead")
   vn.label("05_deserve")
   if strangelove_death == "comforted" then
      zlk(fmt.f(_([["You see, {playername}, we were able to recover the black box from Dr. Strangelove's ship. The audio log is not clear, but it seems like you were there at that moment."]]),
         {playername=player.name()}))
   else
      vn.func( function ()
         pir_points = pir_points-1
      end )
      zlk(fmt.f(_([["You see, {playername}, we were able to recover the black box from Dr. Strangelove's ship. The audio log makes it clear that you were the one to end their life."]]),
         {playername=player.name()}))
   end
   vn.jump("05")

   vn.label("05_kill")
   --[[
   if strangelove_death == "comforted" then
      vn.func( function ()
         lied = lied+1
      end )
   end
   --]]
   vn.func( function ()
      zl_points = zl_points+1
   end )
   zlk(fmt.f(_([["Your honour, {playername} confesses to the murder of Dr. Strangelove."]]),
      {playername=player.name()}))
   vn.jump("05")

   vn.label("05")
   zlk(fmt.f(_([["Although the exact details of what happened between {playername} and Dr. Strangelove is not clear, that should be subject to another trial, what is clear is that {playername} can not be trusted and their testimony should be invalidated."]]),
      {playername=player.name()}))
   zlk(_([["That is all I have to say."
The Za'lek Lawyer sits down.]]))

   vn.scene()
   vn.newCharacter( judge )
   vn.transition()
   judge(fmt.f(_([["{playername}, you may return to your seat."]]),
      {playername=player.name()}))
   log = vne.flashbackTextStart(_("Narrator"))
   log(_([[The dispositions continue with more formalities with all sides calling for motions and objecting, however, it does not seem like much is being added to the arguments.

House Za'lek and House Dvaered seem to be generally on the passive, while Maikki is very aggressive pressing the houses without giving them much room to breath.

Eventually when all argumentation is exhausted the different representatives repeat their main points before judgement is passed.]]))
   vne.flashbackTextEnd()

   --[[
   The total points the player can get are below:

   zl_points in [-2,+2]
   dv_points in [-1,+2]
   pir_points in [-3,+2]
   --]]
   local winner
   vn.func( function ()
      -- Case House Za'lek "wins"
      if zl_points > dv_points and zl_points > pir_points then
         winner="zalek"
      -- Case House Dvaered "wins"
      elseif dv_points > zl_points and dv_points > pir_points then
         winner="dvaered"
      -- Last case "pirates" win
      else
         winner="independent"
      end
      var.push("minerva_judgement_winner", winner)
   end )
   judge(_([["With the deliberations over, it is time to pass judgement."]]))
   judge( function ()
      if winner=="zalek" then
         return _([["After carefully hearing the arguments presented by the different parties, sovereignty of Minerva Station shall be given to House Za'lek. It is a tough decision, however, House Za'lek offers the best guarantees to stability and prosperity for the station and the system."]])
      elseif winner=="dvaered" then
         return _([["After carefully hearing the arguments presented by the different parties, sovereignty of Minerva Station shall be given to House Dvaered. It is a tough decision, however, House Dvaered offers the best guarantees for security and stability for Minerva station and the system."]])
      else
         return _([["After carefully hearing the arguments presented by the different parties, sovereignty of Minerva Station shall be kept independent. It is a tough decision, however, independence offers the best continuity for Minerva Station. Additional supervision will be provided by the Empire until the situation improves."]])
      end
   end )
   judge(_([["This concludes..."]]))

   vn.scene()
   local angrysong = 'snd/sounds/songs/feeling-good-08.ogg'
   vn.music( angrysong )
   vn.newCharacter( kex )
   vn.transition("hexagon")
   kex(_([[Suddenly, Kex flies up to the roof and perches himself on a small indentation.]]))
   kex(_([["It's all a scam! Fraud! Fraud! Fraud! Fraud!"]]))
   vn.na(_([[The people in the room gasp, but don't know how to react to the abrupt development. Security guards draw their weapons, but are hesitant to shoot.]]))
   kex(_([["All this bickering and arguing, yet nothing really changes. Just greedy bastards trying to get their cut of the pie!"]]))
   kex(_([["You really think this is about Minerva Station? And not the experimental weapon laboratory hidden within?"]]))
   vn.na(_([[You can see the judge and House representatives shift around uncomfortably.]]))
   kex(_([["Well, it all ends here!"]]))
   vn.func( function () kex.shader = love_shaders.aura() end )
   vn.sfx( "snd/sounds/activate3.ogg" ) -- activation sound
   kex(_([[Kex's eyes glow red and you hear the activation sound of some sort of weapon.]]))
   vn.func( function () kex.shader = nil end )
   vn.disappear( kex, "slideup" )
   vn.sfx( "snd/sounds/crowdpanic01.ogg" ) -- some yelling
   vn.na(_([[As people start scrambling and yelling, you hear a shot and Kex falls down as chaos unfolds.]]))
   vn.sfx( "snd/sounds/autocannon.ogg" ) -- autocannon
   vn.na(_([[You hear shots being fired left and right as you duck for cover. The Judge's levitating desk crashes in the background creating a small explosion as things take a turn for the worst.]]))
   vn.menu{
      {_([[Go for the door.]]),"06_getout"},
      {_([[Try to find Kex.]]),"06_kex"},
   }

   vn.label("06_kex")
   vn.na(_([["You try to make your way to where Kex fell, but don't find them. As the smoke keeps on filling the room, you have no choice but to head towards to the door and try to save yourself. You jump over bodies and try to push yourself out of the room."]]))
   vn.jump("06")

   vn.label("06_getout")
   vn.na(_([[As smoke fills the room, you jump over bodies and try to push yourself out of the room.]]))
   vn.jump("06")

   vn.label("06")
   vn.na(_([[You hug the corridors and try to get away from the mayhem, trying to stick low so that you don't get caught by Imperial soldiers that are starting to swarm the area.]]))
   vn.na(_([[You are almost at the spaceport when suddenly you hear a rasping voice coming out from a side corridor.]]))

   vn.scene()
   -- Music is a bit slower and sadder
   love_audio.setEffect( "reverb_sad", reverb_preset.drugged() )
   vn.music( minerva.loops.pirate, {pitch=0.6, effect="reverb_sad"} )
   vn.newCharacter( zuri )
   vn.transition()
   zuri(fmt.f(_([["Hey {playername}..."
Zuri coughs a bit, she doesn't look like she's in good shape and seems to be clutching something.]]),
      {playername=player.name()}))
   vn.na(_([[You get closer and you see that she is covered in a fair amount of blood and seems to be holding onto Kex!]]))
   zuri(_([["Things didn't turn out quite how I expected..."
She grimaces in pain as she talks.]]))
   vn.na(_([[You shush her and try to help her stop her bleeding. This doesn't look too good.]]))
   zuri(_([["I'm going to have to ask another favour of you..."
She tries to adjust her position a bit to breath more easily, the pain is clear in her eyes.]]))
   zuri(_([["Give me a second to catch my breath and we can go."]]))

   vn.run()

   if didtrial then
      local winnerstr
      if winner=="zalek" then
         winnerstr = _([[ The judge ended up conceding Minerva Station to House Za'lek.]])
      elseif winner=="dvaered" then
         winnerstr = _([[ The judge ended up conceding Minerva Station to House Dvaered.]])
      else
         winnerstr = _([[ The judge ended up allowing Minerva Station to remain independent.]])
      end
      minerva.log.pirate(fmt.f(_([[You went with Zuri to participate in a trial to decide the future of Minerva Station. Maikki took the role of the Minerva Station defendant when the CEO passed out. You were called as a witness and your integrity was question.{winnerstr} Suddenly, Kex interrupted claimed that Minerva Station was a secret weapon laboratory, before trying to activate a weapon. This caused panic and a firefight to break out. You managed to escape with your life and found a seriously injured Zuri with an unconscious Kex who now need help getting off of {spb}.]]),
         {winnerstr=winnerstr, spb=trialspb}))
      misn.finish(true)
   end
end

-- Make sure can land on the Jade Court
function enter ()
   if system.cur()~=trialsys then return end
   trialspb:landOverride(true)
end
