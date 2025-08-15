local lg = require "love.graphics"
local love = require "love"
local fc = require "colour"
local intro = {}

-- Constants
local INTRO_SPEED = 30
local SIDE_MARGIN = 100
local IMAGE_WIDTH = 300
local IMAGE_FADE  = 0.2
local FADEIN      = 1
local FADEOUT     = 1
local FONT_SIZE   = 16
local TEXT_FADEPX = FONT_SIZE*2

-- Local variables
local data, done, pos, vel, alpha, text_width, text_off, font, nw, nh, firsttick, advance

function intro.load( origdata )
   -- Defaults
   firsttick = false
   done  = false
   pos   = 0
   vel   = INTRO_SPEED
   alpha = 0

   love.graphics.setBackgroundColour( 0, 0, 0, 0 )

   font  = lg.newFont( FONT_SIZE )
   font:setOutline(1)
   nw, nh = lg.getDimensions()
   text_width = nw - 2*SIDE_MARGIN - IMAGE_WIDTH
   text_off = SIDE_MARGIN + IMAGE_WIDTH
   advance = font:getLineHeight()
   pos   = nh

   -- Load and prepare data
   data = {}
   for k,v in ipairs(origdata) do
      if v.type == "text" then
         if v.data then
            local _maxw, wrap = font:getWrap( v.data, text_width )
            for i,w in ipairs(wrap) do
               table.insert( data, {
                  type = "text",
                  data = w,
               } )
            end
         else
            table.insert( data, v )
         end
      elseif v.type == "image" then
         if v.data then
            v.w, v.h = v.data:getDimensions()
            table.insert( data, v )
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

local function draw_image( img )
   if not img or not img.data then return end

   local s = 1
   --if img.h > nh then
   --   s = nh / img.h
   if img.w > IMAGE_WIDTH then
      s = IMAGE_WIDTH / img.h
   end

   --local x = 0
   local x = (IMAGE_WIDTH + SIDE_MARGIN - s*img.w)*0.5
   local y = (nh - s*img.h)*0.5

   lg.setColour( 1, 1, 1, alpha*img.alpha )
   img.data:draw( x, y, 0, s, s )
end

local function smoothstep( edge0, edge1,x )
   local t = math.min( math.max( (x - edge0) / (edge1 - edge0), 0.0), 1.0)
   return t * t * (3.0 - 2.0 * t)
end

local img_prev, img_cur
function intro.draw()
   -- Draw images first
   draw_image( img_prev )
   draw_image( img_cur )

   -- Draw text
   local cg = fc.FontGreen
   local y = pos
   for k,v in ipairs(data) do
      if y > -advance then
         if y >= nh then
            break
         end
         if v.type=="text" then
            if v.data then
               local a = smoothstep( 0, 1, math.min(
                  (nh-y-FONT_SIZE*1.0) / TEXT_FADEPX, -- Bottom
                  (y+FONT_SIZE*0.0) / TEXT_FADEPX -- Top
               ))
               lg.setColour( cg[1], cg[2], cg[3], alpha*a )
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
      if v.type=="text" then
         y = y + advance
      end
   end

   -- Fully advanced so done
   if y < 0 then
      done = true
   end
end

function intro.update( dt )
   -- due to how the stuff is embedded, the first frame can be delayed and mess things up
   if not firsttick then
      firsttick = true
      return
   end

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
