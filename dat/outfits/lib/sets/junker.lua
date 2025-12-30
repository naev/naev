local fmt = require "format"
local set = require "outfits.lib.set"

local DETECT = 15
local LOOT = 20
local PD, DELAY, RANGE, RANGE2

local lib = {}

function lib.init ( noset )
   set.init( _("Junker"), {
      outfit.get("Junker Pack"),
      outfit.get("Junker Plates"),
      outfit.get("Junker Ion Shotter"),
   }, {
      [1] = {
         desc = fmt.f(_("+{detect}% Detection, +{loot}% Boarding Bonus"),
            {detect=DETECT, loot=LOOT, regen=REGEN, units=naev.unit("power")}),
         stats = {
            ["ew_detect"] = DETECT,
            ["loot_mod"] = LOOT,
         },
      },
      [2] = {
         desc = _("Provides anti-munition point defence."),
         func = function (_p, _po, on)
            mem.set_on = on
         end,
      },
   },
   noset )

   local onload_old = onload
   function onload( ... )
      if onload_old then
         onload_old( ... )
      end

      PD = outfit.get("Junker Point Defence")
      local stats = PD:specificstats()
      RANGE = stats.speed * stats.duration
      RANGE2 = RANGE*RANGE
      DELAY = stats.delay
   end

   local init_old = init
   function init( ... )
      if init_old then
         init_old( ... )
      end
      mem.cooldown = 0
   end

   local update_old = update
   function update( p, po, dt )
      if update_old then
         update_old( p, po, dt )
      end

      -- Handle point defense here
      if mem.set_on then
         mem.cooldown = mem.cooldown - dt
         if mem.cooldown <= 0 then
            local m = mem.target
            -- If doesn't exist or out of range, get new target
            local exists = (m and m:exists())
            if (not exists) or
                  (exists and m:pos():dist2( p:pos() ) > RANGE2) then
               local mall = munition.getInrange( p:pos(), RANGE, p )
               if #mall > 0 then
                  m = mall[ rnd.rnd(1,#mall) ] -- Just get a random one
                  mem.target = m
               end
            end
            if m and m:exists() then
               po:munition( p, PD, m )
               mem.cooldown = DELAY
            end
         end
      end
   end
end

return lib
