--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Haven Curse">
 <location>enter</location>
 <chance>100</chance>
 <system>Haven</system>
 <unique />
</event>
--]]
local pp_shaders = require 'pp_shaders'
local lmisn = require "lmisn"
local pilotai = require "pilotai"
local lg = require "love.graphics"
local equipopt = require "equipopt"
local luaspfx = require "luaspfx"
local vn = require "vn"
local fmt = require "format"

local spb, sys = spob.getS("Old Man Jack")
local pos = sys:waypoints("haven_curse_spawn")

function create ()
   evt.finish(false) -- disabled for now
   if not evt.claim( sys, true ) then return evt.finish(false) end

   hook.timer( 1, "heartbeat" )
   hook.enter("finish")
end

function finish ()
   evt.finish(false)
end

local noise_shader
local spin_start, spin_last
local spin_elapsed, spin_msg = 0, 0
local function spin_reset ()
   if spin_msg > 0 then
      player.msg(_("The static abruptly stops."))
   end
   spin_start     = nil
   spin_last      = nil
   spin_elapsed   = 0
   spin_msg       = 0
   if noise_shader then
      shader.rmPPShader( noise_shader )
      noise_shader = nil
   end
end
local function angle_diff( ref, a )
   local d = a-ref
   if d > math.pi then
      d = d - 2*math.pi
   elseif d < -math.pi then
      d = d + 2*math.pi
   end
   return d
end
local derelict
function heartbeat ()
   local v = spb:pos()-player.pos()
   local d, a = v:polar()
   if d > spb:radius() and d < 1000 then
      if not spin_start then
         spin_start  = a
         spin_last   = a
         spin_elapsed= 0
      else
         local diff = angle_diff( spin_last, a )
         if diff < 0 then
            spin_reset()
         else
            spin_elapsed = spin_elapsed + diff
            spin_last = a
            if spin_elapsed > math.pi * 6 then
               local p = pilot.add( "Pirate Hyena", "Derelict", pos, _("Suspicious Derelict"), {naked=true} )
               p:setHealth( 37, 0 )
               p:setDisable(true)
               p:effectAdd("Fade-In")
               p:setHilight()
               p:setVisplayer()
               hook.pilot( p, "exploded", "der_destroyed" )
               hook.pilot( p, "board", "der_boarded" )
               if noise_shader then
                  shader.rmPPShader( noise_shader )
               end
               derelict = p
               lmisn.sfxEerie()
               pilotai.clear()
               pilot.toggleSpawn(false)
               return -- done
            elseif spin_elapsed > math.pi * 4 then
               if noise_shader then
                  shader.rmPPShader( noise_shader )
               end
               noise_shader = pp_shaders.corruption( 1.0 )
               shader.addPPShader( noise_shader )
               -- TODO sound
            elseif spin_elapsed > math.pi * 2 then
               noise_shader = pp_shaders.corruption( 0.5 )
               shader.addPPShader( noise_shader )
               -- TODO sound
            end
         end
      end
   else
      spin_reset()
   end
   hook.timer( 0.1, "heartbeat" )
end

local boss_music
local boss
local fade_factor = 0
local fade_growth = 1/9
function spawn_final ()
   boss:setPos( player.pos() + vec2.newP( 1000, rnd.angle() ) )
   fade_factor = 0
   fade_growth = 0
   boss:effectRm("Black")
   boss:effectAdd("Fade-In Black")
   boss:control(false)
   boss:face( player.pilot() )
   boss:setHostile(true)
   boss:setInvisible(false)
   shader.rmPPShader( noise_shader )

   -- Proper boss music
   boss_music = audio.new( "snd/music/blackmoor_tides.ogg", "stream" )
   boss_music:play()
   boss_music:setLooping(true)
   camera.setZoom()
end

function spawn_flash2 ()
   boss:setPos( player.pos() + vec2.newP( 400, rnd.angle() ) )
   fade_factor = 0
   fade_growth = 1
   hook.timer( 3, "spawn_final" )
end

function spawn_flash1 ()
   boss:setPos( player.pos() + vec2.newP( 300, rnd.angle() ) )
   fade_factor = 0
   fade_growth = 3
   hook.timer( 3, "spawn_flash2" )
end

function spawn_start3 ()
   fade_factor = 3/3
   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 2.0 )
   shader.addPPShader( noise_shader )
   hook.timer( 1, "spawn_flash1" )

   local bpos = player.pos() + vec2.newP( 200, rnd.angle() )
   local p = pilot.add( "Pirate Kestrel", "Marauder", bpos, _("Defiance"), {ai="baddie", naked=true} )
   p:control()
   p:setInvisible( true )
   p:setNoDeath()
   p:effectAdd("Black")
   p:intrinsicSet( "shield_mod", 80 )
   p:intrinsicSet( "armour_mod", 60 )
   p:intrinsicSet( "shield_regen_mod", 50 )
   p:intrinsicSet( "tur_damage", 25 )
   p:intrinsicSet( "fbay_rate", 100 )
   p:intrinsicSet( "fbay_capacity", 100 )
   equipopt.pirate( p, {
      fighterbay = 10,
   } ) -- So intrinsics affect
   local m = p:memory()
   m.comm_greet = _([[You hear the sound of oceans and wild over the communication channel.]])
   m.taunt = nil
   m.bribe_no = _([[The wind is howling over the communication channel.]])

   boss = p
end

function spawn_start2 ()
   fade_factor = 2/3
   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 2.0 )
   shader.addPPShader( noise_shader )
   hook.timer( 3, "spawn_start3" )
end

function spawn_start1 ()
   fade_factor = 1/3
   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 1.0 )
   shader.addPPShader( noise_shader )
   hook.timer( 3, "spawn_start2" )
end

local function spawn_start ()
   noise_shader = pp_shaders.corruption( 0.5 )
   shader.addPPShader( noise_shader )
   hook.timer( 3, "spawn_start1" )
   spb:landDeny(true, _("Atmospheric conditions make it impossible to land now."))
   hook.renderbg( "fade" )
   hook.update( "update" )
   fade_factor = 0
   fade_growth = 1/9
   music.stop(true)
   camera.setZoom(2)
end

function fade ()
   if fade_factor > 0 then
      local w, h = gfx.dim()
      lg.setColour( 0, 0, 0, fade_factor )
      lg.rectangle( "fill", 0, 0, w, h )
   end
end

local boss_won = false
local boss_stage = 0
local boss_adds = {}
function update( dt )
   fade_factor = math.min( 1, fade_factor + dt * fade_growth )
   if boss_won then return end
   if boss and boss:exists() then
      if boss:shield() <= 0 or boss:disabled() then
         if boss_stage < 2 then
            local bpos = boss:pos()
            boss:setHealth( 100-35*(boss_stage+1), 100 )
            luaspfx.blink( boss, bpos )
            boss:effectAdd("Blink")
            boss:outfitRm("all")
            local ships = {}
            if boss_stage==0 then
               ships = {
                  "Pirate Shark",
                  "Pirate Vendetta",
                  "Pirate Ancestor",
               }
               equipopt.pirate( boss, {
                  beam = 10,
                  turret = 10,
                  fighterbay = 0,
               } ) -- So intrinsics affect
            elseif boss_stage==1 then
               ships = {
                  "Pirate Admonisher",
                  "Pirate Vendetta",
                  "Pirate Phalanx",
               }
               equipopt.pirate( boss, {
                  launcher = 10,
                  fighterbay = 0,
               } ) -- So intrinsics affect
            end
            -- "Launch" some new fighters that sort of "pop" out
            for i,s in ipairs(ships) do
               local p = pilot.add( s, "Marauder", boss:pos() )
               p:setHostile(true)
               p:setLeader(boss)
               local a = rnd.angle()
               p:setDir( a )
               p:setVel( vec2.newP( 200, a ) )
               p:effectAdd( "Fade-In" )
               table.insert( boss_adds, p )
            end
            -- Behind player
            boss:setPos( player.pos() + vec2.newP( 2000, player.pilot():dir()+math.pi ) )
            boss_stage = boss_stage + 1
         else
            -- Player won
            boss:setDisable(true)
            boss:setInvincible(true)
            -- Get rid of followers
            for k,f in ipairs(boss:followers()) do
               if f:exists() then
                  f:effectAdd( "Fade-Out" )
                  f:intrinsicSet( "weapon_damage", -1000 ) -- To not kill player
               end
            end
            munition.clear() -- So the player doesn't die
            hook.pilot( boss, "board", "boss_board" )
            pilot.toggleSpawn(true)
            hook.update("music_fadeout")
            boss_won = true
         end
      end
   end
end

local music_fade = 1
function music_fadeout( dt )
   if boss_music then
      music_fade = music_fade - dt*0.5
      if music_fade < 0 then
         boss_music:stop()
         boss_music = nil
      else
         boss_music:setVolume( music_fade )
      end
   end
end

function der_destroyed ()
   spawn_start()
end

function der_boarded ()
   player.unboard()
   derelict:setInvisible(true)
   derelict:effectAdd("Fade-Out")
   player.msg(_([[The ship seems to disappear as you board it.]]))
   spawn_start()
end

function boss_board ()
   player.unboard()

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_([[Against your better judgement, you board the mysterious {shipname}.]]),
      {shipname=boss:name()}))
   vn.run()

   spb:landDeny(false)
   evt.finish(true)
end
