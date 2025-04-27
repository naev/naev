
local smfs = require "shipmemfs"
local fmt = require "format"
local shipstat = naev.shipstats()
local multicore = {}
local engines_comb_dir = "_ec"

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
   if val == 0 then
      return "#n0" -- Correct people bad taste HERE.
   end

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
      return col .. fmt.f("{val} {unit}", {val=str, unit=_(s.unit)})
   else
      return col .. str
   end
end

local function add_desc( stat, nomain, nosec )
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
      col=valcol( p+s, stat.stat.inverted )
   end
   local pref = col..fmt.f("{name}: ",{name=name})
   if p == s then
      return pref..stattostr( stat.stat, base, off, true )
   else
      return pref..fmt.f("{bas} #n/#0 {sec}", {
         bas = stattostr( stat.stat, p, nomain, nosec),
         sec = stattostr( stat.stat, s, nosec, nomain or not nosec),
      })
   end
end

local needs_avg = {
   accel = true,
   turn = true,
   speed = true,
}

local needs_decl = {
   accel = true,
   turn = true,
   speed = true,
   engine_limit =true
}

local function index( tbl, key )
   for i,v in ipairs(tbl) do
      if v and v["name"] == key then
         return i
      end
   end
   return nil
end

local function is_engine( po )
   if po then
      local o = po:outfit()
      local tags = o and o:tags()
      return tags and tags.engine
   end
end

local function is_multiengine( p )
   return p and p:outfitHasSlot("engines_combinator")
end

local function is_secondary( po )
   local ps =po and po:slot()
   return ps and ps.tags and ps.tags.secondary
end

local function marked_n( p, n )
   return smfs.readfile( p, {engines_comb_dir,n,'halted'})
end

local function mark_n( p, n, what )
   local res = smfs.writefile( p, {engines_comb_dir,n,'halted'}, what)

   if res == nil then -- could not write
      warn("Oh-oh")
   else
      local function f(crt)
         return crt or res
      end
      return smfs.updatefile( p, {engines_comb_dir, "needs_refresh"}, f)
   end
end

-- sign:
--  -1 for remove
--   0 for update
--   1 for add
local function update_engines_combinator_if_needed( p, po, sign, t )
   local id = po:id()
   local changed = smfs.readfile( p, {engines_comb_dir, "needs_refresh"})
   local comb = smfs.checkdir( p, {engines_comb_dir} )
   local bef

   if sign == -1 then
      changed = changed or comb[id] ~= nil
      comb[id] = nil
   else
      local combid = smfs.checkdir( p, {engines_comb_dir, id} )
      changed = changed or ((sign == 1) and (comb[id] == nil))

      for k,v in pairs(t or {}) do
         bef = combid[k]
         combid[k] = v
         changed = changed or (bef ~= v)
      end
   end
   smfs.writefile( p, {engines_comb_dir, "needs_refresh"}, changed)

   if changed then
      p:outfitInitSlot("engines_combinator")
   end
   return changed
end

function multicore.init( params )
   -- Create an easier to use table that references the true ship stats
   local stats = tcopy( params )
   local eml_unit

   for k,s in ipairs(stats) do
      s.index = index( shipstat, s[1] )
      s.stat = shipstat[ s.index ]
      s.name = s.stat.name
      s.pri, s.sec = s[2], s[3]
      if s.name == 'engine_limit' then
         eml_unit = s.stat.unit
      end
   end
   -- Sort based on shipstat table order
   table.sort( stats, function ( a, b )
      return a.index < b.index
   end )

   -- Set global properties
   notactive = true -- Not an active outfit
   hidestats = true -- We do hacks to show stats, so hide them


   -- Below define the global functions for the outfit
   function descextra( p, _o, po )
      local nomain, nosec = false, false
      if po then
         if is_secondary( po ) then
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

      local averaged = is_engine( po ) and is_multiengine( p )
      local id = po and po:id()
      local multicore_off = p and po and marked_n( p, id )
      local smid = po and smfs.readdir( p, {engines_comb_dir, id} )
      local total = smfs.readdir( p, {engines_comb_dir.."_total"} )
      local totaleml = (total and total["engine_limit"]) or 0


      for k,s in ipairs(stats) do
         local off = multicore_off and (nosec or nomain) and s.name ~= "mass"
         desc = desc .. "\n" .. add_desc( s, nomain or off, nosec or off )
      end

      if multicore_off ~= nil then
         local status
         if multicore_off == false then
            status = "#g".._("running").."#0"
         else
            status = "#r".._("HALTED").."#0"
         end
         desc = desc .. "\n#b"..fmt.f(_("Working Status: {status}"), {status=status})
      end

      if averaged and multicore_off~=true then
         desc = desc .. "\n\n"
         desc = desc .. fmt.f(_("#oLoad Factor:#0 #y{share}%#0 #o(#g{eml} {t}#0 #oout of#0 #g{total} {t}#0 #o)#0\n"),{
            share = (smid and smid["part"]) or 0,
            eml = (smid and smid["engine_limit"]) or 0,
            total = totaleml,
            t = eml_unit
         })
         for k,s in ipairs(stats) do
            if needs_avg[s.name] then
               desc = desc .. fmt.f(_("#g{display}: +{val} {unit}#0"),{
                  display = s.stat.display, unit = s.stat.unit, val = smid[s.name]
               })
               if total then
                  desc = desc .. "  #y[+" .. fmt.number(smid[s.name]*smid['engine_limit']/totaleml) .. " " .. s.stat.unit .. "]#0"
               end
               desc = desc .. "\n"
            end
         end
      end
      return desc
   end

   local function engines_combinator_refresh( p, po, sign )
      local t
      if sign~=-1 then
         t={}
         local secondary = is_secondary( po )
         for k,s in ipairs(stats) do
            if needs_decl[s.name] then
               local val = (s and ((secondary and s.sec) or s.pri)) or 0
               t[s.name] = val
            end
         end
      end
      return update_engines_combinator_if_needed( p, po, sign, t )
   end

   local function update_stats( p, po)
      local multicore_off = p and po and marked_n( p, po:id() )
      local secondary = is_secondary( po )
      local ie = is_engine( po ) and is_multiengine( p )

      po:clear()
      for k,s in ipairs(stats) do
         if multicore_off ~= true or s.name == 'mass' then
            local val = (secondary and s.sec) or s.pri

            if not (ie and needs_avg[s.name]) then
               po:set( s.name, val )
            end
         end
      end
   end

   local function equip( p, po, sign)
      if p and po then
         local res = true
         if is_engine(po) and is_multiengine( p ) then
            res = res and engines_combinator_refresh( p, po, sign )
         end
         update_stats( p, po)
         return res
      end
   end

   function init( p, po )
      equip( p, po, 1)
   end

   onadd = init

   function onremove( p, po )
      equip( p, po, -1)
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
      if mark_n( p, id, off ) then
         p:outfitInitSlot(id)
      end
   end
end

return multicore
