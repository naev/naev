local vn = require 'vn'

--[[
-- Helper functions and defines for the Taiomi campaigns
--]]
local taiomi = {
   scavenger = {
      name = _("Scavenger Drone"),
      portrait = nil,
      image = 'gfx/ship/drone/drone_hyena_comm.webp',
      colour = nil,
   },
   wornout = {
      name = _("Worn-out Drone"),
      portrait = nil,
      image = 'gfx/ship/drone/drone_comm.webp',
      colour = nil,
   },
   philosopher = {
      name = _("Philosopher Drone"),
      portrait = nil,
      image = 'gfx/ship/drone/drone_comm.webp',
      colour = nil,
   },
   log = {
      main = function( text )
         shiplog.create( "log_taiomi_main", _("Taiomi"), _("Taiomi") )
         shiplog.append( "log_taiomi_main", text )
      end,
   },
   rewards = {
      taiomi01 = 100e3,
   },
}

-- Gets the current progress of the Taiomi campaign
function taiomi.progress ()
   local missions = {
      "Taiomi 1", -- 1
   }
   for i = #missions, 1, -1 do
      if player.misndone( missions[i] ) then
         return i
      end
   end
   return 0
end

-- Helpers to create main characters
function taiomi.vn_scavenger( params )
   return vn.Character.new( taiomi.scavenger.name,
         tmerge( {
            image=taiomi.scavenger.image,
            color=taiomi.scavenger.colour,
         }, params) )
end
function taiomi.vn_wornout( params )
   return vn.Character.new( taiomi.wornout.name,
         tmerge( {
            image=taiomi.wornout.image,
            color=taiomi.wornout.colour,
         }, params) )
end
function taiomi.vn_philosopher( params )
   return vn.Character.new( taiomi.philosopher.name,
         tmerge( {
            image=taiomi.philosopher.image,
            color=taiomi.philosopher.colour,
         }, params) )
end

return taiomi
