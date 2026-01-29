-- luacheck: globals vec2

local function close_enough( x, y )
   return math.abs(x-y) < 1e-8
end

local function close_enough_vec( v1, v2 )
   return close_enough( v1.x, v2.x ) and close_enough( v1.y, v2.y )
end

local v = vec2.new( 10, 5 )
assert( close_enough( v:mod(), 11.180339887498949 ), "v:mod() failed" )
v:set( 5, 10 )
assert( close_enough_vec( v, vec2.new( 5, 10 ) ), "v:set( 5, 10 )" )
assert( close_enough_vec( -v, vec2.new( -5, -10 ) ), "-v" )

v = vec2.new( 10, 5 )
local a = vec2.new( 8, 3 )
assert( close_enough( v:dist(a), 2*1.4142135623730951 ), "v:dist(a)" )
assert( close_enough( v:dist2(a), (2*1.4142135623730951)^2 ), "v:dist2(a)" )
local b = vec2.new( 3, 2 )
assert( close_enough_vec( a+b, vec2.new(11,5) ), "a+b" )
assert( close_enough_vec( a-b, vec2.new(5,1) ), "a-b" )
assert( close_enough_vec( a*5, vec2.new(40,15) ), "a*5" )
assert( close_enough_vec( a*b, vec2.new(24,6) ), "a*b" )
assert( close_enough_vec( a/b, vec2.new(8/3, 3/2) ), "a/b" )

local p = vec2.newP( 10, 3 )
assert( close_enough( p:mod(), 10 ) and close_enough( p:angle(), 3 ), "p:newP()" )

v = vec2.new( 57, 13 )
v:normalize()
assert( close_enough( v:mod(), 1 ), "v:normalize()" )
