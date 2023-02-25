local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local function getStats( p )
   local flow_cost, cooldown, ref, strength, duration
   local size = flow.size( p )
   if size == 1 then
      flow_cost   = 25
      cooldown    = 3
      ref         = outfit.get("Seeking Chakra Small")
      duration    = 5
      strength    = 1
   elseif size == 2 then
      flow_cost   = 50
      cooldown    = 4
      ref         = outfit.get("Seeking Chakra Small")
      duration    = 8
      strength    = 1.25
   else
      flow_cost   = 100
      cooldown    = 5
      ref         = outfit.get("Seeking Chakra Small")
      duration    = 15
      strength    = 1.5
   end
   return flow_cost, cooldown, ref, strength, duration
end

function descextra( p, _o )
   -- Generic description
   if p==nil then
      return "#y".._("Uses flow to create a seeking energy orb that deals damage and disable with a cooldown. Affected ships suffer from decreased movement and firerate. Strength varies depending on the flow amplifier.").."#0"
   end
   local cost, cooldown, ref, strength, duration = getStats( p )
   local refstats = ref:specificstats()
   local damage, disable, penetration = refstats.damage, refstats.disable, refstats.penetration
   local size = flow.size( p )
   local prefix = ""
   if size==1 then
      prefix = _("(Lesser)")
   elseif size==3 then
      prefix = _("(Greater)")
   end
   return fmt.f("#y".._("{prefix} Uses {cost} flow to create a seeking energy orb that deals {damage} ion damage and {disable} disable {penetration}% penetration and a {cooldown} second cooldown. Affected ships suffer from -{strength}% movement and firerate for {duration} seconds. Strength varies depending on the flow amplifier."),
      {prefix=prefix, cost=cost, damage=damage, disable=disable, penetration=100*penetration, cooldown=cooldown, strength=25*strength, duration=duration}).."#0"
end

function init( p, po )
   mem.flow_cost, mem.cooldown, mem.ref, mem.strength, mem.duration = getStats( p )

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
   if not on then return false end

   if mem.timer > 0 then return false end

   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   local dir = p:dir()
   local sw, sh = p:ship():dims()
   local pos = p:pos()+vec2.newP( (sw+sh)*0.5+10, dir )
   po:munition( p, mem.ref, p:target(), dir, pos )
   mem.timer = mem.cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   return true
end

function onimpact( _p, target )
   target:effectAdd( "Chakra Corruption", mem.duration, mem.strength )
end
