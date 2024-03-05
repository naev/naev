--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 02">
 <unique />
 <priority>0</priority>
 <chance>0</chance>
 <location>None</location>
 <notes>
  <done_evt>Onion Society 02 Trigger</done_evt>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion02
--]]
--[[
local fmt = require "format"
local vntk = require "vntk"
local strmess = require "strmess"
local pp_shaders = require 'pp_shaders'
local lg = require "love.graphics"
local onion = require "common.onion"
--]]

--local destpnt, destsys = spob.getS("Gordon's Exchange")
--local money_reward = onion.rewards.misn02

-- Create the mission
function create()
   misn.finish()
end
