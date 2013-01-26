--[[

   empire bounty

   Random pirate hitman mission to kill a unique empire patrolman.

   Author: nloewen

--]]

include "numstring.lua"

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "You see a pirate lord raving about something. A significant crowd has gathered around."

   -- Mission details
   misn_title  = "Empire Patrol bounty"
   misn_reward = "%s credits"
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
include("pilot/empire.lua")
include("jumpdist.lua")


function create ()
   -- Note: this mission does not make any system claims. 
   -- Create the target pirate
   emp_name, emp_ship, emp_outfits = emp_generate()

   -- Get target system
   near_sys = get_emp_system( system.cur() )

   -- Get credits
   credits  = rnd.rnd(2,4) * 100000

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
   misn.setReward( string.format( misn_reward, numstring(credits)) )
   misn.setDesc( string.format( misn_desc, emp_name, near_sys:name() ) )
   misn.markerAdd( near_sys, "low" )
   misn.osdCreate(misn_title, {misn_desc:format(emp_name, near_sys:name())})
   -- Some flavour text
   tk.msg( title[1], text[2] )

   -- Set hooks
   hook.enter("sys_enter")
   last_sys = system.cur()
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
   if not sys:presences()["Empire"] then
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
   faction.modPlayerSingle( "Pirate", 5 )
   
   -- Finish mission
   misn.finish(true)
end


-- Entering a system
function sys_enter ()
   cur_sys = system.cur()
   -- Check to see if reaching target system
   if cur_sys == near_sys then

      -- Choose position
      local pos
      if cur_sys == last_sys then
         pos = player.pilot():pos()
      else
         pos = jump.pos(cur_sys, last_sys)
      end
      local x,y = pos:get()
      local d = rnd.rnd( 1500, 2500 )
      local a = math.atan2( y, x ) + math.pi
      local offset = vec2.new()
      offset:setP( d, a )
      pos = pos + offset

      -- Create the badass enemy
      p     = pilot.add(emp_ship, nil, pos)
      emp   = p[1]
      emp:rename(emp_name)
      emp:setHostile()
      emp:setVisplayer(true)
      emp:setHilight(true)
      emp:rmOutfit("all") -- Start naked
      pilot_outfitAddSet( emp, emp_outfits )
      hook.pilot( emp, "death", "emp_dead" )
      hook.pilot( emp, "jump", "emp_jump" )
   end
   last_sys = cur_sys
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
   if rating < 100 then
      emp_ship, emp_outfits = emp_easy()
   elseif rating < 200 then
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
   -- Not reasonable at the moment.
   --if rnd.rnd() < 0.5 then
      return empire_createHawking(false)
   --else
   --   return empire_createPeacemaker(false)
   --end
end

