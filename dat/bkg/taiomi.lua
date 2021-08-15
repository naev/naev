--[[
   Taiomi, Ship Graveyard

   Dead ships floating everywhere with fancy custom background.
--]]

-- We use the default background too!
require "bkg.default"

local love = require 'love'
local lg = require 'love.graphics'

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
local unused, zmax, zmin = camera.getZoom()
unused = nil
love.x = 0
love.y = 0
love.w = nw
love.h = nh
lg.origin()
-- some helpers to speed up computations
local znw2, znh2 = zmax*nw/2, zmax*nh/2
local nw2, nh2 = nw/2, nh/2

function background ()
   -- Create particles and buffer
   local density = 200*200
   local z = camera.getZoom()
   buffer = 200
   tw = zmax*nw+2*buffer
   th = zmax*nh+2*buffer
   local nparts = math.floor( tw * th / density + 0.5 )

   -- Load graphics
   local images_raw = {
      -- Debris
      { n = 5, i = lg.newImage( 'gfx/spfx/cargo.webp' ), s = 6 },
      { n = 5, i = lg.newImage( 'gfx/spfx/debris0.webp' ), s = 6 },
      { n = 5, i = lg.newImage( 'gfx/spfx/debris1.webp' ), s = 6 },
      { n = 5, i = lg.newImage( 'gfx/spfx/debris2.webp' ), s = 6 },
      { n = 5, i = lg.newImage( 'gfx/spfx/debris3.webp' ), s = 6 },
      { n = 5, i = lg.newImage( 'gfx/spfx/debris4.webp' ), s = 6 },
      { n = 5, i = lg.newImage( 'gfx/spfx/debris5.webp' ), s = 6 },
      { n = 2, i = lg.newImage( 'gfx/spfx/debris_cluster1.webp' ), s = 6 },
      { n = 2, i = lg.newImage( 'gfx/spfx/debris_cluster2.webp' ), s = 6 },
      -- Neutral
      { n = 2, i = lg.newImage( 'gfx/spfx/derelict_llama1.webp' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_koala1.webp' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_koala2.webp' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_mule1.webp' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_phalanx1.webp' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_shark1.webp' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_vendetta1.webp' ), s = 6 },
      { i = lg.newImage( 'gfx/ship/quicksilver/quicksilver.webp' ), s = 10 },
      { i = lg.newImage( 'gfx/ship/rhino/rhino.webp' ), s = 10 },
      -- Pirate
      { n = 3, i = lg.newImage( 'gfx/ship/hyena/hyena.webp' ), s = 8 },
      { n = 2, i = lg.newImage( 'gfx/ship/shark/shark_pirate.webp' ), s = 8 },
      { n = 2, i = lg.newImage( 'gfx/ship/vendetta/vendetta_pirate.webp' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/ancestor/ancestor_pirate.webp' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/phalanx/phalanx_pirate.webp' ), s = 10 },
      { i = lg.newImage( 'gfx/ship/rhino/rhino_pirate.webp' ), s = 10 },
      -- Empire
      { n = 2, i = lg.newImage( 'gfx/ship/lancelot/lancelot_empire.webp' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/shark/shark_empire.webp' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/admonisher/admonisher_empire.webp' ), s = 8 },
      { i = lg.newImage( 'gfx/ship/pacifier/pacifier_empire.webp' ), s = 10 },
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
   for i = 1,nparts*3 do
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
      x = 100,
      y = 150,
      i = lg.newImage( 'gfx/spfx/derelict_goddard_wing.webp' ),
      s = 2,
   }

   -- Set up hooks
   pos = camera.get() -- Computes

   -- Use cache
   local cache = naev.cache()

   -- Create a canvas background
   local function add_bkg( id, n, move, scale, g, salpha, sbeta )
      -- Try to use cache as necessary
      local ct = cache.taiomi
      if ct then
         if ct[id] then
            naev.bkg.image( ct[id].t.tex, 0, 0, move, scale )
            return
         end
      else
         cache.taiomi = {}
      end

      n = n or 100000
      move = move or 0.03
      scale = scale or 1.5
      g = g or 0.5
      salpha = salpha or 5
      sbeta = sbeta or 3
      local w = 2048
      local h = 2048
      local cvs = lg.newCanvas( w, h )
      lg.setCanvas( cvs )
      lg.clear( 0, 0, 0, 0 )
      lg.setColor( g, g, g, 1 )
      local a = math.rad( 20 + 10 * rnd.rnd() )
      local c = math.cos(a)
      local s = math.sin(a)
      for i = 1,n do
         local p = parts_create()
         p.s = 1 / (salpha + sbeta*rnd.rnd())
         local xr = 0.8 * naev.rnd.threesigma()
         local yr = 0.7 * naev.rnd.threesigma()
         local x = xr*c-yr*s
         local y = xr*c+yr*s
         p.x = w*(x+3) / 6
         p.y = h*(y+3) / 6
         lg.draw( p.i.i, p.q, p.x, p.y, 0, p.s )
      end
      lg.setCanvas()
      naev.bkg.image( cvs.t.tex, 0, 0, move, scale )

      -- Store in cache
      cache.taiomi[ id ] = cvs
   end
   -- Create three layers using parallax, this lets us cut down significantly
   -- on the number of ships we have to render to create them
   add_bkg( 0, 6e4, 0.03, 1.5, 0.3, 6, 3 )
   add_bkg( 1, 5e4, 0.05, 1.5, 0.4, 5, 3 )
   add_bkg( 2, 3e4, 0.08, 1.5, 0.5, 4, 3 )

   -- Default nebula background (no star)
   cur_sys = system.cur()
   prng:setSeed( cur_sys:name() )
   background_nebula()
end
function update ()
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
      -- tw and th include 2*buffer
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
local function draw_part( p, s, z )
   local x = (p.x - znw2) / z + nw2
   local y = (p.y - znh2) / z + nh2
   lg.draw( p.i.i, p.q, x, y, 0, p.s * s / z )
end
function renderfg ()
   -- Run the update stuff here
   update()

   local z = camera.getZoom()
   lg.setColor( 1, 1, 1, 1 )
   for k,p in ipairs( bgparts ) do
      draw_part( p, 2, z )
   end
end
function renderov ()
   local z = camera.getZoom()
   lg.setColor( 1, 1, 1, 1 )
   for k,p in ipairs( fgparts ) do
      draw_part( p, 1, z )
   end

   -- Special
   local x, y = pos:get()
   x = (wing.x - x) * wing.s
   y = (wing.y + y) * wing.s
   x = (x - nw2) / z + nw2
   y = (y - nh2) / z + nh2
   lg.draw( wing.i, x, y, 0, wing.s / z )
end
