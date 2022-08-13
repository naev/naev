--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pers">
 <location>load</location>
 <chance>100</chance>
 <priority>11</priority>
</event>
--]]
local lf = require "love.filesystem"

-- luacheck: globals enter timer (Hook funtions passed by name)

-- Parse directory to add personas
local pers_func_list = {}
for k,v in ipairs(lf.enumerate("events/pers")) do
   local requirename = "events.pers."..string.gsub(v,".lua","")
   local pfunc = require( requirename )
   table.insert( pers_func_list, pfunc )
end

function create ()
   hook.enter("enter")
end

local htimer, pers_list, wtotal
function enter ()
   if htimer then
      hook.rm( htimer )
   end

   -- Must not be exclusively claimed
   if not naev.claimTest( system.cur(), true ) then
      return
   end

   -- See what pers we have
   wtotal = 0
   pers_list = {}
   for k,v in ipairs(pers_func_list) do
      local plist = v()
      if plist then
         for i,p in ipairs(plist) do
            -- Set defaults if necessary
            p.w = p.w or 1
            table.insert( pers_list, p )
            wtotal = wtotal + p.w
         end
      end
   end

   -- Nothing to do here
   if #pers_list <= 0 then
      return
   end

   -- Sort by weight (larger first!)
   table.sort( pers_list, function( a, b )
      return a.w > b.w
   end )

   -- Time start timer
   htimer = hook.timer( rnd.rnd()*30, "timer" )
end

local function spawn_pers ()
   local r = rnd.rnd() * wtotal
   local w = 0
   for k,v in ipairs(pers_list) do
      w = w + v.w
      if r < w then
         v.spawn()
         table.remove( pers_list, k ) -- only spawn once per system
         break
      end
   end
end

function timer ()
   -- Value gives a 50% chance of meeting one every 10 minutes of game time
   -- (more often with time compression)
   if rnd.rnd() < 0.034064 then
      spawn_pers()
   end
   htimer = hook.timer( 30, "timer" )
end
