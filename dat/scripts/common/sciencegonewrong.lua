local sciwrong = {}

sciwrong.center_operations = "sciwrong_center_operations"

function sciwrong.addLog( text )
   shiplog.create( "zlk_sciwrong", _("Science Gone Wrong"), _("Za'lek") )
   shiplog.append( "zlk_sciwrong", text )
end

function sciwrong.getCenterOperations()
   local pnt = var.peek( sciwrong.center_operations )
   if not pnt then
      -- Probably old save, just set to Gastan like before
      return spob.getS("Gastan")
   end
   return spob.getS( pnt )
end

return sciwrong
