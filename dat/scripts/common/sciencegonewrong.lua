local sciwrong = {}

sciwrong.center_operations = "sciwrong_center_operations"
   
function sciwrong.addLog( text )
   shiplog.create( "zlk_sciwrong", _("Science Gone Wrong"), _("Za'lek") )
   shiplog.append( "zlk_sciwrong", text )
end

function sciwrong.getCenterOperations()
   local pnt = var.peek( sciwrong.center_operations )
   if not pnt then
      warn("Science Gone Wrong: center of operations not set!")
      return planet.cur(), system.cur() -- Shouldn't happen
   end
   return planet.get( pnt )
end

return sciwrong
