local lg = require "love.graphics"
local love = require "love"
local fc = require "colour"
local intro = {}

local INTRO_SPEED = 30
local SIDE_MARGIN = 100
local IMAGE_WIDTH = 300
local IMAGE_FADE  = 0.2
local FADEIN      = 1
local FADEOUT     = 1
local FONT_SIZE   = 16

local data
local done = false
local pos = 0
local vel = INTRO_SPEED
local alpha = 0
local text_width
local text_off
local font
local nw, nh

function intro.load()
   love.graphics.setBackgroundColour( 0, 0, 0, 0 )

   local c = naev.cache()
   data = c._intro
   font = lg.newFont( FONT_SIZE )
   nw, nh = lg.getDimensions()
   text_width = nw - 2*SIDE_MARGIN - IMAGE_WIDTH
   text_off = SIDE_MARGIN + IMAGE_WIDTH
   local font_height = font:getLineHeight()
   pos = nh

   -- Load and prepare data
   for k,v in ipairs(data) do
      if v.type == "text" then
         if v.data then
            local _maxw, wrap = font:getWrap( v.data, text_width )
            v.h = font_height * math.max( 1, #wrap )
         else
            v.h = font_height
         end
         v.adv = v.h
      elseif v.type == "image" then
         if v.data then
            v.w, v.h = v.data:getDimensions()
            v.x = (IMAGE_WIDTH + SIDE_MARGIN - v.w)*0.5
            v.y = (nh - v.h)*0.5
         end
      end
   end
end

function intro.keypressed( key )
   if key=="escape" then
      done = true
   elseif key=="up" then
      vel = math.max( 0, vel-8 )
   elseif key=="down" then
      vel = math.min( 100, vel+8 )
   elseif key=="space" or key=="return" then
      pos = pos-100
   elseif key=="backspace" then
      pos = pos+100
   end
end

local img_prev, img_cur
function intro.draw()
   -- Draw images first
   if img_prev and img_prev.data then
      lg.setColour( 1, 1, 1, alpha*img_prev.alpha )
      img_prev.data:draw( img_prev.x, img_prev.y )
   end
   if img_cur and img_cur.data then
      lg.setColour( 1, 1, 1, alpha*img_cur.alpha )
      img_cur.data:draw( img_cur.x, img_cur.y )
   end

   -- Draw text
   local cg = fc.FontGreen
   lg.setColour( cg[1], cg[2], cg[3], alpha )
   local y = pos
   for k,v in ipairs(data) do
      if y > -v.h then
         if y >= nh then
            break
         end
         if v.type=="text" then
            if v.data then
               lg.printf( v.data, font, text_off, y, text_width )
            end
         elseif v.type=="image" then
            if not v.active then
               img_prev = img_cur
               img_cur = v
               v.alpha = 0
               v.active = true
            end
         end
      end
      y = y + (v.adv or 0)
   end

   -- Fully advanced so done
   if y < 0 then
      done = true
   end
end

function intro.update( dt )
   -- Global fade in/out
   if not done then
      alpha = math.min( 1, alpha + FADEIN*dt )
      if img_prev then
         img_prev.alpha = img_prev.alpha - IMAGE_FADE*dt
         if img_prev.alpha < 0 then
            img_prev = nil
         end
      end
      if img_cur then
         img_cur.alpha = math.min( 1, img_cur.alpha + IMAGE_FADE*dt )
      end
   else
      alpha = alpha - FADEOUT*dt
      if alpha < 0 then
         return love.event.quit()
      end
   end
   lg.setBackgroundColour( 0, 0, 0, alpha )

   -- Update position
   pos = pos - vel*dt
end

return intro
