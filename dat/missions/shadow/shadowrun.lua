--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Shadowrun">
 <unique />
 <priority>3</priority>
 <chance>20</chance>
 <location>Bar</location>
 <cond>system.get("Klantar"):jumpDist() &lt; 3</cond>
 <notes>
  <campaign>Shadow</campaign>
 </notes>
</mission>
--]]
--[[
   This is the main script for the Shadowrun mission. It's started from the spaceport bar and tries to emulate spaceport bar conversation as part of the mission.
   "shadowrun" stack variable:
   1 = player has met Rebina, but hasn't accepted the mission
   2 = player has accepted Rebina's mission, but has not talked to SHITMAN
   3 = player has talked to SHITMAN
--]]
local portrait = require "portrait"
local fleet = require "fleet"
local fmt = require "format"
local shadow = require "common.shadow"

local pnt, sys = spob.getS("Durea") -- Where SHITMAN lives
local sys2 = system.get("Uhriabi") -- The system where the ship is
local shipname = _("Seiryuu")

local seiryuu -- Non-persistent state
local dateresolution -- Forward-declared functions

function create ()
   if not misn.claim( {sys, sys2} ) then
      misn.finish()
   end

   mem.credits = shadow.rewards.shadowrun
   -- Developer note: changing these numbers may have consequences for translators (if we support more languages later on).
   mem.timelimit1 = 20 -- In STP
   mem.timelimit2 = 50 -- In STP

   misn.setNPC( _("A dark-haired woman"), "neutral/unique/rebina_casual.webp", _("You spot a dark-haired woman sitting at the bar. Her elegant features and dress make her stand out, yet her presence here seems almost natural, as if she's in the right place at the right time, waiting for the right person. You wonder why she's all by herself.") )
end

function accept()
   if var.peek("shadowrun") == 1 then
      tk.msg(_("A dark-haired woman"), _([[Rebina nods at you to acknowledge your existence. "We meet again. I'm glad to see you've not gotten yourself killed yet." She smiles meaningfully. "As it happens I haven't found anyone to take care of my business yet. Perhaps you would reconsider? Allow me to remind you what this is about."]]))
   else
      tk.msg(_("A dark-haired woman"), _([[The woman calmly watches you as you approach her, seemingly not at all surprised to see you. Clad in a plain yet expensive-looking black dress and sipping from her martini, she emits an aura of class that is almost intimidating.
    "Hello," she greets you. "I had a feeling you might want to talk to me. You are different from most..." she gestures at the other patrons of the bar, "And so am I. But where are my manners, I haven't introduced myself. My name is Rebina. I am what you might call a talent hunter. I visit places such as these to find people of exceptional skill. People such as you."
    You begin to introduce yourself, but Rebina waves it away, perhaps because your name doesn't interest her, or possibly because she already knows who you are. "Let's not waste words on idle formalities," she says. "I am here to talk business, and I've got a proposition for you, if you're interested."]]))
   end
   if tk.yesno(_("Rebina's proposition"), _([["What I need is a pilot and a ship. Specifically, I need a skilled pilot and a capable ship. Do you fit that description? I have a feeling you do. You see, what I am about to propose to you is both profitable and dangerous." Rebina takes another sip of her drink before continuing, allowing what she just said to fully register. "I will not lie to you. There are... rivalries out there, and working for me will mean you'll take sides in some of them. People will take notice of you, and some of them will try to kill you."
    You explain that taking risks comes with being an independent pilot and that you took the captain's chair with appropriate resolve, but Rebina pins you with a piercing gaze. "These are no ordinary pirate raids we're talking about," she admonishes you. "If you take this assignment, you will be a painted target. I want you to be well aware of this." There is another pause, but then she continues in a milder tone of voice. "That being said, I can assure you that the reward is well worth the risk. Pull this off, and you'll walk away considerably richer than you were."
    Rebina leans back, levelly meeting your gaze. "That's all I can tell you at this point. You'll get more details only once you accept this job. If you accept this job. What say you?"]])) then
      misn.accept()
      tk.msg(_("Rebina's explanation"), fmt.f(_([["Wonderful!" Rebina gives you a warm, sincere smile. "I don't mind admitting that it isn't easy finding pilots who measure up to my expectations, and finding ones willing to take a risk is more difficult still. I am pleased indeed."
    Then Rebina's expression changes to that of a businesswoman about to ply her trade. "Now, listen up. Contrary to what you may have thought, this assignment isn't about me. It's about a man who goes by the name of Jorek McArthy. The current state of affairs is that Jorek is staying on {pnt} in the {sys} system, and this is not where me and my associates want him to be. Unfortunately, Jorek has attracted some unwanted attention, and we don't want him to focus that attention to us."
    Rebina takes a moment to sip from her drink. "I think you can see where this is going. You are to rendezvous with Jorek, take him aboard your ship, lose whoever's tailing him, then bring him to the {sys2} system. There you will dock with one of our ships, the {plt}, which will take Jorek to his final destination. You will receive your reward from her captain once Jorek is aboard."
    "It's a simple objective, but accomplishing it might require considerable skill." She leans back and smiles. "Still, I have utmost confidence that you can do it. I seldom misjudge those I choose to trust."]]), {pnt=pnt, sys=sys, sys2=sys2, plt=shipname}))

      -- Translator note: If the plural forms are a problem, assume the numbers here are 20 and 50.
      tk.msg(_("Rebina's explanation"), fmt.f(_([["You know what to do," Rebina tells you. "You will find Jorek in the spaceport bar on {pnt}. When you see him, tell him you've come to 'see to his special needs'. Oh, and please be discreet. Don't talk about things you don't need to; the walls have ears in that place. In particular, don't mention any names."
    "You will be on a time schedule. You must meet Jorek within {t1} periods, or he will assume you are not coming and go back into hiding. You must also be at the meeting point {t2} periods from now. If you fail to meet with Jorek within the time limit or if you are prevented from taking him off-world for any other reason, make your way to the {plt} and report what happened. We'll take it from there. If you fail to show up at the designated time, we will assume you have failed, and the {plt} will leave."
    Rebina empties her glass and places it on the bar before rising to her feet. "That will be all. Good luck, and keep your wits about you."
    Then Rebina takes her leave from you and gracefully departs the spaceport bar. You order yourself another drink. You've got the feeling you're going to need it.]]), {pnt=pnt, t1=mem.timelimit1, t2=mem.timelimit2, plt=shipname}))

      -- Set deadlines
      mem.deadline1 = time.get() + time.new(0, mem.timelimit1, 0)
      mem.deadline2 = time.get() + time.new(0, mem.timelimit2, 0)

      misn.setTitle(_("Shadowrun"))
      misn.setReward(_("You were promised riches..."))
      misn.setDesc(fmt.f(_("Fly to planet {pnt} in the {sys} system and talk to Jorek. Once Jorek has boarded your ship, proceed to system {sys2} and board the {plt}."), {pnt=pnt, sys=sys, sys2=sys2, plt=shipname}))
      misn.osdCreate(_("Shadowrun"), {
         fmt.f(_("Fly to planet {pnt} in the {sys} system and pick up Jorek"), {pnt=pnt, sys=sys}),
         fmt.f(_("You have {time} remaining"), {time=(mem.deadline1 - time.get())}),
      })
      mem.misn_marker = misn.markerAdd( pnt, "low" )
      mem.shadowrun = 2

      mem.dateres = 500
      mem.datehook = hook.date(time.new(0, 0, mem.dateres), "date")
      hook.land("land")
      hook.enter("enter")
   else
      tk.msg(_("A dark-haired woman"), _([["I see. What a shame." Rebina's demeanor conveys that she's disappointed but not upset. "I can understand your decision. One should not bite off more than one can chew, after all. It seems I will have to try to find another candidate." She tilts her head slightly. Then, "Although if you change your mind before I do, you're welcome to seek me out again. I'll be around."
    Rebina finishes her drink and gets up. Then, with a cordial wave of her, hand she sweeps out of the door. You momentarily regret not taking her up on her offer, but it passes. You've made the right decision, and that is that.]]))
      var.push("shadowrun", 1) -- For future appearances of this mission
      misn.finish(false)
   end
end

function land()
   local landed = spob.cur()
   if pnt == landed then
      misn.npcAdd( "jorek", _("An unpleasant man."), "neutral/unique/jorek.webp", _("A middle-aged, cranky looking man is sitting at a table by himself. You are fairly certain that this is the fellow you're looking for.") )
      misn.npcAdd( "officer", _("Officer at the bar"), portrait.getMaleMil("Empire"), _("You see a military officer with a drink at the bar. He doesn't seem to be very interested in it, though...") )
      misn.npcAdd( "soldier1", _("Soldier at the news kiosk"), portrait.getMaleMil("Empire"), _("You see a soldier at a news kiosk. For some reason, he keeps reading the same articles over and over again.") )
      for soldier = 1, 2 do
         misn.npcAdd( "soldier2", _("Card-playing soldier"), portrait.getMil("Empire"), _("Two soldiers are sharing a table near the exit, playing cards. Neither of them seems very into the game.") )
      end
   end
end

-- Talking to Jorek
function jorek()
   if mem.shadowrun == 2 then
      if tk.yesno( _("An unpleasant man"), _([[You join the man at his table. He doesn't particularly seem to welcome your company, though, because he gives you a look most people would reserve for particularly unwelcome guests. Determined not to let that get to you, you ask him if his name is indeed Jorek.
    "Yeah, that's me," he replies. "What'd ya want, kid?"
    You explain to him that you've come to see to his special needs. This earns you a sneer from Jorek. "Ha! So you're running errands for the little lady, are you? Oh don't tell me, I've got a pretty good idea what it is you want from me." He leans onto the table, bringing his face closer to yours. "Listen, buddy. I don't know if you noticed, but people are watchin' me. And you too, now that you're talkin' to me. Those goons over there? Yeah, they're here for me. Used to be fancy undercover agents, but I've been sittin' on my ass here for a long time and they figured out I was on to them, so they replaced 'em with a bunch of grunts. Cheaper, see."
    "And it's not just them," Jorek continues. "On your way here, did you see the flotilla of 'patrol ships' hangin' around? You guessed it, they're waitin' for me to split this joint. I'm HOT, kid. If I step onto your ship, you'll be hot too. And you have absolutely no problem with that, is that what you're tellin' me?"]]) ) then
         tk.msg( _("Jorek's scorn"), _([[Jorek roars with laughter. "Hah! Yeah, I'm sure you don't! I know what you're thinkin', I do. You'll take me outta here, pull a heroic bust past them Empire ships, save me, and the day while you're at it, then earn your stripes with the lady, am I right? S'yeah, I bet you'd take on the world for a pretty face and a coy smile." He doesn't so much as make an attempt to keep the mocking tone out of his voice.
    "Well, good for you. You're a real hero, right enough. But you know what? I'm stayin' put. I don't care if you have the vixen's approval. I'm not gettin' on some random Joe's boat just so he can get us both blasted to smithereens."
    Your patience with Jorek's abuse is finally at an end, and you heatedly make it clear to him that your abilities as a pilot aren't deserving of this treatment. Jorek, however, seems unimpressed. He tells you to stick it where the sun doesn't shine, gets up from his chair and squarely deposits himself at another table. Unwilling to stoop to his level, you choose not to follow him.]]) )
      else
         tk.msg( _("Jorek's scorn"), _([[Jorek exhales derisively. "No, I thought not. Probably thought this was going to be a walk in the park, didn't you? But when the chips are down, you back out. Wouldn't want to mess with be big scary Empire, would we?" He raises his voice for this, taunting the military personnel in the bar. They don't show any sign of having even heard Jorek speak. Jorek snorts, then focuses his attention back on you.
    "I've got no use for wusses like yourself. Go on, get out of here. Go back to your ship and beat it off this rock. Maybe you should consider gettin' yourself a desk job, eh?"
    With that, Jorek leaves your table and sits down at a nearby empty one. Clearly this conversation is over, and you're not going to get anything more out of him.]]) )
      end
      mem.shadowrun = 3
      misn.markerMove( mem.misn_marker, sys2 )
   else
      tk.msg( _("Dismissal"), _([[Jorek pointedly ignores you. It doesn't seem like he's willing to give you the time of day any longer. You decide not to push your luck.]]) )
   end
end

function officer()
   tk.msg( _("You were ignored"), _("You try to strike a conversation with the officer, but he doesn't seem interested what you have to say, so you give up.") )
end
function soldier1()
   tk.msg( _("You were shooed away"), _("Leave me alone. Can't you see I'm busy?") )
end
function soldier2()
   tk.msg( _("They didn't need a third player"), _("They don't seem to appreciate your company. You decide to leave them to their game.") )
end

function date()
   -- Deadline stuff
   if mem.deadline1 >= time.get() and mem.shadowrun == 2 then
      dateresolution(mem.deadline1)
      misn.osdCreate(_("Shadowrun"), {
         fmt.f(_("Fly to planet {pnt} in the {sys} system and pick up Jorek"), {pnt=pnt, sys=sys}),
         fmt.f(_("You have {time} remaining"), {time=(mem.deadline1 - time.get())}),
      })
   elseif mem.deadline2 >= time.get() and mem.shadowrun == 3 then
      dateresolution(mem.deadline2)
      misn.osdCreate(_("Shadowrun"), {
         _("You could not persuade Jorek to come with you"),
         fmt.f(_("Fly to the {sys} system and dock with (board) {plt} to report your result"), {sys=sys2, plt=shipname}),
         fmt.f(_("You have {time} remaining"), {time=(mem.deadline2 - time.get())})
      })
      misn.osdActive(2)
   else
      abort()
   end
end

function dateresolution(time)
   if time - time.get() < time.new(0, 0, 5000) and mem.dateres > 30 then
      mem.dateres = 30
      hook.rm(mem.datehook)
      mem.datehook = hook.date(time.new(0, 0, mem.dateres), "date")
   elseif time - time.get() < time.new(0, 1, 0) and mem.dateres > 100 then
      mem.dateres = 100
      hook.rm(mem.datehook)
      mem.datehook = hook.date(time.new(0, 0, mem.dateres), "date")
   elseif time - time.get() >= time.new(0, 1, 0) and mem.dateres < 500 then
      mem.dateres = 500
      hook.rm(mem.datehook)
      mem.datehook = hook.date(time.new(0, 0, mem.dateres), "date")
   end
end

function enter()
   -- Random(?) pirate attacks when get closer to your system, and heavier ones when you fly away from it after meeting SHITMAN
   if system.cur():jumpDist(sys) < 3 and system.cur():jumpDist(sys) > 0 and mem.shadowrun == 2 then
      pilot.clear()
      pilot.toggleSpawn(false)
      fleet.add( 4, "Pirate Hyena", "Pirate", vec2.new(0,0) )
   elseif system.cur():jumpDist(sys) < 3 and system.cur():jumpDist(sys) > 0 and mem.shadowrun == 3 then
      pilot.clear()
      pilot.toggleSpawn(false)
      fleet.add( 4, "Pirate Hyena", "Pirate", vec2.new(0,0) )
      pilot.add( "Pirate Ancestor", "Pirate", vec2.new(0,20) )
      pilot.add( "Pirate Ancestor", "Pirate", vec2.new(-20,0) )
      pilot.add( "Pirate Ancestor", "Pirate", vec2.new(0,-20) )
   end

   -- Empire ships around planet
   if system.cur() == sys then
      pilot.clear()
      pilot.toggleSpawn(false)
      local planetpos = pnt:pos()
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(200,0), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(130,130), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(0,200), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(-130,130), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(-200,0), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(-130,-130), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(0,-200), nil, {ai="empire_idle"} )
      pilot.add( "Empire Pacifier", "Empire", planetpos + vec2.new(130,-130), nil, {ai="empire_idle"} )
   end

   -- Handle the Seiryuu, the last stop on this mission
   if mem.shadowrun >= 2 and system.cur() == sys2 then
      local mypos = vec2.new(-1500, 600)
      seiryuu = pilot.add( "Pirate Kestrel", shadow.fct_fourwinds(), mypos , _("Seiryuu"), {ai="trader"} )

      seiryuu:setActiveBoard(true)
      seiryuu:control()
      seiryuu:setInvincible(true)
      seiryuu:setHilight(true)

      hook.pilot(seiryuu, "board", "board")
      hook.pilot(seiryuu, "death", "abort")
   end
end

function board()
   if mem.shadowrun == 2 then
      -- player reports in without SHITMAN
      tk.msg(_("Empty-handed"), fmt.f(_([[You complete docking operations with the {plt}, well aware that your ship isn't carrying the man they were expecting. At the airlock, you are greeted by a pair of crewmen in grey uniforms. You explain to them that you were unable to bring Jorek to them, and they receive your report in a dry, businesslike manner. The meeting is short. The crewmen disappear back into their ship, closing the airlock behind them, and you return to your bridge.
    You prepare to undock from the {plt}, but before you complete the procedures there is a sudden power spike in your primary systems. All panels go black. In the darkness, the only thing that disturbs the silence is the sound of the {plt} dislodging itself from your docking clamp.
    Seconds later, the computer core reboots itself and your controls come back online, but you find to your dismay that your OS has been reset to factory defaults. All custom content has been lost - including your logs of meeting the {plt}...]]), {plt=shipname}))
      var.push("shadowrun_failed", true)
   else
      -- player reports in with SHITMAN
      tk.msg(_("An unexpected reunion"), fmt.f(_([[You complete docking operations with the {plt}, well aware that your ship isn't carrying the man they were expecting. When the airlock opens, you find yourself face to face with a woman and two crewmen, all wearing grey, featureless uniforms. It takes you a few moments to realize that the woman is in fact Rebina. But this is not the elegant, feminine figure you met in the spaceport bar not too long ago. This woman emits an aura of authority, and you immediately understand that Rebina is in fact captain of the {plt}.
    "Well met, {player}," she greets you. At the same time, the two crewmen that accompanied her push their way past you and disappear in the direction of your cargo hold. You open your mouth to protest, but Rebina raises a hand to forestall you. "There is no cause for concern," she says. "My men are only retrieving that which we sent you to fetch. I assure you that your ship and its cargo will be left undisturbed."
    You explain to Rebina that although you met Jorek, he didn't accompany you on your way here. Rebina gives you a grim smile in return. "Oh, I know that. I never expected you to bring him to us in the first place. You see, it's not Jorek we wanted you to get. It was... that."]]), {plt=shipname, player=player.name()}))
      tk.msg(_("An unexpected reunion"), fmt.f(_([[You follow her gaze, and spot the crewmen making their way back to the airlock, carrying between them a small but apparently rather heavy crate. You are absolutely certain you've never seen it before.
    "That is what Jorek was keeping for us on {pnt}, and that is what we need," Rebina explains. "Jorek is nothing but a decoy to draw the Empire's attention away from our real operations. While you were talking to him, his subordinates secured our cargo aboard your ship. We chose not to inform you about this because, well... It's best you didn't know what was in that crate. I'm sure we understand each other."
    Rebina turns to follow her men back into the ship, but before she closes the airlock hatch she looks back at you over her shoulder, shooting you a casual glance that nevertheless seems to see right through you. "I'm glad to see my trust in you was not misplaced," she remarks. "Perhaps we'll see each other again someday, and when we do perhaps we can do some more business."
    Then she is gone. You stare at the airlock hatch, then down at the credit chip in your hand, first marveling at the wealth it represents and then astonished to realize you can't remember how it got there.]]), {pnt=pnt}))
      player.pay(mem.credits)
   end

   player.unboard()
   seiryuu:setHealth(100, 100)
   seiryuu:changeAI("flee")
   seiryuu:setHilight(false)
   seiryuu:setActiveBoard(false)
   seiryuu:control(false)

   if var.peek("shadowrun") then
     var.pop("shadowrun") -- in case it was used
   end
   shadow.addLog( _([[You participated in an operation for Captain Rebina. You thought you were rescuing a man named Jorek, but it turns out that you were actually helping smuggle something onto Captain Rebina's ship, the Seiryuu. You know next to nothing about Captain Rebina or who she works for.]]) )
   misn.finish(true)
end

function abort()
   misn.finish(false)
end
