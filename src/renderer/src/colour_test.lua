-- luacheck: globals colour

local function close_enough( x, y, tol )
   tol = tol or 1e-8
   return math.abs(x-y) < tol
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
