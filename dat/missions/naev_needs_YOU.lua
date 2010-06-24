--[[

MISSION TITLE: NAEV Needs You!
DESCRIPTION: 
         A mission that pops up to encourage new players to contribute to the project.

      Mission Stages:
         [1]   The player gets a glimpse of the creators of the universe at work.

]]--

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

-- This section stores the strings (text) for the mission.

-- Mission details
   misn_title = "NAEV needs you!"
   misn_reward = "Creative satisfaction and a place in the credits!"
   misn_desc = "Come help NAEV grow."

-- Stage one: something goes wrong in fabric of reality.
   title = {}
   text = {}
   title[1] = "NAEV received SIGSEGV (address not mapped to object)!"
   text[1] = [[As you step out of your ship, suddenly the sunlight flickers. In the sky, the local star turns blue, then a deep purple, and then goes dark. The whole planet plunges into darkness.

Then a voice speaks, louder than an earthquake. It seems to sound inside your bones and from the very centre of the earth. "Oh crap..." it says.

Another voice laughs, musically, as if world itself were singing. "bobbens, did you just destroy the universe, again?"

"Mmmm, hold on," first voice rumbles. "...SIGSEGV... libc start main+0xe5... ah ha!  There's the problem."  You feel something fundamental in the fabric of reality change. The sensation is like sneezing and hiccuping at the same time. "There that should fix it."

A window opens in the darkness, infinitely far away and infinitely large. A face peeps down at you from the height of eternity.

"Everything all right in there?"]] --yes/no choice

-- If yes, the voice of your creator speaks to you:
   title[2] = "The voice of the creator"
   text[2] = [["Good. Sorry for the inconvenience. Please don't worry, I'll have reality reloaded again in just a sec."

The creator turns to go, then pauses.

"By the way, we're working hard on improving life, the universe, and all that, but we're a bit short handed. If you like this existence but think the details could be improved, the universe creation team could use your help. We'd be especially happy to see you if you have any special skill in drawing planets, designing ships, scripting the underlying laws of nature, or composing the music of the spheres. For information on how to contribute, just point your web-of-reality browser to http://code.google.com/p/naev/."

The clockmaker lifts his eyes up beyond your frame of reference and smiles.

"OK then. Pushed. And we're back in 3... 2... 1..."]]

-- If no, the voice of your creator speaks to you:
   title[3] = "The voice of the creator"
   text[3] = [["Oh sorry about that. We're working hard on improving life, the universe, and all that, but the team is a bit short handed. Don't worry, I'll have reality reloaded again in just a sec."
   
The creator turns to go, then pauses.

"I tell you what: the universe creation team could really use your help. If you have any special skill in drawing planets, designing ships, scripting the underlying laws of nature, or composing the music of the spheres we'd be especially happy to see you. For information on how to contribute, just point your web-of-reality browser to http://code.google.com/p/naev/"

The clockmaker lifts his eyes up beyond your frame of reference and smiles.

"OK then. Pushed. And we're back in 3... 2... 1..."]]

end 


-- The mission runs when the player lands.
function create()

      -- Mission should only appear if player has played a bit.
      if player.credits() < 250000 then
         misn.finish(false)
      end

      if music.isPlaying() then
         music.stop()
      end
      music.load( "sirius1")
      music.play()
      if tk.yesno( title[1], text[1] ) then
         misn.accept()
         tk.msg( title[2], text[2])
         misn.setReward( misn_reward )
         misn.setDesc( misn_desc)
         misn.setTitle( misn_title)
         misn.markerAdd( system.cur(), "low" )

     -- Mission ends with a little comment after blasting off.
         hook.enter( "enter_system")

      else
         misn.accept()
         tk.msg( title[3], text[3])
         misn.setReward( misn_reward )
         misn.setDesc( misn_desc)
         misn.setTitle( misn_title)
         misn.markerAdd( system.cur(), "low" )
         
     -- Mission ends with a little comment after blasting off.
         hook.enter( "enter_system")
      end
      music.stop()

end

-- Decides what to do when player either takes off starting planet or jumps into another system
function enter_system()

      player.msg( "Well, that was weird")
      misn.finish( true)

end
