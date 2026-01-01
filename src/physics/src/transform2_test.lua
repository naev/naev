-- luacheck: globals transform

local function close_enough( x, y )
   return math.abs(x-y) < 1e-4
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

t1 = transform.new()
   :rotate2d( math.pi/3)
   :translate( 15, 20 )
   :scale( 1.5, 10 )
t2 = transform.new( {{0.750000,-8.660254,-9.820508},{1.299038,5.000000,22.990381}} )
assert( close_enough_transform( t1, t2 ), "translate + rotate + scale 2" )

t1 = transform.ortho( -3, 5, -7, 11, -13, 17 )
t2 = transform.new( {{0.250000,0.000000,-0.250000},{0.000000,0.111111,-0.222222}} )
assert( close_enough_transform( t1, t2 ) )

t1 = transform.new()
   :translate(3,5)
   :scale(7,2)
   :translate(0,-1)
   :rotate2d( 27 )
t2 = transform.new( {{-2.044972,-6.694632,3.000000},{1.912752,-0.584278,3.000000}} )
local x,y = t1:applyPoint( 13, 17 )
assert( close_enough_transform( t1, t2 ) )
assert( close_enough( x, -137.39336895942688 ) and close_enough( y,  17.93305516242981 ), "applyPoint" )
x,y = t1:applyDim( 17, 13 )
assert( close_enough( x, -121.79472947120667 ) and close_enough( y, 24.921173334121704 ), "applyDim" )

t1 = transform.new()
   :rotate2d( math.pi/3)
   :translate( 15, 20 )
   :scale( 1.5, 10 )
t1 = t1 * transform.new()
   :translate(1, 2)
   :rotate2d(math.pi)
   :scale(3, 5)
t2 = transform.new( {{-2.250000,25.980761,30.461525},{-6.495190,-25.000000,-112.951904}} )
assert( close_enough_transform( t1, t2 ), 'multiplication' )

t1 = transform.new()
   :rotate2d( math.pi/3)
   :translate( 15, 20 )
   :scale( 1.5, 10 )
t2 = transform.new():scale( 1.5, 10 )
   * transform.new():translate( 15, 20 )
   * transform.new():rotate2d( math.pi/3 )
assert( close_enough_transform( t1, t2 ), 'multiplication order' )
