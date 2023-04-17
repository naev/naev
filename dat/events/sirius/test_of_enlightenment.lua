--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Enlightenment">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Enlightenment</system>
</event>
--]]

local sys, sysr
local prevship
local markers

local function marker_set( n, state )
   local m = markers[n]
   m.p:effectClear()
   if state then
      m.p:effectAdd("Psychic Orb On")
   else
      m.p:effectAdd("Psychic Orb Off")
   end
   m.on = state
   m.t = naev.ticksGame()
end

local function marker_toggle( n )
   marker_set( n, not markers[n].on )
end

function create ()
   sys = system.cur()
   sysr = sys:radius()

   -- Hide rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)

   -- Stop and play different music
   music.stop()

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Lesser", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new() )
   pp:intrinsicSet( {
      thrust_mod     = -50,
      speed_mod      = -50,
      turn_mod       = -50,
   }, true ) -- overwrite all

   -- First puzzle
   markers = {}
   local n = 5
   for i=1,n do
      local dir = math.pi*0.5 + (i-1)/n*math.pi*2.0
      local pos = vec2.newP( 200, dir )
      local m = pilot.add("Psychic Orb", "Independent", pos, nil, {ai="dummy"} )
      m:effectAdd("Psychic Orb On")
      m:setNoDeath(true)
      m:setHostile(true)
      hook.pilot( m, "attacked", "puzzle01" )
      markers[i] = { p=m, on=true, t=naev.ticksGame() }
   end
   marker_set( 2, false )
   marker_set( 5, false )

   hook.update( "update" )

   -- Anything will finish the event
   hook.enter( "done" )
end

-- Forces the player (and other ships) to stay in the radius of the system
function update ()
   for k,p in ipairs(pilot.get()) do
      local pos = p:pos()
      local d = pos:dist()
      if d > sysr then
         local _m, dir = pos:polar()
         p:setPos( vec2.newP( sysr, dir ) )
      end
   end
end

function done ()
   -- Restore previous ship
   player.shipSwap( prevship, true, true )

   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   evt.finish()
end

function puzzle01( p )
   local n = 0
   for i,m in ipairs(markers) do
      if m.p==p then
         n = i
         break
      end
   end
   assert( n~=0 )
   local mm = markers[n]
   mm.p:setHealth( 100, 100 )
   mm.p:setVel( vec2.new() )

   -- Only switch every 1.5 seconds
   local t = naev.ticksGame()
   if t-mm.t > 0.5 then
      marker_toggle( n )
      marker_toggle( math.fmod( n,   5 )+1 ) -- One up
      marker_toggle( math.fmod( n+3, 5 )+1 ) -- One below
   end

   -- Check if done
   local allon = true
   for i,m in ipairs(markers) do
      if not m.on then
         allon = false
         break
      end
   end
   if allon then
      -- TODO
      player.msg( "You win!", true )
      for i,m in ipairs(markers) do
         m.m:rm()
      end
      markers = nil
   end
end
