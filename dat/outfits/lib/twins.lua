
local prvdesc=descextra
descextra=function ( p, o, po)
   return prvdesc( p, o, po) .. "\n#b".._("This outfit only works when two are equipped at the same time..").."#0"
end

function onoutfitchange( p, po )
   if p and po then
      local count = 0
      local mismatch = false
      for _k,o in ipairs(p:outfits()) do
         if o and o:type() == "Core Systems (Engine)" then
            if o == po:outfit() then
               count = count+1
            else
               mismatch = true
            end
         end
      end
      setworkingstatus( p, po, count == 2 and mismatch~=true)
   end
end

local oldinit=init

init=function( p, po)
   init=oldinit
   onoutfitchange( p, po)
end

