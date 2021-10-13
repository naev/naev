--[[

   Equips pilots based on mixed integer linear programming

--]]
local function equipopt_load ()
   local equipopt = {}

   equipopt.optimize = require 'equipopt.optimize'
   equipopt.params   = require 'equipopt.params'
   equipopt.cores    = require 'equipopt.cores'

   -- Main equipment functions
   equipopt.generic  = require 'equipopt.templates.generic'
   equipopt.empire   = require 'equipopt.templates.empire'
   equipopt.zalek    = require 'equipopt.templates.zalek'
   equipopt.dvaered  = require 'equipopt.templates.dvaered'
   equipopt.sirius   = require 'equipopt.templates.sirius'
   equipopt.soromid  = require 'equipopt.templates.soromid'
   equipopt.pirate   = require 'equipopt.templates.pirate'
   equipopt.thurion  = require 'equipopt.templates.thurion'
   equipopt.proteron = require 'equipopt.templates.proteron'

   return equipopt
end

local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not equipopt then
         self.__index = equipopt_load ()
      end
      return self.__index[key]
   end
} )
return lazy

