
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
end

return sfx
