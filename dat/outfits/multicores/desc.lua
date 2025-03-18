
local fmt = require "format"

local function vu( val, unit)
   if val=='_' then
      return val
   else
      return val.." "..unit
   end
end

local function col( s, grey, def)
   if grey then
      return "#b"..s..def
   else
      return s
   end
end

local function _add_desc(desc, name, units, base, secondary, def,nomain,nosec)
   return desc..fmt.f(_("\n{name}: {bas} {sep} {sec}"), {
      name=name, units=units,
      sep=col("/",nomain or nosec,def),
      bas=col(vu(base,units),nomain,def),
      sec=col(vu(secondary,units),nosec,def),
   })
end

return _add_desc
