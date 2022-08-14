local atk = {}

local o_shield_booster = outfit.get("Emergency Shield Booster")

function atk.setup( p )
   local added = false

   -- Clean up old stuff
   local m = p:memory()
   m._o = nil
   local o = {}

   -- Check out what interesting outfits there are
   for k,v in ipairs(p:outfits()) do
      if v then
         if v == o_shield_booster then
            o.shield_booster = k
            added = true
         end
      end
   end

   -- Actually added an outfit, so we set the list
   if added then
      m._o = o
   end
end

return atk
