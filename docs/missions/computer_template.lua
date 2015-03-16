--[[

   This is a Naev mission template for missions from the Mission Computer.
   In this document aims to provide a structure on which to build Naev missions.
   The possibilities are endless, so this will not apply to every possible mission.
   For more information on Naev, please visit: http://naev.org/
   Naev missions are written in the Lua programming language: http://www.lua.org/
   There is documentation on Naev's Lua API at: http://api.naev.org/
   You can study the source code of missions in [path_to_Naev_folder]/naev/dat/missions/

   MISSION: <NAME GOES HERE>
   DESCRIPTION: <DESCRIPTION GOES HERE>

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English

-- This section stores the strings (text) for the mission.

-- Mission details. We store some text for the mission with specific variables.

   misn_title = "" 
   misn_reward = ""
   misn_desc = ""

-- Stage one
   title = {}    --Each dialog box has a title.
   text = {}      --We store mission text in tables.  As we need them, we create them.
   title[1] = ""    --Each chunk of text is stored by index in the table.
   text[1] = [[]]     --Use double brackets [[]] for block quotes over several lines.

-- Other stages...
   title[2] = ""
   text[2] = [[]]

-- Comm chatter
   talk = {}
   talk[1] = ""

-- Other text for the mission
   msg_abortTitle = "" 
   msg_abort = [[]]
end


--[[ 
First you need to *create* the mission.  This is *obligatory*.

 ===FOR MISSIONS FROM THE COMPUTER===
Do not run misn.accept() here, but do set the strings for reward/desc/title.
These strings are what will appear in the mission computer screen.

--]]
function create ()
   -- Most missions will need the following to avoid crashing Naev

   -- Mission details: these will appear in the Computer window
   -- You should always set these as soon as possible.
   misn.setTitle( misn_title)
   misn.setReward( misn_reward)
   misn.setDesc( misn_desc)

   -- Markers indicate target system, you may or may not be interested in setting it.
   misn.markerAdd( systemX, "" ) --change as appropriate to point to a system object and marker style.

end

--[[ 
The *accept* function runs when the player accepts the mission from the misssion computer.
*Obligatory* for missions from the computer;
ignored otherwise, unless called from another function.
--]]
function accept ()

--   misn.accept()   -- for missions from the mission computer only.
--   hook.land()    -- only set hooks after accepting.

end

--[[
Use other functions to define other actions.
Connect them to game actions through hooks.
For example you can put hooks in the body of the accept() function.
When the mission ends, use misn.finish( [true or false] ).
The misn.finish function clears the mission from memory and determines if it will appear again.
--]]


--[[
OPTIONAL function that will be run if player aborts the mission.
Nothing happens if it isn't found and the mission fails.
--]]
function abort ()
end

