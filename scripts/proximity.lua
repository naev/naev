-- Poll for player proximity to a point in space.
-- argument trigger: a table containing:
-- location: The target location, OR
-- anchor: the pilot to use as the anchor
-- radius: The radius around the location
-- funcname: The name of the function to be called when the player is in proximity.
-- 
-- Example usage: hook.timer(500, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "function"})
-- Example usage: hook.timer(500, "proximity", {anchor = mypilot, radius = 500, funcname = "function"})
function proximity(trigger)
    if trigger.location ~= nil then
        if vec2.dist(player.pos(), trigger.location) <= trigger.radius then
            _G[trigger.funcname]()
            return
        end
    elseif trigger.anchor ~= nil then
        if trigger.anchor:exists() and vec2.dist(player.pos(), trigger.anchor:pos()) <= trigger.radius then
            _G[trigger.funcname]()
            return
        end
    end
    local proximityTimerHook = hook.timer(500, "proximity", trigger)
    hook.jumpout("proximityCancel", proximityTimerHook)
    hook.land("proximityCancel",    proximityTimerHook)
end

-- Make sure the proximity timer shuts itself off on land or jumpout.
function proximityCancel( hook_id )
    if hook_id ~= nil then
       hook.rm( hook_id )
    end
end
