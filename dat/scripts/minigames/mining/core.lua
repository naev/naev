local mining = {}

local love           = require 'love'
--local audio          = require 'love.audio'
local lg             = require 'love.graphics'
--local fmt            = require 'format'

local frag_background = love.filesystem.read( "background.frag" )
local frag_pointer = love.filesystem.read( "pointer.frag" )
local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

local moving, pointer, pointer_tail, speed, cx, cy, transition, alpha, done, targets, shots, shots_max, shots_timer, z_cur, z_max, mfont
local radius = 200
local img, shd_background, shd_pointer
function mining.load()
   mfont       = lg.newFont(32)
   pointer     = 0
   pointer_tail = -math.pi/4
   moving      = false
   transition  = 0
   shots       = 0
   z_cur       = 1
   -- Outfit-dependent
   speed       = math.pi
   shots_max   = 3

   -- Create constant image
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 0.5, 0.5, 0.5, 1 )
   img = love.graphics.newImage( idata )

   -- Load shaders
   shd_background = lg.newShader( frag_background, vertexcode )
   shd_pointer = lg.newShader( frag_pointer, vertexcode )

   -- TODO center on player
   local lw, lh = lg.getDimensions()
   cx = lw/2
   cy = lh/2

   -- Generate targets
   targets = {}

   targets[1] = {
         pos = 0.6*2*math.pi,
         sway_range = 0.1*2*math.pi,
         sway_period = 1,
         size = math.pi/8,
         reward = 0.5,
         z = 1,
      }
   targets[2] = {
         pos = 0.9*2*math.pi,
         sway_range = 0,
         sway_period = 0,
         size = math.pi/16,
         reward = 0.8,
         z = 1,
      }
   targets[3] = {
         pos = 0.8*2*math.pi,
         sway_range = 0.1*2*math.pi,
         sway_period = 1,
         size = math.pi/8,
         reward = 0.5,
         z = 2,
      }

   z_max = 0
   for k,t in ipairs(targets) do
      t.cur = t.pos
      t.y = 0
      z_max = math.max( z_max, t.z )
   end
   shots_timer = {}
   for i=1,shots_max do
      shots_timer[i] = 0
   end
end

local function shoot ()
   if shots >= shots_max then
      return
   end

   for k,t in ipairs(targets) do
      if z_cur==t.z and math.abs(pointer-t.cur) < t.size then
         t.hit = true
      end
   end

   shots = shots + 1
end

function mining.keypressed( key )
   if key=="q" or key=="escape" then
      transition = 0
      done = 1
   else
      if not moving then
         moving = true
      else
         shoot()
      end
   end
end

local function ease( x )
   if x < 0.5 then
      return 2*x*x
   end
   return -2*x*x + 4*x - 1
end

function mining.draw()
   local lw, lh = love.window.getDesktopDimensions()

   -- Background
   lg.setColor( 0.2, 0.2, 0.2, alpha )
   lg.rectangle( "fill", 0, 0, lw, lh )

   -- Background
   lg.setColor( 0.7, 0.7, 0.7, 0.7*alpha )
   shd_background:send( "radius", radius )
   lg.setShader( shd_background )
   lg.draw( img, cx-radius, cy-radius, 0, radius*2, radius*2 )
   lg.setShader()

   -- Pointer
   shd_pointer:send( "pointer", pointer )
   lg.setColor( 0.0, 1.0, 1.0, 1.0*alpha )
   shd_pointer:send( "radius", radius*1.2 )
   lg.setShader( shd_pointer )
   lg.draw( img, cx-radius, cy-radius, 0, radius*2, radius*2 )
   lg.setShader()

   -- Start Area
   lg.setColor( 0.1, 0.1, 0.1, alpha )
   lg.rectangle( "fill", cx+radius*0.7, cy-2, radius*0.4, 5 )

   -- Targets
   for k,t in ipairs(targets) do
      local y = ease( math.max( 0, 1-t.y ) )
      local a = alpha * y
      if a > 0 then
         local r = radius*0.9
         local s = r*t.size * (y*0.5+0.5)
         local p = t.cur
         if t.hit then
            r = r * y
         else
            r = r * (2-y)
         end
         if t.z == z_cur+1 then
            r = r + 150
         elseif t.m then
            r = r + 150*ease(1-t.m)
         end
         lg.setColor( 1, 0, 0, a )
         lg.circle( "fill", cx+math.cos(p)*r, cy+math.sin(p)*r, s )
      end
   end

   -- Ammunition
   local w = 10
   local h = 20
   local m = 10
   local x = cx - ((w+m)*shots_max-m)/2
   local y = cy + radius + 20
   for i=1,shots_max do
      local a = 1-shots_timer[i]
      if a > 0 then
         lg.setColor( 1, 1, 1, a )
         lg.rectangle( "fill", x+(i-1)*(w+m), y, w, h )
      end
   end

   if z_cur > z_max then
      lg.setColor( 1, 1, 1, alpha )
      lg.printf( "COMPLETE", mfont, cx-100, cy-mfont:getHeight()/2, 200, "center" )
   end
end

--[[
local function compute_bonus ()
   local reward = 0
   for k,t in ipairs(targets) do
      if t.hit then
         reward = reward + t.reward
      end
   end
   return reward
end
--]]

local telapsed = 0
function mining.update( dt )
   --transition = transition + dt*10
   transition = transition + 1
   if transition >= 1 then
      alpha = 1
      if done then
         love.event.quit()
      end
   else
      alpha = ease(transition)
      if done then
         alpha = 1-alpha
      end
   end

   if moving then
      pointer = pointer + speed * dt
      if pointer > 2*math.pi then
         pointer = pointer - math.pi*2
         z_cur = z_cur+1
         for k,t in ipairs(targets) do
            if t.z==z_cur then
               t.m = 0
            end
         end
         if z_cur > z_max then
            moving = false
            pointer = 2*math.pi
         end
      end
   end
   if pointer_tail > 0 or moving then
      pointer_tail = pointer_tail + speed * dt
   end

   for i=1,shots do
      shots_timer[i] = math.min( shots_timer[i]+dt*10, 1 )
   end

   telapsed = telapsed+dt
   for k,t in ipairs(targets) do
      t.cur = t.pos + t.sway_range*math.sin(t.sway_period*telapsed)
      -- Fall in animation
      if t.m then
         t.m = t.m + dt*3
         if t.m > 1 then
            t.m = nil
         end
      end
      -- Fade out animation
      if t.hit or t.z < z_cur then
         t.y = t.y + dt
      end
   end
end

return mining
