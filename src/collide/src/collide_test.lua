-- luacheck: globals vec2 collide

local function close_enough( x, y )
   return math.abs(x-y) < 1e-8
end

local function close_enough_vec( v1, v2 )
   return close_enough( v1.x, v2.x ) and close_enough( v1.y, v2.y )
end

local function test_collision( a, b )
   -- Test number of collisions
   if (a[1]~=nil)~=(b[1]~=nil) or
      (a[2]~=nil)~=(b[2]~=nil) then
      return false
   end
   -- Sort order
   if (not a[1] or close_enough_vec( a[1], b[1] )) and
      (not a[2] or close_enough_vec( a[2], b[2] )) then
      return true
   end
   if (not a[1] or close_enough_vec( a[1], b[2] )) and
      (not a[2] or close_enough_vec( a[2], b[1] )) then
      return true
   end
   return false
end

local s1 = vec2.new( 2, 2 )
local e1 = vec2.new( 4, 2 )
local s2 = vec2.new( 3, 1 )
local e2 = vec2.new( 3, 3 )
assert( close_enough_vec( collide.line_line(
   s1,
   e1,
   s2,
   e2
), vec2.new(3,2) ), "line segment collision 1" )

local off = vec2.new( 3, 0 )
assert( collide.line_line(
   s1+off,
   e1+off,
   s2,
   e2
)==nil, "line segment collision 2" )

assert( close_enough_vec( collide.line_line(
   vec2.new(0,0),
   vec2.new(2,2),
   vec2.new(2,0),
   vec2.new(0,2)
), vec2.new(1,1) ), "line segment collision 3" )

assert( collide.line_line(
   vec2.new(0,0),
   vec2.new(0,2),
   vec2.new(2,0),
   vec2.new(2,2)
) == nil, "line segment collision 4" )

assert( close_enough_vec( collide.line_line(
   vec2.new(0,0),
   vec2.new(0,2),
   vec2.new(0,0),
   vec2.new(2,0)
), vec2.new(0,0) ), "line segment collision 5" )

assert( collide.line_line(
   vec2.new( 0, 0 ),
   vec2.new( 0, 0 ),
   vec2.new( 1, 2 ),
   vec2.new( 1, 3 )
)==nil, "line segment collision 6" )

assert( collide.line_line(
   vec2.new( 1, 0 ),
   vec2.new( 0, 0 ),
   vec2.new( 2, 1 ),
   vec2.new( 2, 1 )
)==nil, "line segment collision 7" )

assert( collide.line_line(
   vec2.new( 0, 0 ),
   vec2.new( 0, 0 ),
   vec2.new( 0, 1 ),
   vec2.new( 0, 1 )
)==nil, "line segment collision 8" )

assert( test_collision( {collide.circle_line(
   vec2.new( 2, 2 ),
   1,
   vec2.new( 2, 0 ),
   vec2.new( 2, 4 )
)}, {vec2.new(2,1), vec2.new(2,3)} ), "circle line collision 1")

assert( test_collision( {collide.circle_line(
   vec2.new( 2, 2 ),
   1,
   vec2.new( 0, 2 ),
   vec2.new( 4, 2 )
)}, {vec2.new(1,2), vec2.new(3,2)} ), "circle line collision 2")

assert( test_collision( {collide.circle_line(
   vec2.new( 3, 3 ),
   1,
   vec2.new( 0, 2 ),
   vec2.new( 4, 2 )
)}, {vec2.new(3,2), nil} ), "circle line collision 3")

assert( test_collision( {collide.circle_line(
   vec2.new( 4, 4 ),
   1,
   vec2.new( 0, 2 ),
   vec2.new( 4, 2 )
)}, {nil, nil} ), "circle line collision 4")

assert( test_collision( {collide.circle_line(
   vec2.new( 4, 4 ),
   10,
   vec2.new( 0, 2 ),
   vec2.new( 4, 2 )
)}, {vec2.new(0,2), vec2.new(4,2)} ), "circle line collision 5")

assert( test_collision( {collide.circle_line(
   vec2.new( 4, 4 ),
   1,
   vec2.new( 0, 0 ),
   vec2.new( 0, 0 )
)}, {nil, nil} ), "circle line collision 6")

assert( test_collision( {collide.circle_line(
   vec2.new( 4, 4 ),
   0,
   vec2.new( 0, 0 ),
   vec2.new( 0, 0 )
)}, {nil, nil} ), "circle line collision 7")

assert( test_collision( {collide.circle_line(
   vec2.new( 4, 4 ),
   0,
   vec2.new( 0, 0 ),
   vec2.new( 0, 3 )
)}, {nil, nil} ), "circle line collision 8")
