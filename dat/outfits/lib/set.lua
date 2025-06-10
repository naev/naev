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
         ["weapon_dmg"] : 25,
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

local function desc()
   local d = fmt.f(_("Set {setname}:"), {setname=SETNAME})
   for n,b in pairs(BONUSES) do
      d = d.."\n"
      if mem and mem.nactive >= n then
         d = d.."#g"
      else
         d = d.."#n"
      end
      d = d..fmt.f(n_([[({n}-piece) {desc}]], [[({n}-pieces) {desc}]], n ),
         {n=n, desc=b.desc}).."#0"
   end
   return d
end

local function set_init( p, po )
   local id = po:id()

   -- See how many of the set are found
   local set_outfits = {} -- Ids of outfits in the set
   local found = {} -- Unique outfits found
   for k,o in ipairs(p:outfits()) do
      if o and inlist( OUTFITS, o ) then
         if not inlist( found, o ) then
            table.insert( found, o )
         end
         table.insert( set_outfits, k )
      end
   end
   local f = #found
   mem.nactive = f

   -- Compute the active stats
   mem.active_stats = {}
   for k,b in pairs(BONUSES) do
      if f >= k and b.stats then
         for n,s in pairs(b.stats) do
            mem.active_stats = (mem.active_stats[n] or 0) + s
         end
      end
      if b.func then
         b.func( p, po, f >= k )
      end
   end

   -- Message the other outfits
   for k,i in ipairs(set_outfits) do
      if i~=id then
         p:outfitMessageSlot( i, "set_active", f )
      end
   end
end

-- Send message to lowest id outfit that will handle everything
local function set_changed( p, po )
   local id = po:id()
   for k,o in ipairs(p:outfits()) do
      if o and k<id and inlist( OUTFITS, o ) then
         p:outfitMessageSlot( k, "set_changed" )
         return
      end
   end
   set_init( p, po )
end

function lib.set( _p, po )
   -- Apply the active stats
   for k,s in pairs(mem.active_stats) do
      po:set( k, s )
   end
end

function lib.init( setname, outfits, bonuses )
   SETNAME = setname
   OUTFITS = outfits
   BONUSES = bonuses

   local descextra_old = descextra
   function descextra( p, o, po )
      local d = ""
      if descextra_old then
         d = d..descextra_old( p, o, po ).."\n"
      end
      d = d..desc()
      return d
   end

   local init_old = init
   function init( p, po )
      if init_old then
         init_old( p, po )
      end
   end

   local onoutfitchange_old = onoutfitchange
   function onoutfitchange( p, po )
      set_changed( p, po )
      if onoutfitchange_old then
         onoutfitchange_old( p, po )
      end
   end

   local message_old = message
   function message( p, po, msg, dat )
      -- Local messages are consumed
      if msg=="set_changed" then
         set_init( p, po )
         return
      elseif msg=="set_active" then
         mem.nactive = dat
         return
      end
      -- Non-consumed messages are passed
      if message_old then
         message_old( p, po, msg, dat )
      end
   end
end

return lib
