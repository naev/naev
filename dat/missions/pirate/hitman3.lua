--[[

   Pirate Hitman 3

   A one time random pirate hitman mission that gets you pirate landing permissions

   Author: nloewen

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "A young buisnessman. I wonder what he is doing here."

   -- Mission details
   misn_title  = "Kill %s"
   misn_reward = "Pirate Landing permision"
   misn_desc   = "There is a empire patrol known as %s who must be terminated. He was last seen near the %s system."
   misn_desc2  = "Return to get your rewards."

   -- Text
   title    = {}
   text     = {}
   title[1] = "Spaceport Bar"
   title[2] = "Mission Complete"
   text[1]  = [[Hello, lets get right too it. I need to hire a pilot, and I've heard from a certain trader that you're good. What do you say about the removal of a empire pilot?]]
   text[2] = [[I thought you would be ok with it. If you compleate this mission, you will receive a pass that will give you landing access at all pirate worlds. If you rat us out, we will find you, and if you fail, well, lets hope that doesn't happen. I hear the empire has some big guns on their side.]]
   text[3] = [[Your back. Congradulations. Here is your landing pass. I look forward to working with you. Now, go plunder something. With that, the young buisnissman turns and leaves. Your begining to question how much of a buisnessman he really is.]]

   -- Messages
   msg      = {}
   msg[1]   = "MISSION SUCCESS! Return for payment."
end



include("dat/missions/pirate/common.lua")


-- Scripts we need
include("scripts/pilot/empire.lua")
include("scripts/jumpdist.lua")


function create ()
   -- Create the target pirate
   emp_name, emp_ship, emp_outfits = emp_generate()
   
   -- Get target system
   near_sys = get_emp_system( system.get() )

   -- Spaceport bar stuff
   misn.setNPC( "Young Businessman",  "none")
   misn.setDesc( bar_desc )
   
   -- hooks to let busninessman approach you
   hook.land( "businessman_timer", "bar" )

   --some other stuff
   misn_base, misn_base_sys = planet.cur()
end

function businessman_timer ()
   hook.timer( 5000, "accept" )
end

--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( title[1], string.format( text[1],
         emp_name, credits, near_sys:name() ) ) then
      misn.finish()
   end
   misn.accept()

   -- Set mission details
   misn.setTitle( string.format( misn_title, near_sys:name()) )
   misn.setReward( string.format( misn_reward, credits) )
   misn.setDesc( string.format( misn_desc, emp_name, near_sys:name() ) )
   misn_marker = misn.markerAdd( near_sys, "low" )

   -- Some flavour text
   tk.msg( title[1], text[2] )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Gets a empireish system
function get_emp_system( sys )
   local s = { }
   local dist = 1
   local target = {}
   while #target == 0 do
      target = getsysatdistance( sys, dist, dist+1, emp_systems_filter, s )
      dist = dist + 2
   end
   return target[rnd.rnd(1,#target)]
end

function emp_systems_filter( sys, data )
   -- Must have Empire
   if not sys:hasPresence( "Empire" ) then
      return false
   end

   -- Must not be safe
   if sys:presence("friendly") > 3.*sys:presence("hostile") then
      return false
   end

   -- Must not already be in list
   local found = false
   for k,v in ipairs(data) do
      if sys == v then
         return false
      end
   end

   return true
end

function misn_finished()
   player.msg( msg[1] )
   misn.setDesc( string.format( misn_desc2, misn_base, misn_base_sys ) )
   misn.markerRm( misn_marker )
   misn_marker = misn.markerAdd( misn_base_sys, "low" )
   hook.land("landed")
end

-- Player won, gives rewards.
function landed ()
   tk.msg(title[2], text[3])

   -- Give factions
   local f = player.getFaction("Pirate")
   if f < 0 then
      f = 0 - f
      player.modFactionRaw( "Pirate", f )
   end
   
   -- Give landing pass   
   player.addOutfit("Pirate Landing Pass")

   -- Finish mission
   misn.finish(true)
end


-- Entering a system
function sys_enter ()
   cur_sys = system.get()
   -- Check to see if reaching target system
   if cur_sys == near_sys then

      -- Create the badass enemy
      p     = pilot.add(emp_ship)
      emp   = p[1]
      emp:rename(emp_name)
      emp:setHostile()
      emp:rmOutfit("all") -- Start naked
      pilot_outfitAddSet( emp, emp_outfits )
      hook.pilot( emp, "death", "misn_finished" )
      hook.pilot( emp, "jump", "emp_jump" )
   end
end


-- Empire Patorl jumped away
function emp_jump ()
   player.msg( string.format(msg[2], emp_name) )

   -- Basically just swap the system
   near_sys = get_emp_system( near_sys )
end


--[[
Functions to create Empire Patrols based on difficulty more easily.
--]]
function emp_generate ()
   -- Get the empire patrols name
   emp_name = empire_name()

   -- Get the empire patols details
   rating = player.getRating()
   if rating < 50 then
      emp_ship, emp_outfits = emp_easy()
   elseif rating < 150 then
      emp_ship, emp_outfits = emp_medium()
   else
      emp_ship, emp_outfits = emp_hard()
   end

   -- Make sure to save the outfits.
   emp_outfits["__save"] = true

   return emp_name, emp_ship, emp_outfits
end
function emp_easy ()
   if rnd.rnd() < 0.5 then
      return empire_createShark(false)
   else
      return empire_createLancelot(false)
   end
end
function emp_medium ()
   if rnd.rnd() < 0.5 then
      return empire_createAdmonisher(false)
   else
      return empire_createPacifier(false)
   end
end
function emp_hard ()
   if rnd.rnd() < 0.5 then
      return empire_createHawking(false)
   else
      return empire_createPeacemaker(false)
   end
end

