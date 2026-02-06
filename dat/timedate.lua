--[[
Handes how to parse and display time.
--]]
function from_string( str )
   local maj, min, inc
   maj, min, inc = str:match("^UST (%d+):(%d+)%.(%d+)$")
   if maj then
      return time.new( tonumber(maj), tonumber(min), tonumber(inc) )
   end
   min, inc = str:match("^(%d+)%.(%d+) stp$")
   if min then
      return time.new( 0, tonumber(min), tonumber(inc) )
   end
   inc = str:match("^(%d+) stu$")
   if inc then
      return time.new( 0, 0, tonumber(inc) )
   end
   error("invalid time format: " .. tostring(str))
end

function to_string( nt )
   local maj, min, inc = nt:split()
   if maj==0 and min==0 then
      return string.format( _("%d stu"), inc )
   elseif maj==0 then
      return string.format( _("%d.%04d stp"), min, inc )
   else
      return string.format( _("UST %d:%04d.%04d"), maj, min, inc )
   end
end

--[[
local time
time = {
   new = function ( a, b, c )
      return {a,b,c, split=time.split}
   end,
   split = function ( self )
      return self[1], self[2], self[3]
   end,
}

local function test( a, b, str )
   assert( a[1]==b[1] and a[2]==b[2] and a[3]==b[3], str )
end

test( time.new( 603, 3726, 2871 ), from_string( "UST 603:3726.2871" ) )
test( time.new( 0,   3726, 2871 ), from_string( "UST 0:3726.2871" ) )
test( time.new( 0,   3726, 2871 ), from_string( "3726.2871 p" ) )
test( time.new( 0,      0, 2871 ), from_string( "UST 0:0000.2871" ) )
test( time.new( 0,      0, 2871 ), from_string( "0.2871 p" ) )
test( time.new( 0,      0, 2871 ), from_string( "2871 s" ) )
test( "UST 603:3726.2871", to_string(from_string( "UST 603:3726.2871" )), "round-trip" )
--]]
