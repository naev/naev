--[[
   Taiomi, Ship Graveyard

   Dead ships floating everywhere with fancy custom background.
--]]

-- We use the default background too!
local starfield = require "bkg.lib.starfield"
local lg = require 'love.graphics'
local taiomi = require "common.taiomi"
local class = require "class"

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
local _z, zmax, _zmin = camera.getZoom()
-- some helpers to speed up computations
local znw2, znh2 = zmax*nw/2, zmax*nh/2
local nw2, nh2 = nw/2, nh/2
local buffer, tw, th, fgparts, bgparts, wing, pos

function background ()
   -- Check to see taiomi progress
   local lastfight = false
   if player.pilot():exists() then
      if taiomi.progress() == 9 and taiomi.inprogress() then
         lastfight = true
      end
   end

   -- Create particles and buffer
   local density = 400
   if lastfight then
      density = 600
   end
   buffer = 200
   tw = zmax*nw+2*buffer
   th = zmax*nh+2*buffer
   local nparts = math.floor( tw * th / math.pow(density,2) + 0.5 )

   -- New class that randomly samples from ship images
   local sImage = class.inheritsFrom( lg.Image )
   sImage._type = "ShipImage"
   function sImage.newImage( s )
      local t = sImage.new()
      t.ship = s
      t.t = {}
      for i=1,16 do
         local c = s:render( rnd.angle(), 0, rnd.angle() )
         table.insert( t.t, c:getTex() )
      end
      t.tex = t.t[1]
      t.w, t.h = t.tex:dim()
      t.s = 1
      return t
   end
   function sImage:draw( id, ... )
      self.tex = self.t[id]
      lg.Image.draw( self, ... )
   end
   function sImage:id()
      return rnd.rnd( 1, #self.t )
   end

   -- Load graphics
   local images_raw = {
      -- Debris
      { n = 3, i = lg.newImage( 'gfx/spfx/cargo' ), s = 6, debris = true },
      { n = 3, i = lg.newImage( 'gfx/spfx/debris0' ), s = 6, debris = true },
      { n = 3, i = lg.newImage( 'gfx/spfx/debris1' ), s = 6, debris = true },
      { n = 3, i = lg.newImage( 'gfx/spfx/debris2' ), s = 6, debris = true },
      { n = 3, i = lg.newImage( 'gfx/spfx/debris3' ), s = 6, debris = true },
      { n = 3, i = lg.newImage( 'gfx/spfx/debris4' ), s = 6, debris = true },
      { n = 3, i = lg.newImage( 'gfx/spfx/debris5' ), s = 6, debris = true },
      { n = 2, i = lg.newImage( 'gfx/spfx/debris_cluster1' ), s = 6, debris = true },
      { n = 2, i = lg.newImage( 'gfx/spfx/debris_cluster2' ), s = 6, debris = true },
      -- Neutral
      { n = 2, i = lg.newImage( 'gfx/spfx/derelict_llama1' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_koala1' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_koala2' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_mule1' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_phalanx1' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_shark1' ), s = 6 },
      { n = 1, i = lg.newImage( 'gfx/spfx/derelict_vendetta1' ), s = 6 },
      { n = 1, i = sImage.newImage( ship.get("Quicksilver") ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get("Rhino") ), s = 1 },
      -- Pirate
      { n = 3, i = sImage.newImage( ship.get('Hyena' ) ), s = 1 },
      { n = 2, i = sImage.newImage( ship.get('Pirate Shark' ) ), s = 1 },
      { n = 2, i = sImage.newImage( ship.get('Pirate Vendetta' ) ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get('Pirate Ancestor' ) ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get('Pirate Phalanx' ) ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get('Pirate Rhino' ) ), s = 1 },
      -- Empire
      { n = 2, i = sImage.newImage( ship.get('Empire Lancelot' ) ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get('Empire Shark' ) ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get('Empire Admonisher' ) ), s = 1 },
      { n = 1, i = sImage.newImage( ship.get('Empire Pacifier' ) ), s = 1 },
   }
   local images = {}
   for k,v in ipairs(images_raw) do
      local w, h = v.i:getDimensions()
      v.w = w / v.s
      v.h = h / v.s
      v.n = v.n or 1
      for i = 1,v.n do
         table.insert( images, v )
      end
   end
   local images_nodebris = {}
   for k,v in ipairs(images) do
      if not v.debris then
         table.insert( images_nodebris, v )
      end
   end

   local function parts_create( nodebris )
      local part = {
         x = tw*rnd.rnd() - buffer,
         y = th*rnd.rnd() - buffer,
      }
      if nodebris then
         part.i = images_nodebris[ rnd.rnd( 1, #images_nodebris ) ]
      else
         part.i = images[ rnd.rnd( 1, #images ) ]
      end
      local img = part.i
      if img.i._type=="ShipImage" then
         part.id = img.i:id()
      else
         local sx = rnd.rnd(1,img.s)-1
         local sy = rnd.rnd(1,img.s)-1
         part.q = lg.newQuad( sx*img.w, sy*img.h, img.w, img.h, img.i )
      end
      part.w = img.w
      part.h = img.h
      return part
   end

   local function parts_sort( a, b )
      return a.s < b.s
   end

   -- Create background
   bgparts = {}
   for i = 1,nparts*3 do
      local part = parts_create( true )
      bgparts[i] = part
      part.s = 1 / (2 + 5*rnd.rnd())
   end
   table.sort( bgparts, parts_sort )

   -- Create foreground
   fgparts = {}
   for i = 1,nparts do
      local part = parts_create( false )
      fgparts[i] = part
      part.s = 1 + rnd.rnd()
   end
   table.sort( fgparts, parts_sort )

   -- Special
   wing = {
      x = 100,
      y = 150,
      i = lg.newImage( 'gfx/spfx/derelict_goddard_wing' ),
      s = 2,
   }

   -- Set up hooks
   pos = camera.pos() -- Computes

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

      n = n or 3e3
      move = move or 0.03
      scale = scale or 1.5
      g = g or 0.8
      salpha = salpha or 5
      sbeta = sbeta or 3
      local w = 2048
      local h = 2048
      local cvs = lg.newCanvas( w, h, {dpiscale=1} )
      local oldcanvas = lg.getCanvas()
      lg.setCanvas( cvs )
      lg.clear( 0, 0, 0, 0 )
      lg.setColour( g, g, g, 1 )
      local angle = math.rad( 20 + 10 * rnd.rnd() )
      for i = 1,n do
         local p = parts_create( true )
         p.s = 1 / (salpha + sbeta*rnd.rnd())
         local x = 0.8 * naev.rnd.threesigma()
         local y = 0.7 * naev.rnd.threesigma()
         p.x = w*(x+3) / 6
         p.y = h*(y+3) / 6
         if p.q then
            p.i.i:draw( p.q,  p.x, p.y, 0, p.s )
         else
            p.i.i:draw( p.id, p.x, p.y, 0, p.s )
         end
      end
      lg.setCanvas(oldcanvas)
      naev.bkg.image( cvs.t.tex, 0, 0, move, scale, angle )

      -- Store in cache
      cache.taiomi[ id ] = cvs
   end
   -- Create three layers using parallax, this lets us cut down significantly
   -- on the number of ships we have to render to create them
   local a = 0.5
   if lastfight then
      a = 0.4
   end
   add_bkg( 0, 9e3, 0.03, 1.5, a+0.0, 6.7, 3 )
   add_bkg( 1, 6e3, 0.05, 1.5, a+0.1, 4.0, 3 )
   add_bkg( 2, 3e3, 0.08, 1.5, a+0.2, 2.5, 3 )

   -- Default nebula background (no star)
   starfield.init{ nolocalstars = true }

   -- Almost no ambient
   gfx.lightAmbient( 0 )
   gfx.lightIntensity( 0.8 * gfx.lightIntensityGet() )
end
local function update ()
   -- Calculate player motion
   local npos = camera.pos()
   local z = camera.getZoom()
   local dx, dy = (pos - npos):get()
   pos = npos

   -- Update the parts
   local function update_part( p )
      p.x = p.x + dx * p.s
      p.y = p.y - dy * p.s
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

      -- See if renderable
      local x = (p.x - znw2) / z + nw2
      local y = (p.y - znh2) / z + nh2
      p.rx = x
      p.ry = y
      local w = p.w * zmax * p.s
      local h = p.h * zmax * p.s
      p.r  = (x > -w and y > -h and x <= nw+w and y <= nh+h)
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

renderbg = starfield.render

local function draw_part( p, s, z )
   if p.r then
      if p.q then
         p.i.i:draw( p.q,  p.rx, p.ry, 0, p.s * s / z )
      else
         p.i.i:draw( p.id, p.rx, p.ry, 0, p.s * s / z )
      end
   end
end
function renderfg ()
   -- Run the update stuff here
   update()

   local z = camera.getZoom()
   lg.setColour( 1, 1, 1, 1 )
   for k,p in ipairs( bgparts ) do
      draw_part( p, 2, z )
   end
end
function renderov ()
   local z = camera.getZoom()
   lg.setColour( 1, 1, 1, 1 )
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
