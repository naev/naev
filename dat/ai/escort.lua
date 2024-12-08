require 'ai.core.core'
require 'ai.core.control.escort'
local ai_setup = require "ai.core.setup"

--[[
   AI for ships that escort the player. Hardcoded into escort.c and
   player_fleet.c.
--]]

-- Do not distress or board
mem.distress   = false
mem.atk_board  = false
-- Some defaults that should get overwritten
mem.aggressive = true
mem.enemyclose = 3e3
mem.leadermaxdist = 8e3
mem.atk_kill   = false

local function test_autonav ()
   -- Don't try to engage in combat. TODO check option
   local autonav, speed = player.autonav()
   if autonav and speed > 1. then
      return false
   end
   return true
end

local should_attack_orig = should_attack
function should_attack( ... )
   if not test_autonav() then return end
   return should_attack_orig( ... )
end

local attacked_orig = attacked
function attacked( ... )
   if not test_autonav() then return end
   return attacked_orig( ... )
end

local create_orig = create
function create ()
   create_orig()
   ai.setcredits( 0 )
   -- So, most escorts should be already equipped from player_fleet.c, so we
   -- don't run equipopt again, and instead want to set up their equipment and
   -- such
   ai_setup.setup( ai.pilot() )
end
