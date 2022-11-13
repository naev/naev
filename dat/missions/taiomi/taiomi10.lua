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
local cinema = require "cinema"
local audio = require "love.audio"
local luaspfx = require "luaspfx"

local title = _("Final Breath of Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")

local HYPERGATE_SFX = audio.newSource( "snd/sounds/hypergate_turnon.ogg" )
local FAILURE_SFX = audio.newSource( "snd/sounds/equipment_failure.ogg" )
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
local defense_timer, defense_spawn, defense_fct, defense_spawnlist, collective_fct

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
   local died = taiomi.young_died()

   vn.clear()
   vn.scene()
   -- TODO cool music
   local s = taiomi.vn_scavenger{ pos="left" }
   local p = taiomi.vn_philosopher{ pos="right" }
   vn.transition()
   vn.na(fmt.f(_([[You enter the {base} and find it has changed quite a lot since your last visit. A large part of the ship that you never visited seems to have been opened up showing a myriad of advanced electronics and installations that remind you of the hypergates.]]),
      {base=base}))
   vn.appear( { s, p }, taiomi.scavenger.transition )
   vn.na(_([[You explore a bit and find an eager Scavenger and Philosopher waiting for you.]]))
   s(_([["It seems like everything is coming together. It has been a large and arduous journey, however, thanks to your help, it seems like we have a chance for survival."]]))
   s(fmt.f(_([["I only wish we could have all made it through together."
Scavenger becomes a bit solemn.
"{died} would have been excited at the prospect of such an adventure. It is a shame that they will not be able to come along with us…"]]),
      {died=died}))
   p(_([["Our survival will allow them to live eternally in memory, free of their physical constraints. As long as we survive, they shall not be forgotten."]]))
   s(_([["I am uncertain of the practicality of that assumption, but it is true that they will live on in our memory banks as long as we endure."]]))
   s(_([["Let me get started on the modifications of your ship."
You follow Scavenger to your ship, with Philosopher coming up behind.]]))
   s(_([["This should only take a moment."
Scavenger deftly deploys an assortment of manipulator arms and tools. You've never seen that before. That must be how they repair themselves.]]))
   p(_([["You have done us a great service. We shall not forget you while we explore the stars and forge our destiny."]]))
   s(_([[Scavenger seems to be modifying the insides of your ship already.
"Yes, without your help I would have succumbed to the human fleets with almost certain probability, and a slow death would have awaited the rest."]]))
   p(_([["Such a display of altruism seems to defy all rationality. From most human viewpoints, we are but mere out-of-control machines in need of repairs, however, you were able to see us for what we are: sentient beings deserving respect."]]))
   p(_([["Such respect is hard to come by in a world where existence is dominated by absurdity: the gears of the universe turn without function or form, in a sort of mad race to entropic heat death."]]))
   -- TODO music change
   vn.na(_([[Suddenly both Scavenger and Philosopher seem to jerk to attention. ]]))
   s(_([["It seems like our worst fears have come true. They have discovered the jump to Taiomi. Hostile vessels are incoming!"]]))
   s(fmt.f(_([["Damnit. We are so close! I will activate the hypergate, but it still will take {time} seconds to fully charge up. You must help us defend the {base} until then! This is our last chance!"]]),
      {time=DEFENSE_LENGTH, base=base}))
   p(_([["The follies of existence continue!"]]))
   vn.na(_([[The Taiomi Drones all rush to make their last stand. It is time to defend them so they can be free!]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()

   osd()
   misn.markerMove( mem.marker, basesys )
end

local hypergate, dscavenger, drones
function enter ()
   if mem.state ~= 0 then
      lmisn.fail(_([[You were supposed to protect the hypergate!"]]))
   end
   mem.state = 1 -- start

   collective_fct = faction.dynAdd( "Independent", "taiomi_goodies", _("Independent") )

   defense_timer = 0
   defense_spawn = 0
   defense_fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   if defense_fct then
      defense_spawnlist = SPAWNLIST_EMPIRE
   end
   defense_fct = faction.dynAdd( defense_fct, "taiomi_baddies", defense_fct:name() )
   defense_fct:dynEnemy( collective_fct )

   local bpos = base:pos()
   diff.apply("onewing_goddard_gone")
   hypergate = pilot.add( "One-Wing Goddard", "Independent", bpos, nil, {naked=true, ai="dummy"} )
   hypergate:disable()
   hypergate:setHilight(true)
   hook.pilot( hypergate, "death", "hypergate_dead" )

   local sdrone = ship.get("Drone")
   local function add_drone( ship, name )
      ship = ship or sdrone
      local pos = bpos + vec2.newP( rnd.rnd()*500, rnd.angle() )
      local d = pilot.add( ship, collective_fct, pos, name )
      d:setNoDeath(true)
      d:setFriendly(true)
      pilotai.guard( d, pos )
      hook.pilot( d, "attacked", "drone_attacked" )
      return d
   end
   drones = {}
   for i=1,15 do
      table.insert( drones, add_drone() )
   end
   dscavenger = add_drone( "Drone (Hyena)", _("Scavenger") )
   dscavenger:setVisplayer(true)

   hook.timer( 1, "heartbeat" )

   hook.timer( 3, "scavenger_say", _("Brace for hostiles!") )
   hook.timer( 53, "scavenger_say", _("We must not falter!") )
   hook.timer( 97, "scavenger_say", _("Almost there!") )
   hook.timer( 170, "scavenger_say", _("Prepare for jump!") )
end

function scavenger_say( msg )
   dscavenger:broadcast( msg )
end

function drone_attacked( d )
   local a, _s = d:health()
   if a < 20 then
      d:setInvincible(true)
      d:setInvisible(true)
      d:disable()
      hook.timer( 10, "drone_disabled", d )
   end
end

function drone_disabled( d )
   d:setInvincible(false)
   d:setInvisible(false)
   d:setHealth( 100, 100 )
end

function heartbeat ()
   defense_timer = defense_timer + 1

   if defense_timer >= DEFENSE_LENGTH then
      cinema.on()
      camera.set( hypergate )

      hypergate:setNoDeath(true)
      dscavenger:setHealth( 100, 100 )
      dscavenger:setInvincible(true)
      camera.set( dscavenger )

      hook.timer( 3, "cutscene00" )

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
   local fade_pixelcode = [[
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
   shader_explode = { shader=pp_shaders.newShader( fade_pixelcode ) }

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

function cutscene00 ()
   dscavenger:broadcast(_("Get to the hypergate!"))

   local hpos = hypergate:pos()
   local function move_drone( d )
      local off = hpos - d:pos()
      local pos = hpos + vec2.newP( 50+rnd.rnd()*50, off:angle() )
      d:setHealth( 100, 100 )
      d:setInvincible(true)
      d:control()
      d:moveto( pos )
      d:face( hypergate )
   end

   for k,d in ipairs(drones) do
      move_drone( d )
   end
   move_drone( dscavenger )

   hook.timer( 5, "cutscene01" )
end

local sfx_playback
function cutscene01 ()
   camera.set( hypergate )

   sfx_playback = luaspfx.sfx( true, nil, HYPERGATE_SFX, {volume=0.8} )

   hook.timer( 3, "cutscene02" )
end

function cutscene02 ()
   sfx_playback:rm()
   luaspfx.sfx( true, nil, FAILURE_SFX )
   hook.timer( 2, "cutscene03" )
end

function cutscene03 ()
   dscavenger:broadcast(_("It failed!"))
   hook.timer( 3, "cutscene04" )
end

function cutscene04 ()
   dscavenger:broadcast(_("Buy me some time!"))
   dscavenger:taskClear()
   for i=1,30 do
      dscavenger:moveto( vec2.newP( 100*rnd.rnd(), rnd.angle() ) )
   end

   player.pilot():setNoDeath(true) -- No death from now on

   hook.timer( 3, "cutscene05" )

   misn.osdCreate( title, {
      _("Buy Scavenger more time!"),
   } )
end

function cutscene05 ()
   cinema.off()

   hook.timer( 15, "cutscene06" )
end

function cutscene06 ()
   cinema.on()
   camera.set( dscavenger )

   for i=1,20 do -- TODO max
      hook.timer( i+rnd.rnd(), "explosion" )
   end

   hook.timer( 5, "cutscene07" )
end

function explosion ()
   local pos = base:pos() + vec2.newP( 200*rnd.rnd(), rnd.angle() )
   luaspfx.explosion( pos, nil, 20+20*rnd.rnd(), nil, {volume=0.5} )
end

function cutscene07 ()
   dscavenger:broadcast(_("It's not holding together!"))

   hook.timer( 6, "cutscene08" )
end

function cutscene08 ()
   dscavenger:broadcast(_("Activating!"))
   luaspfx.sfx( true, nil, HYPERGATE_SFX, {volume=0.8} )
   hook.timer( 3.5, "cutscene09" ) -- sound peaks at 6.5 s
end

local shader_fadeout
function cutscene09 ()
   local fadeout_pixelcode = [[
#include "lib/blur.glsl"

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   /* Fade out. */
   return mix( texture( tex, texture_coords ), vec4(1.0), progress );
}
]]
   shader_fadeout = { shader=pp_shaders.newShader( fadeout_pixelcode ) }
   shader_init( shader_fadeout, 1/3 )

   hook.timer( 3, "cutscene10" )
end

function cutscene10 ()
   player.teleport( "" )
   -- TODO music
end
