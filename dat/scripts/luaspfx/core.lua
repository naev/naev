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

local function __update_table( tbl, dt )
   local toremove = {}
   for k,v in ipairs(tbl) do
      -- Update pitch if necessary.
      local p = luaspfx.__pitch
      if p and v.sound then
         v.sound:setPitch( p )
      end
      -- Update position
      v.pos = v.pos + v.vel * dt
      -- Update time
      v.time = v.time + dt
      if v.time > v.ttl then
         table.insert( toremove, 1, k )
      end
   end
   -- Clean up as necessary (reverse sorted)
   for k,v in ipairs(toremove) do
      table.remove( tbl, v )
   end
end
function __luaspfx_update( dt, realdt )
   local dt_mod = player.dt_mod()
   if dt_mod ~= luaspfx.__last_dt_mod then
      luaspfx.__last_dt_mod = dt_mod
      luaspfx.__pitch = dt_mod
   else
      luaspfx.__pitch = nil
   end
   __update_table( luaspfx.__bg, dt )
   __update_table( luaspfx.__fg, dt )
end

function __luaspfx_render( tbl )
   local cx, cy = camera.get():get()
   local cz = camera.getZoom()
   for k,v in ipairs(tbl) do
      v.efx.time = v.time
      -- Convert coordinates to screen
      local ox, oy = v.pos:get()
      local x = (ox-cx) / cz + nw2
      local y = nh2 - (oy-cy) / cz
      -- Run function (should render)
      v.efx:func( x, y, cz )
   end
end

local function __luaspfx_add( tbl, efx, ttl, pos, vel )
   vel = vel or vec2.new(0,0)
   efx:init( ttl, pos, vel )
   table.insert( tbl, {
      efx=efx,
      pos=pos,
      vel=vel,
      time=0,
      ttl=ttl,
   } )
end

function luaspfx.addbg( efx, params, ttl, pos, vel )
   return __luaspfx_add( luaspfx.__bg, efx, params, ttl, pos, vel )
end

function luaspfx.addfg( efx, params, ttl, pos, vel )
   return __luaspfx_add( luaspfx.__fg, efx, params, ttl, pos, vel )
end

function luaspfx.init()
   luaspfx.__bg = {}
   luaspfx.__fg = {}
   luaspfx.__hookbg = hook.renderbg( "__luaspfx_render", luaspfx.__bg )
   luaspfx.__hookfg = hook.renderfg( "__luaspfx_render", luaspfx.__fg )
   luaspfx.__update = hook.update( "__luaspfx_update" )
end

function luaspfx.exit()
   if luaspfx.__hookbg then hook.rm( luaspfx.__hookbg ) end
   luaspfx.__hookbg = nil
   if luaspfx.__hookfg then hook.rm( luaspfx.__hookfg ) end
   luaspfx.__hookfg = nil
   if luaspfx.__update then hook.rm( luaspfx.__update ) end
end

return luaspfx

