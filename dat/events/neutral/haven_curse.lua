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

local spb, sys = spob.getS("Old Man Jack")
local pos = sys:waypoints("haven_curse_spawn")

function create ()
   evt.finish(false) -- Disabled for now
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
               local p = pilot.add( "Pirate Hyena", "Derelict", pos, _("Mysterious Derelict"), {naked=true} )
               p:setHealth( 37, 0 )
               p:setDisable(true)
               p:effectAdd("Fade-In")
               p:setHighlight()
               hook.pilot( p, "exploded", "der_destroyed" )
               hook.pilot( p, "board", "der_boarded" )
               if noise_shader then
                  shader.rmPPShader( noise_shader )
               end
               derelict = p
               lmisn.sfxEerie()
               pilotai.clear()
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

local boss
local fade_factor = 0
local fade_growth = 1/9
function spawn_final ()
   fade_factor = 0
   fade_growth = 0
   boss:effectRm("Black")
   boss:effectAdd("Fade-In Black")
   boss:control(false)
   boss:setHostile(true)
end

function spawn_flash2 ()
   fade_factor = 0
   fade_growth = 1
   hook.timer( 3, "spawn_final" )
end

function spawn_flash1 ()
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

   boss = pilot.add( "Pirate Kestrel", "Marauder", pos, _("Defiance"), {ai="baddie", naked=true} )
   boss:control()
   boss:setInvisible( true )
   boss:effectAdd("Black")
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
end

function fade ()
   if fade_factor > 0 then
      local w, h = gfx.dim()
      lg.setColour( 0, 0, 0, fade_factor )
      lg.rectangle( "fill", 0, 0, w, h )
   end
end

function update( dt )
   fade_factor = math.min( 1, fade_factor + dt * fade_growth )
end

function der_destroyed ()
   spawn_start()
end

function der_boarded ()
   player.unboard()
   derelict:setInvisible(true)
   derelict:effectAdd("Fade-Out")
end
