--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Naev Needs You!">
 <location>land</location>
 <chance>8</chance>
 <cond>player.credits() &gt; 237451</cond>
 <unique />
</event>
--]]
--[[

EVENT TITLE: Naev Needs You!
DESCRIPTION:
         An event that pops up to encourage new players to contribute to the project.

      Mission Stages:
         [1]   The player gets a glimpse of the creators of the universe at work.

]]--


-- Stage one: something goes wrong in fabric of reality.
local text = {}
text[1] = _([[As you step out of your ship, the sunlight suddenly flickers. The local star in the sky turns blue, then a deep purple, and then goes dark. The whole planet plunges into darkness.
    Then a voice speaks, louder than an earthquake. It seems to echo inside your bones and from the very centre of the galaxy. "Oh crap..." it says.
    Another voice laughs musically, as if world itself were singing. "bobbens, did you just destroy the universe, again?"
    "Mmmm, hold on," the first voice rumbles. "...SIGSEGV... libc start main+0xe5... ah ha!  There's the problem."  You feel something fundamental in the fabric of reality change. The sensation is like sneezing and hiccuping at the same time. "There, that should fix it."
    A window opens in the darkness, infinitely far away and infinitely large. A face peeps down at you from the height of eternity.
    "Everything all right in there?"]]) --yes/no choice

-- If yes, the voice of the creator speaks to you:
text[2] = _([["Good. Sorry for the inconvenience. Please don't worry, I'll have reality reloaded again in just a sec."
    The creator turns to go, then pauses.
    "By the way, we're working hard on improving life, the universe, and all that, but we're a bit short handed. If you like this existence but think the details could be improved, the universe creation team could use your help. We'd be especially happy to see you if you have any special skill in drawing planets, designing ships, scripting the underlying laws of nature, or composing the music of the spheres. For information on how to contribute, just point your web-of-reality browser to naev.org."
    The watchmaker lifts his eyes up beyond your frame of reference and smiles.
    "OK then. Pushed. And we're back in 3... 2... 1..."]])

-- If no, the voice of the creator speaks to you:
text[3] = _([["Oh sorry about that. We're working hard on improving life, the universe, and all that, but the team is a bit short handed. Don't worry, I'll have reality reloaded again in just a sec."
    The creator turns to go, then pauses.
    "I tell you what: the universe creation team could really use your help. If you have any special skill in drawing planets, designing ships, scripting the underlying laws of nature, or composing the music of the spheres we'd be especially happy to see you. For information on how to contribute, just point your web-of-reality browser to naev.org."
    The watchmaker lifts his eyes up beyond your frame of reference and smiles.
    "OK then. Pushed. And we're back in 3... 2... 1..."]])


function create()
      -- Create an eerie atmosphere by cutting off the background music and substituting something spooky
      --disabled until difficulties with the music API are sorted out
      music.play( "sirius1.ogg" )

      -- The big programmer in the sky looks in to ask the player a question
      if tk.yesno( _("Naev received SIGSEGV (address not mapped to object)!"), text[1] ) then
         tk.msg( _("The voice of the creator"), text[2] ) -- if the answer is 'yes'

     -- Mission ends with a little comment after blasting off.
         -- hook.takeoff( "enter_system" )

      else
         tk.msg( _("The voice of the creator"), text[3] ) -- if the answer is 'no', not much different from 'yes'

     -- Mission ends with a little comment after blasting off.
         -- hook.takeoff( "enter_system" )
      end

      -- everything returns to normal
      music.choose( "land" )

      evt.finish( true )
end

--[[
-- A grace note.  Not sure the player will make the connection of the message with the event.
function enter_system()

      player.msg( _("Well, that was weird") )
      evt.finish( true )

end]]--
