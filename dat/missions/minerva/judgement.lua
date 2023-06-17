--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Judgement">
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 6</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</event>
--]]

--[[
   * Player's testimony gets called into question if killed harper or let scavengers live
   * Kex deciphers final stuff and blurts out important thing to get CEO convicted at the end, but by doing that shows off he is an intelligent being and gets sentenced to dissection by the Za'lek.
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local vni = require 'vnimage'
local fmt = require "format"
--local lmisn = require "lmisn"
--local love_shaders = require "love_shaders"

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
   misn.finish(false)

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

   hook.land("land")
   hook.enter("enter")
end

-- Make sure can land on the Jade Court
function enter ()
   if mem.state == 0 then
      if system.cur()~=trialsys then return end
      trialspb:landOverride(true)
      return
   end

   -- TODO cutscene post-trial
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

   -- Main characters
   local zuri = minerva.vn_zuri()
   local kex = minerva.vn_kex()
   local maikki = minerva.vn_maikki()
   local ceo = minerva.vn_ceo()
   local judge = vn.Characters.new( _("Judge Holmes"), {image=vni.genericFemale()} ) -- TODO replace graphics
   local zlk = vn.Character.new( _("Za'lek Lawyer"), { image=zalek_image, color=zalek_colour } )
   local dvd = vn.Character.new( _("Dvaered Representative"), { image=dvaered_image, color=dvaered_colour } )

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
   -- vn.music( ) -- TODO trial music
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
   judge(_([["We are gathered here to day to deliberate over the future of Minerva Station, which has been accused of falling into debauchery and lawlessness."]]))

   vn.func( function ()
      zlk.pos = "farleft"
      dvd.pos = "farright"
      ceo.pos = "farleft"
      maikki.pos = "farleft"
   end )
   judge(_([["The three parties involved are House Za'lek..."]]))
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
She gives an impeccable formal bow.]]))
   maikki(_([["I would like to submit an EJ-7777 motion to substitute the Minera CEO." ]]))
   judge(_([[The judge makes a curious expression and hovers down to collect the papers from Maikki. She quickly skims over it, and gives a sly grin.]]))
   judge(_([["Well it seems like the deliberations will be able to proceed today. Replacing the previous individual, we have Maisie McPherson, who will be representing the independent interests of Minerva Station. Please sit down."]]))
   vn.disappear( maikki, "slideup" )

   vn.run()

   -- Just finish if the player didn't actually go through with it
   if not didtrial then return end

   -- Should takeoff and play cutscene
   mem.state = 1 -- state update ensures the trial doesn't repeat and the player can load a game directly into the cutscene
   player.takeoff()
end
