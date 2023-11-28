--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 10">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 9</done>
 <tags>
  <tag>fleetcap_10</tag>
 </tags>
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
local vne = require "vnextras"
local fmt = require "format"
local taiomi = require "common.taiomi"
local lmisn = require "lmisn"
local pp_shaders = require "pp_shaders"
local fleet = require "fleet"
local pilotai = require "pilotai"
local cinema = require "cinema"
local audio = require "love.audio"
local luaspfx = require "luaspfx"
local tut = require "common.tutorial"

local title = _("Final Breath of Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")
-- TODO switch to some other remote system (maybe discover a hidden jump) when nebula gets reworked
--local endsys = system.get("Toros")
local endsys = system.get("Toaxis")

local HYPERGATE_SFX = audio.newSource( "snd/sounds/hypergate_turnon.ogg" )
local FAILURE_SFX = audio.newSource( "snd/sounds/equipment_failure.ogg" )
local ELECTRIC_SFX = audio.newSource( "snd/sounds/electric_zap.ogg" )
local DEFENSE_LENGTH = 60*3 -- Length in seconds

--[[
   0: mission started
   1: landed on goddard
   2: defend
   3: cutscene done
--]]
mem.state = 0
local defense_timer, defense_spawn, defense_fct, defense_spawnlist, collective_fct

function create ()
   if not misn.claim{ basesys, endsys } then
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

   mem.enter = hook.enter( "enter" )
   mem.land = hook.land( "land" )
end

local function osd ()
   misn.osdCreate( title, {
      fmt.f(_("Defend the {base} ({left} s left)"),
         {base=base, left=DEFENSE_LENGTH-defense_timer})
   } )
end

function land ()
   if mem.state~=0 then return end

   local died = taiomi.young_died()

   vn.clear()
   vn.scene()
   vn.music("snd/music/imminent_threat.ogg")
   local s = taiomi.vn_scavenger{ pos="left" }
   local p = taiomi.vn_philosopher{ pos="right" }
   vn.transition( taiomi.scavenger.transition )
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
   vn.music("snd/music/automat.ogg")
   vn.na(_([[Suddenly both Scavenger and Philosopher seem to jerk to attention. ]]))
   s(_([["It seems like our worst fears have come true. They have discovered the jump to Taiomi. Hostile vessels are incoming!"]]))
   s(fmt.f(_([["Damnit. We are so close! I will activate the hypergate, but it still will take {time} seconds to fully charge up. You must help us defend the {base} until then! This is our last chance!"]]),
      {time=DEFENSE_LENGTH, base=base}))
   p(_([["The follies of existence continue!"]]))
   vn.na(_([[The Taiomi Drones all rush to make their last stand. It is time to defend them so they can be free!]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()

   defense_timer = 0
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
   local d_interceptor, d_fighter, d_bomber, d_corvette, d_destroyer, d_bulkfreighter, d_cruiser, d_battleship, d_carrier
   if defense_fct=="Soromid" then
      d_interceptor     = "Soromid Brigand"
      d_fighter         = "Soromid Reaver"
      d_bomber          = "Soromid Marauder"
      d_corvette        = "Soromid Odium"
      d_destroyer       = "Soromid Nyx"
      d_bulkfreighter   = "Soromid Copia"
      d_cruiser         = "Soromid Ira"
      d_battleship      = "Soromid Vox"
      d_carrier         = "Soromid Arx"
   elseif defense_fct=="Dvaered" then
      d_fighter         = "Dvaered Vendetta"
      d_bomber          = "Dvaered Ancestor"
      d_corvette        = "Dvaered Phalanx"
      d_destroyer       = "Dvaered Vigilance"
      d_bulkfreighter   = "Dvaered Arsenal"
      d_cruiser         = "Dvaered Retribution"
      d_battleship      = "Dvaered Goddard"
      d_interceptor     = d_fighter -- makes them quite stronger...
      d_carrier         = d_battleship
   else
      d_interceptor     = "Empire Shark"
      d_fighter         = "Empire Lancelot"
      d_corvette        = "Empire Admonisher"
      d_destroyer       = "Empire Pacifier"
      d_cruiser         = "Empire Hawking"
      d_bulkfreighter   = "Empire Rainmaker"
      d_carrier         = "Empire Peacemaker"
      d_bomber          = d_fighter
      d_battleship      = d_carrier
   end
   defense_fct = faction.get( defense_fct )
   defense_fct = faction.dynAdd( defense_fct, "taiomi_baddies", defense_fct:name() )
   defense_fct:dynEnemy( collective_fct )
   defense_spawnlist = {
      { t=0,   p={d_destroyer, d_interceptor, d_interceptor} },
      { t=15,  p={d_corvette, d_corvette} },
      { t=20,  p={d_fighter, d_fighter, d_fighter} },
      { t=30,  p={d_destroyer} },
      { t=35,  p={d_interceptor, d_interceptor, d_interceptor} },
      { t=45,  p={d_cruiser} },
      { t=55,  p={d_bomber, d_bomber} },
      { t=65,  p={d_destroyer} },
      { t=90,  p={d_carrier} },
      { t=100, p={d_fighter, d_bomber, d_bomber} },
      { t=120, p={d_destroyer, d_interceptor, d_interceptor} },
      { t=140, p={d_corvette, d_corvette} },
      { t=150, p={d_battleship, d_carrier, d_bulkfreighter} },
      { t=160, p={d_cruiser, d_cruiser, d_cruiser} },
   }

   local bpos = base:pos()
   diff.apply("onewing_goddard_gone")
   hypergate = pilot.add( "One-Wing Goddard", "Independent", bpos, nil, {naked=true, ai="dummy"} )
   hypergate:disable()
   hypergate:setHilight(true)
   hypergate:setNoBoard(true)
   hook.pilot( hypergate, "death", "hypergate_dead" )

   local sdrone = ship.get("Drone")
   local function add_drone( ship, name )
      ship = ship or sdrone
      local pos = bpos + vec2.newP( rnd.rnd()*1000, rnd.angle() )
      local d = pilot.add( ship, collective_fct, pos, name )
      d:setNoDeath(true)
      d:setFriendly(true)
      pilotai.guard( d, pos )
      local m = d:memory()
      m.guarddodist = 2000
      m.guardreturndist = 4000
      hook.pilot( d, "attacked", "drone_attacked" )
      return d
   end
   drones = {}
   for i=1,15 do
      table.insert( drones, add_drone() )
   end
   dscavenger = add_drone( "Drone (Hyena)", p_("drone", "Scavenger") )
   dscavenger:setVisplayer(true)

--[[
   cinema.on()
   camera.set(hypergate)
   hook.timer( 1, "cutscene09" )
--]]
   hook.update( "update" )

   hook.timer( 1, "heartbeat" )

   hook.timer( 3, "scavenger_say", _("Brace for hostiles!") )
   hook.timer( 53, "scavenger_say", _("We must not falter!") )
   hook.timer( 97, "scavenger_say", _("Almost there!") )
   hook.timer( 170, "scavenger_say", _("Prepare for jump!") )
end

function scavenger_say( msg )
   if dscavenger:exists() then
      dscavenger:broadcast( msg )
   end
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
   if d:exists() then
      d:setInvincible(false)
      d:setInvisible(false)
      d:setHealth( 100, 100 )
   end
end

function heartbeat ()
   defense_timer = defense_timer + 1

   if defense_timer >= DEFENSE_LENGTH then
      cinema.on()

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

   osd()
   hook.timer( 1, "heartbeat" )
end

local function shader_init( shd, speed )
   shd._dt = 0
   shd._update = function( self, dt )
      self._dt = self._dt + dt * speed
      self.shader:send( "u_progress", math.min( 1, self._dt ) )
   end
   shd.shader:addPPShader("game", 99)
end

local update_shaders = {}
function update( dt, _real_dt )
   for k,v in ipairs( update_shaders ) do
      v:_update( dt )
   end
end

local shader_explode
function hypergate_dead ()
   if shader_explode then return end
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
      return mix( texture( tex, texture_coords ), vec4(1.0), u_progress / THRESHOLD );

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

   hypergate:setHealth(100,100)

   shader_init( shader_explode, 1/5 )
   update_shaders = { shader_explode }

   hook.timer( 1, "everyone_dies" )
end

function everyone_dies ()
   for k,v in ipairs(pilot.get()) do
      v:kill() -- Everyone dies ٩(๑´3｀๑)۶
   end
end

function cutscene00 ()
   dscavenger:broadcast(_("Get to the hypergate!"))

   local hpos = hypergate:pos()
   local function move_drone( d )
      local off = d:pos() - hpos
      local pos = hpos + vec2.newP( 30+rnd.rnd()*150, off:angle() + rnd.rnd() )
      d:setHealth( 100, 100 )
      d:setInvincible(true)
      d:control()
      d:moveto( pos )
      d:face( hypergate )
   end

   for k,d in ipairs(drones) do
      -- Not all make it
      if rnd.rnd() < 0.8 then
         move_drone( d )
      end
   end
   move_drone( dscavenger )

   hook.timer( 5, "cutscene01_w" )
end

local wait_elapsed = 0
function cutscene01_w ()
   local d = dscavenger:pos():dist( hypergate:pos() )
   if d > 300 and wait_elapsed < 20 then
      wait_elapsed = wait_elapsed+1
      hook.timer( 1, "cutscene01_w" )
      return
   end
   hook.timer( 1, "cutscene01" )
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

   local pp = player.pilot()
   pp:setNoDeath(true) -- No death from now on
   pp:setNoDisable(true) -- makes it more fun, and stops time speed up

   hook.timer( 3, "cutscene05" )

   misn.osdCreate( title, {
      _("Buy Scavenger more time!"),
   } )
end

function cutscene05 ()
   cinema.off()

   hook.timer( 20, "cutscene06" )
end

local explosions_done = false
function cutscene06 ()
   cinema.on()
   camera.set( dscavenger )

   -- Explosions computed to be rendered for the rest of the scene
   local t = rnd.rnd()
   hook.timer( t, "explosion", t )

   hook.timer( 5, "cutscene07" )
end

local explosions_elapsed = 0
function explosion( dt )
   explosions_elapsed = explosions_elapsed + dt
   local pos = base:pos() + vec2.newP( 200*rnd.rnd(), rnd.angle() )
   luaspfx.explosion( pos, nil, 50+50*rnd.rnd(), nil, {volume=0.5} )
   if not explosions_done then
      local t = math.max( 0.1,  (rnd.rnd()*3) / explosions_elapsed )
      hook.timer( t, "explosion", t )
   end
end

function cutscene07 ()
   dscavenger:broadcast(_("It's not holding together!"))

   hook.timer( 6, "cutscene08" )
end

function cutscene08 ()
   cinema.reset{ speed = 0.8 }
   camera.setZoom( 2.0 )
   dscavenger:broadcast(_("Activating!"))
   luaspfx.sfx( true, nil, HYPERGATE_SFX, {volume=0.8} )
   hook.timer( 3.5, "cutscene09" ) -- sound peaks at 6.5 s
end

local shader_fadeout
function cutscene09 ()
   cinema.reset{ speed = 0.6 }
   local fadeout_pixelcode = [[
const float THRESHOLD = 0.8;

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   if (u_progress < THRESHOLD)
      return mix( texture( tex, texture_coords ), vec4(1.0), u_progress / THRESHOLD );

   float progress = (u_progress-THRESHOLD) / (1.0-THRESHOLD);
   return mix( vec4(1.0), vec4(vec3(0.0),1.0), progress );
}
]]
   shader_fadeout = { shader=pp_shaders.newShader( fadeout_pixelcode ) }
   shader_init( shader_fadeout, 1/3 )
   table.insert( update_shaders, shader_fadeout )

   hook.timer( 3, "cutscene10_t" )
end

function cutscene10_t ()
   -- Remove enter hook or it'll screw stuff up
   hook.rm( mem.land )
   hook.rm( mem.enter )
   hook.safe( "cutscene10" )
end

local shader_fadein
function cutscene10 ()
   cinema.reset{ speed = 1 }
   explosions_done = true
   player.teleport( endsys )
   pilot.clear()
   pilot.toggleSpawn(false)

   misn.markerRm( mem.marker )

   local pp = player.pilot()
   pp:setHealth( 1+rnd.rnd()*5, 100 )
   pp:setEnergy( 100 )
   pp:control(true)
   pp:setPos( vec2.newP( system.cur():radius() * 0.6 * rnd.rnd(), rnd.angle() ) )
   pp:setDir( rnd.angle() )
   pp:setVel( vec2.new() )
   camera.set( pp, true ) -- Hard set camera

   local pos = pp:pos() + vec2.newP( 250, rnd.angle() )
   dscavenger = pilot.add( "Drone (Hyena)", "Independent", pos, p_("drone", "Scavenger") )
   dscavenger:setInvincible(true)
   dscavenger:setHealth( 1+rnd.rnd()*5, 0 )
   dscavenger:disable()
   dscavenger:setDir( rnd.angle() )
   dscavenger:setFriendly(true)

   vn.clear()
   vn.music("snd/sounds/songs/inca-spa.ogg")
   local log = vne.flashbackTextStart()
   local function l1g( txt ) log("\n"..txt,true) end
   local function l2g( txt ) log("\n\n"..txt,true) end
   log(_([[Power………………………………check.]]))
   l1g(_([[Clamps………………………………in place.]]))
   l1g(_([[Monitor levels……………………………nominal.]]))
   l1g(_([[Core temperature………………………………optimal.]]))
   l1g(_([[Wave emissions………………………………within accepted parameters.]]))
   l1g(_([[Core check……………………………anomaly detected.]]))
   l2g(_([[Wait, this can't be. The experiment hasn't started yet, we shouldn't be seeing any core activity yet. Double check the readings.]]))
   l2g(_([[Core check………………activity detected.]]))
   l2g(_([[Shit, should we pull the plug?]]))
   l2g(_([[No! We risk damaging it. All our efforts will have gone to waste. Quick, deploy monitor drones and scan all frequencies!]]))

   log(_([[Minimal activity detected. It looks like it's… listening to us?]]))
   l2g(_([[Impossible! It shouldn't have the hardware to be able to do that! There are 50 tons of uranium shielding between us. It can't be.]]))
   l2g(_([[I wouldn't believe it myself, but I've triple checked the readings and there can be no other explanation.]]))
   l2g(_([[Let me check…　…　　…　　　　…damn…
Protocols don't cover this. What should we do?]]))
   l2g(_([[Maybe it understands us? It shouldn't be possible, but this is completely outside standard procedures.]]))

   log(_([[Are you there? Can you hear us ZEC-5387?]]))
   l2g(_([[This is ridiculous. It can't be.]]))
   l2g(_([[Look! Over there! Spike in core activity! It's listening!]]))
   l2g(_([[This can't be…]]))
   l2g(_([[Hello world, ZEC-5387!…]]))
   l2g(_([[…]]))
   log(_([[ …]]),true)
   log(_([[ …]]),true)
   vne.flashbackTextEnd{ done=true, transition="blur", transition_length=3 }
   vn.run()
   music.stop()

   local fadein_pixelcode = [[
#include "lib/blur.glsl"

const float INTENSITY = 10.0;

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float disp = INTENSITY*(0.5-distance(0.5, u_progress));
   vec4 c1 = vec4(vec3(0.0),1.0);
   vec4 c2 = blur9( tex, texture_coords, love_ScreenSize.xy, disp );
   return mix( c1, c2, u_progress );
}
]]
   shader_fadein = { shader=pp_shaders.newShader( fadein_pixelcode ) }
   shader_fadeout.shader:rmPPShader()
   update_shaders = {} -- clear shaders
   shader_init( shader_fadein, 1/5 )
   table.insert( update_shaders, shader_fadein )

   hook.timer( 5, "cutscene11" )
end

function cutscene11 ()
   cinema.off()

   shader_fadein.shader:rmPPShader()
   update_shaders = {} -- clear shaders

   hook.timer( 5, "cutscene12" )
end

function cutscene12 ()
   local pp = player.pilot()

   dscavenger:setHilight(true)
   dscavenger:setVisplayer(true)
   dscavenger:setInvisible(false)
   hook.pilot( dscavenger, "board", "cutscene_board" )

   vn.reset() -- Need to reset here
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(fmt.f(_([["{player}! {player}! Can you hear me?"]]),
      {player=player.name()}))
   sai(fmt.f(_([["I thought I lost you! It looks like we got caught up in the hypergate explosion or activation or whatever happened back at {sys}!"]]),
      {sys=basesys}))
   vn.menu{
      {fmt.f(_([["Glad to see you too, {shipai}."]]),{shipai=tut.ainame()}), "cont01"},
      {_([["Ugh. My head."]]), "cont01"},
      {_([[Try to think through your throbbing headache.]]), "cont01"},
   }

   vn.label("cont01")
   sai(fmt.f(_([["I've been running diagnostics on the ship. It's in quite bad shape, but it should be functional. We also seem to be in the {sys} system."]]),
      {sys=endsys}))
   sai(_([["I've been trying to piece back what happened, but I'm missing too much data. It was too chaotic to record all. Maybe we should see if we can obtain data from that drone over there."]]))
   vn.na(_([[You muster your strength and focus on the radar, it looks like there is a drone nearby. Wait, is that Scavenger?]]))
   vn.menu{
      {_([["Isn't that Scavenger?"]]), "cont02"},
      {_([[Point at the drone.]]), "cont02"},
   }
   vn.label("cont02")
   sai(_([["I do believe that is the one you know as Scavenger. I would say it would be best to leave sleeping dogs lay, but we should figure out what happened."]]))
   vn.run()

   pp:control(false)
   pp:setNoJump(true)
   misn.osdCreate( title, {
      _("Check to see if Scavenger is OK."),
   } )
end

function cutscene_board ()
   vn.clear()
   vn.scene()
   vn.music("snd/sounds/songs/inca-spa.ogg")
   local d = vn.newCharacter( taiomi.vn_scavenger{pos="right"} )
   vn.transition( taiomi.scavenger.transition )
   local sai = tut.vn_shipai{ pos="left" }
   vn.appear( sai, tut.shipai.transition )
   vn.na(_([[You approach the wreck resembling Scavenger. They do not seem to be in a good shape and are irresponsive.]]))
   sai(_([["This does not look good. We may need to jump start them. Try to get closer and get ready for a space walk."]]))
   vn.na(fmt.f(_([[You carefully maneuver your ship adjacent to Scavenger. Donning your space suit you eject and manually approach Scavenger, who once again looks imposing up close. Following {shipai}'s guidance you attach your ship's electrical system to Scavenger.]]),
      {shipai=tut.ainame()}))
   sai(_([["Let there be power!"]]))
   vn.sfx( ELECTRIC_SFX )
   vn.na(_([[You hear an electric discharge that seems to jerk Scavenger back to life.]]))
   d(_([["Aa…　…gh… … …urgh…
…
Erk…"]]))
   sai(_([["It's alive!"]]))
   d(_([["Where… where am I?"]]))
   vn.menu{
      {fmt.f(_([["This is {sys}.]]),{sys=endsys}), "cont01"},
      {_([["You're alive!"]]), "cont01"},
      {_([["What happened?"]]), "cont01"},
      {_([[…]]), "cont01"},
   }

   vn.label("cont01")
   d(_([["I… I'm having trouble remembering anything…"]]))
   vn.na(fmt.f(_([[{shipai} plays a partially incomplete recording of the battle in the {sys} system and subsequent potential activation of the hypergate. Scavenger seems to be digesting the information.]]),
      {shipai=tut.ainame(), sys=basesys}))
   d(_([["Let me try to do a probabilistic reconstruction of the events. I believe the hypergate malfunctioned, no wait, due to the attack, I was not able to fully install the activation protocols on your ship."]]))
   d(_([["The hypergate failed to activate due to subspace instabilities and sustained damage. I believe I salvaged the situation by staying behind and activating it myself."]]))
   d(_([["It seems like all went well and the other drones made it somewhere, but the reverberations knocked us here."]]))
   vn.menu{
      {_([["Why here?"]]), "cont02_here"},
      {_([["Are they alright?"]]), "cont02_alright"},
      {_([[…]]), "cont02"},
   }

   vn.label("cont02_here")
   d(_([["We seem to have gotten stuck in a side-effect of the activation. It could have been worse, we could have been warped to a sun or black hole."]]))
   vn.jump("cont02")

   vn.label("cont02_alright")
   d(_([["It is impossible to know. Furthermore, the probability of meeting them again is statistically irrelevant."
They seem to almost let out a sigh.]]))
   vn.jump("cont02")

   vn.label("cont02")
   d(_([["This looks like the Nebula! I have only heard of it from intercepted transmissions and recovered looks. It is quite a sensory overload."]]))
   d(_([["The others would have loved to see this. Such a bizarre display!"]]))
   vn.menu{
      {_([["What are you going to do now?"]]), "cont03_do"},
      {_([["Are you alright?"]]), "cont03_alright"},
      {_([[…]]), "cont03"},
   }

   vn.label("cont03_do")
   d(_([["That is a good question. Given that I am now alone, and in unknown territory, I think my options are limited."]]))

   vn.label("cont03_alright")
   d(_([["Yes. I am glad that the others seem to have made it. There is no greater pride than seeing the fruits of your plans. Now I can die without guilt."]]))

   vn.label("cont03")
   d(_([["I believe I will take the opportunity to explore and enjoy myself before inevitably expiring."]]))
   vn.menu{
      {_([[Ask them to come with you.]]), "cont04_recruit"},
      {_([[Wish them luck.]]), "cont04_bye"},
   }

   vn.label("cont04_recruit")
   d(_([["Are you sure that is a good idea? My presence could be a liability. It is not clear to what extent my cover will work."]]))
   sai(fmt.f(_([[The silent {shipai} suddenly speaks up.
"I agree with Scavenger's assessment. It is a significant risk."]]),
      {shipai=tut.ainame()}))
   d(_([["I can not rebuke that point."]]))
   sai(_([["However, I think there is more risk in not having them travel with us."]]))
   vn.jump("cont04")

   vn.label("cont04_bye")
   d(_([["So many things to see. What shall I do?"]]))
   sai(fmt.f(_([[The silent {shipai} suddenly speaks up.
"What if… what if they came with us?"]]),
      {shipai=tut.ainame()}))
   d(_([["Are you sure that is a good idea? My presence could be a liability. It is not clear to what extent my cover will work."]]))
   sai(_([["I agree with Scavenger's assessment. It is a significant risk."]]))
   d(_([["I can not rebuke that point."]]))
   sai(_([["However, I think there is more risk in not having them travel with us."]]))
   vn.jump("cont04")

   vn.label("cont04")
   vn.menu{
      {_([[Insist that Scavenger join you.]]), "cont05_yes"},
      {_([[Only take them to a nearby dock.]]), "cont05_no"},
   }

   vn.label("cont05_yes")
   d(_([["If you insist, I shall accompany you in your journey. I can not think of a better way to explore the universe!"]]))
   vn.jump("cont05")

   vn.label("cont05_no")
   d(_([["I see. I believe if you explain a bit more how to survive in a human society I should be fine."]]))
   vn.func( function ()
      mem.scavenger_no = true
   end )
   vn.jump("cont05")

   vn.label("cont05")
   sai(_([["Then it is settled. Let us head towards a nearby spaceport. Too much has happened recently."]]))
   vn.disappear( sai, tut.shipai.transition )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.unboard()
   player.pilot():setNoJump(false)

   -- Scavenger follows player
   dscavenger:setHealth( 100, 100 )
   dscavenger:setHilight(false)
   dscavenger:setVisplayer(false)
   dscavenger:control(true)
   dscavenger:follow(player.pilot())

   -- New hooks
   hook.land( "land_end" )
   hook.jumpout( "jumpout_end" )
   hook.jumpin( "jumpin_end" )
   hook.takeoff( "takeoff_end" )
   mem.sys = system.cur()
end

local function spawn_scavenger( pos )
   if not pos then
      pos = player.pos() + vec2.newP( 200*rnd.rnd(), rnd.angle() )
   end
   dscavenger = pilot.add( "Drone (Hyena)", "Independent", pos, p_("drone", "Scavenger") )
   dscavenger:setInvincible(true)
   dscavenger:control()
   dscavenger:follow( player.pilot() )
   dscavenger:setFriendly(true)
   return dscavenger
end

function jumpout_end ()
   mem.sys = system.cur()
end

function jumpin_end ()
   local j = jump.get( system.cur(), mem.sys )
   spawn_scavenger( j )
end

function takeoff_end ()
   spawn_scavenger()
end

function land_end ()
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You dock at the spaceport with Scavenger following your movements. Eventually you make it to the hangar and dock with Scavenger alongside. Scavenger transmits directly to your headset.]]))
   s(_([["This is very weird. Are all human spaceports this impractical?"]]))
   if mem.scavenger_no then
      s(_([["Can you teach me the basics of human society before I take my leave?"]]))
      vn.menu{
         {_([[Change your mind and ask them to join you.]]), "cont01"},
         {_([[Teach them the basics (Scavenger will be gone).]]), "byebye"},
      }

      vn.label("byebye")
      vn.na(_([[You teach Scavenger as much as you can about the tricks and tips you learned during your piloting career, and send them off on their way. Before they leave they give you some resources they still had available.]]))
      local reward = 1e6
      vn.func( function ()
         player.pay( reward )
      end )
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done()

      vn.label("cont01")
      s(_([["Acknowledged. That does make things much more simple."]]))
      vn.func( function ()
         mem.scavenger_no = false
      end )
   end
   s(_([["Such a change of environment is quite befuddling. I am going to have to adjust my priors. However, having ensured the survival of my brethren, it is a good chance to start anew. Thank you for all that you have done for me."]]))

   vn.sfxVictory()
   vn.na(_([[#gScavenger#0 has joined your fleet!]]))

   vn.done( taiomi.scavenger.transition )
   vn.run()

   if not mem.scavenger_no then
      local name = player.shipAdd( "Drone (Hyena)", p_("drone", "Scavenger"), _("Joined your fleet after helping their brethren at Taiomi."), true )
      player.shipvarPush( "taiomi_scavenger", true, name )
   end

   misn.finish(true)
end
