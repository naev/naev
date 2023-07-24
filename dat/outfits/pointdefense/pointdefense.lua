local range, range2, hitships, trackmin

function onload( o )
   local _dps, _disps, _eps, _trackmax
   _dps, _disps, _eps, range, trackmin, _trackmax = o:weapstats()
   range2 = range*range
   hitships = not o:missShips()
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
   if not (m and pos:dist2( m:pos() ) < range2) then

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
               if e:evasion() < trackmin then
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
   -- Prefer munitions over pilots
   elseif mem.tpilot then
      -- Try to prioritize munitions
      local mall = munition.getInrange( pos, range, p )
      if #mall > 0 then
         m = mall[ rnd.rnd(1,#mall) ] -- Just get a random one
         mem.target = m
         mem.tpilot = false
      end
   end

   -- Try to shoot the target if we have one
   if m then
      po:shoot( p, m, true )
   end
end
