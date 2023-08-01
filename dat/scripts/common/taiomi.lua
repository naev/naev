local vn = require 'vn'

--[[
-- Helper functions and defines for the Taiomi campaigns
--]]
local taiomi = {
   scavenger = {
      name = _("Scavenger Drone"),
      portrait = nil,
      image = 'gfx/ship/drone/drone_hyena_comm.webp',
      colour = { 0.7, 0.8, 1.0 },
   },
   elder = {
      name = _("Elder Drone"),
      portrait = nil,
      image = 'gfx/ship/drone/drone_comm.webp',
      colour = nil,
   },
   philosopher = {
      name = _("Philosopher Drone"),
      portrait = nil,
      image = 'gfx/ship/drone/drone_comm.webp',
      colour = {1.0, 0.65, 1.0},
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
      taiomi04 = 300e3,
      taiomi05 = 500e3,
      taiomi06 = 400e3,
      taiomi07 = 450e3,
      taiomi08 = 400e3,
      taiomi09 = 500e3,
      --taiomi10 has a complex reward
   },
}

local missions = {
   "Taiomi 1", -- 1
   "Taiomi 2", -- 2
   "Taiomi 3", -- 3
   "Taiomi 4", -- 4
   "Taiomi 5", -- 5
   "Taiomi 6", -- 6
   "Taiomi 7", -- 7
   "Taiomi 8", -- 8
   "Taiomi 9", -- 9
   "Taiomi 10", -- 10
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
function taiomi.vn_elder( params )
   local name = taiomi.elder.name
   if not var.peek( "taiomi_drone_elder" ) then
      name = _("Worn-out Drone")
   end
   return vn.Character.new( name,
         tmerge( {
            image=taiomi.elder.image,
            color=taiomi.elder.colour,
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

local function choose_young ()
   if not var.peek( "taiomi_died" ) then
      var.push( "taiomi_died", rnd.rnd(1,2) )
   end
end

function taiomi.young_died ()
   choose_young()
   local died = var.peek( "taiomi_died" )
   if died == 1 then
      return taiomi.younga.name
   else
      return taiomi.youngb.name
   end
end

function taiomi.young_alive ()
   choose_young()
   local died = var.peek( "taiomi_died" )
   if died == 2 then
      return taiomi.younga.name
   else
      return taiomi.youngb.name
   end
end

function taiomi.laboratory ()
   local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   if fct == "Soromid" then
      return spob.getS( "Soromid Databank" )
   end
   return spob.getS( "Jaan" )
end

function taiomi.scavenger_escort ()
   for k,p in ipairs(player.pilot():followers()) do
      if p:shipvarPeek("taiomi_scavenger") then
         return p
      end
   end
   return nil
end

return taiomi
