--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Chicken Rendezvous">
 <location>none</location>
 <chance>0</chance>
 <unique />
 <notes>
  <campaign>Minerva</campaign>
  <done_evt name="Spa Propaganda" />
  <requires name="Chicken Rendezvous" />
 </notes>
</event>
--]]

--[[
-- Triggered from other missions.
--]]
local vn = require 'vn'
local minerva = require 'common.minerva'
local love_shaders = require "love_shaders"

function create()
   local steamshader = love_shaders.steam()
   local angrysong = 'snd/sounds/songs/feeling-good-08.ogg'
   local bgsong = minerva.loops.kex

   vn.clear()
   vn.scene()
   local t = vn.newCharacter( minerva.vn_terminal() )
   vn.transition("hexagon")
   -- TODO fancier intro?
   vn.na(_("You show the winning ticket to the nearest terminal. It suddenly starts blasting music and flashing lights before triumphantly announcing to everyone in the vicinity:"))
   t(_([["CONGRATULATIONS MISTER HARPER BOWDOIN ON WINNING THE EXCLUSIVE STAY AT THE MINERVA SPA."]]))
   vn.na(_("If only the terminal could be more discrete… You follow the terminal as it leads you towards the spa facilities. All the rooms you pass seem to have the same gaudy decorations and similar patrons. You wonder if you will be able to make it back to where you came from."))
   t(_([[The terminal glides around the maze that is Minerva station while its head rotates backwards to face you.
"I BELIEVE YOU WILL BE VERY IMPRESSED WITH THE ACCOMMODATIONS AT THE SPA. THE RENOVATIONS TOOK ALMOST A CYCLE AND IT IS MORE BEAUTIFUL THAN EVER."]]))
   t(_([["TODAY THE WATER IS INFUSED WITH NANOBOTS THAT SPECIALIZE IN ANTI-AGEING, AFTER A DIP YOU WILL LOOK YOUNGER THAN EVER. THESE HAVE BEEN DEVELOPED BY THE ZA'LEK AND SHOULD EVEN SURPASS THE ANTI-AGEING PROPERTIES OF THE SOROMID SKIN LEECHES, WITHOUT EVEN CLOUDING THE WATER."]]))
   vn.na(_("As you walk, you feel that the surrounding decorations are starting to get fancier, you must be nearing the VIP area."))
   t(_([["FURTHERMORE, CYBORG CHICKEN WILL BE JOINING YOU IN THE SPA. DO NOT WORRY, HIS CYBORG IMPLEMENTS ARE ALL WATER-PROOF UNLIKE MY BODY. MAKE SURE TO MAKE MOST OF THIS ONCE IN A LIFETIME OPPORTUNITY."]]))
   t(_([[The terminal stops moving in front of a airlock encrusted with what look to be diamonds on a gold enamel.
"WE HAVE REACHED OUR DESTINATION. PLEASE ENJOY YOURSELF."]]))
   vn.disappear( t, 'slideleft' )
   vn.na(_("The airlock opens as you approach it, and you find yourself at the entrance of the changing rooms. You get rid of your clothes, take a small towel, and proceed to enter the spa area."))
   -- Start the fancy spa scene!
   vn.scene()
   vn.music( 'snd/sounds/loops/riviere.ogg' )
   vn.func( function ()
      local lw, lh = love.graphics.getDimensions()
      vn.setBackground( function ()
         vn.setColour( {1, 1, 1, 1} )
         local oldshader = love.graphics.getShader()
         love.graphics.setShader( steamshader )
         love.graphics.draw( love_shaders.img, 0, 0, 0, lw, lh )
         love.graphics.setShader( oldshader )
      end )
      vn.setUpdateFunc( function( dt )
         steamshader:update(dt)
      end )
   end )
   vn.transition()
   vn.na(_("As you enter the spa, you are hit by a rolling wave of steam, probably infused with the nanobots mentioned by the terminal, and find yourself alone. It smells like some sort of mix between lemons and assorted herbs, but is rather quite pleasant."))
   vn.na(_("You can make out several large pools in the thick fog, and there is a constant background sound of running water. Although not too fancy by the standards of volcanic world natural springs, it is very impressive when you consider that this installation is located inside a space station."))

   -- Special case doing Minerva Pirate mission
   if player.misnActive( "Minerva Pirates 3" ) then
      vn.na(_("Before you enjoy yourself, you plant the listening device you were supposed to in a corner of the room so that it won't be noticed. That should make your current employer happy and get your work out of the way."))
   end

   vn.na(_("There is a sign with instructions on how to properly bath. It seems like you're supposed to wash your body first in small individual stalls before getting into the large baths. You proceed to wash your body thoroughly, getting rid of all the grime from your travels. It's amazing how much dirt accumulates by space travel, probably obtained on all sorts of different planets."))
   vn.na(_("You enter the first bath you find. The hot water seems to seep into every pore of your skin in an incredibly relaxing and soothing manner. You melt into the water and let it all flow in. You should come here more often."))
   vn.na(_("As you lose yourself in space and time, you are brought back to reality by a faint sound of splashing. That's right, you were supposed to be joined in the spa by Cyborg Chicken. Given how great the spa is, you can't help but to question why the management decided to include Cyborg Chicken into this renovation event deal."))
   vn.na(_("The splashing sound stops, and you hear the pit-pat sound of chicken feet approaching you."))

   local cc = minerva.vn_cyborg_chicken()
   vn.appear( cc, "blur" )
   cc(_("Cyborg Chicken comes up to the pool you are in and tests the water by sticking a foot in. Seeming satisfied, it jumps in and starts floating."))
   vn.na(_("Concentrating on the cathartic spa water, you let your thoughts wander, while Cyborg Chicken floats around lazily."))

   if player.misnActive( "Minerva Pirates 3" ) then
      vn.na(_("Is there really a mole at the station? What could have happened to Maikki's father? What is the shady character's objective? It seems like there are many loose ends at Minerva station."))
   --else -- TODO case other missions when implemented
   end

   vn.na(_("Eventually you get out of your thinking stupor and remember that you are in a spa with a chicken, that happens to be a cyborg. This doesn't seem like is something that happens very often."))
   vn.menu( {
      { _("Squawk at the chicken"), "menu1" },
      { _([["Yo chicken!"]]), "menu1" },
   } )
   vn.label("menu1")
   cc(_([[Cyborg Chicken stares at you intensely.]]))
   vn.na(_([[You once again wonder about what the entire point of swimming with a chicken is, even if it is a cyborg one.]]))
   cc(_([[They still are staring at you with a creepy fixation.]]))
   cc(_([[Finally, after what seems like an eternity of being stared down by a chicken, you hear a small faint sound come out of it. However, you can't make out what the sound was.]]))
   vn.me(_([["Pardon me?"]]))
   cc:rename(_("Cyborg Duck?"))
   cc(_([[
They look at you straight in the eyes and begin to speak, slowly getting louder.
"Actually, I'm not a chicken: I'm a duck. Well, genetically speaking, mostly a duck."]]))
   vn.menu( {
      { _("…"), "menu2" },
      { _([["A duck?"]]), "menu2" },
   } )
   vn.label("menu2")
   vn.func( function () cc.shader = love_shaders.aura() end )
   vn.music( angrysong, nil, true )
   cc(_([["A bloody duck! With this stupid excuse for a cybernetic implant that can't do shit. What the hell am I supposed to do as a duck? Shit on the floor? Eat bird food? What kind of life is that? Maybe I can pop out eggs and sell them for a living? Oh wait, I'm a damn male."
They continue rambling furiously.]]))
   vn.menu( {
      { _([["Are you okay?…"]]), "menu3_ok" },
      { _([["Quack"]]), "menu3_quack" },
   } )
   vn.label("menu3_ok")
   cc(_([["Do I look like I'm bloody OK? Stuck in the middle of nowhere as a bloody duck!"]]))
   vn.jump("menu3")
   vn.label("menu3_quack")
   cc(_([["Bloody hell! What did I deserve to get mocked in a space station in the middle of nowhere!"]]))
   vn.jump("menu3")
   vn.label("menu3")
   cc(_([["Forced to serve blackjack cards all day to idiots who think they can get rich in a rigged game. No, you are not going to surprise your spouse or lover or whatever with a ton of credits, you're going to end up broke crying in the toilet stall. Bloody who do they think they are."]]))
   cc(_([["Is this what is to become of me? The damn attraction of a perverse station lit up with neon lights that attracts idiots as surely as moths to flame. Whatever happened to me, Kex, intrepid explorer of the nebula…"]]))
   vn.menu( {
      { _([["Kex…!"]]), "menu4" },
      { _([["Intrepid explorer…?"]]), "menu4" },
   } )
   vn.label("menu4")
   vn.func( function () cc.shader = nil end )
   vn.musicStop( angrysong )
   vn.music( bgsong, nil, true )
   cc:rename(_("Kex"))
   cc(_([["Yeah, the one and only. I guess you want to hear the full story."
They take a deep sigh.
"You look like a good kid, and I'm bloody fed up with everything to give a shit anymore."]]))
   cc(_([["I am Kex McPherson… scratch that… more like was Kex McPherson before I was turned into this travesty. Brave explorer of the nebula."
His eyes get a bit teary as he starts to recall the past.
"You see, I was lucky enough to not get caught in the Incident, although most of my friends and family were caught up in it."]]))
   cc(_([["I knew I couldn't just sit still and decided to see if I could find anything about what happened or any clues or pretty much anything. I was young and naïve, my daughter had recently been born, and I wanted to make sure the world would be a better place for her. You know, like not repeating the mistakes of the past, and figure out what went wrong."]]))
   cc(_([["But the nebula was not a kind mistress. It sort of gets into your bones you know? Especially right after the Incident, it was very unstable. Lost a lot of good fellow explorers due to explosions and the Nebula madness. It sort of turns you into a monster, incapable of rational thought, and they would usually succumb to their own uncontrolled greed."]]))
   cc(_([["Horrible thing to see, but I was careful and never caught it. We were bringing back many artefacts, some of things that we could make out, like parts of civilian vessels, but sometimes we found really weird shit, you know? Stuff that we had no idea where it came from."]]))
   cc(_([["I still had to make a living, and would sell many of those artefacts to the Za'lek. They would buy pretty much anything. No idea what the hell they were doing with it, but I still had my wife and daughter to tend to, and it paid the bills. Sometimes life doesn't really give you options, you just have to make do."]]))
   cc(_([["Me and my first mate Mireia Sibeko, we would spend most of our days travelling in and out of the nebula. Once Cerberus Outpost in Doeston got set up, it was much easier to do stuff, but we were already exploring the new reality much before that station was up and running. I miss those days, the good old times."]]))
   cc(_([["One day, I don't remember what happened, but apparently we got caught up into an accident or something. Everything went dark, and when I woke up, I was in that sick bastards laboratory being chopped up and rebuilt. I don't really have much memory of all that traumatic experience. You can't really expect to remember much when you are blacking out from pure agony."]]))
   cc(_([["However, I will never forget his bloody name. The guy who did this to me, Strangelove, he experimented on me, torturing me, and turned me into this monstrosity. I'm a bloody fusion of hell knows what poultry and horrible Za'lek technology."]]))
   cc(_([["Sometimes the implants start ringing really loud in my head, and I pass out from the pain… I never asked for this. I'm condemned to live my days in solitude. Can't bear to think of letting my family see me in this sorry state. I'd rather have them think I died a great explorer."
He looks depressed.]]))
   cc(_([["Apparently, the bastard had some outstanding debt or something, and I was taken by some cretins who brought me here as their slave pet. I was able to feign stupidity to avoid any issues. Pretending I had some blackjack software or something also saved me from potentially worse fates."]]))
   cc(_([["And now here I am, just watching time go back, incapable of doing anything."
He seems to be fidgety.]]))
   cc(_([["However, I'm fed up with this bullshit. This isn't living, this is rotting away. I yearn for the stars. Maybe getting back out in space will make me forget all this mockery of an existence."]]))
   cc(_([[Sighing, he looks you in the eyes.
"You only value what you have when you lose it, kid. You should make most of what you have, lady fortune sometimes will deal you a bad one and there might not be any coming back."]]))
   cc(_([["I'll tell you what. Talking to you has been therapeutic or some shit like that. We could talk some time or something. Here, I'll give you the address to a place I like to hang out at the station. It's quiet enough that nobody will give us trouble. You can find me there when I'm off my shifts."]]))
   cc(_([["Speaking of shifts, I believe I am late to my next one. See ya."
Kex slides out of the water, ruffles his feather and fades into the steam. You hear the sound of his duck feet grow fainter until it is quiet except for the sound of running water.]]))
   vn.disappear( cc, "blur" )
   vn.musicStop( bgsong )

   vn.na(_("You are left once more alone in the spa to unpack all the information unleashed to you by Kex. With nothing better to do, you once again melt into the thermal water and let your mind wander."))
   vn.na(_("Eventually you leave the baths, get changed back into your space clothes and head out towards the station."))
   -- Go back to normal BG
   vn.scene()
   vn.music()
   vn.func( function ()
      vn.setBackground()
      vn.setUpdateFunc()
   end )
   vn.transition()
   vn.na(_("The noise and lights once again jar you back to reality harshly. At least you feel younger from the anti-ageing nanobots. Maybe you should get a spa for your ship…"))
   vn.done("hexagon")
   vn.run()

   minerva.log.kex(_("You found out that Cyborg Chicken is Kex, Maikki's father. He told you of a spot you can meet up with him on Minerva station.") )

   evt.finish(true)
end
