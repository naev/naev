local class = require 'class'
local pp_shaders = require 'pp_shaders'

local osh = {}

osh.OutfitShader = class.inheritsFrom()
function osh.new( fragcode )
   local oshader = osh.OutfitShader.new()
   oshader.ppshader = pp_shaders.newShader( fragcode )
   oshader.fade = 3
   return oshader
end
function osh.OutfitShader:on()
   self._progress = math.max( 0, self._progress or 0 )
   self.ppshader:send( "progress", self._progress )
   if not self._shader then
      self._shader = shader.addPPShader( self.ppshader, "game" )
   end
end
function osh.OutfitShader:off()
   if self._shader then
      self._progress = math.min( 1, self._progress )
      self.ppshader:send( "progress", self._progress )
   end
end
function osh.OutfitShader:update_on( dt )
   if self._shader and self._progress < 1 then
      self._progress = self._progress + dt * self.fade
      self.ppshader:send( "progress", self._progress )
   end
end
function osh.OutfitShader:update_cooldown( dt )
   if self._shader then
      if self._progress > 0 then
         self._progress = self._progress - dt * self.fade
         self.ppshader:send( "progress", self._progress )
      else
         shader.rmPPShader( self.ppshader )
         self._shader = nil
      end
   end
end
function osh.OutfitShader:force_off()
   if self._shader then
      shader.rmPPShader( self.ppshader )
      self._shader = nil
   end
end

return osh
