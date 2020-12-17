--[[
-- Image
--]]
local class = require 'class'
local data = require 'love.data'
local filesystem = require 'love.filesystem'

local image = {}
image.ImageData = class.inheritsFrom( data.Data )
image.ImageData._type = "ImageData"
function image.newImageData( ... )
   local arg = {...}
   local w, h, d
   local t = type(arg[1])
   if t=="number" then
      w = arg[1]
      h = arg[2]
      d = naev.data.new( w*h*4, "number" )
   elseif t=="string" then
      local f = filesystem.newFile(arg[1])
      d, w, h = naev.tex.readData( f )
   else
      love._unimplemented()
   end
   local newd = image.ImageData.new()
   newd.w = w
   newd.h = h
   newd.d = d
   return newd
end
local function _id_pos(self,x,y) return 4*(y*self.w+x) end
function image.ImageData:getDimensions() return self.w, self.h end
function image.ImageData:getWidth() return self.w end
function image.ImageData:getHeight() return self.h end
function image.ImageData:getPixel( x, y )
   local pos = _id_pos(self,x,y)
   local r = self.d:get( pos+0 )
   local g = self.d:get( pos+1 )
   local b = self.d:get( pos+2 )
   local a = self.d:get( pos+3 )
   return r, g, b, a
end
function image.ImageData:setPixel( x, y, r, g, b, a )
   local pos = _id_pos(self,x,y)
   self.d:set( pos+0, r )
   self.d:set( pos+1, g )
   self.d:set( pos+2, b )
   self.d:set( pos+3, a )
end
function image.ImageData:paste( source, dx, dy, sx, sy, sw, sh )
   -- probably very slow
   for x = 0,sw-1 do
      for y = 0,sh-1 do
         self:setPixel( dx+x, dy+y, source:getPixel( sx+x, sy+y ) )
      end
   end
end


return image
