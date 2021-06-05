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


function __intable( t, val )
   for k,v in ipairs(t) do
      if v==val then
         return true
      end
   end
   return false
end


function __getscantarget ()
   -- See if we should scan a pilot
   local p = ai.pilot()
   local pv = {}
   local inserted = {}
   for k,v in ipairs(p:getVisible()) do
      -- Only care about leaders
      local l = v:leader()
      if l and l:exists() then
         v = l
      end

      if not __intable( inserted, v ) then
         if __wanttoscan(p,v) then
            local d = ai.dist( v )
            local m = v:mass()
            table.insert( pv, {p=v, d=d, m=m} )
         end
         table.insert( inserted, v )
      end
   end
   inserted = nil
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
