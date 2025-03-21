
local fmt = require "format"
local shipstat = naev.shipstats()
local multicore = {}

local function valcol( val, inverted )
   if inverted then
      val = -val
   end
   if val > 0 then
      return "#g"
   elseif val < 0 then
      return "#r"
   else
      return "#n"
   end
end

local function stattostr( s, val, grey, unit )
   local col
   if grey then
      col = "#n"
   else
      col = valcol( val, s.inverted )
   end
   local str
   if val > 0 then
      str = "+"..fmt.number(val)
   else
      str = fmt.number(val)
   end
   if unit and s.unit then
      return col..fmt.f("{val} {unit}", {val=str, unit=_(s.unit)})
   end
   return col..str
end

local function add_desc( stat, nomain, nosec )
   local name = _(stat.stat.display)
   local base = stat.pri
   local secondary = stat.sec

   local p = base or 0
   local s = secondary or 0

   local col = valcol( p+s, stat.stat.inverted )
   local pref = col..fmt.f("{name}: ",{name=name})
   if p==s then
      return pref..stattostr( stat.stat, base, false, true )
   else
      return pref..fmt.f("{bas} #n/#0 {sec}", {
         bas = stattostr( stat.stat, p, nomain, false ),
         sec = stattostr( stat.stat, s, nosec, true ),
      })
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

      local desc
      if nomain then
         desc = "#o"..fmt.f(_("Equipped as {type} core"),{type="#y"..p_("core","secondary").."#o"}).."#0"
      elseif nosec then
         desc = "#o"..fmt.f(_("Equipped as {type} core"),{type="#r"..p_("core","primary").."#o"}).."#0"
      else
         desc = "#n"..fmt.f(_("Properties for {pri} / {sec} slots"),{
            pri="#r"..p_("core","primary").."#n",
            sec="#y"..p_("core","secondary").."#n",
         }).."#0"
      end
      for k,s in ipairs(stats) do
         desc = desc.."\n"..add_desc( s, nomain, nosec )
      end
      return desc
   end

   function init( _p, po )
      local secondary = po and po:slot() and po:slot().tags and po:slot().tags.secondary
      for k,s in ipairs(stats) do
         local val = (secondary and s.sec) or s.pri
         po:set( s.name, val )
      end
   end
end

return multicore
