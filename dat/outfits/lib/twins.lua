
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

local old_init = init
init=function ( p, po)
   init=old_init
   onoutfitchange( p, po)
end

local old_onadd=onadd
onadd=function( p, po)
   if p and po then
      if old_onadd then
         old_onadd( p, po)
      end
      print (p)
      --if not p:outfitHasSlot('engines_secondary') then
      --   if p:outfitRmSlot('engines') and p==player.pilot() then
      --      player.outfitAdd(po:outfit())
      --   end
      --end
   end
end

