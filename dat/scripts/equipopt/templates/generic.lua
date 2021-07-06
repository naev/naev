local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

--[[
-- @brief Does Generic pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_generic( p, opt_params, cores, outfits )
   opt_params  = opt_params or nil
   cores       = cores or "standard"
   outfits     = outfits or cores

   -- Choose parameters and make Pirateish
   local params = eparams.choose( p )
   params = mt.merge_tables( params, opt_params )

   -- Get stuff
   local ocores = ecores.get( p, { all=cores } )
   local ooutfits = eoutfits[ outfits ].set

   -- Try to equip
   return optimize.optimize( p, ocores, ooutfits, params )
end

return equip_generic
