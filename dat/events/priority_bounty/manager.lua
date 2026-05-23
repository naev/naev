--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Priority Bounty Manager">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[

   Manager to spawn Priority Hand-Crafted Bounties

--]]
--local fmt = require "format"
--local bounty = require "common.bounty"
local lf = require "love.filesystem"
local misn_test = require "misn_test"

function create ()
   mem.bounty_list = {}
   for k,v in ipairs(lf.getDirectoryItems("events/priority_bounty/bounties")) do
      local filename = "events.priority_bounty.bounties."..string.gsub(v,".lua","")
      local b     = require( filename )
      b.filename  = filename
      b.var       = b.var or v
      table.insert( mem.bounty_list, b )
   end

   hook.land("land")
   if player.isLanded() then
      hook.safe("land")
   end
end

local function good_candidate( b )
   -- Not already done
   if var.peek( b.var ) then
      return false
   end

   if not naev.claimTest( {b.system}, true ) then
      return false
   end

   if b.system:jumpDist( system.cur() ) > 9 then
      return false
   end

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

   -- No active priority bounty
   if player.misnActive("Priority Bounty") then
      return
   end

   hook.safe( "try_gen" )
end

function try_gen()
   -- Find candidates
   local candidates = {}
   for k,v in ipairs(mem.bounty_list) do
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
   nc._priority_bounty = bounty
   naev.missionStart("Priority Bounty")
   nc._priority_bounty = nil
end
