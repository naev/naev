local lib = {}

function lib.init( set )
   set.init( _("Corsair"), {
         outfit.get("Corsair Systems"),
         --outfit.get("Corsair Hull"),
         --outfit.get("Corsair Engines"),
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
end

return lib
