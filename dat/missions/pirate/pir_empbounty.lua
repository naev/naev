--[[

   Pirate Hitman 3

   Random pirate hitman mission to kill a unique empire patrolman.

   Author: nloewen

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "You see a pirate lord raving about something. A significant crowd has gathered around."

   -- Mission details
   misn_title  = "Empire Patrol bounty near %s"
   misn_reward = "%d credits"
   misn_desc   = "There is a bounty on the head of the Empire Patrol known as %s who was last seen near the %s system."

   -- Text
   title    = {}
   text     = {}
   title[1] = "Spaceport Bar"
   text[1]  = [[It seems like the bounty is on the head of an Empire Patrol working in the area known as %s for %d credits. It seems like he was last seen in the %s system. Quite a few other pirates seem interested and it looks like you'll have to outrace them.
   
Will you take up the bounty?]]
   text[2] = [[You roll up your sleeve and head off to your ship.]]

   -- Messages
   msg      = {}
   msg[1]   = "MISSION SUCCESS! Payment received."
   msg[2]   = "Pursue %s!"
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

   -- Get credits
   credits  = rnd.rnd(2,4) * 10000

   -- Spaceport bar stuff
   misn.setNPC( "Pirate Lord", pir_getLordRandomPortrait() )
   misn.setDesc( bar_desc )
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
   misn.markerAdd( near_sys, "low" )

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

-- Player won, gives rewards.
function give_rewards ()
   -- Give monies
   player.pay(credits)

   -- Give factions
   player.modFaction( "Pirate", 5 )
   
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
      hook.pilot( emp, "death", "emp_dead" )
      hook.pilot( emp, "jump", "emp_jump" )
   end
end


-- Empire Patrol is dead
function emp_dead ()
   player.msg( msg[1] )
   give_rewards()
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

