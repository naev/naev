--[[--
Plays a sound.

@usage luaspfx.sfx( pos, vel, sfx ) -- Plays at pos with velocity vel
@usage luaspfx.sfx( true, nil, sfx ) -- Global but modified by time speed-up
@usage luaspfx.sfx( false, nil, sfx ) -- Global but not modified by time like GUI events, etc.
--]]
local function sfx( pos, vel, source, params )
   params = params or {}
   local ttl = source:getDuration()*1.1
   local s = spfx.new( ttl, nil, nil, nil, nil, pos, vel, source )
   local ss = s:sfx()
   if params.dist_ref or params.dist_max then
      local dist_ref = params.dist_ref or 500
      local dist_max = params.dist_max or 25e3
      ss:setAttenuationDistances( dist_ref, dist_max )
   end
   if params.volume then
      ss:setVolume( params.volume )
   end
   if params.pitch then
      ss:setPitch( params.pitch )
   end
   if params.effect then
      ss:setEffect( params.effect )
   end
   return s
end

return sfx
