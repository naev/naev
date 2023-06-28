--[[

Example of what can be done with Lua ship API to extend regular ship functionality.

--]]
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
