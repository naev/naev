local range, range2, hitships, trackmax

function onload( o )
   local _dps, _disps, _eps, _trackmin
   _dps, _disps, _eps, range, _trackmin, trackmax = o:weapstats()
   range2 = range*range
   hitships = not o:missShips()
end

function init( _p, _po )
   mem.on = true
   mem.target = nil -- current target
   mem.tpilot = false -- hether or not the target is a pilot
end

function ontoggle( _p, _po, on )
   mem.on = not on
   return false
end

function onshoot( _p, _po, _on )
   -- Doesn't fire normally,
   return false
end

function update( p, po, _dt )
   if not mem.on then return end

   local pos = p:pos()
   local m = mem.target

   -- Clear target if doesn't exist
   if not m or not m:exists() then
      mem.target = nil
      mem.tpilot = false
      m = nil
   else
      -- Do range check
      local d2 = pos:dist2( m:pos() )
      if d2 > range2 then
         mem.target = nil
         mem.tpilot = false
         m = nil
      end
   end

   -- See if we want to retarget, want to prioritize munitions
   if not m or mem.tpilot then
      -- Try to prioritize munitions
      local mall = munition.getInrange( pos, range, p )
      if #mall > 0 then
         m = mall[ rnd.rnd(1,#mall) ] -- Just get a random one
         mem.target = m
         mem.tpilot = false
      end

      -- If no current target, shoot at enemies too
      if not m and hitships then
         local pall = p:getEnemies( range )
         if #pall > 0 then
            local ptarget = {}
            for k,e in ipairs(pall) do
               if e:signature() < trackmax then
                  table.insert( ptarget, e )
               end
            end
            if #ptarget > 0 then
               m = ptarget[ rnd.rnd(1,#mall) ]
               mem.target = m
               mem.tpilot = true
            end
         end
      end
   end

   -- Try to shoot the target if we have one
   if m then
      po:shoot( p, m, true )
   end
end
