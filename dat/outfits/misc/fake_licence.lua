notactive = true -- Doesn't become active

local fmt = require "format"
local vntk = require "vntk"

local PRICE

function onload( o )
   PRICE = o:price()
end

function descextra ()
   return "#b".._("By purchasing this license, you will reset your reputation with all independent governments to 0.").."#0"
end

local function canbuy ()
   local fct = faction.get("Independent")
   for k,sys in ipairs(system.getAll()) do
      if sys:reputation( fct ) < 0 then
         return true
      end
   end
   return _("You are not in need of a fake license.")
end

function price( _q )
   return fmt.credits(PRICE), canbuy(), false
end

function buy( _q )
   local nobuy = canbuy()
   if nobuy~=true then
      return false, nobuy
   end
   player.pay( -PRICE )
   faction.setReputationGlobal( "Independent",  0 )
   vntk.msg(_("New You"), _([[With your newly acquired identity, you have cleared your record with all independent governments.]]))
   return true, 0 -- Doesn't actually get added
end

function sell( _q )
   return false, _("You can not sell this outfit.")
end
