--[[

   Equips pilots based on mixed integer linear programming

--]]
local equipopt = {}

equipopt.optimize = require 'equipopt.optimize'
equipopt.params = require 'equipopt.params'
equipopt.cores = require 'equipopt.cores'

-- Custom equip functions
equipopt.zalek = require 'equipopt.templates.zalek'

return equipopt
