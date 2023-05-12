--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Maikki's Father 2">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Maikki's Father 1</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
-- Maikki (Maisie McPherson) asks you to find her father, the famous pilot Kex
-- McPherson. She heard rumours he was still alive and at Minerva station.
-- Player found out that Za'lek doing stuff with the ship cargo.
--
-- 1. Told to try to find who could have been involved and given three places to look at.
-- Hint 1: Jorla in Regas (University)
-- Hint 2: Cantina Center in Qulam (Trade Hub)
-- Hint 3: Jurai in Hideyoshi's Star (University)
-- 2. After talking to all three, the player is told to go to Naga in Damien (Underwater University)
-- 3. Mentions eccentric colleague at Westhaven
-- 4. Eccentric colleague makes the player mine some stupid stuff for him.
-- 5. Tells you he saved him and sent him off to "Minerva Station".
-- 6. Go back and report to Maikki confirming her info.
--
-- Eccentric Scientist in "Westhaven" (slightly senile with dementia).
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local love_shaders = require 'love_shaders'
local fmt = require "format"

local hint1_name = _("Prof. Sato") -- Computer Science / Mathematics
local hint1_portrait = "zalek1.png"
local hint1_image = "zalek1.png"
local hint1_colour = nil

local hint2_name = _("Prof. Stova") -- Material Science
local hint3_portrait = "zalek3.webp"
local hint3_image = "zalek3.webp"
local hint2_colour = nil

local hint3_name = _("Prof. Hsu") -- Philosophy
local hint2_portrait = "zalek2.png"
local hint2_image = "zalek2.png"
local hint3_colour = nil

local hint4_name = _("Dr. Cayne") -- Dr. Shrimp
local hint4_portrait = "drshrimp.png"
local hint4_image = "drshrimp.png"
local hint4_colour = nil

local ecc_portrait = minerva.strangelove.portrait

local hintpnt = {
   "Jorla",
   "Cantina Center",
   "Jurai",
   "Naga",
}
local hintsys = {}
for k,v in ipairs(hintpnt) do
   hintsys[k] = spob.get(v):system()
end
local eccpnt = "Strangelove Lab"
local eccdiff = "strangelove"
local eccsys = system.get( "Westhaven" )
local eccpos = vec2.new( 7500, -6000 ) -- Should coincide with "Strangelove Lab"

-- Mission states:
--  nil: mission not yet accepted
--    0: Going to the three hints
--    1: Go to fourth hint
--    2: Go to westhaven
--    3: Found base
--    4: Destroy drones
--    5: Got the artefacts
--    6: Going back to Minerva Station
mem.misn_state = nil
local defense_systems, feral_drone_boss -- Non-persistent state
local hintosd -- Forward-declared functions

function create ()
   if not misn.claim( eccsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.maikki.name, minerva.maikki.portrait, minerva.maikki.description )
   misn.setDesc( _("Maikki wants you to help her find out what happened to her father.") )
   misn.setReward( _("???") )
   misn.setTitle( _("Finding Maikki's Father") )
end


function accept ()
   approach_maikki()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   -- Set up mission stuff
   mem.markerhint1 = misn.markerAdd( spob.get(hintpnt[1]), "low")
   mem.markerhint2 = misn.markerAdd( spob.get(hintpnt[2]), "low")
   mem.markerhint3 = misn.markerAdd( spob.get(hintpnt[3]), "low")
   hintosd()
   hook.land("generate_npc")
   hook.load("generate_npc")
   hook.enter("enter")

   -- Re-add Maikki if accepted
   generate_npc()
end


function hintosd ()
   local osd = {
      _("Investigate the Za'lek"),
   }
   local function addhint( id )
      table.insert( osd, fmt.f(_("\tGo to {pnt} in {sys}"), {pnt=_(hintpnt[id]), sys=hintsys[id]}) )
   end

   if mem.misn_state==0 then
      if not mem.visitedhint1 then
         addhint(1)
      end
      if not mem.visitedhint2 then
         addhint(2)
      end
      if not mem.visitedhint3 then
         addhint(3)
      end
   elseif mem.misn_state==1 then
      addhint(4)
   else
      return
   end

   misn.osdCreate( _("Finding Maikki's Father"), osd )
end



function approach_maikki ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikki() )
   vn.music( minerva.loops.maikki )
   local function seekoutmsg ()
      maikki(fmt.f(_([["The three Za'lek researchers you should seek out are:
{name1} in the {sys1} system,
{name2} in the {sys2} system,
and {name3} in the {sys3} system."]]), {
         name1=hint1_name, sys1=hintsys[1],
         name2=hint2_name, sys2=hintsys[2],
         name3=hint3_name, sys3=hintsys[3],
      }))
   end
   vn.transition("hexagon")

   vn.na(_("You approach Maikki who seems to have a fierce determination in her eyes."))
   if mem.misn_state==nil then
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
         misn.accept()
         mem.misn_state = 0
         minerva.log.maikki(_("You agreed to continue helping Maikki find her father. She told you to try to find hints from three Za'lek researchers.") )
      end )
      maikki(_([["I think we should be able to find out what happened to my father's ship in the nebula. It seems like someone is very interested on stuff that is being found in the nebula and is behind the scavengers you met. Whoever is behind them could also be related to whatever happened to the ship in the first place."]]))
      maikki(_([["I ran a check on Za'lek researchers that would likely be interested in stuff taken from the nebula and there seems to be fewer than I imagined. I was able to get a list of three researchers. I'll give you the details on them and you should pay them a visit to see if they know anything."]]))
      maikki(_([[She leans in close to you and looks into your eyes with fierce determination.
"You will do whatever it takes to find him right?"]]))
      vn.menu( {
         { _([["Yes!"]]), "doall" },
         { _([["Maybe…"]]), "dosome" },
         { _([["No"]]), "donone" },
      } )
      vn.label("doall")
      vn.func( function ()
         var.push("maikki_response","yes")
      end )
      maikki(_([["You know the Za'lek can be very stubborn at times and may need some convincing."
She winks at you.]]))
      vn.jump("introcont")

      vn.label("dosome")
      maikki(_([["I thought that after your experience in the nebula you would understand how dangerous this can get."]]))
      vn.func( function ()
         var.push("maikki_response",nil)
      end )
      vn.jump("introcont")

      vn.label("donone")
      vn.func( function ()
         var.push("maikki_response","no")
      end )
      maikki(_([[She glares at you.
"I hope you're joking. This is very important to me and I hope you don't lose this lead…"]]))
      vn.jump("introcont")

      vn.label("introcont")
      seekoutmsg()
      maikki(_([["Apparently the three researchers used to work together in some project about the nebula's origins, but never really made anything public. I totally think that they must know something about this; the disappearance of my father was big news!"]]))
      maikki(_([[She looks at you expectantly.
"I will stay here and continue to search for him. Please pay those creepy Za'lek researchers a visit and see if you can find out what happened to my father!"]]))
   end

   local opts = {
      {_("Ask about the researchers"), "researchers"},
      {_("Ask about her father (Kex)"), "father"},
      {_("Leave"), "leave"},
   }
   -- TODO more options as more researchers are found
   if mem.misn_state==6 then
      table.insert( opts, 1, {_("Tell her want you found"), "news"} )
   end
   vn.label( "menu" )
   vn.menu( opts )

   vn.label( "researchers" )
   maikki(_([["The researchers I told you about were involved in some sort of project trying to find out the origins of the Incident by analyzing artefacts taken from the nebula. Most of it is classified so I wasn't able to get much information."]]))
   maikki(_([["Apparently the project was disbanded for some reason or other, and some members went missing. However, I was able to track down a few of them, but it wasn't too easy. They have some weird system called tenure that nobody understands and makes them move constantly from research laboratory to research laboratory. So droll!"]]))
   seekoutmsg()
   maikki(_([["I don't think it will be easy for you to get information from the Za'lek. They never give a straight answer and it's all "that is illogical" and "ma'am that beaker of acid is not a toy". So boring!"
She frowns and shakes her head.]]))
   -- TODO messages regarding each of the researchers
   vn.jump( "menu_msg" )

   vn.label( "father" )
   maikki(_([["I don't remember much about my father, most of what I know about him are from stories from my late mother and pilots who knew him."]]))
   maikki(_([[Her eyes light up.
"Did you know that while scavenging near the Incident, an asteroid hit his thrusters making them point the wrong way and he had to fly backwards through five systems in the nebula while chased by pirates? Sounds crazy!"]]))
   maikki(_([["They say he was also the first pilot to chart all the systems in the Nebula after the Incident. Not even getting his leg crushed while recovering parts of the last emperor's flagship stopped him from going back and back to the nebula."]]))
   maikki(_([[Her eyes darken a little and her voice softens.
"His obsession with finding the truth meant he was away from home most of the time and my mother was probably very lonely. She told me he never came to any of my birthdays…"]]))
   maikki(_([[Her eyes light up again.
"However, those were exceptional times. Most of the universe and our history as humans was lost! I would love to be able to meet him again and be able to ask him all sorts of things."]]))
   maikki(_([["Since you found his ship in the nebula we have to get to the bottom of what happened! There is no way this was an accident, and I'm sure the Za'lek are involved in this. Since you didn't find a body, he has to be alive! We have to find out what happened and set things right!"
Her eyes sparkle with determination.]]))
   vn.jump( "menu_msg" )

   vn.label("news")
   vn.na(_("You tell her about your plight with the Za'lek researchers, and Dr. Strangelove, in particular. This includes the good news that Kex should be at Minerva Station, taken into custody by some thugs."))
   maikki(_([["The Za'leks were assholes as expected, but I'm glad you found out what I thought: that he's somewhere here being held captive or something… This is still all very weird though."]]))
   maikki(_([["I'm a bit worried about that Dr. Strangelove. What exactly did he mean? Is he even a real doctor? We still don't even know what happened in the nebula nor what they want with my father. Instead of answering questions we keep on finding new questions. I suppose that this is progress?…"]]))
   maikki(_([["Anyway, if we can believe Dr. Strangelove, my father is alive and somewhere here! I don't think we have any reason to doubt him. Za'leks don't tend to lie, they only bend the truth. All we have to do is find my father now and everything should fall in place."]]))
   maikki(_([[She is visibly excited.
"It is all coming together! I will finally be able to meet him again! This is so great! I don't know what to tell him first. Do you think he'll recognize me?"]]))
   maikki(_([["I don't have a lead at the moment, but it can't be far. Keep your eyes open and if you find anything that looks suspicious, please get in touch with me! I'll also be keeping my eyes open."]]))
   maikki(_([["Oh, I almost forgot. I got lucky with a Pachinko machine and won a lot of Minerva Tokens and a lifetime supply of parfaits! Since I don't need the tokens to buy anything anymore, here, you can take them."]]))
   vn.sfxVictory()
   vn.func( function ()
      -- no reward, yet...
      mem.mission_finish = true
      minerva.tokens_pay( 500 ) -- roughly 1M if you consider winning rates
      minerva.log.maikki(_("You reported to Maikki what Dr. Strangelove told you about her father. She doesn't have any leads at the moment, but it does seem like he is at Minerva Station." ) )
   end )
   vn.na(fmt.reward(minerva.tokens_str(500)))
   maikki(_([["I'll be around here if you find anything."]]))
   vn.na(_("You take your leave. Without any leads, it might prove hard to find where Kex is. You wonder what your next steps should be…"))
   vn.done("hexagon")

   vn.label( "menu_msg" )
   maikki(_([["Is there anything you would like to know about?"]]))
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.na(_("You take your leave to continue the search for her father."))
   vn.done("hexagon")
   vn.run()

   -- Can't run it in the VN or it causes an error
   if mem.mission_finish then
      misn.finish(true)
   end
end


function generate_npc ()
   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_maikki", minerva.maikki.name, minerva.maikki.portrait, minerva.maikki.description )

   elseif spob.cur() == spob.get( hintpnt[1] ) then
      misn.npcAdd( "approach_hint1", hint1_name, hint1_portrait, _("You see a person in a fancy lab coat. It seems like they are enjoying their time off.") )

   elseif spob.cur() == spob.get( hintpnt[2] ) then
      misn.npcAdd( "approach_hint2", hint2_name, hint2_portrait, _("You see a person in a fancy lab coat. It seems like they are enjoying their time off.") )

   elseif spob.cur() == spob.get( hintpnt[3] ) then
      misn.npcAdd( "approach_hint3", hint3_name, hint3_portrait, _("You see a person in a fancy lab coat. It seems like they are enjoying their time off.") )

   elseif mem.misn_state >= 1 and  spob.cur() == spob.get( hintpnt[4] ) then
      misn.npcAdd( "approach_hint4", hint4_name, hint4_portrait, _("You see a young fellow intently reading a book. There seems to be a shrimp floating in a nearby aquarium bowl.") )

   elseif diff.isApplied(eccdiff) and spob.cur() == spob.get(eccpnt) and mem.misn_state < 6 then
      mem.npc_ecc = misn.npcAdd( "approach_eccentric", _("Hologram Projector"), ecc_portrait, _("An old decrepit hologram projector sits in the corner. It looks like you could use this to communicate with the owner of the station.") )
   end
end


local function visitedhints ()
   local visits = 0
   if mem.visitedhint1 then
      visits = visits + 1
   end
   if mem.visitedhint2 then
      visits = visits + 1
   end
   if mem.visitedhint3 then
      visits = visits + 1
   end
   return visits
end


local function visited ()
   if mem.misn_state==0 and visitedhints()==3 then
      mem.misn_state = 1
      mem.markerhint4 = misn.markerAdd( spob.get(hintpnt[4]), "low" )
      minerva.log.maikki(_("You met the three researchers that Maikki told you about and found out a lead about another researcher.") )
   end
   hintosd()
end


local function lasthint( prof )
   if visitedhints() > 2 then
      vn.sfxBingo()
      prof(fmt.f(_([["Oh, I suddenly remembered. There was also a post-doctoral research candidate working on the project by the name of Cayne. I think he was last working at {pnt} in the {sys} system."]]), {pnt=_(hintpnt[4]), sys=hintsys[4]}))
      -- The mission state will be updated afterwards
   end
end


function approach_hint1 ()
   if not mem.visitedhint1 then
      mem.visitedhint1 = true
      misn.markerRm( mem.markerhint1 )
      visited()
   end

   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint1_name, { image=hint1_image, color=hint1_colour } )
   vn.transition()

   vn.na(_("You approach the professor."))
   prof(_([["Hello, how can I help you?"]]))
   vn.na(_("You inquire to about whether or not they are interested in artefacts from the nebula."))
   prof(_([["Very interesting, but I stopped working on that line of research several cycles ago. It was really a mess of a project. The project leader was pushing us very hard due to the other competition."]]))
   prof(_([["In the end, some papers' results were falsified and the project leader was barred from doing research. That was too much for them and they took their own life. How illogical!"]]))
   prof(_([["If you are interested in nebula research, you should go visit the museum of nebula artefacts. Although due to administrative changes, I don't think it'll open again for a few cycles."]]))
   prof(_([["Oh, you might have good luck talking to other members of the project. However, most of us have moved on from that traumatic experience and are now researching new topics."]]))
   prof(_([["Did I mention I have started working with infinitely meta-recursive hyper-tables? They are a fascinating type of data structure based on self-referencing Grassman manifold quasi-projections. They even implicitly handle non-convex elliptic reflections without any meta-heuristics!"
They are getting excited.]]))
   prof(_([["Although my grant has been rejected five times, the damn review board is still pushing the obsolete Riemann algebroid universal approximator theory and won't listen to anything else, they are definitely the future! I think they may even have a practical application!"
Their excitement grows.]]))
   vn.na(_("They don't seem like they will stop talking anytime soon… You take your leave as they start rambling in a trance-like state."))
   lasthint( prof )

   vn.run()
end


function approach_hint2 ()
   if not mem.visitedhint2 then
      mem.visitedhint2 = true
      misn.markerRm( mem.markerhint2 )
      visited()
   end

   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint2_name, { image=hint2_image, color=hint2_colour } )
   vn.transition()

   vn.na(_("You approach the professor."))
   prof(_([["What can I do for you?"]]))
   vn.na(_("You inquire to about whether or not they are interested in artefacts from the nebula."))
   prof(_([[They rubs their temples.
"Ah, the nebula artefacts… Not very good experiences with those, no, not really interested in them anymore."]]))
   prof(_([["I was part of a really ambitious project to try to track down the origin of the Incident, which just end up being a bureaucratic nightmare."]]))
   prof(_([["You see, the bigger the research project, the more the project leader and co-leaders want to have meetings and goals and work packages."
They shudder when they says the word "work packages".]]))
   prof(_([["At the end, we were just chaining meetings and answering e-mails and not getting anything done. We ended up handing in a 5-page report written by an intern for the mid-project evaluation and almost lost all the funding. The project leader panicked and it all went to hell from there."]]))
   prof(_([["All nebula research is doomed to end that way. I haven't seen a project succeed yet, despite all the proposal calls attempting to address the issue."]]))
   vn.na(_("You inquire about other project members."))
   prof(_([["The only logical step after that traumatic experience is to get as far away as possible from nebula research. As far as I know all the project members moved to new topics, although none as exciting as mine."]]))
   prof(_([["Have you heard of graphene infused hydro nano lattices? By taking sheets of graphene and creating strong lateral strain under the effect of intense Coriolis electro-magnetic induction fields, it is possible to obtain a behaviour similar to turbulent hydrofoils. And what's even better is that they naturally form octahedral quasi-lattices that allow absorbing most low-spectrum frequencies!"]]))
   prof(_([["They could change material science as we know it! Imagine being able to create materials with almost any property you desire! We do still have to solve the problem of the subatomic crystallite implosion, but once I finish recovering the notes from the crater left at my last laboratory, I should be able to solve it in no time. By the way, since the accident with my last assistant, I'm looking for a new one. Would you be interested?"]]))
   vn.na(_("You get away as fast as you can from them as they keep on rambling."))
   lasthint( prof )

   vn.run()
end


function approach_hint3 ()
   if not mem.visitedhint3 then
      mem.visitedhint3 = true
      misn.markerRm( mem.markerhint3 )
      visited()
   end

   vn.clear()
   vn.scene()
   local prof = vn.newCharacter( hint3_name, { image=hint3_image, color=hint3_colour } )
   vn.transition()

   vn.na(_("You approach the professor."))
   prof(_([["Hello."]]))
   vn.na(_("You inquire to about whether or not they are interested in artefacts from the nebula."))
   prof(_([["Ah, such fond memories. I have never been interested in the artefacts from the nebula themselves, but I was part of a large project dealing with them as part of the philosophical and ethical committee."]]))
   prof(_([["It is a very fascinating topic. Since there is no way to prove the existence of an absolute reality, from a practical point of view, there are infinite relative realities, where everyone has their own. Everything else is an illusion derived from us projecting our understanding and reality onto the reality of others, deforming them to a way we can interpret them with our imperfect minds."]]))
   prof(_([["Recently, due to the illogical imbalance between science and humanities, all large Za'lek projects are required to have at least a 10% of humanities members. This has given me lots of opportunities to work on many interesting projects."]]))
   prof(_([["Oh yes, on the nebula artefacts. I remember having fun conversations with our colleagues about whether or not the artefacts from the nebula actually do exist or not given that we are using imperfect sensory organs to see, touch, and feel them. You see, given that we only perceive reality through our imperfect organs, can we actually know whether or not there is an absolute objective reality?"]]))
   vn.na(_("You inquire about the other members of the project."))
   prof(_([["Other members? I recall a few, but they liked me so much, they promoted me and gave me a windowless room in the basement. What an honour! Anyway, back to what I was saying."]]))
   prof(_([["We must also further question not only our sensory organs, and the existence of nebula artefacts, but our own existence. We apparently are able to think and some would argue that by this, the only thing we can prove, given imperfect sensory data, is our own existence. However, I argue that this too is an illusion, and that our existence itself is something we can't prove."]]))
   prof(_([["Every instant, what we perceive as ourselves is ceasing to exist and a new existence, which, while very close to the "ourselves" from the previous instance, is, arguably, a completely new existence. This fluidity makes it not only impossible to perceive nor understand our self as it is, but also makes it impossible to draw a line between different individuals…"]]))
   vn.na(_("You thank them and run away while they keeps on talking to themselves."))
   lasthint( prof )

   vn.run()
end


function approach_hint4 ()
   local name
   if mem.met_drshrimp then
      name = _("Dr. Shrimp")
   else
      name = hint4_name
   end

   vn.clear()
   vn.scene()
   local drshrimp = vn.newCharacter( name, { image=hint4_image, color=hint4_colour } )
   local shrimp = vn.newCharacter( _("Floating Shrimp"), { color={0.4, 0.6, 1.0} } )
   vn.transition()

   vn.na(_("You approach the young man who has POST-DOCTORAL RESEARCHER written on his lab coat. He seems to be really into a book titled 'SHRIMP: Anatomical studies of the Neo-neo-neocaridina species'."))
   drshrimp(_([["…"]]))
   vn.na(_("You wait to see if they notice your presence. While the young man reading the book hasn't, it does seem like the strange shrimp-like creature floating in an aquarium near them has. You feel like it's staring into your soul."))
   shrimp(_([[After what seems to be an eternity of a staring contest between you and the shrimp, you hear a loud beep and the shrimp's speaker begins to make a noise.
"PERSON. PERSON."]]))
   drshrimp(_([[The young man grumbles.
"I told you I already submitted my temporal research proposal clarification application yesterday."]]))
   if not mem.met_drshrimp then
      drshrimp(_([[The young man suddenly breaks out of his reading stupor and looks at you as if you had appeared out of thin air.
"Wait, what? Who are you?"]]))
      shrimp(_([["PERSON. PERSON. PERSON."]]))
      drshrimp(_([["C'mon Calliope, I already know that. Here, have a pellet."]]))
      shrimp:rename(_("Calliope"))
      shrimp(_([[A pellet of shrimp food is released into the floating aquarium. The shrimp wastes no time in scarfing it down.
   "HAPPY. GOOD."]]))
      drshrimp(p_("Dr. Shrimp", [["What do you want?"]]))
      vn.na(_("You explain to him that you are looking for information related to nebula artefacts."))
      drshrimp(_([["Ah, that hellish project. Was doomed from the start, you know. The full-time professors and researchers let their egos get to their heads and it derailed spectacularly. I'm glad it ended as it did, or I would be still stuck in that purgatory."]]))
      drshrimp(_([["I don't think most of the people remember me, but I was the one stuck doing most of the work. If you can call it that."]]))
      drshrimp:rename(_("Dr. Shrimp"))
      drshrimp(_([["My name is Cayne, but you can call me Dr. Shrimp. What would you like to know?"]]))
      vn.func( function () mem.met_drshrimp = true end )
   else
      drshrimp(_([[The young man suddenly breaks out of his reading stupor and looks at you as if you had appeared out of thin air.
"Ah, it is you again. Is there anything else you would want to know about?"]]))
   end

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         {_([["Dr. Shrimp?"]]), "drshrimp" },
         {_("Ask about nebula artefacts"), "artefacts" },
         {_("Ask about other members."), "members" },
         {_("Leave"), "leave" },
      }
      if mem.asked_drshrimp then
         table.insert( opts, 1, {_("Ask about Calliope"), "calliope"} )
      end
      if mem.asked_strangelove then
         table.insert( opts, 1, {_("Ask about Dr. Strangelove"), "strangelove"} )
      elseif mem.asked_members and mem.asked_artifacts then
         table.insert( opts, 1, {_("Anything else?…"), "strangelove"} )
      end
      return opts end )

   vn.label("drshrimp")
   drshrimp(fmt.f(_([["After the nebula project, like most other researchers on the team, I got the hell away from nebula research. One day while visiting {pnt} I found out about fresh-water shrimp breeding and became enthralled. One thing led to another, and now I'm doing shrimp research."]]), {pnt=_(hintpnt[4])}))
   drshrimp(_([["You see, shrimp are fascinating creatures. They have a really fast reproduction cycle, and reproduce in large numbers, allowing for simple genetic manipulation. While most are bred for colours and physical traits, some have also been trained for mental traits. Calliope here is an example of an extremely mentally capable shrimp."
He taps the tank of the floating shrimp next to him.]]))
   drshrimp(_([["The capabilities of these shrimp are endless, and they are helping us understand much more about genetic modification than the brute-force approaches of the Soromid, although our technology is still lagging behind."]]))
   drshrimp(_([["And what's best, look how cute they are!"
He enthusiastically points towards Calliope.
"Isn't she the cutest?"]]))
   drshrimp(_([["After focusing on shrimp for a few cycles, people started calling me Dr. Shrimp and the name stuck. Honestly, it's the best nickname they could give me. Makes me feel like getting my PhD was finally worth it!"]]))
   vn.func( function () mem.asked_drshrimp = true end )
   vn.jump("menu_msg")

   vn.label("calliope")
   drshrimp(_([["Calliope was bred for her mental faculties. With her special floating aquarium she is able to articulate up to 20 unique words! It's incredible! Not only that, but we kept her good looks! Plus as a neo-neo-neocaridina, she is much larger than neo-neocaridinas."]]))
   shrimp(_([["FOOD. FOOD. FOOD."]]))
   drshrimp(_([["What would I do without her?"
He activates her feeding system and a food pellet drops out.]]))
   shrimp(_([["HAPPY. HAPPY."]]))
   drshrimp(_([["She's the best shrimp we've been able to produce so far. She isn't capable of self-awareness yet, and tends to forget words if she isn't properly trained, but it is a start. By expanding their neural functionality we expect them to be compatible with neuro-implants eventually. This will open up a whole set of new functionality, given that their reaction speed and neuro-motor ability is superb."]]))
   drshrimp(_([["I named her after a muse from some archaic religion I found while researching. She is the only thing that keeps me going during my 17th post doctoral fellowship."]]))
   vn.jump("menu_msg")

   vn.label("artefacts")
   drshrimp(_([["What sort of artefacts we were dealing with? All sorts. Pretty much anything we could get our hands on. We were trying to get a hold of stuff closer to the origin, near Sol, but, most of the time, scavengers brought us stuff from the outer areas. Many times they were even trying to sell us random debris as nebula artefacts, but that's really easy to detect."]]))
   drshrimp(_([["You see, the nebula emits a specific type of radiation. We aren't too familiar with it, but it does alter the subatomic particles of space debris in nearly imperceptible ways. Our tools could detect the alterations easily."]]))
   drshrimp(_([["When the project failed, most of the artefacts were confiscated to who knows where, but I managed to keep a nut from what I think is a space station, although not entirely sure. It's a bit damaged beyond recognition you see."]]))
   drshrimp(_([["With the failure of most nebula projects, I don't think there is anybody buying nebula artefacts anymore. Not much of a market for them, and most easy to access debris has all been scavenged anyway. Some collectors are still interested in that, but that's about it. Furthermore, it's not entirely legal if you catch my drift."]]))
   vn.func( function () mem.asked_artifacts = true end )
   vn.jump("menu_msg")

   vn.label("members")
   drshrimp(_([["Most of the project members were full professors, which is probably why it failed. Us post-doctoral researchers do all the work and get none of the credit."]]))
   drshrimp(_([["After it went to hell, many of the researchers lost their posts or went into hiding. The few I know that still are active are Prof. Stova, Prof. Sato, and Prof. Hsu. I think they even got some stupid promotion, while the other post docs and I lost our jobs."]]))
   drshrimp(_([["It's pretty incredible to think that there were originally 100ish people on the project. Yet even with all the people fired for falsification of results, not a single tenured post opened! It's ridiculous! If industry wasn't so horrible, I would quit academia in an instant!"]]))
   drshrimp(_([["Now that I think of it, there was another guy. What was his name? I totally forget. Oh well."]]))
   vn.func( function () mem.asked_members = true end )
   vn.jump("menu_msg")

   vn.label("strangelove")
   drshrimp(_([["Now I remember! There was a another post-doctoral researcher who worked with me. He was a bit weird and kept obsessing over the nebula artefacts. Quite a few went missing during the project and I think it was probably him who was taking them."]]))
   drshrimp(_([["He was really upset when the project got cancelled, threw a big tantrum and all. He was locked in his office for days until they managed to coax him out. Nobody really did much as we were all busy dealing with all the paperwork of the project cancellation."]]))
   vn.sfxEerie()
   drshrimp(fmt.f(_([["Eventually he sort of disappeared. Last I heard, he said he was going to {sys}, which is a bit strange, because not only is there not a research centre there, but there isn't even an inhabited planet nor station!"]]), {sys=eccsys}))
   drshrimp(_([["It's really weird but if you are really interested, I suppose you could try to take a look around there. The whole thing gives me the me the creeps though."]]))
   vn.func( function ()
      mem.asked_strangelove = true
      if mem.misn_state==1 then
         mem.misn_state = 2
         misn.osdCreate( _("Finding Maikki's Father"), {fmt.f(_("\tGo to {sys}"), {sys=eccsys})} )
         misn.markerRm( mem.markerhint4 )
         mem.marker_ecc = misn.markerAdd( eccsys, "low" )
         minerva.log.maikki(_("You found about a strange researcher who appears to be in Westhaven and is related to the nebula research.") )
      end
   end )
   vn.jump("menu_msg")

   vn.label("menu_msg")
   drshrimp(_([["Is there anything else you would like to know?"]]))
   vn.jump("menu")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function enter ()
   if mem.misn_state==2 and system.cur() == eccsys then
      pilot.clear()
      pilot.toggleSpawn(false)
      hook.timer( 30.0, "ecc_timer" )
   elseif mem.misn_state==4 and system.cur() == eccsys then
      pilot.clear()
      pilot.toggleSpawn(false)

      local fdrone = faction.dynAdd( "Independent", "Feral Drone", _("Feral Drone"), {ai="drone_miner"} )
      local function spawn_single( ship, pos )
         local p = pilot.add( ship, fdrone, pos )
         p:setNoJump(true)
         p:setNoLand(true)
         return p
      end

      -- Spawn the drones
      local pos = eccpos + vec2.newP( 5000, rnd.angle() )
      mem.attacked_feral_drones = false
      local b = spawn_single( "Za'lek Heavy Drone", pos )
      feral_drone_boss = b
      b:rename(_("Feral Alpha Drone"))
      b:setHilight(true)
      hook.pilot( b, "attacked", "ecc_feral_boss_attacked" )
      hook.pilot( b, "death", "ecc_feral_boss_dead" )
      b:setNoboard(true)
      local num = 4
      for i=1,num do
         local fpos = pos + vec2.newP( 50, 2*math.pi*i/num )
         local p = spawn_single( "Za'lek Light Drone", fpos )
         p:rename(_("Feral Drone"))
         p:setLeader( b )
         hook.pilot( p, "attacked", "ecc_feral_boss_attacked" )
      end
   end
end


function ecc_timer ()
   player.msg(_("Your ship has detected a curious signal originating from inside the system."), true)
   player.autonavReset()
   mem.sysmarker = system.markerAdd( eccpos, _("Curious Signal") )
   hook.timer( 0.5, "ecc_dist" )
end


function ecc_dist ()
   local pp = player.pilot()
   local dist = pp:pos():dist( eccpos )

   if dist < math.min( pp:detectedDistance(), 3000 ) then
      system.markerRm( mem.sysmarker )
      local spawners = {
         "Za'lek Heavy Drone",
         "Za'lek Light Drone",
         "Za'lek Light Drone",
         "Za'lek Light Drone",
      }
      defense_systems = {}
      for k,v in ipairs(spawners) do
         local pos = eccpos + vec2.newP( rnd.rnd(0,100), rnd.angle() )
         local p = pilot.add( v, "Strangelove", pos, _("Security Drone"), {ai="baddiepos"} )
         p:setHostile()
         table.insert( defense_systems, p )
      end
      player.autonavAbort(_("Hostiles detected!"))
      defense_systems[1]:broadcast(_("UNAUTHORIZED VESSEL DETECTED. ELIMINATING."))
      hook.timer( 1.0, "ecc_drone_dead_check" )
      return
   end
   hook.timer( 0.5, "ecc_dist" )
end


function ecc_drone_dead_check ()
   local new_defense = {}
   for k,p in ipairs(defense_systems) do
      if p:exists() then
         table.insert( new_defense, p )
      end
   end
   defense_systems = new_defense

   -- All dead
   if #defense_systems==0 then
      hook.timer( 5.0, "ecc_timer_dead" )
   else
      hook.timer( 1.0, "ecc_drone_dead_check" )
   end
end


function ecc_timer_dead ()
   player.msg(_("Your ship detects that one of the asteroids isn't what it seems…"))
   vn.sfxEerie()
   diff.apply( eccdiff )
   mem.misn_state = 3
   minerva.log.maikki(_("You were attacked by a Za'lek security system and found a laboratory disguised as an asteroid in Westhaven.") )
end


function ecc_feral_boss_dead ()
   local paperbg = love_shaders.paper()
   vn.clear()
   vn.scene()
   vn.music( 'snd/sounds/loops/creepy_guitar.ogg' )
   vn.func( function ()
      vn.setBackground( function ()
         vn.setColor( {0.2, 0.2, 0.2, 0.8} )
         paperbg:draw( 0, 0 )
      end )
      vn.setShader( love_shaders.corruption{ strength=0.5 } )
   end )
   local voice = vn.newCharacter( _("Unknown Voice") )
   vn.transition( "hexagon", 3 ) -- Really slow fade in so the player stops mashing keys due to combat (keypresses aren't processed in animations)
   vn.na(_("While the drone is blowing up, you receive a faint, audio-only transmission."))
   vn.sfxEerie()
   voice(_([["Thank you for setting me free…"]]))
   vn.na(_("You wonder what that was about as you watch the drone thrash as it blows up. Westhaven is a really weird place."))
   vn.na(_("From the ship scraps you are able to find a very damaged… thing… You guess this is what Dr. Strangelove was referring to as a nebula artefact. Strangely, your ship sensors identify it as mainly biological material…"))
   vn.done( "hexagon" )
   vn.run()

   local c = commodity.new( N_("Nebula Artefact?"), N_("A very damaged thing that seems to be mainly biological. I guess this is the nebula artefact?") )
   mem.nebula_artifacts = misn.cargoAdd( c, 0 )
   mem.misn_state = 5
   misn.osdCreate( _("Finding Maikki's Father"), {_("Go back to Dr. Strangelove")} )
   minerva.log.maikki(_("You recovered a nebula artefact that Dr. Strangelove wanted from feral drones.") )
end

local drone_msgs = {
   _("Just destroy me and put me out of my misery."),
   _("Why am I still alive?"),
   _("I can't deal with this anymore."),
   _("Please end me!"),
   _("Living is suffering."),
   _("It all hurts."),
   _("Please kill me!"),
}

function ecc_feral_boss_attacked( _p )
   if not mem.attacked_feral_drones then
      mem.attacked_feral_drones = true
      feral_drone_boss:broadcast( drone_msgs[1] )
      mem.drone_msgid = 0
      hook.timer( 2.0, "ecc_feral_boss_msg" )

      -- We go with nebula music
      music.play("nebu_battle1.ogg")
   end
end

function ecc_feral_boss_msg ()
   mem.drone_msgid = (mem.drone_msgid % #drone_msgs)+1
   if feral_drone_boss:exists() then
      feral_drone_boss:broadcast( drone_msgs[ mem.drone_msgid ] )
      hook.timer( 7.0, "ecc_feral_boss_msg" )
   end
end

function approach_eccentric ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.strangelove )
   local dr = vn.newCharacter( minerva.vn_strangelove{ shader=love_shaders.hologram() } )
   vn.transition( "electric" )

   if not mem.ecc_visitedonce then
      vn.na(_("The hologram projector flickers and what appears to be a grumpy old man appears into view. He doesn't look very pleased to be disturbed."))
      dr(_([["How did you get in there? Who are you!"]]))
      vn.na(_("You explain to him that you are looking for information about nebula artefacts."))
      dr(_([["Who sent you here? Was it Dr. Bob? That weasel was always after my precious artefacts. Well, he can't have them! I got all this with my hard work, sweat, and tears! They're all mine!"
He cackles manically.]]))
      dr(_([["I have hidden them very well. Even though you somehow got past my security system and made it into my laboratory, you'll never find them!"
You glance at a crate labelled 'NEBULA ARTEFACTS #082' in the corner of the room.]]))
      dr(_([["Anyway, I am very busy now, yes? All this science won't do itself. Almost have a new specimen ready and it will be better than ever! The old ones were fairly inadequate."]]))
      mem.ecc_visitedonce = true
      minerva.log.maikki(_("You met an eccentric researcher named Dr. Strangelove in Westhaven." ))
   else
      vn.na(_("The hologram projector flickers and Dr. Strangelove comes into view. He doesn't look very happy to see you again."))
   end
   vn.label("menu_msg")
   dr(p_("Dr. Strangelove", [["What do you want?"]]))
   vn.label("menu")
   vn.menu( function ()
      local opts = {
         {_("Ask about the nebula"), "nebula"},
         {_("Ask about this place"), "laboratory"},
         {_("Leave"), "leave"},
      }
      if mem.misn_state==5 then
         table.insert( opts, 1, {_("Hand over the artefact"), "handover"} )
      elseif mem.misn_state==4 then
         table.insert( opts, 1, {_("Ask about job"), "job"} )
      end
      return opts
   end )

   vn.label("nebula")
   dr(_([["Ah yes, the nebula. The pinnacle of human creation! Isn't it just mesmerizing and beautiful to look at? I've always been attracted to it ever since the Incident destroyed my university. Destruction is so pretty, is it not?"]]))
   dr(_([["I recently had the great opportunity to work directly on nebula research. It was a fabulous project with hundreds of the brightest Za'lek minds working in tandem! Not as brilliant as me, but the numbers were what mattered."
He grins as nostalgia takes him over.]]))
   dr(_([["We were able to have dedicated teams recovering all sorts of incredible items! We even found the remains of a Proteron replicator! It was only able to replicate cheese fondue, but it was incredible. I integrated it with a drone platform and that has kept me healthy and in shape since! It should be in the bar if you want to try it."
You see a greasy robot in the corner that is boiling some sort of brown liquid. Is that… cheese?]]))
   dr(_([["Anyway, it is a real shame that the bureaucrats in central station killed the project in its prime. At least I was able to bring most of the devices here and keep acquiring more artefacts afterwards. Haven't found anything as fancy as the replicator yet, but the bodies have been very interesting."
He coughs, wracking his body.]]))
   vn.menu( {
      {_([["Bodies…?"]]), "bodies"},
      {_([["What devices?"]]), "bodies"},
   } )
   vn.label("bodies")
   dr(_([["Ahaha. You are interested, no? That information won't come cheap. I have a job that I would like you to do, and, in exchange, I might give you the information you seek."]]))
   -- skip back to message if already accepted job
   vn.func( function () if mem.misn_state>=4 then vn.jump("menu_msg") end end )
   vn.menu( {
      {_("Accept the job"), "jobaccept"},
      {_("Refuse the job"), "jobrefuse"},
   } )
   vn.label("jobrefuse")
   dr(_([["You don't scratch my back and I don't scratch yours."]]))
   vn.jump("menu_msg")

   vn.label("jobaccept")
   dr(_([["I see you are braver than you look. I have a bit of an issue. You see, I was upgrading some of my drones with some nebula artefacts to test to see if the functionality could be improved. Sadly, an incompatibility resulted in the drones going… ahem… feral."]]))
   dr(_([["I was going to try to deal with them with my security drones, but as you can guess, that is no longer an option since you smashed them to smithereens."
He glares at you.]]))
   dr(_([["While the drones themselves are dispensable, I need you to recover the nebula artefacts used in their upgrading. They should be roaming around the asteroid field and should be fairly easy to find. Make sure to recover the parts in one piece!"]]))
   vn.func( function ()
      misn.osdCreate( _("Finding Maikki's Father"), {
         fmt.f(_("Recover nebula artefacts from the {sys} asteroid field"), {sys=eccsys}),
      } )
      mem.misn_state = 4
      minerva.log.maikki(_("You accepted Dr. Strangelove's request to recover nebula artefacts from feral drones in Westhaven." ) )
   end )
   vn.jump("menu_msg")

   vn.label("job")
   dr(_([["It should be an easy job. All you have to do is recover the nebula artefacts from the feral drones. They may put up a fight or not be easy to catch, but I expect that a pilot who blasted through all my security drones will have no problem with a few feral drones. Try to bring the artefacts in one piece."
He glares at you.]]))
   vn.jump("menu_msg")

   vn.label("laboratory")
   dr(_([["How do you like it? Cutting edge Za'lek research! I had to borrow a lot of things from my past jobs, but they can't have it back. It's all mine now! I made sure everything is covered in my scent so that they won't be able to appropriate it!"
He smiles mischievously.]]))
   dr(_([["My drones dug it out and it's the cosiest place I have ever lived in, but I don't get many guests. In fact, you are my…"
He starts counting on his fingers intently.
"…first guest! Yes, that's it, the first true living biological being other than me to visit!"
You don't like how he puts emphasis on 'living biological'…]]))
   dr(_([["Feel free to use the facilities as you please, but stay out of the backroom."
Given the smell of the entire laboratory and especially the horrible scents wafting from the backroom, you feel like it is best to heed his advice for your own safety.]]))
   vn.jump("menu_msg")

   vn.label("handover")
   vn.na(_("You show him the nebula artefact and suddenly a couple of small drones whirl into the room and start to perform a deeper inspection."))
   dr(_([["Energy levels 7%… I see, I see… minor flux instability… not that bad…
no vitals… not very good… mmmm… nebula radiation at minimum… can't be helped I guess…"]]))
   dr(_([[He goes on for what seems like an eternity before finally remembering that you are there.
"Why are you still here? Go back to wherever the hell you came from!"]]))
   vn.menu( {
      {_([["What about my information?"]]), "hatethisguy"},
      {_([["What the hell is this thing?"]]), "hatethisguy"},
   } )
   vn.label("hatethisguy")
   dr(_([["I see… You don't grasp the importance of this artefact. I guess it can't be helped with such an inferior intellect as yours."
He sneers.]]))
   dr(_([["This is my latest creation, an amalgamate of ancient nebula technology and life. Think of this as something that not only surpasses any of the Soromid biotechnology, but also Za'lek cybertechnology. It is the peak of technological advancement!"]]))
   dr(_([["While it has an incredible potential, there are… some complications still. The mental faculties tend to lack stability. While this one is a very interesting failure, it is still a failure at heart. Most of my other creations have had much more success."]]))
   dr(_([["What was it you wanted?"]]))
   vn.na(_("You ask him if he knows anything about Kex's ship in the nebula."))
   dr(_([["Ah yes! I remember that wreck. A very, very curious one indeed. At that time I was exploring the nebula myself, and we came upon this most curious wreck that had clear signs of fighting. While it is not uncommon for scavengers to squabble among themselves, this one had most of the damage on the inside. The damage on the outside was clearly done later to confuse people who were investigating like us."
He coughs.]]))
   dr(_([["While it was not a pre-incident ship nor had any interesting characteristics, what was inside was surprising. There were two bodies still warm to the touch, it seems like time passes differently in such deep nebula. Normally I would ignore such things, however, they had been infused with so much nebula radiation that they seemed to have potential!"
He starts to get excited.]]))
   dr(_([["That's when I realized it, humankind and the nebula are one and the same. Our futures are intertwined! By absorbing the nebula into the body, one can transcend humanity and live forever among the stars! There is no limit to the potential infused in the nebula! Now I get it! All my research had led up to this!"]]))
   dr(_([["And that's when it hit me, by amalgamating nebula and flesh, we can transcend ourselves! I had to do it; it had to be done! And it worked, oh boy did it work. There were some difficulties with flesh incompatibility, but with suitable replacements I was able to solve it."
He laughs manically.]]))
   dr(_([["They were oh so perfect, like Adam and Eve, ready to create a new perfect humanity. However, it isn't as simple as that. The man, an ungrateful fool, had too much attachment to his previous life. How weak… That's when I realized it. You need a perfect mind and perfect flesh to become the perfect being. With the nebula, the perfect flesh can be done, but the mind must also be perfect. It was so simple!"
He seems delirious.]]))
   dr(_([["So I did it. I tried to upgrade myself. But it isn't so simple, you see, it's the near-death experience that is necessary. On healthy flesh it just deteriorates the body."
He coughs, convulsing.]]))
   dr(_([["The mind was good enough, but not my body. No, and I don't have much time. I'm wasting away. So close and here wasting away. What uselessness."
He lowers his gaze.]]))
   vn.na(_("You ask what happened to the 'individuals' in the experiments."))
   dr(_([["I did try to track him down, last I checked he got caught by thugs at Minerva Station. Nasty place in the Limbo system. No nebula and very stale space. Not beautiful at all."
He coughs softly.]]))
   dr(_([["She tried to help me, but she too was weak of mind, and one day left for the stars. I couldn't stop her, but I thought I was close enough. I just need more time. One more cycle…"
His voice gets softer and softer as he keeps on mumbling.]]))
   vn.na(_("You try to get his attention again, but it doesn't seem to work. He seems to have fallen into a stupor. You feel you have enough information to report to Maikki again."))
   vn.func( function ()
      misn.cargoRm( mem.nebula_artifacts )
      mem.misn_state = 6
      misn.npcRm( mem.npc_ecc )
      misn.osdCreate( _("Finding Maikki's Father"), {_("Report back to Maikki in the Limbo system")} )
      misn.markerAdd( spob.get("Minerva Station"), "low")
      misn.markerRm( mem.marker_ecc )
      minerva.log.maikki(_("You learned that Dr. Strangelove saved what appears to be Kex and another individual from a wreck in the nebula. Kex appears to have run away and is likely held by thugs at Minerva station." ) )
   end )
   vn.na(_("You leave behind the hologram projection and hope you won't have to deal with Dr. Strangelove in the future."))
   vn.done( "electric" )

   vn.label("leave")
   vn.na(_("You turn off the hologram projector and Dr. Strangelove's image flickers and disappears."))
   vn.done( "electric" )
   vn.run()
end
