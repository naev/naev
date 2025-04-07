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

   -- effect goes from 3 to 7
   if hasunguided then
      po:set( "launch_damage", 4 )
   else
      po:clear()
   end
end
