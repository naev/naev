--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Cool Space Stuff">
  <trigger>enter</trigger>
  <chance>100</chance>
 </event>
 --]]
 --[[
 -- This script handles cool background effects by being created every time the
 -- player enters a system. Afterwards if necessary, it will render fancy
 -- things or just end.
 --]]

local love = require 'love'
local lg = require 'love.graphics'

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
love.x = 0
love.y = 0
love.w = nw
love.h = nh
lg.origin()

-- List of special systems, for other systems this event just finishes
systems = {
   Taiomi = "taiomi_init"
}

function create ()
   local syshooks = systems[ system.cur():nameRaw() ]
   if not syshooks then
      evt.finish()
   end
   _G[syshooks]()

   hook.enter("endevent")
   hook.land("endevent")
end
function endevent () evt.finish() end


--[[
-- Taiomi, Ship Graveyard
--]]
function taiomi_init ()
   -- Music is fixed
   -- TODO make this not be a horrible hack :/
   music.stop()
   song = audio.new('snd/sounds/songs/inca-spa.ogg')
   song:setLooping( true )
   song:play()


   -- Create particles and buffer
   local density = 200*200
   buffer = 500
   tw = love.w+2*buffer
   th = love.h+2*buffer
   local nparts = math.floor( tw * th / density + 0.5 )

   -- Load graphics
   local images_raw = {
      -- Debris
      { n = 5, i = lg.newImage( 'gfx/spfx/cargo.webp' ), s = 6 },
      { n = 10, i = lg.newImage( 'gfx/spfx/debris0.webp' ), s = 6 },
      { n = 10, i = lg.newImage( 'gfx/spfx/debris1.webp' ), s = 6 },
      { n = 10, i = lg.newImage( 'gfx/spfx/debris2.webp' ), s = 6 },
      { n = 10, i = lg.newImage( 'gfx/spfx/debris3.webp' ), s = 6 },
      { n = 10, i = lg.newImage( 'gfx/spfx/debris4.webp' ), s = 6 },
      { n = 10, i = lg.newImage( 'gfx/spfx/debris5.webp' ), s = 6 },
      -- Neutral
      { n = 2, i = lg.newImage( 'gfx/ship/llama/llama.png' ), s = 8 },
      { n = 2, i = lg.newImage( 'gfx/ship/koala/koala.png' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/quicksilver/quicksilver.png' ), s = 10 },
      { i = lg.newImage( 'gfx/ship/mule/mule.png' ), s = 10 },
      { i = lg.newImage( 'gfx/ship/rhino/rhino.png' ), s = 10 },
      -- Pirate
      { n = 3, i = lg.newImage( 'gfx/ship/hyena/hyena.png' ), s = 8 },
      { n = 2, i = lg.newImage( 'gfx/ship/shark/shark_pirate.png' ), s = 8 },
      { n = 2, i = lg.newImage( 'gfx/ship/vendetta/vendetta_pirate.png' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/ancestor/ancestor_pirate.png' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/phalanx/phalanx_pirate.png' ), s = 10 },
      { i = lg.newImage( 'gfx/ship/rhino/rhino_pirate.png' ), s = 10 },
      -- Empire
      { n = 2, i = lg.newImage( 'gfx/ship/lancelot/lancelot_empire.png' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/shark/shark_empire.png' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/admonisher/admonisher_empire.png' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/pacifier/pacifier_empire.png' ), s = 10 },
   }
   images = {}
   for k,v in ipairs(images_raw) do
      local w, h = v.i:getDimensions()
      v.w = w / v.s
      v.h = h / v.s
      v.n = v.n or 1
      for i = 1,v.n do
         table.insert( images, v )
      end
   end

   local function parts_create ()
      local part = {}
      part.x = tw*rnd.rnd() - buffer
      part.y = th*rnd.rnd() - buffer
      part.i = images[ rnd.rnd( 1, #images ) ]
      local img = part.i
      local sx = rnd.rnd(1,img.s)-1
      local sy = rnd.rnd(1,img.s)-1
      part.q = lg.newQuad( sx*img.w, sy*img.h, img.w, img.h, img.i )
      return part
   end

   local function parts_sort( a, b )
      return a.s < b.s
   end

   -- Create background
   bgparts = {}
   for i = 1,nparts*10 do
      local part = parts_create()
      bgparts[i] = part
      part.s = 1 / (2 + 5*rnd.rnd())
   end
   table.sort( bgparts, parts_sort )

   -- Create foreground
   fgparts = {}
   for i = 1,nparts do
      local part = parts_create()
      fgparts[i] = part
      part.s = 1 + rnd.rnd()
   end
   table.sort( fgparts, parts_sort )

   -- Special
   wing = {
      x = 200,
      y = 100,
      i = lg.newImage( 'gfx/spfx/derelict_goddard_wing.png' ),
      s = 2,
   }

   -- Set up hooks
   pos = camera.get() -- Computes
   hook.renderbg( "taiomi_renderbg" )
   hook.renderfg( "taiomi_renderfg" )
   hook.update( "taiomi_update" )
end
function taiomi_update ()
   -- Calculate player motion
   local npos = camera.get()
   local diff = npos - pos
   local dx, dy = diff:get()
   dx = -dx
   pos = npos

   -- Update the parts
   local function update_part( p )
      p.x = p.x + dx * p.s
      p.y = p.y + dy * p.s
      if p.x < -buffer then
         p.x = p.x + tw
      elseif p.x > tw-buffer then
         p.x = p.x - tw
      end
      if p.y < -buffer then
         p.y = p.y + th
      elseif p.y > th-buffer then
         p.y = p.y - th
      end
   end

   -- Update background
   for k,p in ipairs( bgparts ) do
      update_part( p )
   end
   
   -- Update foreground
   for k,p in ipairs( fgparts ) do
      update_part( p )
   end
end
function taiomi_renderbg ()
   lg.setColor( 1, 1, 1, 1 )
   for k,p in ipairs( bgparts ) do
      lg.draw( p.i.i, p.q, p.x, p.y, 0, p.s * 2 )
   end
end
function taiomi_renderfg ()
   lg.setColor( 1, 1, 1, 1 )
   for k,p in ipairs( fgparts ) do
      lg.draw( p.i.i, p.q, p.x, p.y, 0, p.s )
   end

   -- Special
   local x, y = pos:get()
   x = (wing.x - x) * wing.s
   y = (wing.y + y) * wing.s
   lg.draw( wing.i, x, y, 0, wing.s )
end
