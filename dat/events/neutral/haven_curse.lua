--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Haven Curse">
 <location>enter</location>
 <chance>100</chance>
 <system>Haven</system>
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
local der = require 'common.derelict'

local spb, sys = spob.getS("Old Man Jack")
local pos = sys:waypoints("haven_curse_spawn")

local REWARD1 = outfit.get("Corsair Systems")

local ghost, ghost_waypoints
local ghost_pos = 1
local ghost_off = false
function ghost_idle ()
   ghost_pos = (ghost_pos % #ghost_waypoints) + 1
   ghost:moveto( ghost_waypoints[ghost_pos], false, false )
end

local fct
function create ()
   if not var.peek("testing") then evt.finish(false) end
   if not evt.claim( sys, true ) then return evt.finish(false) end

   fct = faction.dynAdd( "Marauder", "Haven Ghost", _("Marauder"), {clear_enemies=true, clear_allies=true} )

   hook.timer( 1, "heartbeat" )
   hook.enter("finish")

   ghost_waypoints = {}
   local center = spb:pos()
   local N = 30
   for i = 1,N do
      table.insert( ghost_waypoints, center + vec2.newP( 500, 2*math.pi / N * (i-1) ) )
   end
   ghost = pilot.add("Pirate Hyena", fct, ghost_waypoints[#ghost_waypoints], _("Suspicious Pirate Hyena") )
   ghost:setNoDeath()
   ghost:setNoDisable()
   ghost:control(true)
   ghost:intrinsicSet( "speed_mod", -80 )
   ghost:intrinsicSet( "accel_mod", -80 )
   hook.pilot( ghost, "idle", "ghost_idle" )
   hook.pilot( ghost, "hail", "ghost_hailed" )
   hook.pilot( ghost, "attacked", "ghost_messed" )
   ghost_idle()
   ghost:setNoRender(true)
   ghost:setInvisible(true)
   hook.timer( 10, "ghost_back" )
end

function ghost_messed ()
   if not ghost:flags("invisible") then
      ghost:effectAdd( "Fade-Out Norender" )
      hook.timer( 5+5*rnd.rnd(), "ghost_back" )
   end
end

function ghost_hailed ()
   player.commClose()
   ghost_messed()
end

function ghost_back ()
   if not ghost:exists() or ghost_off then return end
   if ghost:effectHas("Fade-In") then return end
   ghost:setHealth(100,100)
   ghost:setNoRender(false)
   ghost:setInvisible(false)
   ghost:effectAdd("Fade-In")
   hook.timer( 10+10*rnd.rnd(), "ghost_messed" )
end

function finish ()
   evt.finish(false)
end

local noise_shader
local spin_start, spin_last
local spin_elapsed = 0
local shader_level = 0
local function spin_reset ()
   spin_start     = nil
   spin_last      = nil
   spin_elapsed   = 0
   shader_level   = 0
   if noise_shader then
      shader.rmPPShader( noise_shader )
      noise_shader = nil
   end
   if ghost_off then
      ghost_back()
      ghost_off = false
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
   if d > spb:radius()*0.8 and d < 1000 then
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
               local p = pilot.add( "Pirate Hyena", fct, pos, _("Suspicious Derelict"), {naked=true} )
               p:setHealth( 37, 0 )
               p:setDisable()
               p:effectAdd("Fade-In")
               p:setHilight()
               p:setVisplayer()
               hook.pilot( p, "attacked", "der_attacked" )
               hook.pilot( p, "board", "der_boarded" )
               if noise_shader then
                  shader.rmPPShader( noise_shader )
               end
               derelict = p
               lmisn.sfxEerie()
               pilotai.clear()
               pilot.toggleSpawn(false)
               return -- done
            elseif spin_elapsed > math.pi * 4 and shader_level < 2 then
               if noise_shader then
                  shader.rmPPShader( noise_shader )
               end
               noise_shader = pp_shaders.corruption( 1.0 )
               shader.addPPShader( noise_shader )
               shader_level = 2
               if not ghost_off then
                  ghost_off = true
                  ghost_messed()
               end
               -- TODO sound
            elseif spin_elapsed > math.pi * 2 and shader_level < 1 then
               noise_shader = pp_shaders.corruption( 0.5 )
               shader.addPPShader( noise_shader )
               shader_level = 1
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
   boss:setDir( rnd.angle() )
   fade_factor = 0
   fade_growth = 0
   boss:effectRm("Black")
   boss:effectAdd("Fade-In Black")
   boss:control(false)
   boss:setHostile(true)
   boss:setInvisible(false)
   shader.rmPPShader( noise_shader )

   -- Proper boss music
   boss_music = audio.new( "snd/music/blackmoor_tides.ogg" )
   --boss_music = audio.new( "snd/music/blackmoor_tides.ogg", "stream" )
   boss_music:play()
   boss_music:setLooping(true)
   camera.setZoom()
end

function spawn_flash2 ()
   boss:setPos( player.pos() + vec2.newP( 400, rnd.angle() ) )
   boss:setDir( rnd.angle() )
   fade_factor = 0
   fade_growth = 1
   hook.timer( 3, "spawn_final" )
end

function spawn_flash1 ()
   boss:setPos( player.pos() + vec2.newP( 300, rnd.angle() ) )
   boss:setDir( rnd.angle() )
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
   local p = pilot.add( "Pirate Kestrel", fct, bpos, _("Defiance"), {ai="baddie", naked=true} )
   p:control()
   p:setInvisible( true )
   p:setNoDeath()
   p:effectAdd("Black")
   p:setHilight(true)
   p:intrinsicSet( "ew_detect", 200 )
   p:intrinsicSet( "ew_track", -50 )
   p:intrinsicSet( "shield_mod", 100 )
   p:intrinsicSet( "armour_mod", 100 )
   p:intrinsicSet( "shield_regen_mod", 50 )
   p:intrinsicSet( "absorb", 20 )
   p:intrinsicSet( "fbay_rate", 100 )
   p:intrinsicSet( "fbay_capacity", 100 )
   p:intrinsicSet( "fbay_movement", 50 )
   p:intrinsicSet( "jam_chance", 50 )
   equipopt.pirate( p, {
      fighterbay = 10,
   } ) -- So intrinsics affect
   local m = p:memory()
   m.comm_greet = _([[You hear the sound of oceans and wild over the communication channel.]])
   m.taunt = nil
   m.bribe_no = _([[The wind is howling over the communication channel.]])
   hook.pilot( p, "attacked", "boss_attacked" )
   boss = p
end

-- To avoid stealth nuke spam cheese, the boss will blink and clear lockons
-- when taking damage with no visible enemies
function boss_attacked( _b, attacker )
   if attacker:withPlayer() then
      for k,p in ipairs(boss:getVisible( 6e3 )) do
         if boss:areEnemies( p ) then
            return
         end
      end
      if #boss:getEnemies( 3e3 ) > 0 then return end
      -- No nearby enemies
      local bpos = boss:pos()
      luaspfx.blink( boss, bpos )
      boss:jamLockons()
      boss:effectAdd("Blink")
      boss:setPos( player.pos() + vec2.newP( 1500, player.pilot():dir()+math.pi ) )
   end
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
   ghost_off = true
   ghost_messed()
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
            boss:jamLockons()
            boss:effectAdd("Blink")
            boss:outfitRm("all")
            local ships = {}
            local radius
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
               radius = 1000
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
               radius = 2000
            end
            -- "Launch" some new fighters that sort of "pop" out
            for i,s in ipairs(ships) do
               local p = pilot.add( s, fct, boss:pos() )
               p:setHostile(true)
               p:setLeader(boss)
               local a = rnd.angle()
               p:setDir( a )
               p:setVel( vec2.newP( 200, a ) )
               p:effectAdd( "Fade-In" )
               table.insert( boss_adds, p )
            end
            -- Behind player
            boss:setPos( player.pos() + vec2.newP( radius, player.pilot():dir()+math.pi ) )
            boss_stage = boss_stage + 1
         else
            -- Player won
            boss:setDisable()
            boss:setInvincible(true)
            -- Get rid of followers
            for k,f in ipairs(boss:followers()) do
               if f:exists() then
                  f:effectAdd( "Fade-Out" )
                  f:intrinsicSet( "weapon_damage", -1000 ) -- To not kill player
               end
            end
            player.pilot():jamLockons() -- So the player shouldn't die
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

function der_attacked ()
   derelict:setHealth(100,100)
   derelict:effectAdd("Fade-Out")
   spawn_start()
end

function der_boarded ()
   if player.outfitNum(REWARD1) > 0 then
      derelict:effectAdd("Fade-Out")
      spawn_start()
      return
   end

   vn.reset()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.transition()
   vn.na(_([[You board the suspicious derelict, not entirely sure how it appeared here. It has weird markings you haven't seen before on the hull.]]))
   vn.na(fmt.f(_([[You enter the ship, expecting a crowded {ship} interior, but instead find the interior is completely stripped empty, including even the interior walls. However, what is more puzzling, is there seems to be medium ship systems jammed in the middle. It almost looks like it wants to be taken with you.]]),
      {ship=derelict:ship()}))
   vn.na(_([[Obligingly, you extract the systems through the cockpit after you release the canopy, and bring it aboard your ship.]]))
   vn.func( function ()
      player.outfitAdd(REWARD1)
   end )
   vn.sfxEerie()
   vn.na(fmt.reward(REWARD1))
   vn.na(_([[You leave the derelict with a feeling that something bad will happen if you mess with it further.]]))
   vn.sfx( der.sfx.unboard )
   vn.run()
end

function boss_board ()
   player.unboard()

   vn.reset()
   vn.scene()
   vn.sfx( der.sfx.board )
   local voice = vn.newCharacter( _("Voice"), {colour={0.8, 0.2, 0.2}} )
   vn.transition()
   vn.na(fmt.f(_([[Against your better judgement, you board the mysterious {shipname}. It is covered with strange markings you don't identify.]]),
      {shipname=boss:name()}))
   vn.na(_([[From the moment you enter the ship, you feel something is amiss, and a throbbing pain starts at the back of your head.]]))
   voice(_([[]]))
   vn.sfx( der.sfx.unboard )
   vn.run()

   spb:landDeny(false)
   evt.finish(true)
end
