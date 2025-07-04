--[[

Example of what can be done with Lua outfits. This is only the Lua-side of the
API and must be set in the XML file. This only works for *Modifier* outfits!

* All functions are optional. In the case they don't exist, nothing happens.
* ontoggle is a special case that allows the player to toggle the outfit if it exists.
* All functions take the pilot equipping the outfit 'p' and the pilot outfit 'po' as parameters.
* They are run in a shared state. All global variables will be the same for all outfits of the same type.
* You can use the 'mem' table to access memory for the particular outfit being used.
* You have access to the pilotoutfit API which is meant for manipulating po.
* Use the <desc_extra> field in the XML when describing what the Lua does.
--]]
-- The 'notactive' variable controls whether the outfit is considered an active
-- outfit or not. By default, it is set to nil and thus the outfit is
-- considered an active outfit. It can be used to have outfits with custom
-- descriptions or prices while not being an active outfit.
notactive = false

-- The 'hidestats' variable controls whether or not the outfit hides stats that
-- have been set with po:set() from the equipment interface. By default they
-- are computed in real time and shown.
hidestats = false

-- The onload is run once when the outfit Lua is loaded. Useful for setting up
-- variables. The passed variable is the outfit itself.
function onload( _o )
end

-- Called to get the extra description of the outfit. This gets appended to the
-- summary and description, and should be short. The function receives the
-- pilot p if applicable (or nil otherwise), the current outfit o, and the
-- pilot outfit slot po (or nil if not applicable). The function should return
-- a translated string.
function descextra( _p, _o, _po )
   return _("This outfit is very cool.")
end

-- Called when the price of the outfit for the player is check
-- Returns 4 values, the cost string, whether the player can buy it,
-- whether the player can sell it, and what to display for the player "You
-- have:" field. Note the last return is optional and defaults to displaying
-- the player's owned credits when not specified.
function price( q )
   local pricestr = string.format("%d credits",500*q) -- Use format library instead
   local canbuy = true
   local cansell = true
   local youhave = string.format("%d credits",player.credits())
   return pricestr, canbuy, cansell, youhave
end

-- Run when the player tries to buy an outfit. Should determine whether the
-- outfit was bought or not (deducting costs), or return false and the reason
-- for not being able to buy the outfit.
function buy( q )
   player.pay( -500*q, "outfit_buy" )
   return true, q
   --return false, reason
end

-- Run when the player tries to sell an outfit. Should determine whether the
-- outfit was sold or not (deducting costs), or return false and the reason
-- for not being able to sell the outfit.
function sell( q )
   player.pay( 500*q, "outfit_sell" )
   return true, q
   --return false, reason
end

-- The init is run when the pilot is created or enters a new system (takeoff/jumpin)
function init( _p, _po )
end

-- The function is run when the outfit is added to the pilot
-- This function is run _AFTER_ the outfit is added, so the po struct is still valid.
function onadd( _p, _po )
end

-- The function is run when the outfit is removed from the pilot.
-- This function is run _BEFORE_ the outfit is removed, so the po struct is still valid.
function onremove( _p, _po )
end

-- The function is run when any outfit is changed on the pilot
-- This is guaranteed to be run _AFTER_ onadd and onremove functions are called.
function onoutfitchange( _p, _po )
end

-- The update function is run periodically every 1/10 seconds
-- 'dt' is the time elapsed since last time run in seconds
function update( _p, _po, _dt )
end

-- When the pilot is out of energy, this function triggers. Note that before
-- this triggers, 'ontoggle( p, po false, false )' will be run if it exists.
-- This is especially useful for outfits that can't be toggled, but want to
-- turn off when they run out of energy.
function outofenergy( _p, _po )
end

-- The onhit function is run when the pilot 'p' takes damage
-- armour is the amount of armour damage taken (in GJ)
-- shield is the amount of shield damage taken (in GJ)
-- attacker is the pilot that attacked
function onhit( _p, _po, _armour, _shield, _attacker )
end

-- The onshoot function is run when the pilot 'p' shoots ANY weapon. This includes primary / secondary / instant weapon sets
function onshootany( _p, _po )
end

-- The ontoggle function is run when the outfit is part of a toggle weapon set
-- and is toggled by the player. If it was not "on" when toggled, the on value
-- will be true, it will be false otherwise.
-- It returns a boolean whether or not the outfit was actually triggered.
function ontoggle( _p, _po, _on, _natural )
   return false
end

-- The onstealth function runs when the pilot stealths or destealths
-- stealthed the current stealth status of the pilot
function onstealth( _p, _po, _stealthed )
end

-- Run when the pilot is scanned by a scanner pilot.
function onscanned( _p, _po, _scanner )
end

-- Run when the pilot scans a target pilot.
function onscan( _p, _po, _target )
end

-- The cooldown function is triggered when both cooldown starts and when
-- it ends. The done is a boolean value which indicates whether or not it
-- finished. In the case done is false, opt will indicate the number of
-- seconds the cooldown will take. If done is true, then opt will be a
-- a boolean indicating whether or not it successfully completed.
function cooldown( _p, _po, _done, _opt )
end

-- Triggered when pilot lands (player only)
function land( _p, _po )
end

-- Triggered when pilot takes off (player only)
function takeoff( _p, _po )
end

-- Triggered when pilot jumps in (player only)
-- Run AFTER the player is in the new system.
function jumpin( _p, _po )
end

-- Triggered when pilot boards a target ship
function board( _p, _po, _target )
end

-- Triggered when the player double taps a specific key, which can be either "accel", "left", or "right". Should return true if it activated something
function keydoubletap( _p, _po, _key )
end

-- Triggered when the player releases a specific key, same as "keydoubletap"
function keyrelease( _p, _po, _key )
end

-- Triggered when the outfit receives a message from some source.
function message( _p, _po, _msg, _data )
end

-- Run when the pilot is dead and about to start exploding
function ondeath( _p, _po )
end

-- Run when any particle shot by p impacts the target. Unlike onimpact, p is
-- guaranteed to exist.
function onanyimpact( _p, _po, _target, _pos, _vel, _o )
end

--[[
   Below are WEAPONS ONLY. However, they can be triggered by munitions,
   which share the environment with the ORIGINAL spawning outfit, not necessarily
   the true outfit. So if you create a "Laser MK1" munition from a new outfit
   called "NewOutfit", it would use the environment of "NewOutfit", and not "Laser
   MK1".

   Note that these are calculated with particles.
--]]

-- Triggered when the weapon is about to shoot and can be used to control
-- whether or not it should be able to shoot. By returning true, it'll start
-- firing the weapon, while returning false will block in from shooting.
function onshoot( _p, _po )
   return false
end

-- Triggered when a particle shot by p impacts the target. p may not exist if
-- the pilot that shot the weapon ceased to exist.
function onimpact( _p, _target, _pos, _vel, _o )
end

--- Triggered when a particle times out without hitting a target. p may not
--exist if the pilot that shot the weapon ceased to exist.
function onmiss( _p, _pos, _vel, _o )
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
