local range, range2

function onload( o )
   local _dps, _disps, _eps, _tmin, _tmax
   _dps, _disps, _eps, range, _tmin, _tmax = o:weapstats()
   range2 = range*range
end

function init( _p, _po )
   mem.on = true
   mem.target = nil -- current target
   mem.tpilot = false -- hether or not the target is a pilot
end

function ontoggle( _p, _po, _on )
   -- Doesn't fire normally, TODO some way to turn on/off
   return false
end

function update( p, po, _dt )
   local pos = p:pos()
   local m = mem.target
   -- Clear target if doesn't exist
   if m and not m:exists() then
      mem.target = nil
      mem.tpilot = false
      m = nil
   end
   -- See if we want to retarget
   if not (m  and pos:dist2( m:pos() ) < range2) or mem.tpilot then

      -- Try to prioritize munitions
      local mall = munition.getInrange( pos, range, p )
      if #mall > 0 then
         m = mall[ rnd.rnd(1,#mall) ] -- Just get a random one
         mem.target = m
         mem.tpilot = false
      end

      -- If no current target, shoot at enemies too
      if not m then
         local pall = p:getEnemies( range )
         if #pall > 0 then
            m = pall[ rnd.rnd(1,#mall) ]
            mem.target = m
            mem.tpilot = true
         end
      end
   end

   -- Try to shoot the target if we have one
   if m then
      po:shoot( p, m )
   end
end
