-- luacheck: globals colour

local function close_enough( x, y, tol )
   tol = tol or 1e-8
   return math.abs(x-y) < tol
end

local function close_enough_table( v1, v2, tol )
   local good = true
   for k,v in ipairs(v1) do
      good = good and close_enough( v, v2[k], tol )
   end
   return true
end

local function close_enough_col( a, b, tol )
   local ok = true
   for k,v in ipairs{'r','g','b','a'} do
      ok = ok and close_enough( a[v], b[v], tol )
   end
   return ok
end

local lin = colour.new( 0.5, 0.5, 0.5 )
local gam = colour.new( 0.5, 0.5, 0.5, nil, true )
assert( lin ~= gam, "gamma and linear are identical" )
assert( lin == gam:to_linear(), "to_linear() failed" )
assert( gam == lin:to_gamma(), "to_gamma() failed" )
assert( lin == lin:to_gamma():to_linear(), "roundtrip linear -> gamma -> linear failed" )
assert( lin == colour.new_named("grey50"), "new_named failed" )

local h,s,v = 180, 0.5, 0.5
local hsv = colour.new_hsv( h, s, v )
local nh, ns, nv = hsv:hsv()
assert( close_enough(h,nh) and close_enough(s,ns,1e-6) and close_enough(v,nv), "hsv:hsv() failed" )

local col = colour.new_named("Aqua")
hsv = col.new_hsv( col:hsv() )
assert( close_enough_col( col, hsv, 1e-6 ), "hsv roundtrip failed" )

local r, g, b
h,s,v = 120, 0.3, 0.6
r,g,b = colour.hsv_to_rgb(h,s,v)
assert( h~=r and s~=g and v~=b )
assert( close_enough_table( {colour.rgb_to_hsv(h,s,v)}, {h,s,v}, 1e-6 ), "hsv_to_rgb_to_hsv" )
r,g,b = 0.8, 0.2, 0.4
h,s,v = colour.rgb_to_hsv(r,g,b)
assert( h~=r and s~=g and v~=b )
assert( close_enough_table( {colour.hsv_to_rgb(h,s,v)}, {r,g,b}, 1e-6 ), "rgb_to_hsv_to_rgb" )
