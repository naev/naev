--[[--
Functions for handling vnis in Naev.

@module vnimage
--]]
local portrait = require "portrait"

local vni = {}

local neutral_m = {
   "neutral/male1.webp",
   "neutral/male2.webp",
   "neutral/male3.webp",
   "neutral/male4.webp",
   "neutral/male5.webp",
   "neutral/male6.webp",
   "neutral/male7.webp",
   "neutral/male8.webp",
   "neutral/male9.webp",
   "neutral/male10.webp",
   "neutral/male11.webp",
   "neutral/male12.webp",
   "neutral/male13.webp",
   "neutral/miner1.webp",
   "neutral/thief1.webp",
   "neutral/thief2.webp",
}
local neutral_f = {
   "neutral/female1.webp",
   "neutral/female2.webp",
   "neutral/female3.webp",
   "neutral/female4.webp",
   "neutral/female5.webp",
   "neutral/female6.webp",
   "neutral/female7.webp",
   "neutral/female8.webp",
   "neutral/female9.webp",
   "neutral/female10.webp",
   "neutral/female11.webp",
   "neutral/female12.webp",
   "neutral/female13.webp",
   "neutral/miner2.webp",
   "neutral/thief3.webp",
   "neutral/thief4.webp",
}
function vni.genericMale()
   local p = neutral_m[ rnd.rnd(1,#neutral_m) ]
   return portrait.getFullPath(p), p
end
function vni.genericFemale()
   local p = neutral_f[ rnd.rnd(1,#neutral_f) ]
   return portrait.getFullPath(p), p
end
function vni.generic()
   if rnd.rnd() < 0.5 then
      return vni.genericFemale()
   end
   return vni.genericMale()
end

local sirius_fyrra_m = {
   "sirius/sirius_fyrra_m1.webp",
   "sirius/sirius_fyrra_m2.webp",
   "sirius/sirius_fyrra_m3.webp",
   "sirius/sirius_fyrra_m4.webp",
   "sirius/sirius_fyrra_m5.webp",
}
local sirius_shiara_m = {
   "sirius/sirius_shiara_m1.webp",
   "sirius/sirius_shiara_m2.webp",
   "sirius/sirius_shiara_m3.webp",
   "sirius/sirius_shiara_m4.webp",
   "sirius/sirius_shiara_m5.webp",
}
local sirius_fyrra_f = {
   "sirius/sirius_fyrra_f1.webp",
   "sirius/sirius_fyrra_f2.webp",
   "sirius/sirius_fyrra_f3.webp",
   "sirius/sirius_fyrra_f4.webp",
   "sirius/sirius_fyrra_f5.webp",
}
local sirius_shiara_f = {
   "sirius/sirius_shiara_f1.webp",
   "sirius/sirius_shiara_f2.webp",
   "sirius/sirius_shiara_f3.webp",
   "sirius/sirius_shiara_f4.webp",
   "sirius/sirius_shiara_f5.webp",
}
vni.sirius = {}
function vni.sirius.fyrraMale()
   local p = sirius_fyrra_m[ rnd.rnd(1,#sirius_fyrra_m) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.shiaraMale()
   local p = sirius_shiara_m[ rnd.rnd(1,#sirius_shiara_m) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.serraMale()
   -- TODO
   --local p = sirius_serra_m[ rnd.rnd(1,#sirius_serra_m) ]
   --return portrait.getFullPath(p), p
   return vni.sirius.shiaraMale()
end
function vni.sirius.fyrraFemale()
   local p = sirius_fyrra_f[ rnd.rnd(1,#sirius_fyrra_f) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.shiaraFemale()
   local p = sirius_shiara_f[ rnd.rnd(1,#sirius_shiara_f) ]
   return portrait.getFullPath(p), p
end
function vni.sirius.serraFemale()
   -- TODO
   --local p = sirius_serra_f[ rnd.rnd(1,#sirius_serra_f) ]
   --return portrait.getFullPath(p), p
   return vni.sirius.shiaraFemale()
end
function vni.sirius.fyrra()
   if rnd.rnd() < 0.5 then
      return vni.sirius.fyrraFemale()
   end
   return vni.sirius.fyrraMale()
end
function vni.sirius.Shiara()
   if rnd.rnd() < 0.5 then
      return vni.sirius.ShiaraFemale()
   end
   return vni.sirius.ShiaraMale()
end
function vni.sirius.Serra()
   if rnd.rnd() < 0.5 then
      return vni.sirius.SerraFemale()
   end
   return vni.sirius.SerraMale()
end

return vni
