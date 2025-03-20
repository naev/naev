
local fmt = require "format"
local shipstat = naev.shipstats()
local multicore = {}

local function vu( val, unit)
   local num
   if val=="_" or val==0 then
      num="_"
   else
      num=val
   end

   if num=="_" or unit == nil or unit == "" then
      return fmt.f("{num}",{num=num})
   else
      return fmt.f("{val} {unit}",{val=val,unit=unit})
   end
end

local function col( s, grey, def)
   if grey then
      return "#n"..s..def
   else
      return s
   end
end

local function sign(n)
   if n == nil or n=="_" then
      return 0
   else
      return n
   end
end

local function add_desc(stat, nomain, nosec )
   local name = stat.stat.display
   local base = stat.pri
   local secondary = stat.sec
   local units = stat.stat.unit

   local p=sign(base)
   local s=sign(secondary)

   local def
   if stat.stat.inverted then
      def = ((p+s <= 0) and "#g") or "#r"
   else
      def = ((p+s >= 0) and "#g") or "#r"
   end

   local pref=fmt.f("\n{def}{name}: ",{def=def,name=_(name)})

   if base==secondary then
      return pref..vu(base,units)
   else
      return pref..col( vu(base,units),nomain,def)..col( "/",nomain or nosec,def)..col(vu(secondary,units),nosec, def)
--      return fmt.f("\n{name}: {bas} {sep} {sec}", {
--         name = _(name),
--         sep = col( "/", nomain or nosec, stat),
--         bas = col( vu(base,units), nomain, stat),
--         sec = col( vu(secondary,units), nosec, stat),
--      })
   end
end

local function index( tbl, key )
   for i,v in ipairs(tbl) do
      if v and v["name"]==key then
         return i
      end
   end
   return nil
end

function multicore.init( params )
   -- Create an easier to use table that references the true ship stats
   local stats = tcopy( params )

   for k,s in ipairs(stats) do
      s.index = index( shipstat, s[1] )
      s.stat = shipstat[ s.index ]
      s.name = s.stat.name
      s.pri = s[2]
      s.sec = s[3]
   end
   -- Sort based on shipstat table order
   table.sort( stats, function ( a, b )
      return a.index < b.index
   end )

   -- Set global properties
   notactive = true

   -- Below define the global functions for the outfit
   function descextra( _p, _o, po )
      local nomain, nosec = false, false
      if po then
         if po:slot().tags.secondary then
            nomain = true
         else
            nosec = true
         end
      end

      local desc = ""
      for _k,s in ipairs(stats) do
         desc = desc..add_desc( s, nomain, nosec )
      end
      return desc
   end

   function init( _p, po )
      local secondary = po:slot().tags.secondary
      for k,s in ipairs(stats) do
         local val = (secondary and s.sec) or s.pri
         po:set( s.name, val )
      end
   end
end

return multicore
