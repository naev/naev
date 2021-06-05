function __wanttoscan( p, target )
   -- Don't care about stuff that doesn't need scan
   if not __needs_scan( target ) then
      return false
   end

   -- Don't care about allies
   if ai.isally(target) then
      return false
   end

   return true
end


function __getscantarget ()
   -- See if we should scan a pilot
   local p = ai.pilot()
   for k,v in ipairs(p:getVisible()) do
      -- Only care about leaders
      local l = v:leader()
      if l and l:exists() then
         v = l
      end

      -- See if we want to scan
      if __wanttoscan(p,v) then
         return v
      end
   end
   return nil
end


-- Default task to run when idle
function idle ()
   if mem.loiter == nil then mem.loiter = 3 end
   if mem.loiter == 0 then -- Try to leave.
       local planet = ai.landplanet( mem.land_friendly )
       -- planet must exist
       if planet == nil or mem.land_planet == false then
          ai.settimer(0, rnd.int(1000, 3000))
          ai.pushtask("enterdelay")
       else
          mem.land = planet:pos()
          ai.pushtask("hyperspace")
          if not mem.tookoff then
             ai.pushtask("land")
          end
       end
   else -- Stay. Have a beer.
      local target = __getscantarget()
      if target then
         ai.pushtask("scan", target)
         return
      else
         -- Go to a random locatioe
         sysrad = rnd.rnd() * system.cur():radius()
         angle = rnd.rnd() * 2 * math.pi
         ai.pushtask("__moveto_nobrake", vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad))
      end
   end
   mem.loiter = mem.loiter - 1
end


-- Settings
mem.land_friendly = true -- Land on only friendly by default
mem.doscans       = true -- Patrolling so check all ships as much as possible
