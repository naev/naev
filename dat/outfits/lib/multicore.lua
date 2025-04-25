
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
      return "#n."
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
      return col..fmt.f("{val} {unit}", {val=str, unit=_(s.unit)})
   else
      return col..str
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
   if off then
      col="#n"
   else
      col=valcol( p+s, stat.stat.inverted )
   end
   local pref = col..fmt.f("{name}: ",{name=name})
   if p==s then
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
      if v and v["name"]==key then
         return i
      end
   end
   return nil
end

local function is_engine( po )
   if po then
      local o=po:outfit()

      return o and o:tags() and o:tags().engine
   end
end

local function is_multiengine( p )
   return p and p:outfitHasSlot("engines_combinator")
end

local function is_secondary( po )
   return po and po:slot() and po:slot().tags and po:slot().tags.secondary
end

local function marked_n( p, n )
   return smfs.read( p, {"_ec",n,'halted'})
end

local function mark_n( p, n, what )
   local res = smfs.write( p, {"_ec",n,'halted'}, what)

   if res == nil then -- could not write
      print "Oh-oh"
   end
   p:shipMemory()[engines_comb_dir.."_needs_refresh"] = res
end

-- sign:
--  -1 for remove
--   0 for update
--   1 for add
local function update_engines_combinator_if_needed( p, po, sign, t )
   local sm = p:shipMemory()
   local id = po:id()
   local changed = sm[engines_comb_dir.."_needs_refresh"]
   local bef

   sm[engines_comb_dir] = sm[engines_comb_dir] or {}

   if sign == -1 then
      if sm[engines_comb_dir] then
         changed = changed or (sm[engines_comb_dir][id] ~= nil)
         sm[engines_comb_dir][id] = nil
      end
   else
      sm[engines_comb_dir] = sm[engines_comb_dir] or {}
      changed = changed or (sign==1) and (sm[engines_comb_dir][id] == nil)

      sm[engines_comb_dir][id] = sm[engines_comb_dir][id] or {}
      for k,v in pairs(t or {}) do
         bef = sm[engines_comb_dir][id][k]
         sm[engines_comb_dir][id][k] = v
         changed = changed or (bef ~= v)
      end
   end
   sm[engines_comb_dir.."_needs_refresh"] = changed

   if changed then
      --print("update!")
      p:outfitAddSlot(outfit.get("Engines Combinator"),"engines_combinator")
   end
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
      local sm
      local total

      if averaged then
         sm = p:shipMemory() and p:shipMemory()[engines_comb_dir] or {}
         total = p:shipMemory() and p:shipMemory()[engines_comb_dir.."_total"] or {}
         total = total and total['engine_limit'] or 0
      end

      local multicore_off = p and po and marked_n( p, po:id() )
      for k,s in ipairs(stats) do
         local off = multicore_off and (nosec or nomain) and s.name~="mass"
         desc = desc.."\n"..add_desc( s, nomain or off, nosec or off )
         if averaged and s.name == "engine_limit" then
            desc = desc .. fmt.f(_("\n\t#o[#0 #y{share}%#0 #oout of {total}#0 #o]#0"),{
               share = sm[po:id()] and sm[po:id()]['part'] or 0,
               total=total
            })
         end
      end

      if multicore_off ~= nil then
         desc = desc .. "\n#bWorking Status: #0"
         if multicore_off == false then
            desc = desc .. "#grunning#0"
         else
            desc = desc .. "#rHALTED#0"
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

   function setworkingstatus( p, po, on)
      if p and po then
         local off
         if on == nil then
            off = nil
         else
            off = not on
         end
         mark_n( p, po:id(), off )
         if update_engines_combinator_if_needed( p, po, 0, nil ) then
            print(fmt.f("{id} {stat}",{ id = po:id(), stat = off}))
            update_stats( p, po)
         end
      end
   end

end

return multicore
