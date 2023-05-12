--[[--
Functions for handling cinematic scenes.

@module cinema
--]]
local cinema = {}

local old = {}

--[[--
Enables cinematic mode.

   @param params Parameters.
--]]
function cinema.on( params )
   if cinema._on then return end

   local pp = player.pilot()
   local plts = pp:followers()
   table.insert( plts, pp )

   for k,p in ipairs(plts) do
      old[ p ] = {
         invincible = p:flags("invincible"),
         control = p:flags("manualcontrol"),
      }

      p:control(true)
      p:brake()
      p:setInvincible(true)
   end

   player.cinematics( true, params )

   cinema._on = true
end

--[[--
Disables cinematic mode.
--]]
function cinema.off ()
   if not cinema._on then return end

   for p,o in pairs(old) do
      p:control( o.control )
      p:setInvincible( o.invincible )
   end

   camera.set()
   player.cinematics( false )

   old = {}
   cinema._on = false
end

--[[--
Allows for toggling parameters.
--]]
function cinema.reset( params )
   if not cinema._on then return end

   player.cinematics( true, params )
end

return cinema
