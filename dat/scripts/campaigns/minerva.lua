--[[
-- Helper functions and defines for the Minerva Station campaigns
--]]
local vn = require 'vn'
local colour = require 'colour'
local portrait = require 'portrait'
require 'numstring'

local minerva = {
   -- Main Characters
   chicken = {
      name = _("Cyborg Chicken"),
      portrait = "cyborg_chicken.png",
      image = "cyborg_chicken.png",
      colour = {0.9, 0.5, 0.1}, -- Orangish
   },
   kex = {
      name = _("Kex"),
      description = _("You see Kex taking a break at his favourite spot at Minerva station."),
      portrait = "cyborg_chicken.png",
      image = "cyborg_chicken.png",
      colour = {0.9, 0.5, 0.1}, -- Orangish
   },
   maikki = {
      name = _("Maikki"),
      description = _("You see a very cutely dressed young woman. She seems to have a worried expression on her face."),
      portrait = "maikki.png",
      image = "maikki.png",
      colour = {1, 0.73, 0.97}, -- Pink :D
   },
   maikkiP = {
      name = _("Pirate Lord Maikki"),
      description = _("TODO"),
      portrait = "maikki.png",
      image = "maikki.png",
      colour = {1, 0.73, 0.97}, -- Pink :D
   },
   terminal = {
      name = _("Terminal"),
      description = _("A terminal with which you can check your current token balance and buy items with tokens."),
      portrait = "minerva_terminal.png",
      image = "minerva_terminal.png",
      colour = {0.8, 0.8, 0.8},
   },
   pirate = {
      name = _("Sketchy Individual"),
      realname = _("Zuri"),
      portrait = "pirate/pirate5.webp", -- TODO REPLACE
      description = _("You see a sketchy-looking individual, they seem to have their gaze on you."),
      image = portrait.getFullPath("pirate/pirate5.webp"),
      colour = nil,
   },
   -- Secondary characters
   strangelove = {
      name = _("Dr. Strangelove"),
      portrait = "strangelove.png",
      image = "strangelove.png",
      colour = colour.FontPurple, -- Purplish (close to nebula?)
   },
   ceo = {
      name = _("Minerva CEO"),
      portrait = "minervaceo.png",
      image = "minervaceo.png",
      description = _("The CEO of Minerva Station."),
      colour = nil,
   },

   log = {
      kex = {
         idstr = "log_minerva_kex",
         logname = _("Kex"),
         logtype = _("Minerva Station"),
      },
      maikki = {
         idstr = "log_minerva_maikki",
         logname = _("Finding Maikki's Father"),
         logtype = _("Minerva Station"),
      },
      pirate = {
         idstr = "log_minerva_pirate",
         logname = _("Shady Jobs at Minerva"),
         logtype = _("Minerva Station"),
      },
   },

   loops = {
      maikki = 'snd/sounds/songs/mushroom-background.ogg',
      kex = 'snd/sounds/songs/feeling-good-05.ogg',
      pirate = 'snd/sounds/songs/hip-hop-background.ogg',
      strangelove = 'snd/sounds/songs/space-exploration-08.ogg',
      conflict = "snd/sounds/songs/run-for-your-life-00.ogg",
      news = "snd/sounds/songs/news.ogg",
   },
}

local function _merge_tables( p, params )
   params = params or {}
   for k,v in pairs(params) do p[k] = v end
   return p
end

-- Helpers to create main characters
function minerva.vn_cyborg_chicken( params )
   return vn.Character.new( minerva.chicken.name,
         _merge_tables( {
            image=minerva.chicken.image,
            color=minerva.chicken.colour,
         }, params) )
end
function minerva.vn_kex( params )
   return vn.Character.new( minerva.kex.name,
         _merge_tables( {
            image=minerva.kex.image,
            color=minerva.kex.colour,
         }, params) )
end
function minerva.vn_maikki( params )
   return vn.Character.new( minerva.maikki.name,
         _merge_tables( {
            image=minerva.maikki.image,
            color=minerva.maikki.colour,
         }, params) )
end
function minerva.vn_maikkiP( params )
   return vn.Character.new( minerva.maikkiP.name,
         _merge_tables( {
            image=minerva.maikkiP.image,
            color=minerva.maikkiP.colour,
         }, params) )
end
function minerva.vn_terminal( params )
   return vn.Character.new( minerva.terminal.name,
         _merge_tables( {
            image=minerva.terminal.image,
            color=minerva.terminal.colour,
         }, params) )
end
function minerva.vn_pirate( params )
   return vn.Character.new( minerva.pirate.name,
         _merge_tables( {
            image=minerva.pirate.image,
            color=minerva.pirate.colour,
         }, params) )
end
function minerva.vn_zuri( params )
   return vn.Character.new( minerva.pirate.realname,
         _merge_tables( {
            image=minerva.pirate.image,
            color=minerva.pirate.colour,
         }, params) )
end
function minerva.vn_ceo( params )
   return vn.Character.new( minerva.ceo.name,
         _merge_tables( {
            image=minerva.ceo.image,
            color=minerva.ceo.colour,
         }, params) )
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
function minerva.tokens_str( amount )
   return gettext.ngettext(
      "#p%s Minerva Token#0",
      "#p%s Minerva Tokens#0", amount ):format(
         numstring(amount) )
end

-- Maikki stuff
function minerva.maikki_mood_get()
   return var.peek( "maikki_mood" ) or 0
end
function minerva.maikki_mood_mod( amount )
   var.push( "maikki_mood", minerva.maikki_mood_get()+amount )
end

return minerva
