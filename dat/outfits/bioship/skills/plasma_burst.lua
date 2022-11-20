--<desc_extra>Deals 300 plasma damage to all hostile ships in 200 range.</desc_extra>

local damage = 300
local penetration = 0.5
local radius = 200
local cooldown = 20

local function activate( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end

   local pos = p:pos()
   for k,t in ipairs(p:getEnemies( radius )) do
      local norm, angle = (t:pos() - pos):polar()
      local mod = 1 - norm / radius
      local mass = math.pow( damage / 15, 2 )
      -- Damage and knockback
      t:damage( damage, 0, penetration, "impact", p )
      t:knockback( mass, vec2.newP( mod*radius, angle ), pos, 1 )
   end

   -- TODO visuals and sound

   mem.timer = cooldown
   po:state("cooldown")
   po:progress(1)

   return true
end

function init( p, po )
   mem.timer = nil
   po:state("off")
   mem.isp = (p == player.pilot())
end

function update( _p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   po:progress( mem.timer / cooldown )
   if mem.timer <= 0 then
      po:state("off")
      mem.timer = nil
   end
end

function ontoggle( p, po, on )
   if on then
      return activate( p, po )
   end
end
