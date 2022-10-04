local optimize = require 'equipopt.optimize'
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

--[[
-- @brief Does Generic pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_generic( p, opt_params, cores, outfits )
   opt_params  = opt_params or {}
   cores       = cores or "standard"
   outfits     = outfits or cores

   -- Choose parameters and make Pirateish
   local params = eparams.choose( p )
   params = tmerge( params, opt_params )

   -- Get stuff
   local ocores = ecores.get( p, { all=cores } )
   local ooutfits = eoutfits[ outfits ].set
   if opt_params.outfits_add then
      ooutfits = eoutfits.merge{ ooutfits, opt_params.outfits_add }
   end

   -- Set some pilot meta-data
   local mem = p:memory()
   mem.equip = { type="generic", level=cores } -- TODO should we use cores or outfits as a reference? :/

   -- Try to equip
   return optimize.optimize( p, ocores, ooutfits, params )
end

return equip_generic
