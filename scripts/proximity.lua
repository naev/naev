-- Poll for player proximity to a point in space.
-- argument trigger: a table containing:
-- location: The target location, OR
-- anchor: the pilot to use as the anchor
-- radius: The radius around the location
-- funcname: The name of the function to be called when the player is in proximity.
-- 
-- Example usage: hook.timer(500, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "function"})
-- Example usage: hook.timer(500, "proximity", {anchor = mypilot, radius = 500, funcname = "function"})
function proximity( trigger )
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

    -- Check global proxmitiy table
    if __proximity_tbl == nil then
       __proximity_tbl = {}
       __proximity_tbl["__save"] = true
       __proximity_tbl["id"] = 0
    end

    -- Assign ID if necessary
    if trigger.__id == nil then
       __proximity_tbl["id"] = __proximity_tbl["id"]+1
       trigger.__id = __proximity_tbl["id"]
       __proximity_tbl[ trigger.__id ] = trigger
    end

    -- First time hook is set
    if trigger.hook_tbl == nil then
       trigger.hook_tbl = {}
       hook.enter("proximityCancel", trigger)
    end

    -- Set new timer hook
    local hook_id = hook.timer(500, "proximity", trigger)
    trigger.hook_tbl[1] = hook_id
end

-- Make sure the proximity timer shuts itself off on land or jumpout.
function proximityCancel( trigger )
   if trigger ~= nil then
       hook.rm( trigger.hook_tbl[1] )
       __proximity_tbl[ trigger.__id ] = nil
    end
end


