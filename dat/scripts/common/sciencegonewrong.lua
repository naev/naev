local vn = require "vn"

local sciwrong = {}

sciwrong.center_operations = "sciwrong_center_operations"

sciwrong.geller = {
   name = _("Dr. Geller"),
   portrait = "zalek/unique/geller.webp",
   image = "gfx/portraits/zalek/unique/geller.webp",
   colour = nil,
}

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

function sciwrong.vn_geller( params )
   return vn.Character.new( sciwrong.geller.name,
      tmerge( {
         image=sciwrong.geller.image,
         color=sciwrong.geller.colour,
      }, params) )
end

return sciwrong
