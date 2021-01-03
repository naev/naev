--[[
-- Helper functions and defines for the Minerva Station campaigns
--]]
local vn = require 'vn'

local minerva = {
   -- Main Characters
   chicken = {
      name = _("Cyborg Chicken"),
      portrait = "cyborg_chicken",
      image = "cyborg_chicken.png",
      colour = nil,
   },
   maikki = {
      name = _("Maikki"),
      portrait = "maikki",
      image = "maikki.png",
      colour = {1, 0.73, 0.97},
   },
}

-- Helpers to create main characters
function minerva.vn_cyborg_chicken()
   return vn.Character.new( minerva.chicken.name,
         { image=minerva.chicken.image, color=minerva.chicken.colour } )
end
function minerva.vn_maikki()
   return vn.Character.new( minerva.maikki.name,
         { image=minerva.maikki.image, color=minerva.maikki.colour } )
end

-- Token stuff
-- Roughly 1 token is 1000 credits
function minerva.tokens_get()
   return var.peek( "minerva_tokens" ) or 0
end
function minerva.tokens_get_gained()
   return var.peek( "minerva_tokens_gained" ) or 0
end
function minerva.tokens_pay( amount )
   local v = minerva.tokens_get()
   var.push( "minerva_tokens", v+amount )
   -- Store lifetime earnings
   if amount > 0 then
      v = var.peek( "minerva_tokens_gained" ) or 0
      var.push( "minerva_tokens_gained", v+amount )
   end
end

return minerva
