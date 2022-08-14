local atk = {}

local o_shield_booster = outfit.get("Emergency Shield Booster")

function atk.setup( p )
   -- Clean up old stuff
   local m = p:memory()
   m._o = {}

   -- Check out what interesting outfits there are
   for k,v in ipairs(p:outfits()) do
      if v then
         if v == o_shield_booster then
            m._o.shield_booster = k
         end
      end
   end
end

return atk
