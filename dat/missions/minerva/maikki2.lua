--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Maikki's Father 2">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>0</chance>
   <location>Bar</location>
   <planet>Minerva Station</planet>
   <done>Maikki's Father 1</done>
  </avail>
 </mission>
--]]

--[[
-- Maikki (Maisie McPherson) asks you to find her father, the famous pilot Kex
-- McPherson. She heard rumours he was still alive and at Minerva station.
-- Player found out that Za'lek doing stuff with the ship cargo.
--
-- 1. Told to try to find who could have been involved and given three places to look at.
-- Hint 1: Jorlas in Regas (University)
-- Hint 2: Cantina Station in Qulam (Trade Hub)
-- Hint 3: Jurai in Hideyoshi's Star (University)
-- 2. After talking to all three, the player is told to go to Naga in Damien (Underwater University)
-- 3. Mentions eccentric colleague at Westhaven
-- 4. Eccentric colleague makes the player mine some stupid stuff for him.
-- 5. Tells you he saved him and sent him off to "Minerva Station".
-- 6. Go back and report to Maikki confirming her info.
--
-- Eccentric Scientist in "Westhaven" (slightly senile with dementia).
--]]
local minerva = require "minerva"
local portrait = require 'portrait'
local vn = require 'vn'
require 'numstring'

maikki_name = minerva.maikki.name
maikki_description = minerva.maikki.description
maikki_portrait = minerva.maikki.portrait
maikki_image = minerva.maikki.image
maikki_colour = minerva.maikki.colour

hint1_name = _("Prof. Sato")
hint1_description = _("Foo")
hint1_portrait = "zalek1"
hint1_image = "zalek1.png"
hint1_colour = nil

hint2_name = _("Prof. Stova")
hint2_description = _("Foo")
hint2_portrait = "zalek1"
hint2_image = "zalek1.png"
hint2_colour = nil

hint3_name = _("Dr. Cayne")
hint3_description = _("Foo")
hint3_portrait = "zalek1"
hint3_image = "zalek1.png"
hint3_colour = nil

hint4_name = _("Prof. Hsu")
hint4_description = _("Foo")
hint4_portrait = "zalek1"
hint4_image = "zalek1.png"
hint4_colour = nil

ecc_name = _("Prof. Strangelove")
ecc_description = nil -- unneeded
ecc_portrait = "zalek1"
ecc_image = "zalek1.png"
ecc_colour = nil

misn_title = _("Finding Father")
misn_reward = _("???")
misn_desc = _("Maikki wants you to help her find her father.")

hintpnt = {
   "Jorlan",
   "Cantina Station",
   "Jurai",
   "Naga",
}
hintsys = {}
for k,v in ipairs(hintpnt) do
   hintsys[k] = planet.get(v):system():nameRaw()
end
eccpnt = "Strangelove Lab"
eccdiff = "strangelove"
eccsys = "Westhaven"

-- Mission states:
--  nil: mission not yet accepted
--    0: Going to the three hints
--    1: Go to fourth hint
--    2: Go to westhaven
--    3: Found base
--    4: Mining stuff
--    5: Going back to Minerva Station
misn_state = nil


function create ()
   if not misn.claim( system.get(eccsys) ) then
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

   -- Set up mission stuff
   markerhint1 = misn.markerAdd( system.get(hintsys[1]), "low")
   markerhint2 = misn.markerAdd( system.get(hintsys[2]), "low")
   markerhint3 = misn.markerAdd( system.get(hintsys[3]), "low")
   hintosd()
   hook.land( "land" )
   hook.enter( "enter" )

   -- Re-add Maikki if accepted
   land()
end


function hintosd ()
   local osd = {
      _("Investigate the Za'lek"),
   }
   local function addhint( id )
      table.insert( osd, string.format(_("\tGo to %s in %s"), _(hintpnt[id]), _(hintsys[id])) )
   end

   if misn_state==0 then
      if not visitedhint1 then
         addhint(1)
      end
      if not visitedhint2 then
         addhint(2)
      end
      if not visitedhint3 then
         addhint(3)
      end
   elseif misn_state==1 then
      addhint(4)
   end

   misn.osdCreate( misn_title, osd )
end



function approach_maikki ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikki() )
   local function seekoutmsg ()
      maikki(string.format(_([["The three Za'lek researchers you should seek out are:
%s in the %s system,
%s in the %s system,
and %s in the %s system."]]),
         hint1_name, hintsys[1],
         hint2_name, hintsys[2],
         hint3_name, hintsys[3]
      ))
   end
   vn.fadein()

   vn.na(_("You approach Maikki who seems to have a fierce determination in her look."))
   if misn_state==nil then
      maikki(_([[She looks encouraged by your findings in the nebula.
"From what you told me, I think I have a good idea for our next lead. Would you be interested in helping again?"]]))
      vn.menu( {
         { _("Help Maikki again"), "accept" },
         { _("Decline to help"), "decline" },
      } )
      vn.label( "decline" )
      vn.na(_("You feel it is best to leave her alone for now and disappear into the crowds leaving her once again alone to her worries."))
      vn.done()

      vn.label( "accept" )
      vn.func( function ()
         misn_accept()
         misn_state = 0
      end )
      maikki(_([["I think we should be able to find out what happened to my father's ship in the nebula. It seems like someone is very interested on stuff that is being found in the nebula and is behind the scavengers you met. Whoever is behind them could also be related to whatever happened to the ship in the first place."]]))
      maikki(_([["I ran a check on Za'lek researchers that would likely be interested in stuff taken from the nebula and there seems to be less than I imagined. I was able to get a list of three researchers. I'll give you the details on them and you should pay them a visit to see if they know anything."]]))
      maikki(_([[She leans in close to you and looks into your eyes with fierce determination.
"You will do whatever it takes to find him right?"]]))
      vn.menu( {
         { _([["Yes!"]]), "doall" },
         { _([["Maybe..."]]), "dosome" },
         { _([["No"]]), "donone" },
      } )
      vn.label("doall")
      vn.func( function ()
         minerva.maikki_mood_mod( 1 )
      end )
      maikki(_([["You know the Za'lek can be very stubborn at times and may need some convincing."
She winks at you.]]))
      vn.jump("introcont")

      vn.label("dosome")
      maikki(_([["i thought that after your experience in the nebula you would understand how dangerous this can get."]]))
      vn.jump("introcont")

      vn.label("donone")
      vn.func( function ()
         minerva.maikki_mood_mod( -1 )
      end )
      maikki(_([[She glares at you.
"I hope you're joking. This is very important to me and I hope you don't lose this lead..."]]))
      vn.jump("introcont")

      vn.label("introcont")
      seekoutmsg()
      maikki(_([["Apparently the three researchers used to work together in some project about the nebula origins, but never really made anything public. I totally think that they must know something about this, the disappearance of my father was big news!"]]))
      maikki(_([[She looks at you expectantly.
"I will stay here and search for him more. Please pay those creepy Za'lek researchers a visit and see if you can find out what happened to my father!"]]))
   end

   local opts = {
      {_("Ask about the researchers"), "researchers"},
      {_("Ask about her father (Kex)"), "father"},
      {_("Leave"), "leave"},
   }
   -- TODO more options as more researchers are found
   vn.label( "menu" )
   vn.menu( opts )

   vn.label( "researchers" )
   maikki(_([["The researchers I told you about were involved in some sort of project trying to find out the origins of the incident by analyzing artifacts taken from the nebula. Most of it is classified so I wasn't able to get much information."]]))
   maikki(_([["Apparently the project was disbanded for some reason or other, and some members went missing. However, I was able to track down a few of them, but it wasn't too easy. They have some weird system called tenure that nobody understands and makes them move constantly from research laboratory to research laboratory. So droll!"]]))
   seekoutmsg()
   maikki(_([["I don't think it will be easy for you to get information from the Za'lek, they never give a straight answer and it's all "that is illogical" and "ma'am that beaker of acid is not a toy". So boring!"
She frowns and shakes her head to the sides.]])
   -- TODO messages regarding each of the researchers
   vn.jump( "menu_msg" )

   vn.label( "father" )
   maikki(_([["I don't remember much about my father, most of what I know about him is stories from my late mother and stories told by pilots who knew him."]]))
   maikki(_([[Her eyes light up.
"Did you know that while scavenging near the incident, an asteroid hit his thrusters making them point the wrong way and he had to fly through five systems backwards through the nebula while chased by pirates? Sounds crazy!"]]))
   maikki(_([["He was also said to be the first pilot to chart the entire systems in the Nebula after the incident. Not even getting his leg crushed while recovering parts of the last emperor's flagship stopped him from going back and back to the nebula."]]))
   maikki(_([[Her eyes darken a little and her voice softens.
"His obsession with finding the truth did mean he was away from home most of the time and my mother was probably very lonely. She told me he never came to any of my birthdays..."]]))
   maikki(_([[Her eyes light up again.
"However, those were exceptional times. Most of the universe and our history as humans was lost! I would like to love to be able to meet him again and be able to ask all sorts of things."]]))
   maikki(_([["After you finding his ship in the nebula we have to get to the bottom of what happened! There is no way this was an accident, and I'm sure the Za'lek are involved in this. Since you didn't find a body, he has to be alive! We have to find out what happened and set things right!"
Her eyes sparkle with determination.]]))
   vn.jump( "menu_msg" )

   vn.label( "menu_msg" )
   maikki(_([["Is there anything you would like to know about?"]]))
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.na(_("You take your leave to continue the search for her father."))
   vn.fadeout()
   vn.run()
end


function land ()
   if planet.cur() == planet.get("Minerva Station") then
      npc_maikki = misn.npcAdd( "approach_maikki", minerva.maikki.name, minerva.maikki.portrait, minerva.maikki.description )

   elseif planet.cur() == planet.get( hintpnt[1] ) then
      npc_hint1 = misn.npcAdd( "approach_hint1", hint1_name, hint1_portrait, hint1_description )

   elseif planet.cur() == planet.get( hintpnt[2] ) then
      npc_hint2 = misn.npcAdd( "approach_hint2", hint2_name, hint2_portrait, hint2_description )

   elseif planet.cur() == planet.get( hintpnt[3] ) then
      npc_hint3 = misn.npcAdd( "approach_hint3", hint3_name, hint3_portrait, hint3_description )

   elseif misn_state >= 1 and  planet.cur() == planet.get( hintpnt[3] ) then
      npc_hint4 = misn.npcAdd( "approach_hint4", hint4_name, hint4_portrait, hint4_description )

   elseif diff.isApplied(eccdiff) and planet.cur() == planet.get(eccpnt) then
      npc_ecc = misn.npcAdd( "approach_eccentric", ecc_name, ecc_portrait, ecc_description )

   end
end


function visited ()
   if misn_state==0 and visitedhint1 and visitedhint2 and visitedhint3 then
      misn_state = 1
      markerhint4 = misn.markerAdd( system.get(hintsys[4]) )
   end
   hintosd()
end


function approach_hint1 ()
   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint1_name, { image=hint1_image, color=hint1_colour } )
   vn.fadein()

   prof([["Blah"]])

   vn.fadeout()
   vn.run()

   if not visitedhint1 then
      visitedhint1 = true
      misn.markerRm( markerhint1 )
      visited()
   end
end


function approach_hint2 ()
   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint2_name, { image=hint2_image, color=hint2_colour } )
   vn.fadein()

   prof([["Blah"]])

   vn.fadeout()
   vn.run()

   if not visitedhint2 then
      visitedhint2 = true
      misn.markerRm( markerhint2 )
      visited()
   end
end


function approach_hint3 ()
   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint3_name, { image=hint3_image, color=hint3_colour } )
   vn.fadein()

   prof([["Blah"]])

   vn.fadeout()
   vn.run()

   if not visitedhint3 then
      visitedhint3 = true
      misn.markerRm( markerhint3 )
      visited()
   end
end


function approach_hint4 ()
   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint4_name, { image=hint4_image, color=hint4_colour } )
   vn.fadein()

   prof([["Blah"]])

   vn.fadeout()
   vn.run()

   if misn_state==1 then
      misn_state = 2
      misn.markerRm( markerhint4 )
      misn.markerAdd( system.get(eccsys), "low" )
      misn.osdCreate( misn_title, {string.format(_("\tGo to %s"), _(eccsys))} )
   end
end


function approach_eccentric ()
end


function enter ()
   if system.cur() == system.get(eccsys) then
      -- TODO security protocol
      diff.apply( eccdiff )
   end
end

