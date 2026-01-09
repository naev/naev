local fmt = require "format"

notactive = true

local THRESHOLD = 50
local STATS = {
   Small = {
      armour_regen = 3,
      mass = 20, -- Shadow is 25
      absorb = 7,
   },
   Medium = {
      armour_regen = 6,
      mass = 90 ,-- Ghost is 110/130
      absorb = 7,
   },
   Large= {
      armour_regen = 12,
      mass = 410, -- Phantasm is 450/550
      absorb = 7,
   },
}

function descextra( _p, _o, po )
   local s = _([[Properties depend on size of the equipped slot:]]).."\n"
   if not po then
      for k,sz in ipairs{"Large", "Medium", "Small"} do
         local stats = STATS[ sz ]
         s = s.."#n"..fmt.f(_("{size}:#0 {mass} {mass_units}, {absorb}%, {regen} {regen_units} Armour Regeneration"),
            {size=sz,
            mass=stats.mass, mass_units=naev.unit("mass"),
            absorb=stats.absorb,
            regen=stats.armour_regen, regen_units=naev.unit("power"),
         } ).."\n"
      end
   else
      s = s.."   "..fmt.f(_("Equipped on a {size} slot."),
         {size=po:slot().size }).."\n"
   end
   return s.."#g"..fmt.f(_([[Regeneration effect is doubled when armour is below {threshold}%.]]),
      {threshold=THRESHOLD}).."#0"
end

function init( _p, po )
   mem.stats = STATS[ po:slot().size ]
   assert( mem.stats ~= nil )
   for k,v in pairs(mem.stats) do
      po:set( k, v )
   end
   mem.on = false
end

function update( p, po, _dt )
   if p:armour() < THRESHOLD and not mem.on then
      po:set( "armour_regen", mem.stats.armour_regen * 2 )
      mem.on = true
   end
end
