--[[

   MISSION: Hot dogs from space
   DESCRIPTION: An old man who owns a hot dog factory wants to go to space

   The old man has elevated pressure in his cochlea so he can't go to space.
   He's getting old and wants to go to space before he dies. He owns a hot dog
   factory and will pay you in hot dogs (food). Because of his illness, you
   can't land on any planet outside the system (the player doesn't know this).

   NOTE: This mission is best suited in systems with 2 or more planets, but can
   be used in any system with a planet.

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English

-- This section stores the strings (text) for the mission.

-- Bar information, describes how he appears in the bar
   bar_desc = "You see an old man with a cap on, on which the letters R-E-Y-N-I-R are imprinted."

-- Mission details. We store some text for the mission with specific variables.
   misn_title = "Rich reward from space!"
   misn_reward = "Lots of cash"
   misn_desc = "Reynir wants to travel to space and will reward you richly."

   cargoname = "Food"

-- Stage one
   title = {}    --Each dialog box has a title.
   text = {}      --We store mission text in tables.  As we need them, we create them.
   title[1] = "Spaceport Bar"    --Each chunk of text is stored by index in the table.
   text[1] = [["Do you like money?"]]     --Use double brackets [[]] for block quotes over several lines.

   text[2] = [["Ever since I was a kid I've wanted to go to space. However, my doctor says I can't go to space because I have an elevated pressure in my cochlea, a common disease around here.
    "I am getting old now, as you can see. Before I die I want to travel to space, and I want you to fly me there! I own a hot dog factory, so I can reward you richly! Will you do it?"]]
   text[3] = [["Thank you so much! Just fly me around in the system, preferably near %s."]]

   title[4] = "Reynir"
   text[4] = [[Reynir walks out of the ship. You notice that he's bleeding out of both ears. "Where have you taken me?! Get me back to %s right now!!"]]
   text[5] = [["Thank you so much! Here's %s tons of hot dogs. They're worth more than their weight in gold, aren't they?"]]
   text[6] = [[Reynir walks out of the ship, amazed by the view. "So this is how %s looks like! I've always wondered... I want to go back to %s now, please."]]
   text[7] = [[Reynir doesn't look happy when you meet him outside the ship.
    "I lost my hearing out there! Damn you!! I made a promise, though, so I'd better keep it. Here's your reward, %d tons of hot dogs..."]]

-- Comm chatter -- ??
   talk = {}
   talk[1] = ""

-- Other text for the mission -- ??
   osd_msg = {}
   osd_msg[1] = "Fly around in the system, preferably near %s."
   osd_msg[2] = "Take Reynir home to %s."
   msg_abortTitle = "" 
   msg_abort = [[]]
end


function create ()
   -- Note: this mission does not make any system claims. 
   misn.setNPC( "Reynir", "neutral/unique/reynir" )
   misn.setDesc( bar_desc )

   -- Mission variables
   misn_base, misn_base_sys = planet.cur()
   misn_bleeding = false
end


function accept ()
   -- make sure there are at least 2 inhabited planets
   if (function () 
            local count = 0
            for i, p in ipairs (system.cur():planets()) do
               if p:services()["inhabited"] then
                  count=count+1
               end
            end
            return count > 1
         end) ()
      and tk.yesno( title[1], text[1] ) and tk.yesno( title[1], text[2] ) then

      misn.accept()  -- For missions from the Bar only.

      misn.setTitle( misn_title )
      misn.setReward( misn_reward )
      misn.setDesc( misn_desc )
      hook.land( "landed" )

      tk.msg( title[4], string.format(text[3], misn_base:name()) )
      misn.osdCreate(misn_title, {osd_msg[1]:format(misn_base:name())})
      cargoID = misn.cargoAdd( "Civilians", 0 )
   end

end


function landed()
   -- If landed on misn_base then give reward
   if planet.cur() == misn_base then
      misn.cargoRm( cargoID )
      if misn_bleeding then
         reward = math.min(1, player.pilot():cargoFree())
         reward_text = text[7]
      else
         reward = player.pilot():cargoFree()
         reward_text = text[5]
      end
      tk.msg( title[4], string.format(reward_text, reward) )
      player.pilot():cargoAdd( cargoname, reward )
      misn.finish(true)
   -- If we're in misn_base_sys but not on misn_base then...
   elseif system.cur() == misn_base_sys then
      tk.msg( title[4], string.format(text[6], planet.cur():name(), misn_base:name()) )
      misn.osdCreate(misn_title, {osd_msg[2]:format(misn_base:name())})
   -- If we're in another system then make Reynir bleed out his ears ;)
   else
      tk.msg( title[4], string.format(text[4], misn_base:name()) )
      misn.osdCreate(misn_title, {osd_msg[2]:format(misn_base:name())})
      misn_bleeding = true
   end
end

-- TODO: There should probably be more here
function abort ()
   -- Remove the passenger.
   misn.cargoRm( cargoID )
   misn.finish(false)
end
