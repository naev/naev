local set = require "outfits.lib.set"
local corsair_spfx = require "luaspfx.corsair"

local lib = {}

local INTERVAL = 0.1

function lib.init ()
   set.init( _("Corsair"), {
         outfit.get("Corsair Systems"),
         outfit.get("Corsair Hull Plating"),
         outfit.get("Corsair Engine"),
      }, {
         [2] = {
            desc = _("-10% Stealth Range"),
            stats = {
               ["ew_stealth"] = -10,
            },
         },
         [3] = {
            desc = _("+33% Action Speed for 10 seconds after leaving stealth."),
            func = function (_p, _po, on)
               mem.set_on = on
            end,
         },
      },
      true )

   local init_old = init
   function init( p, po )
      if init_old then
         init_old( p, po )
      end
      mem.c_elapsed = 0
   end

   local onstealth_old = onstealth
   function onstealth( p, po, stealthed )
      if onstealth_old then
         onstealth_old( p, po, stealthed )
      end
      -- Add effect when dropping out of stealth
      if mem.set_on and not stealthed then
         p:effectAdd("Corsair")
      end
   end

   local update_old = update
   function update( p, po, dt )
      if update_old then
         update_old( p, po, dt )
      end
      if mem.set_on then
         mem.c_elapsed = mem.c_elapsed + dt
         if mem.c_elapsed > INTERVAL then
            mem.c_elapsed = mem.c_elapsed % INTERVAL
            local mod
            if p:flags("stealth") then
               mod = 100
            else
               mod = 20
            end
            corsair_spfx( p, p:pos(), p:vel()+vec2.newP(mod,rnd.angle()) )
         end
      end
   end
end

return lib
