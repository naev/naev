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
      func = function ( p, po )
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
local NACTIVE = 0
local DESCRIPTION = ""
local HASSTATS = false
local active_stats = {}

local function desc ()
   local d = fmt.f(_("Set {setname}:"), {setname=SETNAME})
   for n,b in pairs(BONUSES) do
      d = d.."\n"
      if n >= NACTIVE then
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
   NACTIVE = f

   -- Compute the active stats
   active_stats = {}
   for k,b in pairs(BONUSES) do
      if f >= k and b.stats then
         for n,s in pairs(b.stats) do
            active_stats = (active_stats[n] or 0) + s
         end
      end
   end

   -- Apply the active stats
   for k,s in pairs(active_stats) do
      po:set( k, s )
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
      if o and k<id then
         p:outfitMessageSlot( k, "set_changed" )
         return
      end
   end
   set_init( p, po )
end

local function hasstats ()
   for k,b in pairs(BONUSES) do
      if b.stats then
         return true
      end
   end
   return false
end

function lib.init( setname, outfits, bonuses )
   SETNAME = setname
   OUTFITS = outfits
   BONUSES = bonuses
   HASSTATS = hasstats()
   DESCRIPTION = desc()

   local descextra_old = descextra
   function descextra( p, o, po )
      local d = ""
      if descextra_old then
         d = d..descextra_old( p, o, po ).."\n"
      end
      d = d..DESCRIPTION
      return d
   end

   local init_old = init
   function init( p, po )
      if init_old then
         init_old( p, po )
      end
   end

   local onadd_old = onadd
   function onadd( p, po )
      if onadd_old then
         onadd_old( p, po )
      elseif HASSTATS then
         po:clear()
      end
      set_changed( p, po )
   end

   local onremove_old = onremove
   function onremove( p, po )
      if onremove_old then
         onremove_old( p, po )
      elseif HASSTATS then
         po:clear()
      end
      set_changed( p, po )
   end

   local message_old = message
   function message( p, po, msg, dat )
      -- Local messages are consumed
      if msg=="set_changed" then
         set_init( p, po )
         return
      elseif msg=="set_active" then
         NACTIVE = dat
         DESCRIPTION = desc()
         return
      end
      -- Non-consumed messages are passed
      if message_old then
         message_old( p, po, msg, dat )
      end
   end
end

return lib
