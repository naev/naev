local pp_shaders = require 'pp_shaders'

function shader_new( fragcode )
   return pp_shaders.newShader( fragcode )
end

shader_fade = 3
function shader_on()
   mem._progress = math.max( 0, mem._progress or 0 )
   ppshader:send( "progress", mem._progress )
   if not mem._shader then
      mem._shader = shader.addPPShader( ppshader, "game" )
   end
end
function shader_off()
   if mem._shader then
      mem._progress = math.min( 1, mem._progress )
      ppshader:send( "progress", mem._progress )
   end
end
function shader_update_on( dt )
   if mem._shader and mem._progress < 1 then
      mem._progress = mem._progress + dt * shader_fade
      ppshader:send( "progress", mem._progress )
   end
end
function shader_update_cooldown( dt )
   if mem._shader then
      if mem._progress > 0 then
         mem._progress = mem._progress - dt * shader_fade
         ppshader:send( "progress", mem._progress )
      else
         shader.rmPPShader( ppshader )
         mem._shader = nil
      end
   end
end
function shader_force_off()
   if mem._shader then
      shader.rmPPShader( ppshader )
      mem._shader = nil
   end
end
