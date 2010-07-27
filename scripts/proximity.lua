-- Poll for player proximity to a point in space.
-- argument trigger: a table containing:
-- location: The target location
-- radius: The radius around the location
-- funcname: The name of the function to be called when the player is in proximity.
-- 
-- Example usage: hook.timer(500, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "function"})
function proximity(trigger)
    if vec2.dist(player.pos(), trigger.location) <= trigger.radius then
        _G[trigger.funcname]()
    else
        hook.timer(500, "proximity", trigger)
    end
end

