--[[

Example of what can be done with Lua outfits. This is only the Lua-side of the
API and must be set in the XML file.

* All functions are optional. In the case they don't exist, nothing happens.
* ontoggle is a special case that allows the player to toggle the outfit if it exists.
* All functions take the pilot equipping the outfit 'p' and the pilot outfit 'po' as parameters.
* They are run in a shared state. All global variables will be the same for all outfits of the same type.
* You can use the 'mem' table to access memory for the particular outfit being used.
* You have access to the pilotoutfit API which is meant for manipulating po.
* Use the <desc_extra> field in the XML when describing what the Lua does.
--]]

-- The init is run when the pilot is created
function init( p, po )
end

-- The update function is run periodically every 1/3 seconds
-- 'dt' is the time elapsed since last time run in seconds
function update( p, po, dt )
end

-- When the pilot is out of energy, this function triggers. Note that before
-- this triggers, 'ontoggle( p, po false )' will be run if it exists.
-- This is especially useful for outfits that can't be toggled, but want to
-- turn off when they run out of energy.
function outofenergy( p, po )
end

-- The onhit function is run when the pilot 'p' takes damage
-- armour is the amount of armour damage taken (in MJ)
-- shield is the amount of shield damage taken (in MJ)
-- attacker is the pilot that attacked
function onhit( p, po, armour, shield, attacker )
end


-- The ontoggle function allows the oufit to be toggled by the player
-- on is whether it was toggled "on" or "off" and is a boolean value
-- the function returns whether or not the outfit changed state
function ontoggle( p, po, on )
   return false
end


--[[
   Example of an activated outfit implemented fully in Lua
--]]
local cooldown = 10 -- 10 seconds cooldown
local active = 5 -- 5 seconds active
local function enable( po )
   mem.active = true
   mem.timer = active
   po:state( "on" )
end
local function disable( po )
   mem.active = false
   mem.timer = cooldown
   po:state( "cooldown" )
end
function init( p, po )
   mem.active = false
   po:state( "off" )
end
function update( p, po, dt )
   -- Update timer
   mem.timer = mem.timer - dt

   -- Check if need to turn it off
   if mem.active then

      -- Active time over
      if mem.timer < 0 then
         disable( po )
         return
      end
   else

      -- Mark cooldown is over
      if mem.timer < 0 then
         po:state( "off" )
      end
   end
end
function ontoggle( p, po, on )
   if on then
      -- Ignore already active
      if mem.active then return end
      -- Cooldown not over yet
      if mem.timer > 0 then return end
      enable()
   else
      -- Ignore already not active
      if not mem.active then return end
      disable()
   end
end
