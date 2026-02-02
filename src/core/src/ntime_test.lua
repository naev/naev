-- luacheck: globals time

-- Test some simple functions
assert( time.new(17,37,13)==time.new(17,37,13), "eq" )
assert( time.new(17,35,13)~=time.new(17,37,13), "!eq" )
assert( time.new(2,3,4)+time.new(1,2,3)==time.new(3,5,7), "add" )
assert( time.new(11,12,13)-time.new(1,2,3)==time.new(10,10,10), "sub" )
assert( time.new(2,3,4)==time.fromnumber( time.new(2,3,4):tonumber() ), "number roundtrip" )

-- Test comparisons
assert( time.new(1,0,0) > time.new(0,1,1) )
assert( time.new(0,100000,0) <= time.new(100,0,0) )

-- Test some global stuff
local t = time.new( 603, 3726, 2871 )
time.set_current( t )
assert( time.cur()==t, "test set and cur" )
time.inc( time.new( 0, 0, 100 ) )
assert( time.cur()==t+time.new( 0, 0, 100 ), "test inc" )
