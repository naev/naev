--[[
   Pure Lua implementation of special effects for use with missions
--]]
local love = require 'love'
local lg = require 'love.graphics'
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

local luaspfx = {}
luaspfx.effects = require 'luaspfx.effects'

local function __luaspfx_render( dt, tbl )
   local toremove = {}
   local cx, cy = camera.get():get()
   local cz = camera.getZoom()
   for k,v in ipairs(tbl) do
      -- Update time
      v.time = v.time + dt
      if v.time < v.ttl then
         v.params.time = v.time
         -- Update position
         v.pos = v.pos + v.vel * dt
         -- Convert coordinates to screen
         local ox, oy = v.pos:get()
         local x, y = ox-x, oy-y
         x = (x-nw2) / z + nw2
         y = nh2 - (y-nh2) / z
         -- Run function (should render)
         v.func( v.params, x, y, z )
      else
         table.insert( toremove, 1, k )
      end
   end

   -- Clean up as necessary (reverse sorted)
   for k,v in ipairs(toremove) do
      table.remove( tbl, v )
   end
end

local function __luaspfx_add( tbl, efx, params, ttl, pos, vel )
   params = params or {}
   if efx.create then
      efx.create( params, ttl, pos, vel )
   end
   table.insert( tbl, {
      efx=efx,
      func=efx.func,
      params=params,
      pos=pos,
      time=0,
      ttl=ttl,
      vel=vel or vec2.new(0,0),
   } )
end

function luaspfx.addbg( func, params, ttl, pos, vel )
   return __luaspfx_add( luaspfx.__bg, func, params, pos, vel )
end

function luaspfx.addfg( func, params, ttl, pos, vel )
   return __luaspfx_add( luaspfx.__fg, func, params, pos, vel )
end

function luaspfx.init()
   luaspfx.__bg = {}
   luaspfx.__fg = {}
   luaspfx.__hookbg = hook.renderbg( "__luaspfx_render", luaspfx.__bg )
   luaspfx.__hookfg = hook.renderfg( "__luaspfx_render", luaspfx.__fg )
end

function luaspfx.exit()
   if luaspfx.__hookbg then hook.rm( luaspfx.__hookbg ) end
   luaspfx.__hookbg = nil
   if luaspfx.__hookfg then hook.rm( luaspfx.__hookfg ) end
   luaspfx.__hookfg = nil
end

return luaspfx
