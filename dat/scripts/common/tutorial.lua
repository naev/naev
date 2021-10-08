--[[

   Common stuff for tutorial / ship AI stuff

--]]
local vn = require "vn"
local mt = require 'merge_tables'
local love_shaders = require "love_shaders"
local portrait = require 'portrait' -- temporary for now

local tut = {}

-- TODO replace with real portrait
local pshipai  = portrait.get()
tut.shipai = {
   portrait = pshipai,
   image = portrait.getFullPath( pshipai ),
   name = _("Ship AI"),
   color = { 0, 1, 1 },
   transition = "electric",
}

function tut.ainame ()
   return var.peek("shipai_name") or tut.shipai.name
end

function tut.vn_shipai( params )
   return vn.Character.new( tut.ainame(),
         mt.merge_tables( {
            image=tut.shipai.image,
            color=tut.shipai.colour,
            shader=love_shaders.hologram{strength=0.2},
         }, params) )
end

function tut.log( text )
   shiplog.create( "tutorial", _("Tutorial"), _("Neutral") )
   shiplog.append( "tutorial", text )
end

-- Capsule function for tk.msg that disables all key input WHILE the msg is open.
function tut.tkMsg( title, msg, keys )
    naev.keyDisableAll()
    enableBasicKeys()
    tk.msg(title, msg)
    if keys ~= nil then
       enableKeys(keys)
    else
       naev.keyEnableAll()
    end
end

-- Capsule function for enabling the keys passed to it in a table, plus some defaults.
function tut.enableKeys( keys )
    naev.keyDisableAll()
    for i, key in ipairs(keys) do
        naev.keyEnable(key, true)
    end
    enableBasicKeys()
end

-- Capsule function for enabling basic, important keys.
function tut.enableBasicKeys()
    local alwaysEnable = { "speed", "menu", "screenshot", "console" }
    for i, key in ipairs(alwaysEnable) do
        naev.keyEnable(key, true)
    end
end

-- Capsule function for naev.keyGet() that adds a color code to the return string.
function tut.getKey( command )
    return "#b" .. naev.keyGet(command) .. "#0"
end

-- Disables tutorial
function tut.isDisabled ()
   return var.peek("tut_disable")
end

-- Resets all tutorial variables
function tut.reset ()
   var.pop( "tut_disable" )
   var.pop( "tut_nebvol" )
   var.pop( "tut_afterburner" )
   var.pop( "tut_fighterbay" )
   var.pop( "tut_buyship" )
   var.pop( "tut_timedil" )
end

return tut
