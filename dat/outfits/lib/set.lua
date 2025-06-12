--[[

   Library for outfit sets

NAME = _("Foo")
OUTFITS = {
   outfit.get("A"),
   outfit.get("B"),
}
BONUSES = {
   [2] = {
      desc = _("Foo"),
      stats = {
         ["weapon_dmg"] = 25,
      },
      func = function ( p, po, on )
         -- Do something not about stats
      end,
   },
}

--]]
local lib = {}
local fmt = require "format"

local SETNAME = ""
local OUTFITS = {}
local BONUSES = {}
local NOSET = false

local function count_active( p )
   local found = {} -- Unique outfits found
   local lowest
   for k,o in ipairs(p:outfits()) do
      if o and inlist( OUTFITS, o ) then
         lowest = lowest or k
         if not inlist( found, o ) then
            table.insert( found, o )
         end
      end
   end
   return found, lowest
end

local function desc()
   local p = player.pilot()
   local nactive = (p:exists() and #count_active( p )) or 0
   local d = fmt.f(_("Set {setname}:"), {setname=SETNAME})
   -- Assume a max of 5-piece sets, reasonable I think
   for n=1,5 do
      local b = BONUSES[n]
      if b then
         d = d.."\n"
         if nactive>=n then
            d = d.."#g"
         else
            d = d.."#n"
         end
         d = d..fmt.f(n_([[({n}-piece) {desc}]], [[({n}-pieces) {desc}]], n ),
            {n=n, desc=b.desc}).."#0"
      end
   end
   return d
end

local function set_init( p, po )
   -- See how many of the set are found
   local found, lowest = count_active( p )
   local f = #found
   mem.nactive = f

   -- Compute the active stats if it is the lowest id
   local ismain = (lowest==po:id())
   mem.active_stats = {}
   for k,b in pairs(BONUSES) do
      if ismain and f >= k and b.stats then
         for n,s in pairs(b.stats) do
            mem.active_stats = (mem.active_stats[n] or 0) + s
         end
      end

      -- Run function if applicable, but will be false for all but primary
      if b.func then
         b.func( p, po, ismain and (f >= k) )
      end
   end

   -- If we want to set (e.g., not multicore), we'll set here
   if not NOSET then
      lib.set( p, po )
   end
end

function lib.set( _p, po )
   if mem.active_stats then
      -- Apply the active stats
      for k,s in pairs(mem.active_stats) do
         po:set( k, s )
      end
   end
end

function lib.init( setname, outfits, bonuses, noset )
   SETNAME = setname
   OUTFITS = outfits
   BONUSES = bonuses
   NOSET = noset

   local descextra_old = descextra
   function descextra( p, o, po )
      local d = ""
      if descextra_old then
         d = d..descextra_old( p, o, po ).."\n"
      end
      d = d..desc()
      return d
   end

   local onoutfitchange_old = onoutfitchange
   function onoutfitchange( p, po )
      set_init( p, po )
      if onoutfitchange_old then
         onoutfitchange_old( p, po )
      end
   end
end

return lib
