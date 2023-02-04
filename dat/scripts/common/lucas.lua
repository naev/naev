local vn = require "vn"

local lucas = {}

lucas.lucas = {
   portrait = "lucas.webp",
   image = "lucas.webp",
   name = _("Lucas"),
   colour = nil,
   transition = "hexagon",
}

function lucas.vn_lucas( params )
   return vn.Character.new( lucas.lucas.name,
         tmerge( {
            image=lucas.lucas.image,
            color=lucas.lucas.colour,
         }, params) )
end

function lucas.log( text )
   shiplog.create( "lucas", _("Lucas"), _("Lucas") )
   shiplog.append( "lucas", text )
end

return lucas
