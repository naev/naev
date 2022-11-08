--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 10">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 9</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 10

   Defend the hypergate and final cutscene!
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local lmisn = require "lmisn"
local pp_shaders = require "pp_shaders"
local fleet = require "fleet"
local pilotai = require "pilotai"

local title = _("Final Breath of Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")

local DEFENSE_LENGTH = 60*3 -- Length in seconds
local SPAWNLIST_EMPIRE = {
   { p={"Empire Pacifier", "Empire Shark", "Empire Shark"}, t=0 },
   { p={"Empire Admonisher", "Empire Admonisher"}, t=15 },
   { p={"Empire Lancelot", "Empire Lancelot", "Empire Lancelot"}, t=20 },
   { p={"Empire Pacifier"}, t=30 },
   { p={"Empire Shark", "Empire Shark", "Empire Shark" }, t=35 },
   { p={"Empire Hawking"}, t=45 },
   { p={"Empire Lancelot", "Empire Lancelot" }, t=55 },
   { p={"Empire Pacifier"}, t=65 },
   { p={"Empire Peacemaker"}, t=90 },
   { p={"Empire Lancelot", "Empire Lancelot" }, t=100 },
   { p={"Empire Pacifier", "Empire Shark", "Empire Shark"}, t=120 },
   { p={"Empire Admonisher", "Empire Admonisher"}, t=140 },
   { p={"Empire Peacemaker", "Empire Peacemaker", "Empire Rainmaker"}, t=150 },
   { p={"Empire Hawking", "Empire Hawking", "Empire Hawking"}, t=160 },
}

--[[
   0: mission started
   1: landed on goddard
   2: defend
   3: cutscene done
--]]
mem.state = 0
local defense_timer, defense_spawn, defense_fct, defense_spawnlist

function create ()
   if not misn.claim{ basesys } then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )

   misn.setDesc(_("You have been tasked to help the citizens of Taiomi reach their freedom!"))
   misn.setReward(_("Freedom for the Taiomi Citizens!"))

   mem.marker = misn.markerAdd( base )
   misn.osdCreate( title, {
      fmt.f(_("Land on {spob} ({sys})"),{spob=base, sys=basesys})
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

local function osd ()
   misn.osdCreate( title, {
      fmt.f(_("Defend the {base} ({left} s left)"),
         {base=base, left=DEFENSE_LENGTH-defense_timer})
   } )
end

function land ()
   defense_fct = faction.get( var.peek( "taiomi_convoy_fct" ) or "Empire" )

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   s(_([[""]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()
end

local hypergate
function enter ()
   if mem.state ~= 0 then
      lmisn.fail(_([[You were supposed to protect the hypergate!"]]))
   end

   defense_timer = 0
   defense_spawn = 0
   defense_fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   if defense_fct then
      defense_spawnlist = SPAWNLIST_EMPIRE
   end
   osd()
   misn.markerMove( mem.marker, basesys )
   mem.state = 1 -- start

   local pos = base:pos()
   diff.apply("onewing_goddard_gone")
   hypergate = pilot.add( "One-Wing Goddard", "Independent", pos, nil, {naked=true, ai="dummy"} )
   hypergate:disable()
   hypergate:setHilight(true)
   hook.pilot( hypergate, "death", "hypergate_dead" )

   hook.timer( 1, "heartbeat" )
end

function heartbeat ()
   defense_timer = defense_timer + 1

   if defense_timer >= DEFENSE_LENGTH then
      -- TODO win cinematics
      return
   end

   local spawn = defense_spawnlist[ defense_spawn+1 ]
   if spawn and spawn.t and defense_timer > spawn.t then
      for k,p in ipairs( fleet.add( 1, spawn.p, defense_fct ) ) do
         pilotai.guard( p, base:pos() )
         p:setHostile()
      end
      defense_spawn = defense_spawn+1
   end

   hook.timer( 1, "heartbeat" )
end

local shader_explode
function hypergate_dead ()
   -- Activate hypergate with big flash
   local pixelcode = [[
#include "lib/blur.glsl"

const float THRESHOLD = 0.4;
const float INTENSITY = 10.0;

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   /* Fade in. */
   if (u_progress < THRESHOLD)
      return mix( texture( tex, texture_coords ), vec4(1.0), progress / THRESHOLD );

   /* Fade out. */
   float progress = (u_progress-THRESHOLD) / (1.0-THRESHOLD);
   float disp = INTENSITY*(0.5-distance(0.5, progress));
   vec4 c1 = vec4(1.0);
   vec4 c2 = blur9( tex, texture_coords, love_ScreenSize.xy, disp );
   return mix( c1, c2, progress );
}
]]
   shader_explode = { shader=pp_shaders.newShader( pixelcode ) }

   player.cinematics(true)
   camera.set( base:pos() )
   hook.timer( 3, "explode" )
end

local function shader_init( shd, speed )
   shd._dt = 0
   shd._update = function( self, dt )
      self._dt = self._dt + dt * speed
      self.shader:send( "u_progress", math.min( 1, self._dt ) )
   end
   shd.shader:addPPShader("game", 99)
end

function explode ()
   shader_init( shader_explode, 5 )
   hook.timer( 1, "everyone_dead" )
end

function everyone_dead ()
   for k,v in ipairs(pilot.get()) do
      v:kill() -- Everyone dies ٩(๑´3｀๑)۶
   end
end
