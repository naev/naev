--[[
-- Pure image processing library
-- TODO move slow parts to C
-- ]]
local image = require 'love.image'

local imageproc = {
}

local function _toimage( input )
   if type(input)=="string" then
      return image.newImageData( input )
   end
   return input
end
local function _grey( r, g, b )
   return 0.2989*r + 0.5870*g + 0.1140*b
end

--[[
-- @brief Applies a map to an image, interpolating between colA and colB based on
-- greyscale values.
--]]
function imageproc.applyMap( img, colA, colB )
   img = _toimage(img)
   local ra, ga, ba = colA[1], colA[2], colA[3]
   local rb, gb, bb = colB[1], colB[2], colB[3]
   local function _colormap( x, y, r, g, b, alpha )
      local a = _grey( r, g, b )
      local b = 1-a
      return a*ra+b*rb, a*ga+b*gb, a*ba+b*bb, alpha
   end
   local w, h = img:getDimensions()
   local out = image.newImageData( w, h )
   out:paste( img, 0, 0, 0, 0, w, h )
   return out:mapPixel( _colormap )
end

local function _greyscale( x, y, r, g, b, a )
   local gr = _grey( r, g, b )
   return gr, gr, gr, a
end
--[[
-- @brief Converts an image to greyscale.
--]]
function imageproc.greyscale( img )
   img = _toimage(img)
   local w, h = img:getDimensions()
   local out = image.newImageData( w, h )
   out:paste( img, 0, 0, 0, 0, w, h )
   return out:mapPixel( _greyscale )
end

--[[
-- @brief Creates a scanline effect on an image.
--]]
function imageproc.scanlines( img, bandlength, bandgap )
   img = _toimage(img)
   local function _scanlines( x, y, r, g, b, a )
      if (y % (bandlength+bandgap)) < bandlength then
         return 0, 0, 0, 1
      end
      return r, g, b, a
   end
   local w, h = img:getDimensions()
   local out = image.newImageData( w, h )
   out:paste( img, 0, 0, 0, 0, w, h )
   return out:mapPixel( _scanlines )
end


--[[
-- @brief Does weighted addition of two images.
--
-- @note Equation is imgA * weightA + imgB * weightB + bias
--]]
function imageproc.addWeighted( imgA, imgB, weightA, weightB, bias )
   imgA = _toimage(imgA)
   imgB = _toimage(imgB)
   -- Touching ImageData internals is not very good...
   local out = image.ImageData.new()
   out.w = imgA.w
   out.h = imgA.h
   out.d = naev.data.addWeighted( imgA.d, imgB.d, weightA, weightB, bias )
   return out
end

--[[
-- Applies blur to an image.
--]]
function imageproc.blur( img, k )
   img = _toimage(img)
   if k==nil or type(k) ~= "table" then
      -- Create kernel
      local b
      if type(k)=='number' then
         b = k
      else
         b = 2
      end
      local kw = 2*b+1
      local kh = kw
      k = image.newImageData( kw, kh )
      local sum = kw*kh
      for u = 0,kh-1 do
         for v = 0,kw-1 do
            local val = 1/sum
            k:setPixel( u, v, val, val, val, val )
         end
      end
   end
   local out = image.ImageData.new()
   out.d, out.w, out.h = naev.data.convolve2d( img.d, img.w, img.h, k.d, k.w, k.h )
   --[[
   local bw = (#k-1)/2
   local bh = (#k[1]-1)/2
   -- Create image
   local w, h = img:getDimensions()
   local nw = w + 2*bw
   local nh = h + 2*bh
   local iin = image.newImageData( nw+2*bw, nh+2*bh )
   iin:paste( img, bw, bh, 0, 0, w, h )
   local out = image.newImageData( nw, nh )
   -- Blur (very slow in pure Lua)
   for su = 0,nw-1 do
      for sv = 0,nh-1 do
         local r, g, b, a
         r = 0
         g = 0
         b = 0
         a = 0
         for ku = -bw,bw do
            local kku = k[ku+bw+1]
            for kv = -bh,bh do
               local kkv = kku[ku+bh+1]
               local u = su+ku+bw
               local v = sv+kv+bh
               local pr, pg, pb, pa = iin:getPixel( u, v )
               r = r + pr*kkv
               g = g + pg*kkv
               b = b + pb*kkv
               a = a + pa*kkv
            end
         end
         out:setPixel( su, sv, r, g, b, a )
      end
   end
   --]]
   return out
end

--[[
-- @brief Applies a Gaussian blur to an image.
--]]
function imageproc.blurGaussian( img, sigma, k )
   local k = k or 2
   local w, h = img:getDimensions()
   -- Set up kernel
   local b = k 
   local K = image.newImageData( kw, kh )
   local sum = 0
   for u = 0,2*b do
      for v = 0,2*b do
         local num = math.pow((u-b), 2)+math.pow((v-b), 2)
         local den = 2*math.pow(sigma,2)
         local val = math.exp(-num/den)
         K:setPixel( u, v, val, val, val, val )
         sum = sum + val
      end
   end
   for u = 0,2*b do
      for v = 0,2*b do
         local r, g, b, a = K:getPixel( u, v )
         K:setPixel( u, v, r/sum, g/sum, b/sum, a/sum )
      end
   end
   -- Blur
   return imageproc.blur( img, K )
end

--[[
-- @brief Shifts an image by dx and dy. Does not wrap.
--]]
function imageproc.shift( img, dx, dy )
   img = _toimage(img)
   local w, h = img:getDimensions()
   local out = image.newImageData( w, h )
   out:paste( img, dx, dy, 0, 0, w-dx, h-dy )
   return out
end

--[[
-- @brief Creates a fancy hologram effect on an image.
--]]
function imageproc.hologram( img )
   img = _toimage(img)
   -- Adapted from https://elder.dev/posts/open-source-virtual-background/
   local w, h = img:getDimensions()
   local holo = imageproc.applyMap( img, {0, 1, 0.5}, {0, 0, 1} )
   local bandlength = math.max( math.floor( 2/300*h + 0.5 ), 2 )
   local bandgap = math.max( math.floor( 3/300*h + 0.5 ), 3 )
   holo = imageproc.scanlines( holo, bandlength, bandgap )
   -- Ghosting
   local k = 3
   local holo_blur = imageproc.blur( holo, k )
   local bg = image.newImageData( holo_blur:getDimensions() )
   bg = bg:paste( img, k, k, 0, 0, w, h )
   return imageproc.addWeighted( bg, holo_blur, 0.5, 0.6 )
end

return imageproc
