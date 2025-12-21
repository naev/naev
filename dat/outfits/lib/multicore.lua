local multicore = {}

local shipstat = naev.shipstats()

local fmt = require "format"

local DBG = false
local multiengines = require "outfits/lib/multiengines"
local tfs = multiengines.tfs
local is_mobility = multiengines.is_mobility

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

local function stattostr( s, val, grey, unit, gb )
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
      str = "+" .. fmt.number(val)
   else
      str = fmt.number(val)
   end
   if unit and s.unit then
      str = str .. " " .. _(s.unit)
   end
   return col .. str
end

local function add_desc( stat, nomain, nosec, gb, basic )
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
      col = "#n"
   else
      col = valcol(p+s, stat.stat.inverted, gb)
   end
   local pref = col .. fmt.f(_("{name}: "), {name=name})
   if p == s or basic then
      return pref .. stattostr(stat.stat, base, off, true, gb)
   else
      return pref .. fmt.f(_("{bas} #n/#0 {sec}"), {
         bas = stattostr(stat.stat, p, nomain, nosec, gb),
         sec = stattostr(stat.stat, s, nosec, nomain or not nosec, gb),
      })
   end
end

local function index( tbl, key )
   for i, v in ipairs(tbl) do
      if v and v["name"] == key then
         return i
      end
   end
   return nil
end

local function is_primary_or_secondary( o )
   if o:slotExtra()~=nil then
      return true, true
   end
   local _slot_name, _slot_size, slot_prop = o:slot()
   if slot_prop:match" %(Secondary%)$" then
      return false, true
   else
      return true, false
   end
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

local SETFUNC
function multicore.init( params, setfunc )
   -- Create an easier to use table that references the true ship stats
   local stats = tcopy(params)
   local pri_en_stats = {}
   local sec_en_stats = {}
   SETFUNC = setfunc

   for k, s in ipairs(stats) do
      s.index = index(shipstat, s[1])
      s.stat = shipstat[ s.index ]
      s.name = s.stat.name
      s.pri, s.sec = s[2], s[3] or s[2]
      -- force engine_limit to be last in order
      if s.name == "engine_limit" then
         s.index = #shipstat + 1
      end
      if multiengines.is_mobility[s.name] then
         pri_en_stats[s.name] = s.pri
         sec_en_stats[s.name] = s.sec
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
   local descextra_old = descextra
   function descextra( p, o, po )
      local basic = var.peek("hide_multi")
      local nomain, nosec = false, false
      if po then
         if is_secondary(po) then
            nomain = true
         else
            nosec = true
         end
      end

      local desc = ''

      if not basic then
         if nomain then
            desc = desc .. "#o" .. fmt.f(_("Equipped as {type} core"),
               {type="#y"..p_("core", "secondary").."#o"}) .. "#0"
         elseif nosec then
            desc = desc .. "#o" .. fmt.f(_("Equipped as {type} core"),
               {type="#r"..p_("core", "primary").."#o"}) .. "#0"
         else
            local epri, esec = is_primary_or_secondary( o )
            if epri and esec then
               desc = desc .. "#n" .. fmt.f(_("Properties for {pri} / {sec} slots"), {
                  pri= "#r" .. p_("core", "primary") .. "#n",
                  sec= "#y" .. p_("core", "secondary") .. "#n",
               }) .. "#0"
            else
               local slotname
               if epri then
                  slotname = "#r" .. p_("core", "primary") .. "#n"
               else
                  slotname = "#y" .. p_("core", "secondary") .. "#n"
               end
               desc = desc .. "#n" .. fmt.f(_("Properties for {slot} slots"), {
                  slot=slotname,
               }) .. "#0"
            end
         end
      end

      local id = po and po:id()
      local averaged = id and is_multiengine(p, po)
      local multicore_off = nil
      local smid

      if averaged then
         smid = p:outfitMessageSlot("engines", "ask", id)
         multicore_off = smid and smid['halted']
         averaged = smid
      end

      for k, s in ipairs(stats) do
         local off = multicore_off and (nosec or nomain) and s.name ~= "mass"
         if desc~='' then
            desc = desc..'\n'
         end
         desc = desc .. add_desc(s, nomain or off, nosec or off, averaged and is_mobility[s.name] and s.name ~= "engine_limit", basic)
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

      if averaged and multicore_off ~= true and id then
         local totaleml = smid and smid['total'] or 0
         local share = math.floor(0.5 + (100*smid['engine_limit'])/smid['total'])
         desc = desc .. fmt.f(
            _("\n\n#oLoad Factor: #y{share}%#0  #o(#g{eml} {t}#0 #o/#0 #g{total} {t}#0 #o)#0\n"),
            {
               eml = (smid and smid["engine_limit"]) or 0,
               total = totaleml, share = share, t = _(multiengines.mobility_stats['engine_limit'].unit)
            }
         )
         for _k, s in ipairs(stats) do
            if is_mobility[s.name] and s.name~="engine_limit" then
               desc = desc .. fmt.f(_("#g{display}: #b+{val} {unit}#0"), {
                  display = _(s.stat.display), unit = _(s.stat.unit), val = fmt.number(smid[s.name] or 0) })
               desc = desc .. _("  #y=>#0  #g+") .. fmt.number((smid[s.name] or 0)*share/100) .. p_("unit", " ") .. _(s.stat.unit) .. "#0\n"
            end
         end
      end

      if not basic and o and o:slotExtra() == nil then
         local _epri, esec = is_primary_or_secondary( o )
         local slot_msg
         if esec then
            slot_msg = '#y' .. _('secondary') .. '#b'
         else
            slot_msg = '#r' .. _('primary') .. '#b'
         end
         local msg = fmt.f(_('This outfit can only be equipped as {slot}.'), {slot=slot_msg})
         desc = desc .. '\n#b' .. msg .. '#0'
      end
      if descextra_old then
         desc = desc.. '\n' .. descextra_old(p, o, po)
      end
      return desc
   end

   local function mystats( po, sign )
      return (sign ~= -1) and ((is_secondary(po) and sec_en_stats) or pri_en_stats)
   end

   local function equip( p, po, sign )
      if p and po then
         local ie = is_multiengine(p, po)
         local secondary = is_secondary(po)
         local id = po:id()
         local multicore_off = nil

         mem.stats = {}
         if ie then
            if DBG then
               tfs.append(p:shipMemory(), {"history"}, (sign==-1 and "u" or "") .. 'eq_' .. tostring(po:id()))
            end
            if secondary then
               p:outfitMessageSlot("engines", "here", {id = id, sign = sign, t = mystats(po, sign)})
            else
               local sh = p:ship()
               sh = sh and sh:getSlots()

               for n, o in ipairs(p:outfits()) do
                  if is_engine(sh[n]) and o then
                     p:outfitMessageSlot(n, "pliz", sign)
                  end
               end
            end
            p:outfitMessageSlot("engines", "done")
            local smid = p:outfitMessageSlot("engines", "ask", id)
            multicore_off = smid and smid['halted']
         end

         for k, s in ipairs(stats) do
            if multicore_off ~= true or s.name == 'mass' then
               local val = (secondary and s.sec) or s.pri

               if not (ie and is_mobility[s.name]) then
                  mem.stats[s.name] = val
               end
            end
         end

         -- Deferred setting of stats
         multicore.set(p, po)
      end
   end

   local onadd_old = onadd
   function onadd( p, po )
      equip(p, po, 1)
      if onadd_old then
         onadd_old(p, po)
      end
   end

   local init_old = init
   function init( p, po )
      equip(p, po, 1)
      if init_old then
         init_old(p, po)
      end
   end

   local message_old = message
   function message( p, po, msg, dat )
      mem.gathered_data = mem.gathered_data or {}
      if not p or not po then
         warn("message on nil p/po")
      elseif not is_multiengine(p, po) then
         warn("message on non-multiengine slot")
      else
         if DBG then
            tfs.append(p:shipMemory(), {"history"}, '>' .. msg .. '_' .. tostring(po:id()))
         end
         if msg == "pliz" then
            p:outfitMessageSlot("engines", "here", {id = po:id(), sign = dat, t = mystats(po, dat)})
         elseif is_secondary(po) then
            warn('message "'.. msg ..'" send to secondary (ignored)')
         elseif msg == "halt" then
            if multiengines.halt_n(mem.gathered_data, dat.id, dat.off) then
               multiengines.refresh(mem.gathered_data, po, false)
               multicore.set(p, po)
            end
         elseif msg == "ask" then
            return multiengines.engine_stats(mem.gathered_data, dat)
         elseif msg == "here" then
            multiengines.decl_engine_stats(mem.gathered_data, dat.id, dat.sign, dat.t)
         elseif msg == "done" then
            multiengines.refresh(mem.gathered_data, po, true)
            multicore.set(p, po)
         elseif msg == "wtf?" then
            return mem.gathered_data
         else
            warn('Unknown message: "' .. msg .. '"')
         end
      end

      if message_old then
         message_old(p, po, msg, dat)
      end
   end

   local onremove_old = onremove
   function onremove( p, po )
      equip(p, po, -1)
      if onremove_old then
         onremove_old(p, po)
      end
   end

   -- We'll use broader than onadd and onremove here
   if setfunc then
      local onoutfitchange_old = onoutfitchange
      function onoutfitchange( p, po )
         if onoutfitchange_old then
            onoutfitchange_old(p, po)
         end
         multicore.set(p, po)
      end
   end

   return multicore
end

function multicore.set( p, po )
   local fuel = p:fuel()
   --po:clear()
   if mem.stats then
      for s, val in pairs(mem.stats) do
         po:set(s, val)
      end
   end
   -- Apply a set function if applicable
   if SETFUNC then
      SETFUNC(p, po)
   end
   p:setFuel( math.max( fuel, p:fuel() ) )
end

function multicore.setworkingstatus( p, po, on )
   if p and po then
      local id = po:id()
      local off

      if on == nil then
         off = nil
      else
         off = not on
      end
      p:outfitMessageSlot("engines", "halt", {id=id, off=off})
   end
end

return multicore
