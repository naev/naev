--[[
   Helpers for dealing with derelict or abandoned ships in general.
--]]
local derelict = {}

derelict.sfx = setmetatable( {}, {
   __index = function( self, key )
      derelict.sfx = {
         board = audio.newSource( "snd/sounds/spaceship_door_open.ogg" ),
         unboard = audio.newSource( "snd/sounds/spaceship_door_close.ogg" ),
         ambient = audio.newSource( "snd/sounds/loops/derelict.ogg" )
      }
      return derelict.sfx[key]
   end
} )

return derelict
