--[[
AI for pers unique pilots.

Meant to be pretty flexible. You probably want to define the following:
* mem.ad -- message spammed across the system
* mem.comm_greet -- greeting message when hailed
* mem.taunt -- message to spam when engaging the enemy
--]]
require 'ai.core.core'
require 'ai.core.idle.advertiser'
require 'ai.common.pers'
