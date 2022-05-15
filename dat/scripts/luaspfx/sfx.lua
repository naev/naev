
local function sfx( ttl, pos, source, params )
   params = params or {}
   local s = spfx.new( ttl, nil, nil, nil, nil, pos, nil, source )
   local ss = s:sfx()
   if params.dist_ref or params.dist_max then
      local dist_ref = params.dist_ref or 500
      local dist_max = params.dist_max or 25e3
      ss:setAttenuationDistances( dist_ref, dist_max )
   end
end

return sfx
