--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Skirmish Benchmark">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
--[[
   Benchmarks some skirmish to see how efficient Naev can handle the chaos
--]]
local fmt = require "format"

local DT = 10

function create ()
   player.teleport("Adraia", true) -- System with no asteroids
   pilot.clear()
   pilot.toggleSpawn(false)
   player.pilot():setInvincible(true)
   local function add_pilot( ship, faction )
      local pos = vec2.newP( system.cur():radius()*0.9*math.sqrt(rnd.rnd()), rnd.angle() )
      local p = pilot.add( ship, faction, pos )
      p:setVisplayer(true)
   end
   for i = 1,200 do
      add_pilot( "Empire Lancelot", "Empire" )
   end
   for i = 1,300 do
      add_pilot( "Drone", "Collective" )
   end

   hook.timer( 0, "start" )
   hook.update( "update" )
   hook.timer( DT, "average" )
   hook.enter( "enter" )
end

local start_time
function start ()
   start_time = naev.ticks()
end

function enter ()
   evt.finish()
end

local dt_list = {}
function update ()
   table.insert( dt_list, naev.fps() )
end

function average ()
   local avg = 0
   local wrst = math.huge
   for k,dt in ipairs(dt_list) do
      avg = avg + dt
      if dt < wrst then
         wrst = dt
      end
   end
   local data = {DT=DT,avg=avg/#dt_list,wrst=wrst,elapsed=naev.ticks()-start_time}
   print(fmt.f([[
Real time to do {DT} seconds: {elapsed} s
Average FPS over {DT} seconds: {avg} s
Worst FPS over {DT} seconds: {wrst} s]],
   data ))

   naev.trigger("benchmark", data)
end
