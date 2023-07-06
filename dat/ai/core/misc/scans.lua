--[[
-- Handles
--]]
local scans = {}

--[[
-- Checks to see if a pilot is visible
-- Assumes the pilot exists!
--]]
function scans.check_visible( target )
   if target:flags("invisible") then
      return false
   end
   local self = ai.pilot()

   -- Carried ships depend on the visibility of their parent
   local checker = (mem.carried and self:leader()) or self

   -- Pilot still sees the target: continue attack
   if checker:inrange( target ) then
      return true
   end

   -- Pilots on manual control (in missions or events) never loose target
   -- /!\ This is not necessary desirable all the time /!\
   -- TODO: there should probably be a flag settable to allow to outwit pilots under manual control
   if self:flags("manualcontrol") then
      return true
   end
   return false
end


--[[
-- Aborts current task and tries to see what happened to the target.
--]]
function scans.investigate( target )
   local p = ai.pilot()
   ai.settarget(p) -- Un-target
   ai.poptask()

   -- No need to investigate: target has jumped out.
   if target:flags("jumpingout") then
      return
   end

   -- Guess the pilot will be randomly between the current position and the
   -- future position if they go in the same direction with the same velocity
   local ttl = ai.dist(target) / p:stats().speed_max
   local fpos = target:pos() + vec2.newP( target:vel():mod()*ttl, target:dir() ) * rnd.rnd()
   ai.pushtask("inspect_moveto", fpos )
end


function scans.push( target )
   -- Send a message if applicable
   local msg = mem.scan_msg or _("Prepare to be scanned.")
   ai.pilot():comm( target, msg )
   ai.pushtask( "scan", target )
end


--[[
-- Tries to get close to scan the enemy
--]]
function scans.scan( target )
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Try to investigate if target lost
   if not scans.check_visible( target ) then
      scans.investigate( target )
      return
   end

   -- Set target
   ai.settarget( target )
   local p = ai.pilot()

   -- Done scanning
   if ai.scandone() then -- Note this check MUST be done after settarget
      table.insert( mem.scanned, target )
      ai.poptask()
      if target:hasIllegal( p:faction() ) then
         ai.hostile( target )
         ai.pushtask( "attack", target )
         local msg = mem.scan_msg_bad or _("Illegal objects detected! Do not resist!")
         p:comm( target, msg )
         mem.found_illegal = true

         -- Player gets faction hit and more hostiles on them
         if target == player.pilot() then
            for k,v in ipairs(p:getAllies(5000, nil, true, false, true)) do
               v:setHostile(true)
               v:memory().found_illegal = true
            end
            -- Small faction hit
            p:faction():modPlayer( -1 )
         end
      else
         local msg = mem.scan_msg_ok or _("Thank you for your cooperation.")
         p:comm( target, msg )

         -- Tell friends about the scanning
         local f = p:faction()
         for k,v in ipairs(pilot.get(f)) do
            p:msg( v, "scanned", target )
         end
         for kf,vf in ipairs(f:allies()) do
            for k,v in ipairs(pilot.get(vf)) do
               p:msg( v, "scanned", target )
            end
         end
      end
      return
   end

   -- Get stats about the enemy
   local dist = ai.dist(target)

   -- Get closer and scan
   ai.iface( target )
   if dist > 1000 then
      ai.accel()
   end
end

--[[
-- Check to see if a ship needs to be scanned.
--]]
local function __needs_scan( target )
   if not mem.scanned then
      return false
   end
   -- Don't scan immediately
   if target:memory().elapsed < 20 then
      return false
   end
   -- See if have already been scanned
   for k,v in ipairs(mem.scanned) do
      if target==v then
         return false
      end
   end
   return true
end


--[[
-- Whether or not we want to scan, ignore players for now
--]]
local function __wanttoscan( p, target )
   -- Bribed pilots don't care about scanning player ships
   if p:flags("bribed") and target:withPlayer() then
      return false
   end

   -- Don't care about stuff that doesn't need scan
   if not __needs_scan( target ) then
      return false
   end

   -- We always want to scan the player (no abusing allies)
   --[[
   if target == player.pilot() then
      return true
   end
   --]]

   -- Don't care about allies nor enemies (should attack instead)
   if ai.isally(target) or ai.isenemy(target) then
      return false
   end

   return true
end


--[[
-- Tries to get find a good target to scan with some heuristics based on mass
-- and distance
--]]
function scans.get_target ()
   -- See if we should scan a pilot
   local p = ai.pilot()
   local pv = {}
   do
      local inserted = {}
      -- TODO probably try to change this to getInrange() instead to be a bit faster
      for k,v in ipairs(p:getVisible()) do
         -- Only care about leaders
         local l = v:leader()
         if l then
            v = l
         end

         if not inlist( inserted, v ) then
            if __wanttoscan( p, v ) then
               local d = ai.dist2( v )
               local m = v:mass()
               table.insert( pv, {p=v, d=d, m=m} )
            end
            table.insert( inserted, v )
         end
      end
   end
   -- We do a sort by distance and mass categories so that the AI will prefer
   -- larger ships before trying smaller ships. This is to avoid having large
   -- ships chasing after tiny ships
   local pm = p:mass()
   local pmh = pm * 1.5
   local pml = pm * 0.75
   table.sort( pv, function(a,b)
      if a.m > pmh and b.m > pmh then
         return a.d < b.d
      elseif a.m > pmh then
         return true
      elseif b.m > pmh then
         return false
      elseif a.m > pml and b.m > pml then
         return a.d < b.d
      elseif a.m > pml then
         return true
      elseif b.m > pml then
         return false
      else
         return a.d < b.d
      end
   end )

   if #pv==0 then
      return nil
   end
   return pv[1].p
end

return scans
