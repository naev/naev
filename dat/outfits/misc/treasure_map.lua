notactive = true -- Doesn't become active

local fmt = require "format"
local vn = require "vn"
local treasure = require "common.treasure_hunt"

local COST

function onload( o )
   COST = o:price()
end

function buy( _q )
   if treasure.maps_owned() > 3 then
      return false, _("You have too many active treasure maps already!")
   end

   local data = treasure.create_treasure_hunt()
   if not data then
      warn("Unable to generate treasure map.")
      return false, _("The map turns out to be fake, and you refuse to buy it.")
   end

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_("You obtained a treasure map - {name}."),
      {name="#b"..data.name.."#0"}))
   vn.run()

   player.pay( -COST )
   treasure.give_map_from( data )

   return true, 0 -- Doesn't actually get added
end

function sell( _q )
   return false, _("You can not sell this outfit.")
end
