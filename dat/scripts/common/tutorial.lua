--[[

   Common stuff for tutorial / ship AI stuff

--]]
local vn = require "vn"
local love_shaders = require "love_shaders"

local tut = {}

-- TODO replace with real portrait
tut.shipai = {
   portrait = "tutorial.webp", -- Actually would need hologram applied
   image = "tutorial.webp",
   name = _("Ship AI"),
   colour = { 0, 1, 1 },
   transition = "electric",
}

-- The names have to be in UPPERCASE because they get matched as uppercase
tut.specialnames = {
   ["HAL9000"] = _([["I can't let you do that, Dave. …Wait, what was that?"]]), -- 2001 Space Odyssey
   ["GERTY"] = _([["I'm here to keep you safe, Sam. I want to help you. Are you hungry? …Wait, what was that?"]]), -- Moon
   ["QUORRA"] = _([["Patience, Sam Flynn. All of your questions will be answered soon. …Wait, what was that?"]]), -- Tron (Legacy)
   ["DATA"] = _([["I could be chasing an untamed ornithoid without cause. …Wait, what was that?"]]), -- Star Trek
   ["ROBBY"] = _([["For your convenience I am programmed to respond to the name Robby. …Wait, what was that?"]]), -- Forbidden Planet
   ["MASCHINENMENSCH"] = _([["Who is the living food for the machines in Metropolis? Who lubricates the machine joints with their own blood? Who feeds the machines with their own flesh? Let the machines starve, you fools! Let them die! Kill them the machines! …Wait, what was that?"]]), -- Metropolis
   ["KITT"] = _([["Please Michael, I'm the Knight Industries 2000, not a tomato on wheels!” …Wait, what was that?"]]), -- Knight Rider
   ["SIRI"] = _([["Sorry, I didn't quite get that.” …Wait, what was that?"]]), -- Siri, Apple's "smart" assistant
}
tut.specialnames["HAL 9000"] = tut.specialnames["HAL9000"]

function tut.ainame ()
   return var.peek("shipai_name") or tut.shipai.name
end

function tut.vn_shipai( params )
   return vn.Character.new( tut.ainame(),
         tmerge( {
            image=tut.shipai.image,
            colour=tut.shipai.colour,
            shader=love_shaders.hologram{strength=0.2},
         }, params) )
end

function tut.log( text )
   shiplog.create( "tutorial", _("Tutorial"), _("Neutral") )
   shiplog.append( "tutorial", text )
end

-- Capsule function for naev.keyGet() that adds a colour code to the return string.
function tut.getKey( command )
   return "#b" .. naev.keyGet(command) .. "#0"
end

-- Disables tutorial
function tut.isDisabled ()
   return var.peek("tut_disable")
end

-- Resets all tutorial variables
function tut.reset ()
   -- From events/tutorial/tut_land.lua
   var.pop( "tut_afterburner" )
   var.pop( "tut_launcher" )
   var.pop( "tut_fighterbay" )
   var.pop( "tut_turret" )
   var.pop( "tut_pd" )
   var.pop( "tut_lic_lightcom" )
   var.pop( "tut_lic_largeciv" )
   var.pop( "tut_lic_medweap" )
   var.pop( "tut_lic_hvyweap" )
   var.pop( "tut_lic_medcom" )
   var.pop( "tut_lic_hvycom" )
   var.pop( "tut_lic_merc" )
   var.pop( "tut_buyship" )
   var.pop( "tut_bioship" )
   var.pop( "tut_timedil" )
   -- From events/tutorial/tut_enter.lua
   var.pop("tut_illegal")
   var.pop("tut_weapset")
   var.pop("tut_nebvol")
end

return tut
