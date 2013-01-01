--[[

   This is a Naev mission template.
   In this document aims to provide a structure on which to build many Naev missions.
   The possibilities are endless, so this will not apply to every possible mission.
   For more information on Naev, please visit: http://code.google.com/p/naev/ 
   Naev missions are written in the Lua programming language: http://www.lua.org/
   There is documentation on Naev's Lua API at: http://bobbens.dyndns.org/naev-lua/index.html
   You can study the source code of missions in [path_to_Naev_folder]/naev/dat/missions/

   MISSION: <NAME GOES HERE>
   DESCRIPTION: <DESCRIPTION GOES HERE>

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English

-- This section stores the strings (text) for the mission.

-- Bar information, describes how the person appears in the bar
   bar_desc = ""

-- Mission details. We store some text for the mission with specific variables.
   misn_title = "" 
   misn_reward = 0
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

You have to set the NPC and the description. These will show up at the bar with
the character that gives the mission and the character's description.
--]]
function create ()
   misn.setNPC( "A Guy", "none" )
   misn.setDesc( bar_desc )
end


--[[
This is an *obligatory* part which is run when the player approaches the character.

 ===FOR MISSIONS FROM THE BAR or OTHER PLACES===
Run misn.accept() here, this enables the mission to run when the player enters the bar.
If the mission doesn't get accepted, it gets trashed.
Also set the mission details.
--]]
function accept ()
   -- Most missions will need the following to avoid crashing Naev

   -- This will create the typical "Yesn/No" dialogue when mission is created
   -- at bar.  It returns true if yes was selected.
   if tk.yesno( title[1], text[1] ) then

      -- Generally the first thing you want to do when giving the mission is accept
      -- it so that functions work as expected.
      misn.accept()  -- For missions from the Bar only.

      -- Mission details:
      -- You should always set mission details right after accepting the mission
      misn.setTitle( misn_title)
      misn.setReward( misn_reward)
      misn.setDesc( misn_desc)
      -- Markers indicate a target system on the map, it may not be needed
      -- depending on the type of mission you're writing.
      misn.markerAdd( systemX, "" ) --change as appropriate to point to a system object and marker style.
   end

end


--[[
Use other functions to define other actions.
Connect them to game actions through hooks.
For example you can put hooks in the body of the main create() and accept() functions.
When the mission ends, use misn.finish( [true or false] ).
The misn.finish() function clears the mission from memory and determines if it will appear again.
--]]


--[[
OPTIONAL function that will be run if player aborts the mission.
Nothing happens if it isn't found and the mission fails.
--]]
function abort ()
end
