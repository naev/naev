notactive = true

function init( p, po )
   local hasunguided = false
   for k,o in ipairs(p:outfits()) do
      if o and o:typeBroad()=="Launcher" then
         local _dps, _disable, _eps, _range, _trackmin, _trackmax, _lockon, _iflockon, guided = o:weapstats()
         if not guided then
            hasunguided = true
            break
         end
      end
   end

   -- Doubles effect
   if hasunguided then
      -- It's multiplicative so we add a bonus such that multiplied by the base bonus we get double the original amount
      po:set( "launch_damage", 110/1.05-100 )
   else
      po:clear()
   end
end
