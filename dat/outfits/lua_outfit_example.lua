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
function init( _p, _po )
end

-- The update function is run periodically every 1/3 seconds
-- 'dt' is the time elapsed since last time run in seconds
function update( _p, _po, _dt )
end

-- When the pilot is out of energy, this function triggers. Note that before
-- this triggers, 'ontoggle( p, po false )' will be run if it exists.
-- This is especially useful for outfits that can't be toggled, but want to
-- turn off when they run out of energy.
function outofenergy( _p, _po )
end

-- The onhit function is run when the pilot 'p' takes damage
-- armour is the amount of armour damage taken (in MJ)
-- shield is the amount of shield damage taken (in MJ)
-- attacker is the pilot that attacked
function onhit( _p, _po, _armour, _shield, _attacker )
end

-- The onshoot function is run when the pilot 'p' shoots. This includes primary / secondary / instant weapon sets
function onshoot( _p, _po )
end

-- The ontoggle function allows the oufit to be toggled by the player
-- on is whether it was toggled "on" or "off" and is a boolean value
-- the function returns whether or not the outfit changed state
function ontoggle( _p, _po, _on )
   return false
end

-- The onstealth function runs when the pilot stealths or destealths
-- stealthed the current stealth status of the pilot
function onstealth( _p, _po, _stealthed )
end

-- The cooldown function is triggered when both cooldown starts and when
-- it ends. The done is a boolean value which indicates whether or not it
-- finished. In the case done is false, opt will indicate the number of
-- seconds the cooldown will take. If done is true, then opt will be a
-- a boolean indicating whether or not it successfully completed.
function cooldown( _p, _po, _done, _opt )
end


--[[
   Example of an activated outfit implemented fully in Lua
--]]
local cooldown = 10 -- 10 seconds cooldown
local active = 5 -- 5 seconds active
local function enable( po )
   if mem.timer and mem.timer > 0 then return true end
   mem.active = true
   mem.timer = active
   po:state( "on" )
   return true
end
local function disable( po )
   mem.active = false
   mem.timer = cooldown
   po:state( "cooldown" )
   return true
end
function init( _p, po )
   disable(po)
   po:state( "off" )
   mem.timer = nil
end
function update( _p, po, dt )
   if not mem.timer then return end

   -- Update timer
   mem.timer = mem.timer - dt

   -- Check if need to turn it off
   if mem.active then
      po:progress( mem.timer / active )
      -- Active time over
      if mem.timer < 0 then
         disable( po )
      end
   else
      po:progress( mem.timer / cooldown )
      -- Mark cooldown is over
      if mem.timer < 0 then
         po:state( "off" )
         mem.timer = nil
      end
   end
end
function ontoggle( _p, _po, on )
   if on then
      -- Ignore already active
      if mem.active then return end
      return enable()
   else
      -- Ignore already not active
      if not mem.active then return end
      return disable()
   end
end
