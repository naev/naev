--[[
-- This is the mission part of the shipwrecked Space Family mission, started from a random event.
-- See dat/events/neutral/shipwreck.lua
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   shipname = "August" --The ship will have a unique name

   title = {}
   text = {}
   directions = {}
   title[1] = ""
   text[1]=[[Dr. Talbot is nearly beside herself with relief when you agree to help. "Oh, thank you thank you thank you. I've thought about what is needed for so long! There is a lot we will need, but I think the things to focus on right now are things related to energy and fuel. Recover as much fuel as you can, and look for batteries, reactors, and capacitors. Forget about solar panels, though. Once I have those, I can start powering equipment back up. Good luck, captain. I will start preparing the equipment."]]

   -- Mission details
   misn_title = "Fuel and Parts Recovery"
   misn_reward = ""
   misn_desc = {}
   misn_desc[1] = "Search the debris field and board derelict craft to recover parts and fuel for Dr. Talot."

   -- Aborted mission
   msg_abortTitle = "A parting of ways"
   msg_abort_space = [[You unceremoniously shove your passengers out of the airlock and into the coldness of space. You're done playing taxi; it's time to get back to important things!]]
   msg_abort_landed = [[You unceremoniously shove your passengers out of the airlock, leaving them to their fate on this planet. You're done playing taxi; it's time to get back to important things!]]

   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Fuel and Parts Recovery"
   osd_msg[1]   = {"Search the debris field and board derelict craft to recover parts and fuel for Dr. Talot."}
end


include("scripts/jumpdist.lua")


function create ()
   misn.accept() -- You boarded their ship, now you're stuck with them.
   misn.setTitle( misn_title )
   misn.setReward( misn_reward )
   misn.setDesc( misn_desc[1] )

	-- Dr. Talbot thanks you
   tk.msg(title[1], text[1])
   player.takeoff()
   
end



function land()
	--the good doctor gets her goodies here and the sends you out for more
end

function abort ()
   if inspace then
      tk.msg(msg_abortTitle, msg_abort_space)
   else
      tk.msg(msg_abortTitle, msg_abort_landed)
   end
   misn.finish(false)
end
