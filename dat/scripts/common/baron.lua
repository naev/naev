--[[

   Baron Common Functions

--]]
local vn = require "vn"
local utf8 = require "utf8"
local strmess = require "strmess"

local baron = {}

-- Function for adding log entries for miscellaneous one-off missions.
function baron.addLog( text )
   shiplog.create( "baron", _("Baron"), _("Baron") )
   shiplog.append( "baron", text )
end

function baron.vn_baron( params )
   return vn.Character.new( _("Baron Sauterfeldt"),
         tmerge( {
            portrait="neutral/unique/baron_sauterfeldt.webp",
            image="neutral/unique/baron_sauterfeldt.webp",
         }, params) )
end

baron.rewards = {
   baron = 300e3,
   prince = 500e3, -- The price of each artefact will always be 15% of this, so at most the player will be paid 85% and at least 55%.
}

-- Function that tries to misspell whatever string is passed to it.
function baron.mangle( str )
   return strmess.mangle( str, math.ceil(utf8.len(str)/6) )
end

return baron
