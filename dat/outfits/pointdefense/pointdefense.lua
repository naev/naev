local fmt = require "format"
local range, range2, hitships, trackmax
local pd = {}

local SWITCH_TIME = 1 -- Time to try to switch with bad targets

function pd.setTrack( track )
   trackmax = track
end

function onload( o )
   local _dps, _disps, _eps, _trackmin, trackmax_local
   _dps, _disps, _eps, range, _trackmin, trackmax_local = o:weapstats()
   trackmax = trackmax or trackmax_local -- Replace if not defined
   range2 = range*range -- Effective range
   hitships = not o:missShips()
end

function descextra( _p, _o )
   if not hitships then
      return "#b".._("When on, automatically fires at nearby incoming missiles.").."#0"
   end
   if trackmax then
      return "#b"..fmt.f(_("When on, automatically fires at nearby incoming missiles and hostile ships, prioritizing missiles and then ships with under {trackmax} {unit} signature.").."#0",
         {trackmax=fmt.number(trackmax), unit=naev.unit("distance")})
   end
   return "#b".._("When on, automatically fires at nearby incoming missiles and hostile ships, prioritizing missiles and then ships.").."#0"
end

function init( _p, po )
   if mem.on == nil then
      -- Defaults to on, TODO option?
      mem.on = true
      po:state("on")
   end
   mem.target = nil -- current target
   mem.tpilot = false -- whether or not the target is a pilot
   mem.badtarget = false
   mem.dt = 0
end

function ontoggle( _p, _po, on, nat )
   if not nat then
      return false
   end
   mem.on = on
   mem.dt = 0
   return true
end

function onshoot( _p, _po )
   -- Doesn't fire normally,
   return false
end

local function tsort( a, b )
   return a:signature() < b:signature()
end

function update( p, po, dt )
   if not mem.on then return end
   if p:disabled() then return end

   mem.dt = mem.dt+dt

   local pos = p:pos()
   local m = mem.target

   -- Clear target if doesn't exist
   if not m or not m:exists() or (mem.tpilot and m:disabled()) or (mem.badtarget and mem.dt > SWITCH_TIME) then
      mem.target = nil
      mem.tpilot = false
      mem.badtarget = false
      m = nil
   else
      -- Do range check
      local d2 = pos:dist2( m:pos() )
      if d2 > range2 then
         mem.target = nil
         mem.tpilot = false
         mem.badtarget = false
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
         mem.badtarget = false
      end

      -- If no current target, shoot at enemies too
      if not m and hitships then
         local pall = p:getEnemies( range, nil, nil, false, true )
         if #pall > 0 then
            local ptarget = {}
            if trackmax then
               for k,e in ipairs(pall) do
                  if e:signature() < trackmax then
                     table.insert( ptarget, e )
                  end
               end
            else
               ptarget = pall
            end
            if #ptarget > 0 then
               m = ptarget[ rnd.rnd(1,#mall) ]
               mem.target = m
               mem.tpilot = true
               mem.badtarget = false
            else
               local t = p:target()
               if inlist( pall, t ) then
                  -- Prefer target if available
                  m = t
               else
                  -- Default to smallest signature of available targets
                  table.sort( pall, tsort )
                  m = pall[1]
               end
               mem.target = m
               mem.tpilot = true
               mem.badtarget = true
            end
         end
      end
   end

   -- Try to shoot the target if we have one
   if m then
      po:shoot( p, m, true )
   end
end

return pd
