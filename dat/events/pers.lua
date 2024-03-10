--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pers">
 <location>load</location>
 <chance>100</chance>
 <priority>11</priority>
</event>
--]]
local lf = require "love.filesystem"


-- Parse directory to add personas
local pers_func_list = {}
for k,v in ipairs(lf.getDirectoryItems("events/pers")) do
   local requirename = "events.pers."..string.gsub(v,".lua","")
   local pfunc = require( requirename )
   table.insert( pers_func_list, pfunc )
end

function create ()
   hook.enter("enter")
end

local htimer, pers_list, wtotal, spawned, spawn_chance
function enter ()
   if htimer then
      hook.rm( htimer )
   end

   -- Must not be exclusively claimed
   local scur = system.cur()
   if not naev.claimTest( scur, true ) then
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

   -- Value gives a 50% chance of meeting one every 10 minutes of game time
   -- (more often with time compression)
   spawn_chance =  0.034064
   if scur == system.get("Zied") then
      spawn_chance = 0.3
   end

   -- Time start timer
   spawned = {} -- Initialize spawned list
   htimer = hook.timer( rnd.rnd()*30, "timer" )
end

function pers_attacked( _p, attacker, dmg, pt )
   if attacker:withPlayer() then
      pt.dmgp = pt.dmgp + dmg
      if pt.onattack then
         pt.onattack( attacker, dmg )
      end
   else
      pt.dmgo = pt.dmgo + dmg
   end
end

function pers_death( _p, attacker, pt )
   if pt.dmgp > pt.dmgo or (attacker and attacker:withPlayer()) then
      if pt.ondeath then
         pt.ondeath( attacker, pt )
      end
   end
   if pt.ondeathany then
      pt.ondeathany( attacker, pt )
   end
end

local function spawn_pers ()
   if not pilot.canSpawn() then
      return
   end

   local r = rnd.rnd() * wtotal
   local w = 0
   for k,v in ipairs(pers_list) do
      w = w + v.w
      if r < w then
         local p = v.spawn()
         if type(p) ~= "table" then
            p = {p}
         end

         -- Set hooks and such as necessary
         for i,pp in ipairs(p) do
            local pt = {
               p = pp,
               dmgp = 0,
               dmgo = 0,
               onattack = pp.onattack,
               ondeath = pp.ondeath,
               ondeathany = pp.ondeathany,
            }
            hook.pilot( pp, "attacked", "pers_attacked", pt )
            hook.pilot( pp, "death", "pers_death", pt )
            table.insert( spawned, pt )
         end

         table.remove( pers_list, k ) -- only spawn once per system
         wtotal = wtotal - v.w
         return
      end
   end
end

function timer ()
   -- Spawn more in Zied
   if rnd.rnd() < spawn_chance then
      spawn_pers()
   end
   htimer = hook.timer( 30, "timer" )
end
