--[[

   This is a NAEV mission template.
   In this document aims to provide a structure on which to build many NAEV missions.
   The possibilities are endless, so this will not apply to every possible mission.
   For more information on NAEV, please visit: http://code.google.com/p/naev/ 
   NAEV missions are written in the Lua programming language: http://www.lua.org/
   There is documentation on NAEV's Lua API at: http://bobbens.dyndns.org/naev-lua/index.html
   You can study the source code of missions in [path_to_NAEV_folder]/naev/dat/missions/

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

 ===FOR MISSIONS FROM THE BAR===
Do run misn.accept() here, this enables the mission to run when the player enters the bar.
If the mission doesn't get accepted, it gets trashed.
Also set the mission details.
--]]

function create()
-- Most missions will need the following to avoid crashing NAEV

-- Mission details:
      misn.setTitle( misn_title)
      misn.setReward( misn_reward)
      misn.setDesc( misn_desc)
      misn.setMarker( systemX, "" ) --change as appropriate to point to a system object and marker style.

--      misn.accept()  -- For missions from the Bar only.

end

--[[ 
The *accept* function runs when the player accepts the mission from the misssion computer.
*Obligatory* for missions from the computer;
ignored otherwise, unless called from another function.
--]]
function accept()

--      misn.accept()   -- for missions from the mission computer only.

end

--[[
Use other functions to define other actions.
Connect them to game actions through hooks.
For example you can put hooks in the body of the main create() and accept() functions.
When the mission ends, use misn.finish( [true or false] ).
The misn.finish function clears the mission from memory and determines if it will appear again.
--]]
