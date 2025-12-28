-- luacheck: globals vec2

local function close_enough( x, y )
   return math.abs(x-y) < 1e-5
end

local function close_enough_transform( t1, t2 )
   local d1 = t1:get()
   local d2 = t2:get()
   for i = 1,3 do
      for j = 1,3 do
         if not close_enough( d1[i][j], d2[i][j] ) then
            return false
         end
      end
   end
   return true
end

local t1, t2

t1 = transform.new()
t2 = transform.new( {{1,0,0}, {0,1,0}} )
assert( close_enough_transform( t1, t2 ), "identity" )

t1 = transform.new():translate( 3, 5 )
t2 = transform.new( {{1,0,3}, {0,1,5}} )
assert( close_enough_transform( t1, t2 ), "translate" )

t1 = transform.new():scale( 3, 5 )
t2 = transform.new( {{3,0,0}, {0,5,0}} )
assert( close_enough_transform( t1, t2 ), "scale" )

t1 = transform.new():rotate2d( math.pi*0.5 )
t2 = transform.new( {{0,-1,0}, {1,0,0}} )
assert( close_enough_transform( t1, t2 ), "rotate2d" )

t1 = transform.new()
   :translate( -10, 10 )
   :scale( 5, 3 )
t2 = transform.new( {{5,0,-10},{0,3,10}} )
assert( close_enough_transform( t1, t2 ), "translate + scale" )

t1 = transform.new()
   :translate( 10.0, 20.0 )
   :rotate2d( math.pi/6 )
t2 = transform.new({{0.8660254, -0.5, 10.0},{0.5, 0.8660254, 20.0}})
assert( close_enough_transform( t1, t2 ), "translate + rotate" )

t1 = transform.new()
   :translate(1, 2)
   :rotate2d(math.pi)
   :scale(3, 5)
t2 = transform.new( {{-3,0,1}, {0,-5,2}} )
assert( close_enough_transform( t1, t2 ), "translate + rotate + scale" )
