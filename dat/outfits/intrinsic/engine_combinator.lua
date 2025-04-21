
local fmt = require "format"

descextra=function ( p, _o, _po)
   local sm = p:shipMemory()
   local count = sm._engine_count

   if not count then
      return ""
   end

   return  fmt.f("{_speed}",sm)
end

