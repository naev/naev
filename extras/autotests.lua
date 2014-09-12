--[[
   Some small autotests meant to be used 
--]]

function test_vec2()
   local v, v2
   local fail = 0

   local function vcheck( v, x, y, s )
      local vx, vy = v:get()
      if vx ~= x or vy ~= y then
         print( string.format( '%s) Got %f x %f, expected %f x %f...', s, vx, vy, x, y ) )
         return 1
      end
      return 0
   end

   -- Add test
   v  = vec2.new( 1, 2 )
   v2 = v + vec2.new( 2, 3 )
   v:add( 3, 4 )
   fail = fail + vcheck( v,  1+3, 2+4, 'add: v' )
   fail = fail + vcheck( v2, 1+2, 2+3, 'add: v2' )

   -- Sub test
   v  = vec2.new( 1, 2 )
   v2 = v - vec2.new( 2, 3 )
   v:sub( 3, 4 )
   fail = fail + vcheck( v,  1-3, 2-4, 'sub: v' )
   fail = fail + vcheck( v2, 1-2, 2-3, 'sub: v2' )

   -- Mul test
   v  = vec2.new( 1, 2 )
   v2 = v * 2
   v:mul( 3 )
   fail = fail + vcheck( v,  1*3, 2*3, 'mul: v' )
   fail = fail + vcheck( v2, 1*2, 2*2, 'mul: v2' )

   -- Div test
   v  = vec2.new( 6, 12 )
   v2 = v / 2
   v:div( 3 )
   fail = fail + vcheck( v,  6/3, 12/3, 'div: v' )
   fail = fail + vcheck( v2, 6/2, 12/2, 'div: v2' )

   return fail
end

function run_test( failed, str )
   if failed > 0 then
      print( string.format( "Test '%s' failed %d testcases", str, failed ) )
      return 1
   end
   return 0
end

fail = 0
fail = fail+run_test( test_vec2(), 'vec2' )
if fail==0 then
   print( 'All tests completed successfully!' )
end


