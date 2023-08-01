--[[
-- Helper functions and defines for the Minerva Station campaigns
--]]
local vn = require 'vn'
local colour = require 'colour'
local fmt = require "format"
local equipopt = require "equipopt"

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
      portrait = "maikki_pirate.webp",
      image = "maikki_pirate.webp",
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
      portrait = "zuri.webp",
      description = _("You see a sketchy-looking individual, they seem to have their gaze on you."),
      image = "zuri.webp",
      colour = {0.73, 1, 0.73},
   },
   zuri = {
      name = _("Zuri"),
      portrait = "zuri.webp",
      description = _("You see Zuri who seems to be motioning for you to come."),
      image = "zuri.webp",
      colour = {0.73, 1, 0.73},
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
      portrait = "minervaceo.webp",
      image = "minervaceo.webp",
      description = _("The CEO of Minerva Station."),
      colour = nil,
   },
   mole = {
      name = _("Mole"),
      portrait = "minervamole.webp",
      image = "minervamole.webp",
      colour = nil,
   },
   scavengera = {
      name = _("Scavenger A"),
      portrait = "scavenger1.png",
      image = "scavenger1.png",
      colour = nil,
   },
   scavengerb = {
      name = _("Scavenger B"),
      portrait = "scavenger1.png",
      image = "scavenger1.png",
      colour = nil,
   },
   scavengers = {
      name = _("Scavengers"),
      portrait = "scavenger1.png",
      image = "scavenger1.png",
      description = _("You see a pair of dirty looking fellows talking loudly among themselves."),
      colour = nil,
   },

   log = {
      kex = function( text )
         shiplog.create( "log_minerva_kex", _("Kex"), _("Minerva Station") )
         shiplog.append( "log_minerva_kex", text )
      end,
      maikki = function( text )
         shiplog.create( "log_minerva_maikki", _("Finding Maikki's Father"), _("Minerva Station") )
         shiplog.append( "log_minerva_maikki", text )
      end,
      pirate = function( text )
         shiplog.create( "log_minerva_pirate", _("Shady Jobs at Minerva"), _("Minerva Station") )
         shiplog.append( "log_minerva_pirate", text )
      end,
      misc = function( text )
         shiplog.create( "log_minerva_misc", _("Miscellaneous"), _("Minerva Station") )
         shiplog.append( "log_minerva_misc", text )
      end,
   },

   loops = {
      maikki      = 'snd/sounds/songs/mushroom-background.ogg',
      kex         = 'snd/sounds/songs/feeling-good-05.ogg',
      pirate      = 'snd/sounds/songs/hip-hop-background.ogg',
      strangelove = 'snd/sounds/songs/space-exploration-08.ogg',
      conflict    = "snd/sounds/songs/run-for-your-life-00.ogg",
      news        = "snd/sounds/songs/news.ogg",
   },

   rewards = {
      maikki1 = nil, -- Chase scavengers and the works
      maikki2 = nil, -- Find Dr. Strangelove, gets paid in tokens (500 ~= 1M creds)
      kex1 = 400e3, -- Capture cargo from ship (or kill)
      kex2 = 400e3, -- Steal from baroness eve
      kex3 = 600e3, -- Major malik - VR deathmatch
      kex4 = 1e6, -- Kill Jie (kestrel)
      kex5 = 1e6, -- Bounty hunters + Dr. Strangelove's death
      pirate1 = 400e3, -- Harass Dvaered thugs
      pirate2 = 400e3, -- Destroy drones
      pirate3 = 500e3, -- Get ticket from harper
      pirate4 = 800e3, -- Defend torture ship destroying Dvaered Goddard!!
      pirate5 = 800e3, -- Za'lek hacking station
      pirate6 = 900e3, -- Take down both targets
   },
}

-- Helpers to create main characters
function minerva.vn_cyborg_chicken( params )
   return vn.Character.new( minerva.chicken.name,
         tmerge( {
            image=minerva.chicken.image,
            color=minerva.chicken.colour,
         }, params) )
end
function minerva.vn_kex( params )
   return vn.Character.new( minerva.kex.name,
         tmerge( {
            image=minerva.kex.image,
            color=minerva.kex.colour,
         }, params) )
end
function minerva.vn_maikki( params )
   return vn.Character.new( minerva.maikki.name,
         tmerge( {
            image=minerva.maikki.image,
            color=minerva.maikki.colour,
         }, params) )
end
function minerva.vn_maikkiP( params )
   return vn.Character.new( minerva.maikkiP.name,
         tmerge( {
            image=minerva.maikkiP.image,
            color=minerva.maikkiP.colour,
         }, params) )
end
function minerva.vn_terminal( params )
   return vn.Character.new( minerva.terminal.name,
         tmerge( {
            image=minerva.terminal.image,
            color=minerva.terminal.colour,
         }, params) )
end
function minerva.vn_pirate( params )
   return vn.Character.new( minerva.pirate.name,
         tmerge( {
            image=minerva.pirate.image,
            color=minerva.pirate.colour,
         }, params) )
end
function minerva.vn_zuri( params )
   return vn.Character.new( minerva.zuri.name,
         tmerge( {
            image=minerva.zuri.image,
            color=minerva.zuri.colour,
         }, params) )
end
function minerva.vn_ceo( params )
   return vn.Character.new( minerva.ceo.name,
         tmerge( {
            image=minerva.ceo.image,
            color=minerva.ceo.colour,
         }, params) )
end
function minerva.vn_strangelove( params )
   return vn.Character.new( minerva.strangelove.name,
         tmerge( {
            image=minerva.strangelove.image,
            color=minerva.strangelove.colour,
         }, params) )
end
function minerva.vn_mole( params )
   return vn.Character.new( minerva.mole.name,
         tmerge( {
            image=minerva.mole.image,
            color=minerva.mole.colour,
         }, params) )
end

-- Token stuff
-- Roughly 1 token is 1000 credits
local tokens = N_("Minerva Token")
function minerva.tokens_get()
   return player.inventoryOwned( tokens )
end
function minerva.tokens_get_gained()
   return var.peek( "minerva_tokens_gained" ) or 0
end
function minerva.tokens_pay( amount )
   player.inventoryAdd( tokens, amount )
   -- Store lifetime earnings
   if amount > 0 then
      local v = var.peek( "minerva_tokens_gained" ) or 0
      var.push( "minerva_tokens_gained", v+amount )
   end
end
function minerva.tokens_str( amount )
   return "#p"..gettext.ngettext(
      "%s Minerva Token",
      "%s Minerva Tokens", amount ):format(
         fmt.number(amount) ).."#0"
end

-- Maikki stuff
function minerva.maikki_mood_get()
   local mood = 0
   if var.peek("maikki_gave_drink") then
      mood = mood+1
   end
   local response = var.peek("maikki_response")
   if response then
      if response=="no" then
         mood = mood-1
      elseif response=="yes" then
         mood = mood+1
      end
   end
   return mood
end

-- Spawns the pink demon at location pos
function minerva.pink_demon( pos, params )
   params = tmerge( {naked=true}, params or {} )
   local p = pilot.add( "Pirate Kestrel", "Wild Ones", pos, _("Pink Demon"), params )
   equipopt.pirate( p, {
      type_range = {
         ["Launcher"] = { max = 0 },
         ["Turret Launcher"] = { max = 0 },
      }
   } )
   p:intrinsicSet( "fwd_damage", 15 )
   p:intrinsicSet( "tur_damage", 15 )
   p:intrinsicSet( "fwd_dam_as_dis", 30 )
   p:intrinsicSet( "tur_dam_as_dis", 30 )
   return p
end

function minerva.fct_wildones()
   local id = "minerva_wildones"
   local f = faction.exists( id )
   if f then
      return f
   end
   return faction.dynAdd( "Wild Ones", id, _("Wild Ones"),
         {clear_enemies=true, clear_allies=true, player=0} )
end

--[[
List of mission variables:

- maikki_gave_drink (true, nil)
- maikki_response ("yes", "no", nil)
- maikki_scavengers_alive (true, nil)
- harper_ticket ("credits", "tokens", "free", "stole" )
- strangelove_death ("unplug", "comforted", "shot", nil)

--]]

return minerva
