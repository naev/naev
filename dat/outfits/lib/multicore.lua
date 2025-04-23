
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

local function is_secondary( po )
   return po and po:slot() and po:slot().tags and po:slot().tags.secondary
end


--[[
mf=tostring

local function strstk( p)
   local acc = ""
   if p then
      local sm = p:shipMemory()

      if sm then
         for i,v in pairs(sm._stk) do
            acc = acc .. " " .. fmt.number(i)
         end
      end
   end
   return acc
end
--]]

-- sign:
--  -1 for remove
--   0 for update
--   1 for add
local function engine_combinator_needs_update( p, po, sign )
   local sm = p:shipMemory()
   local found

   sm._stk = sm._stk or {}

   found = (sm._stk[po:id()] == true)
   --print(" [" .. strstk(p) .. " ] . " .. fmt.number(po:id()) .. " " .. fmt.number(sign))

   if sign == -1 then
      if found then
         sm._stk[po:id()] = nil
      end
      return found
   elseif found then
      return sign == 0
   elseif sign == 0 then
      print("Trying to update a non-equipped engine.")
      return false
   else
      sm._stk[po:id()] = true
      return true
   end
end

function multicore.init( params )
   -- Create an easier to use table that references the true ship stats
   local stats = tcopy( params )

   -- possible values:
   --  - false -> status running
   --  - true -> status halted
   --  - nil -> no status (always running)
   local multicore_off = true

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
   function descextra( _p, _o, po )
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

      local averaged=is_engine( po )
      for k,s in ipairs(stats) do
         local off = multicore_off and (nosec or nomain) and s.name~="mass"
         desc = desc.."\n"..add_desc( s, nomain or off, nosec or off )
         if averaged and nomain and (not nosec) and needs_avg[s.name] then
            desc = desc.."  #o[avg with#0 #rpri#0#o]#0"
         end
      end
      if po and (multicore_off ~= nil) then
         desc = desc .. "\n#bWorking Status: #0"
         if multicore_off==false then
            desc = desc .. "#grunning#0"
         else
            desc = desc .. "#rHALTED#0"
         end
      end
      return desc
   end

   local function engine_combinator_refresh( p, po, delta, delta_c )
      local secondary = is_secondary( po )
      local sm=p:shipMemory()

      sm._engine_count = (sm._engine_count or 0) + delta
      for k,s in ipairs(stats) do
         if (multicore_off~=true) and needs_avg[s.name] then
            local val = (secondary and s.sec) or s.pri
            sm["_"..s.name] = (sm["_"..s.name] or 0) + (val * delta_c)
         end
      end
      p:outfitRmIntrinsic(outfit.get("Engine Combinator"))
      p:outfitAddIntrinsic(outfit.get("Engine Combinator"))
   end

   local function onoff_mul()
      return ((multicore_off~=true) and 1) or -1
   end

   local function update_stats( po)
      local secondary = is_secondary( po )
      local ie = is_engine( po )

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

   -- nil: non-togglable  false: off  true: on
   local function workingstatus_change( _p, _po, on)
      local before = multicore_off

      if on == nil then
         multicore_off = nil
      else
         multicore_off = not on
      end
      --if before~=multicore_off then
      --   print(" " .. mf(before) .. " -> " .. mf(multicore_off) .. " [" .. fmt.number(po:id()) .. "]")
      --end
      return (before and (not multicore_off or multicore_off == nil)) or (not before and multicore_off)
   end

   local function equip( p, po, sign)
      local ssign

      if sign == -1 then
         if not workingstatus_change( p, po, false) then
            ssign = 0
         end
      else
         workingstatus_change( p, po, nil)
         ssign = sign
      end
      if is_engine(po) and  engine_combinator_needs_update( p , po, sign ) then
         engine_combinator_refresh( p, po, sign, ssign )
      end
      update_stats( po)
   end

   function init( p, po )
      if p and po then
         --print ("init " .. mf(multicore_off) .. " [" .. fmt.number(po:id()) .. "]")
         equip( p, po, 1)
      end
   end

   function onadd( p, po )
      if p and po then
         --print ("onadd " .. mf(multicore_off) .. " [" .. fmt.number(po:id()) .. "]")
         equip( p, po, 1)
      end
   end

   function onremove( p, po )
      if p and po then
         --print ("onremove " .. mf(multicore_off) .. " [" .. fmt.number(po:id()) .. "]")
         equip( p, po, -1)
      end
   end

   function setworkingstatus(  p, po, on)
      local doit
      if is_engine(po) then
         -- we can check this is still equipped
         doit = engine_combinator_needs_update( p , po, 0)
      else
         doit = true
      end

      if doit and workingstatus_change( p, po, on) then
         if is_engine(po) then
            engine_combinator_refresh( p, po, 0, onoff_mul())
         end
         update_stats( po)
      end
   end

end

return multicore
