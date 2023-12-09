--[[
-- Helper stuff for the Crimson Gauntlet
--]]
local vn = require 'vn'
local fmt = require "format"

local gauntletsys = system.get("Crimson Gauntlet")

local totoran = {
   guide = {
      name = _("Crimson Gauntlet Guide"),
      portrait = "dvaered_thug1.png",
      image = "dvaered_thug1.png",
      desc = _("Information on the Crimson Gauntlet."),
      colour = {0.9, 0.1, 0.25},
   }
}

-- Helpers to create main characters
function totoran.vn_guide( params )
   return vn.Character.new( totoran.guide.name,
         tmerge( {
            image=totoran.guide.image,
            colour=totoran.guide.colour,
         }, params) )
end


-- Emblem stuff
local emblems = N_("Totoran Emblem")
function totoran.emblems_get()
   return player.inventoryOwned( emblems )
end
function totoran.emblems_get_gained()
   return var.peek( "totoran_emblems_gained" ) or 0
end
function totoran.emblems_pay( amount )
   player.inventoryAdd( emblems, amount )
   -- Store lifetime earnings
   if amount > 0 then
      local v = var.peek( "totoran_emblems_gained" ) or 0
      var.push( "totoran_emblems_gained", v+amount )
   end
end
function totoran.emblems_str( amount )
   return "#r"..gettext.ngettext(
      "%s Crimson Emblems",
      "%s Crimson Emblems", amount ):format(
         fmt.number(amount) ).."#0"
end

function totoran.clear_pilots ()
   local pp = player.pilot()
   pilot.clear()
   for k,p in ipairs(pp:followers()) do
      if p:flags("carried") then
         p:rm()
      else
         p:setHide( true ) -- Don't remove or it'll mess cargo
      end
   end
end

function totoran.enter_the_ring ()
   -- Teleport the player to the Crimson Gauntlet and hide the rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   gauntletsys:setHidden(false)

   -- Set up player stuff
   player.pilot():setPos( vec2.new( 0, 0 ) )
   -- Disable escorts if they exist
   var.push("hired_escorts_disabled",true)
   player.teleport( gauntletsys )
   var.pop("hired_escorts_disabled")

   -- Clean up pilots
   totoran.clear_pilots()
end

function totoran.leave_the_ring ()
   local pp = player.pilot()
   -- Clear pilots so escorts get docked
   totoran.clear_pilots()
   -- Fix the map up
   gauntletsys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   -- Undo player invincibility stuff and land
   pp:setHide( true ) -- clear hidden flag
   pp:setInvincible( false )
   pp:setInvisible( false )
   player.cinematics( false )
   -- Restore the escorts
   for k,p in ipairs(pp:followers()) do
      p:setHide( false ) -- Don't remove or it'll mess cargo
   end
end

return totoran
