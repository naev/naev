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
   younga = {
      name = _("Hugonn"), -- Odin's raven
      portrait = nil,
      image = 'gfx/ship/drone/drone_comm.webp',
      colour = nil,
   },
   youngb = {
      name = _("Muninn"), -- Odin's raven
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
      taiomi01 = 300e3,
      taiomi02 = 300e3,
      taiomi03 = 350e3,
   },
}

local missions = {
   "Taiomi 1", -- 1
   "Taiomi 2", -- 2
   "Taiomi 3", -- 3
}

-- Gets the current progress of the Taiomi campaign
function taiomi.progress ()
   for i = #missions, 1, -1 do
      if player.misnDone( missions[i] ) then
         return i
      end
   end
   return 0
end

-- Checks to see if a mission is in progress
function taiomi.inprogress ()
   for k,v in ipairs(missions) do
      if player.misnActive(v) then
         return true
      end
   end
   return false
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
function taiomi.vn_younga( params )
   return vn.Character.new( taiomi.younga.name,
         tmerge( {
            image=taiomi.younga.image,
            color=taiomi.younga.colour,
         }, params) )
end
function taiomi.vn_youngb( params )
   return vn.Character.new( taiomi.youngb.name,
         tmerge( {
            image=taiomi.youngb.image,
            color=taiomi.youngb.colour,
         }, params) )
end

function taiomi.laboratory ()
   local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   if fct == "Soromid" then
      return spob.getS( "Soromid Databank" )
   end
   return spob.getS( "Zhiru" )
end

return taiomi
