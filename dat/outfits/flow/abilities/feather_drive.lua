local audio = require 'love.audio'
local luaspfx = require "luaspfx"
local blink = require "luaspfx.feather_drive"
local flow = require "ships.lua.lib.flow"
local fmt = require "format"
local chakraexp = require "luaspfx.chakra_explosion"

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

local function getStats( p, size )
   local flow_cost, cooldown, masslimit, range, drain
   size = size or flow.size( p )
   if size == 1 then
      flow_cost   = 40
      range       = 500
      masslimit   = 600^2 --squared
      cooldown    = 7
      drain       = 100
   elseif size == 2 then
      flow_cost   = 80
      range       = 500
      masslimit   = 1500^2 --squared
      cooldown    = 8
      drain       = 200
   else
      flow_cost   = 160
      range       = 500
      masslimit   = 5000^2 --squared
      cooldown    = 9
      drain       = 400
   end
   return flow_cost, masslimit, range, cooldown, drain
end

function descextra( p, _o )
   local size
   if p then
      size = flow.size( p )
   else
      size = 0
   end
   local s = "#y".._([[TODO]]).."#0"
   for i=1,3 do
      local cost, masslimit, range, cooldown, drain = getStats( nil, i )
      local pfx = flow.prefix(i)
      if i==size then
         pfx = "#b"..pfx.."#n"
      end
      s = s.."\n"..fmt.f(_("#n{prefix}:#0 {cost} flow, {cooldown} s cooldown, {range} range, {masslimit} tonne limit, {drain} MJ energy drained per ship hit"),
         {prefix=pfx, cost=cost, range=range, cooldown=cooldown, masslimit=masslimit, drain=drain}).."#0"
   end
   return s
end

function init( p, po )
   mem.flow_cost, mem.masslimit, mem.range, mem.cooldown, mem.drain = getStats( p )
   mem.timer = 0
   po:state("off")
end

function update( _p, po, dt )
   if mem.timer < 0 then return end

   mem.timer = mem.timer - dt
   if mem.timer < 0 then
      po:state("off")
   else
      po:progress( mem.timer / mem.cooldown )
   end
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return false end

   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Use flow
   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   -- Blink!
   local dist = mem.range
   local m = p:mass()
   m = m*m
   -- We use squared values here so twice the masslimit is 25% efficiency
   if m > mem.masslimit then
      dist = dist * mem.masslimit / m
   end
   local pos = p:pos()
   local newpos = pos + vec2.newP( dist, p:dir() )
   blink( pos, (newpos-pos)*2+p:vel() ) -- Blink afterimage
   p:effectAdd( "Feather Drive" ) -- Cool "blink in" effect
   p:setPos( newpos )
   mem.timer = mem.cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   -- Calculate hits
   local centerpos = (pos+newpos)*0.5
   local pw, ph = p:ship():dims()
   local pr = (pw+ph)*0.25
   local drained = 0
   for k,t in ipairs(p:getEnemies( dist*0.5+100, centerpos, true, false, true )) do
      local tp = t:pos()
      local tw, th = t:ship():dims()
      local tr = (tw+th)*0.25
      local col1, col2 = vec2.collideCircleLine( tp, tr+pr, pos, newpos )
      if col1 then
         col2 = col2 or col1
         local col = (col1+col2)*0.5
         t:effectAdd("Chakra Corruption")
         chakraexp( col, p:vel(), tr )
         local e = t:energy(true)
         local d = math.min( mem.drain, e )
         t:setEnergy( e-d, true )
         drained = drained + d

         naev.trigger( "feather_drive", t, p )
      end
   end
   p:setEnergy( p:energy(true)+drained, true )

   -- Play the sound
   luaspfx.sfx( pos, p:vel(), sfx )

   return true
end
