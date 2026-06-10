local pir = require 'equipopt.templates.pirate_base'
local optimize = require 'equipopt.optimize'
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

local function choose_one( t ) return t[ rnd.rnd(#t) ] end

local function hulls_nexus_cargo( _secondary, size )
   if size=="Large" then
      return choose_one{ "Nexus Phantasm Weave", "S&K Large Cargo Hull" }
   elseif size=="Medium" then
      return choose_one{ "Nexus Ghost Weave", "S&K Medium Cargo Hull" }
   else
      return choose_one{ "Nexus Shadow Weave", "S&K Small Cargo Hull" }
   end
end

--[[
-- @brief Does Generic pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_free_trader( p, opt_params, cores, outfits )
   opt_params  = opt_params or {}
   cores       = cores or "standard"
   outfits     = outfits or cores

   -- Choose parameters and make Pirateish
   local params = eparams.choose( p )
   params = tmerge_r( params, opt_params )

   -- Outfits (based on pirate)
   local ooutfits = pir.outfits
   if opt_params.outfits_add then
      ooutfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- Get cores, but overwrite hull to use either shadow or cargo
   local ocores = ecores.get( p, { all=cores } )
   if ocores.hull then
      ocores.hull = hulls_nexus_cargo
   end
   if ocores.hull_secondary then
      ocores.hull_secondary = hulls_nexus_cargo
   end

   -- Set some pilot meta-data
   local mem = p:memory()
   mem.equip = { type="generic", level=cores } -- TODO should we use cores or outfits as a reference? :/

   -- Try to equip
   return optimize.optimize( p, ocores, ooutfits, params )
end

return equip_free_trader
