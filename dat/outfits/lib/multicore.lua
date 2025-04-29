
local multicore = {}

local shipstat = naev.shipstats()

local fmt = require "format"

local multiengines = require "outfits/lib/multiengines"
local is_mobility = multiengines.is_mobility
local halted_n = multiengines.halted_n


local function valcol( val, inverted, gb )
   if inverted then
      val = -val
   end
   if val > 0 then
      if gb then
         return "#b"
      else
         return "#g"
      end
   elseif val < 0 then
      return "#r"
   else
      return "#n"
   end
end

local function stattostr( s, val, grey, unit, gb)
   if val == 0 then
      return "#n0" -- Correct people bad taste HERE.
   end
   local col
   if grey then
      col = "#n"
   else
      col = valcol(val, s.inverted, gb)
   end
   local str
   if val > 0 then
      str = "+"..fmt.number(val)
   else
      str = fmt.number(val)
   end
   if unit and s.unit then
      str = str .. " " .. _(s.unit)
   end
   return col .. str
end

local function add_desc( stat, nomain, nosec, gb )
   local name = _(stat.stat.display)
   local base = stat.pri
   local secondary = stat.sec
   local off = nomain and nosec

   local p = base or 0
   local s = secondary or 0

   nomain = nomain or p == 0
   nosec = nosec or s == 0
   local col
   if off or (nomain and nosec) then
      col="#n"
   else
      col=valcol(p+s, stat.stat.inverted, gb)
   end
   local pref = col .. fmt.f("{name}: ",{name=name})
   if p == s then
      return pref .. stattostr(stat.stat, base, off, true, gb)
   else
      return pref .. fmt.f("{bas} #n/#0 {sec}", {
         bas = stattostr(stat.stat, p, nomain, nosec, gb),
         sec = stattostr(stat.stat, s, nosec, nomain or not nosec, gb),
      })
   end
end

local function index( tbl, key )
   for i,v in ipairs(tbl) do
      if v and v["name"] == key then
         return i
      end
   end
   return nil
end

local function is_secondary_slot( ps )
   return ps and ps.tags and ps.tags.secondary
end

local function is_secondary( po )
   return po and is_secondary_slot(po:slot())
end

local function is_engine( sl )
   return sl and sl.tags and sl.tags.engines
end

local function is_multiengine( p, po )
   local sh = p and p:ship()
   sh = sh and sh:getSlots()
   local n = po and po:id()
   local sl = sh and n and sh[n]

   return is_engine(sl) and (is_secondary_slot(sl) or is_engine(sh[n+1]))
end

function multicore.init( params )
   -- Create an easier to use table that references the true ship stats
   local stats = tcopy(params)
   local eml_unit

   for k,s in ipairs(stats) do
      s.index = index(shipstat, s[1])
      s.stat = shipstat[ s.index ]
      s.name = s.stat.name
      s.pri, s.sec = s[2], s[3]
      if s.name == 'engine_limit' then
         eml_unit = s.stat.unit
      end
   end
   -- Sort based on shipstat table order
   table.sort(stats, function ( a, b )
      return a.index < b.index
   end)

   -- Set global properties
   notactive = true -- Not an active outfit
   hidestats = true -- We do hacks to show stats, so hide them


   -- Below define the global functions for the outfit
   function descextra( p, _o, po )
      local nomain, nosec = false, false
      if po then
         if is_secondary(po) then
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

      local averaged = is_multiengine(p, po)
      local id = po and po:id()
      local multicore_off = halted_n(p, id)
      local smid = multiengines.engine_stats(p, id)
      local total = multiengines.total()
      local totaleml = (total and total["engine_limit"]) or 0

      for k,s in ipairs(stats) do
         local off = multicore_off and (nosec or nomain) and s.name ~= "mass"
         desc = desc .. "\n" .. add_desc(s, nomain or off, nosec or off, averaged and is_mobility[s.name])
      end

      if multicore_off ~= nil then
         local status
         if multicore_off == false then
            status = "#g" .. _("running") .. "#0"
         else
            status = "#r" .. _("HALTED") .. "#0"
         end
         desc = desc .. "\n#o" .. fmt.f(_("Working Status: {status}"), {status=status})
      end

      if averaged and multicore_off ~= true then
         local share = (smid and smid["part"]) or 0
         desc = desc .. fmt.f(_("\n\n#oLoad Factor: #y{share}%#0  #o(#g{eml} {t}#0 #o/#0 #g{total} {t}#0 #o)#0\n"),{
            eml = (smid and smid["engine_limit"]) or 0,
            total = totaleml, share = share, t = eml_unit
         })
         for k,s in ipairs(stats) do
            if is_mobility[s.name] then
               desc = desc .. fmt.f(_("#g{display}:#0 #b+{val} {unit}#0"),{
                  display = s.stat.display, unit = s.stat.unit, val = fmt.number(smid[s.name]) })
               desc = desc .. "  #y=>#0  #g+" .. fmt.number(smid[s.name]*share/100) .. " " .. s.stat.unit .. "#0\n"
            end
         end
      end
      return desc
   end

   local function engines_combinator_refresh( p, po, sign )
      local t
      if sign~=-1 then
         t={}
         local secondary = is_secondary(po)
         for k,s in ipairs(stats) do
            if multiengines.is_param[s.name] then
               local val = (s and ((secondary and s.sec) or s.pri)) or 0
               t[s.name] = val
            end
         end
      end
      if multiengines.decl_engine_stats(p, po, sign, t) then
         p:outfitInitSlot("engines")
         return true
      else
         return false
      end
   end

   local function update_stats( p, po)
      local multicore_off = halted_n(p, po and po:id())
      local secondary = is_secondary(po)
      local ie = is_multiengine(p, po)

      po:clear()
      for k,s in ipairs(stats) do
         if multicore_off ~= true or s.name == 'mass' then
            local val = (secondary and s.sec) or s.pri

            if not (ie and is_mobility[s.name]) then
               po:set(s.name, val)
            end
         end
      end
   end

   local function equip(p, po, sign)
      if p and po then
         if is_multiengine(p, po) then
            engines_combinator_refresh(p, po, sign)
         end
         update_stats(p, po)
      end
   end

   function onadd( p, po )
      equip(p, po, 1)
   end

   function init( p, po )
      if(multiengines.lock( p )) then
         onadd( p, po)
         multiengines.unlock( p )
      end
   end

   function onremove( p, po )
      equip(p, po, -1)
   end

end

function multicore.setworkingstatus( p, po, on)
   if p and po then
      local id = po:id()
      local off

      if on == nil then
         off = nil
      else
         off = not on
      end
      if multiengines.halt_n(p, id, off) then
         p:outfitInitSlot(id)
      end
   end
end

return multicore
