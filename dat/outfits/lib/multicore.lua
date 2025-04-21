
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
   speed = true,
   turn = true,
   accel = true,
}

local function averaging_mod( s )
   local suff='_mod'
   return s.pri==0 and s.sec==-50 and
      (s.name):sub(#s.name-#suff+1,#s.name)==suff and
      needs_avg[ (s.name):sub(0,#s.name-#suff) ]
end

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

--[[ sign:
    -1 for remove
     0 for update
     1 for add
--]]
local function engine_combinator_needs_update( p, po, sign )
   local sm = p:shipMemory()
   local found

   sm._stk = sm._stk or {}
   for i,v in ipairs(sm._stk) do
      if v == po then
         found=i
         break
      end
   end
   if sign == -1 then
      if found then
         sm._stk.remove(found)
      else
         return false
      end
   else
      if found then
         if sign == 1 then
            return false
         end
      else
         table.insert(sm._stk,0,po)
      end
   end
   return true
end

local function engine_combinator_refresh( p )
   p:outfitAddIntrinsic(outfit.get("Engine Combinator"))
end

function multicore.init( params )
   -- Create an easier to use table that references the true ship stats
   local stats = tcopy( params )

   -- possible values:
   --  - false -> status running
   --  - true -> status halted
   --  - nil -> no status (always running)
   local multicore_off = nil

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

      local averaged=false
      if multicore_off~=true then
         for k,s in ipairs(stats) do
            averaged = averaged or averaging_mod( s )
         end
      end
      for k,s in ipairs(stats) do
         if not averaging_mod( s ) then
            local off=multicore_off and (nosec or nomain) and s.name~="mass"
            desc = desc.."\n"..add_desc( s, nomain or off, nosec or off )
            if averaged and nomain and (not nosec) and needs_avg[s.name] then
               desc = desc.."   #o(avg with#0 #rpri#0#o)#0"
            end
         end
      end
      if po and multicore_off ~= nil then
         desc = desc .. "\n#bWorking Status: #0"
         if multicore_off==false then
            desc = desc .. "#grunning#0"
         else
            desc = desc .. "#rHALTED#0"
         end
      end
      return desc
   end

   local function changed( p, po, delta, delta_c )
      if is_engine(po) then
         local sm = p:shipMemory()
         local secondary = is_secondary( po )

         if engine_combinator_needs_update( p , po, delta ) then
            sm._engine_count = (sm._engine_count or 0) + delta
            for k,s in ipairs(stats) do
               if multicore_off~=true and needs_avg[s.name] then
                  local val = (secondary and s.sec) or s.pri
                  sm["_"..s.name] = (sm["_"..s.name] or 0) + (val * delta_c)
               end
            end
            engine_combinator_refresh( p )
         end
      end
   end

   local function onoff_mul()
      return (multicore_off~=true and 1) or 0
   end

   function onadd( p, po )
      changed( p, po, onoff_mul(), 1)
   end

   function onremove( p, po )
      changed( p, po, -onoff_mul(), -1)
   end

   local function update_stats( po)
      local secondary = is_secondary( po )
      local ie=is_engine( po )

      po:clear()
      for k,s in ipairs(stats) do
         if multicore_off~=true or s.name=='mass' then
            local val=(secondary and s.sec) or s.pri

            if not (ie and needs_avg[s.name]) then
               po:set( s.name, val )
            end
         end
      end
   end

   function init( p, po )
      update_stats( po)
      changed( p, po, 0, 1)
   end

   function setworkingstatus( p, po, on)
      local before = multicore_off

      if on==nil then
         multicore_off = nil
      else
         multicore_off = not on
      end
      if (before and not multicore_off) or (not before and multicore_off) then
         update_stats( po)
         if multicore_off then
            changed( p, po, 0, -1)
         else
            changed( p, po, 0, 1)
         end
      end
   end
end

return multicore
