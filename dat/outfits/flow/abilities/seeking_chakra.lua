local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local function getStats( p )
   local flow_cost, cooldown, ref
   local size = flow.size( p )
   if size == 1 then
      flow_cost   = 25
      cooldown    = 3
      ref         = outfit.get("Seeking Chakra Small")
   elseif size == 2 then
      flow_cost   = 50
      cooldown    = 4
      ref         = outfit.get("Seeking Chakra Small")
   else
      flow_cost   = 100
      cooldown    = 5
      ref         = outfit.get("Seeking Chakra Small")
   end
   return flow_cost, cooldown, ref
end

function descextra( p, _o )
   -- Generic description
   if p==nil then
      return "#y".._("Uses flow to create a seeking energy orb that deals damage and disable with a cooldown. Strength varies depending on the flow amplifier.").."#0"
   end
   local cost, cooldown, ref = getStats( p )
   local refstats = ref:specificstats()
   local damage, disable = refstats.damage, refstats.disable
   local size = flow.size( p )
   local prefix = ""
   if size==1 then
      prefix = _("(Lesser)")
   elseif size==3 then
      prefix = _("(Greater)")
   end
   return fmt.f("#y".._("{prefix} Uses {cost} flow to create a seeking energy orb that deals {damage} ion damage and {disable} disable with a {cooldown} second cooldown. Strength varies depending on the flow amplifier."),
      {prefix=prefix, cost=cost, damage=damage, disable=disable, cooldown=cooldown}).."#0"
end

function init( p, po )
   mem.flow_cost, mem.cooldown, mem.ref = getStats( p )

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
   target:effectAdd("Chakra Corruption")
end
