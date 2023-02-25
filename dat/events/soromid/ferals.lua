--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Feral Bioships">
 <location>enter</location>
 <chance>100</chance>
 <cond>system.cur():tags().haze==true and #system.cur():presences() &lt;= 0</cond>
</event>
--]]
local ferals = require "common.ferals"
local luaspfx = require "luaspfx"
local fleet = require "fleet"
local pilotai = require "pilotai"

--luacheck: globals leave pheromones spawn_ferals delay_sfx ferals_discovered heartbeat (Hook functions passed by name)

local targetsys = system.get( "Fertile Crescent" )

function create ()
   local scur = system.cur()

   -- Inclusive claim
   if not evt.claim( scur, true ) then evt.finish() end

   -- Special case final destination
   if scur == targetsys then
      for i = 1,rnd.rnd(20,30) do
         local shp
         local r = rnd.rnd()
         if i==1 or r < 0.2 then
            shp = "Kauweke"
         elseif r < 0.5 then
            shp = "Taitamariki"
         else
            shp = "Nohinohi"
         end
         pilot.add( shp, ferals.faction(), vec2.newP( scur:radius() * 0.8 * rnd.rnd(), rnd.angle() ) )
      end

      hook.jumpout("leave")
      hook.land("leave")
      hook.custom("bioship_pheromones", "pheromones")
      return
   end

   local function has_inhabited_spob( sys )
      if #sys:presences() > 0 then
         return true
      end
      for k,p in ipairs(sys:spobs()) do
         local s = p:services()
         if s.land and s.inhabited then
            return true
         end
      end
      return false
   end

   -- Must be uninhabited
   if has_inhabited_spob( scur ) then
      evt.finish(false)
   end

   -- Needs a direct path to targetsys
   for k,j in ipairs( system.cur():jumpPath( targetsys, true ) ) do
      if has_inhabited_spob( j:dest() ) then
         evt.finish(false)
      end
   end

   hook.jumpout("leave")
   hook.land("leave")
   hook.custom("bioship_pheromones", "pheromones")
end

--event ends on player leaving the system or landing
function leave ()
    evt.finish()
end

local function whalesound( pos )
   local sfx
   if rnd.rnd() < 0.5 then
      sfx = ferals.sfx.spacewhale1
   else
      sfx = ferals.sfx.spacewhale2
   end
   luaspfx.sfx( pos, nil, sfx, { dist_ref = 5e3, dist_max = 50e3 } )
end

local plts, nextjump, lastsys
local spawned = false
function pheromones ()
   if not spawned then
      spawned = true
      hook.timer( 5, "spawn_ferals" )
   else
      if plts then
         for k,p in ipairs(plts) do
            if p:exists() then
               hook.timer( 3, "delay_sfx", p:pos() )
               return
            end
         end
      end
   end
end

function delay_sfx( pos )
   whalesound( pos )
end

function spawn_ferals ()
   local jumps = system.cur():jumpPath( targetsys, true )
   nextjump = jumps[1]
   local pos = (nextjump:pos() - player.pos())*0.8 + player.pos()
   pos = pos + vec2.newP( 3000*rnd.rnd(), rnd.angle() )

   lastsys = (#jumps==1)

   mem.mrk = system.markerAdd( pos+vec2.newP( 2000*rnd.rnd(), rnd.angle() ), _("Signal"), 4000 )
   player.msg(_("You have detected an unknown signal!"), true)
   player.autonavReset( 1 )

   whalesound( pos )

   local bioships = {}
   local r = rnd.rnd()
   if r < 0.5 then
      table.insert( bioships, "Taitamariki" )
      for i=1,rnd.rnd(0,3) do
         table.insert( bioships, "Nohinohi" )
      end
   else
      for i=1,rnd.rnd(1,3) do
         table.insert( bioships, "Nohinohi" )
      end
   end
   plts = fleet.add( 1, bioships, ferals.faction(), pos )
   for k,p in ipairs(plts) do
      p:control()
      p:stealth()
      hook.pilot( p, "discovered", "ferals_discovered" )
   end
end

function ferals_discovered ()
   if mem.mrk then
      system.markerRm( mem.mrk )
      mem.mrk = nil
   end
   for k,p in ipairs(plts) do
      if p:exists() then
         p:control(false)
      end
   end
   player.autonavReset( 5 )
   if plts[1]:exists() then
      -- Just try to go to the next system
      pilotai.hyperspace( plts[1], nextjump )
      if lastsys then
         hook.timer( 1, "heartbeat" )
      end
   end
end

function heartbeat ()
   if plts[1]:exists() then
      if plts[1]:flags( "jumpingout" ) and not nextjump:known() then
         player.msg(_("You have discovered a jump point!"),true)
         player.autonavReset( 1 )
         nextjump:setKnown(true)
         return
      end
      hook.timer( 1, "heartbeat" )
   end
end
