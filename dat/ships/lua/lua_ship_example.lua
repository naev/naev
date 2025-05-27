--[[

Example of what can be done with Lua ship API to extend regular ship functionality.

--]]

update_dt = 0.1 -- Determines how often the 'update' function gets called. Defaults to 0.1 if not set.

-- This is run when needing to tag on an extra description
function descextra( _p, _s )
end

-- The init is run when the pilot is created or enters a new system (takeoff/jumpin)
function init( _p )
end

-- The cleanup function is run when the pilot is about to be removed
function cleanup( _p )
end

-- Run each frame for the pilot
function update( _p, _dt )
end

-- Run when the pilot begins to blow up
function explode_init( _p )
end

-- Runs in place of update when the pilot is blowing up
function explode_update( _p, _dt  )
end

-- Runs when the pilot 'p' shoots ANY weapon.
function onshootany( _p )
end

-- The cooldown function is triggered when both cooldown starts and when
-- it ends. The done is a boolean value which indicates whether or not it
-- finished. In the case done is false, opt will indicate the number of
-- seconds the cooldown will take. If done is true, then opt will be a
-- a boolean indicating whether or not it successfully completed.
function cooldown( _p, _done, _opt )
end
