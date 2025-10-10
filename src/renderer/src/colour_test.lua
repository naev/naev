local function close_enough( x, y )
   return math.abs(x-y) < 1e-8
end

local function close_enough_col( a, b )
   local ok = true
   for k,v in ipairs{'r','g','b','a'} do
      ok = ok and close_enough( a[v], b[v] )
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
assert( close_enough(h,h) and close_enough(s,ns) and close_enough(v,nv), "hsv:hsv() failed" )

local col = colour.new_named("Aqua")
local hsv = col.new_hsv( col:hsv() )
assert( close_enough_col( col, hsv ), "hsv roundtrip failed" )
