notactive = true
local range = 3000

-- Init function run on creation
local t
function init( p, _po )
   t = 0
   p:effectAdd("Shield Aura", math.huge)
end
function update( p, _po, dt )
   t = t + dt
   if t < 1 then
      return
   end

   local mod = p:shipstat("ew_detect",true)

   t = t-1
   for k,a in ipairs(p:getAllies( range*mod, nil, nil, nil, true )) do
      a:effectAdd("Shield Aura")
   end
end
