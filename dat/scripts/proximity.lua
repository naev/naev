--[[--
   Provides a composable proximity callback: you can use "proximity" as a hook, with the function to call if the player's in range
   as an argument.

   @module proximity
--]]


--[[--
Poll for player proximity to a point in space.
   @tparam table trigger Contains: "location" (destination point) or "anchor" (destination pilot), "radius" (maximum distance),
                         "focus" (source pilot, defaulting to the player), and "funcname" (what to call if within the radius).
   @usage hook.timer(0.5, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "function"})
   @usage hook.timer(0.5, "proximity", {anchor = mypilot, radius = 500, funcname = "function"})
   @usage hook.timer(0.5, "proximity", {anchor = mypilot, radius = 500, funcname = "function", focus = theirpilot})
--]]
function proximity( trigger )
   if trigger.focus == nil then
      trigger.focus = player.pilot()
   end
   _proximity(trigger)
end

--[[--
This variant assumes a proximity hook between two ships, and trigger when the anchor ship scans the focus ship (fuzzy detection doesn't trigger).
   @tparam table trigger Contains: "anchor" (destination pilot, defaulting to the player), "focus" (the pilot that's polled for),
                         "funcname" (what to call if "anchor" detects "focus").
			 --
   @usage hook.timer(0.5, "proximityScan", {focus = theirpilot, funcname = "function"}) -- Triggers when theirpilot is detected by the player
   @usage hook.timer(0.5, "proximityScan", {anchor = theirpilot, focus = mypilot, funcname = "function"}) -- Triggers when mypilot is detected by theirpilot
--]]
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
        if trigger.anchor:exists() and trigger.focus:exists() and vec2.dist(trigger.focus:pos(), trigger.anchor:pos()) <= trigger.radius then
            _G[trigger.funcname]()
            return
        end
    end

    -- Check global proxmity table
    if mem.__proximity_tbl == nil then
       mem.__proximity_tbl = {}
       mem.__proximity_tbl["id"] = 0
    end

    -- Assign ID if necessary
    if trigger.__id == nil then
       mem.__proximity_tbl["id"] = mem.__proximity_tbl["id"]+1
       trigger.__id = mem.__proximity_tbl["id"]
       mem.__proximity_tbl[ trigger.__id ] = trigger
    end

    -- First time hook is set
    if trigger.hook_tbl == nil then
       trigger.hook_tbl = {}
       hook.enter("proximityCancel", trigger)
    end

    -- Set new timer hook
    local hook_id = hook.timer(0.5, "_proximity", trigger)
    trigger.hook_tbl[1] = hook_id
end

function _proximityScan( trigger )
    if trigger.focus ~= nil and trigger.anchor:exists() and trigger.focus:exists() then
        local _seen, scanned = trigger.anchor:inrange(trigger.focus)
        if scanned then
            _G[trigger.funcname]()
            return
        end
    end

    -- Check global proxmitiy table
    if mem.__proximity_tbl == nil then
       mem.__proximity_tbl = {}
       mem.__proximity_tbl["id"] = 0
    end

    -- Assign ID if necessary
    if trigger.__id == nil then
       mem.__proximity_tbl["id"] = mem.__proximity_tbl["id"]+1
       trigger.__id = mem.__proximity_tbl["id"]
       mem.__proximity_tbl[ trigger.__id ] = trigger
    end

    -- First time hook is set
    if trigger.hook_tbl == nil then
       trigger.hook_tbl = {}
       hook.enter("proximityCancel", trigger)
    end

    -- Set new timer hook
    local hook_id = hook.timer(0.5, "_proximityScan", trigger)
    trigger.hook_tbl[1] = hook_id
end

-- Make sure the proximity timer shuts itself off on land or jumpout.
function proximityCancel( trigger )
   if trigger ~= nil then
       hook.rm( trigger.hook_tbl[1] )
       mem.__proximity_tbl[ trigger.__id ] = nil
    end
end
