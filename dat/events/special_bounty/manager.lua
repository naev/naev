--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Special Bounty Manager">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[

   Manager to spawn Special Hand-Crafted Bounties

--]]
--local fmt = require "format"
--local bounty = require "common.bounty"
local lf = require "love.filesystem"
local misn_test = require "misn_test"

local bounty_list
function create ()
   bounty_list = {}
   for k,v in ipairs(lf.getDirectoryItems("events/special_bounty")) do
      table.insert( bounty_list, require( "events.special_bounty."..string.gsub(v,".lua","") ) )
   end

   hook.land("land")
end

local function good_candidate( b )
   -- Not already done
   if not var.peek( b.var ) then
      return false
   end

   -- TODO not too far away and claimable

   -- Conditional met if exists
   if b.cond and not b.cond() then
      return false
   end

   return true

end

function land ()
   -- Some checks like normal bounties
   local spb = spob.cur()
   if not spb:services().missions then
      return
   end
   local fct = spb:faction()
   if not fct or not fct:tags().generic or not misn_test.mercenary() then
      return
   end

   -- No active special bounty
   if player.misnActive("Special Bounty") then
      return
   end

   -- Find candidates
   local candidates = {}
   for k,v in ipairs(bounty_list) do
      if good_candidate(v) then
         table.insert( candidates, v )
      end
   end
   if #candidates <= 0 then
      return
   end

   -- Initialize stuff as necessary
   local bounty = tcopy( candidates[ rnd.rnd(#candidates) ] )

   -- Trigger bounty
   local nc = naev.cache()
   nc._special_bounty = bounty
   naev.missionStart("Special Bounty")
   nc._special_bounty = nil
end
