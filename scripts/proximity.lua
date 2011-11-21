-- Poll for player proximity to a point in space.
-- argument trigger: a table containing:
-- location: The location, OR
-- anchor: the pilot to use as the anchor for the trigger area
-- radius: The radius around the location or anchor
-- focus: The pilot that's polled for. If omitted, defaults to the player.
-- funcname: The name of the function to be called when the player is in proximity.
-- 
-- Example usage: hook.timer(500, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "function"})
-- Example usage: hook.timer(500, "proximity", {anchor = mypilot, radius = 500, funcname = "function"})
-- Example usage: hook.timer(500, "proximity", {anchor = mypilot, radius = 500, funcname = "function", focus = theirpilot})
function proximity( trigger )
   if trigger.focus == nil then
      trigger.focus = player.pilot()
   end
   _proximity(trigger)
end

-- This variant assumes a proximity hook between two ships, and trigger when the anchor ship scans the focus ship (fuzzy detection doesn't trigger).
-- argument trigger: a table containing:
-- anchor: The pilot to use as the anchor for the trigger area.  If omitted, defaults to the player.
-- focus: The pilot that's polled for.
-- funcname: The name of the function to be called when the player is in proximity.
--
-- Example usage: hook.timer(500, "proximityScan", {focus = theirpilot, funcname = "function"}) -- Triggers when theirpilot is detected by the player
-- Example usage: hook.timer(500, "proximityScan", {anchor = theirpilot, focus = mypilot, funcname = "function"}) -- Triggers when mypilot is detected by theirpilot
function proximityScan( trigger )
   if trigger.anchor == nil then
      trigger.anchor = player.pilot()
   end
   _proximityScan(trigger)
end

function _proximity( trigger )
    if trigger.location ~= nil then
        if vec2.dist(trigger.focus:pos(), trigger.location) <= trigger.radius then
            _G[trigger.funcname]()
            return
        end
    elseif trigger.anchor ~= nil then
        if trigger.anchor:exists() and trigger.focus.exists() and vec2.dist(trigger.focus:pos(), trigger.anchor:pos()) <= trigger.radius then
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
    local hook_id = hook.timer(500, "_proximity", trigger)
    trigger.hook_tbl[1] = hook_id
end

function _proximityScan( trigger )
    if trigger.focus ~= nil and trigger.anchor:exists() and trigger.focus:exists() then
        seen, scanned = trigger.anchor:inrange(trigger.focus)
        if scanned then
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
    local hook_id = hook.timer(500, "_proximityScan", trigger)
    trigger.hook_tbl[1] = hook_id
end

-- Make sure the proximity timer shuts itself off on land or jumpout.
function proximityCancel( trigger )
   if trigger ~= nil then
       hook.rm( trigger.hook_tbl[1] )
       __proximity_tbl[ trigger.__id ] = nil
    end
end

-- Make sure the proximity timer shuts itself off on land or jumpout.
function proximityCancel( trigger )
   if trigger ~= nil then
       hook.rm( trigger.hook_tbl[1] )
       __proximity_tbl[ trigger.__id ] = nil
    end
end


