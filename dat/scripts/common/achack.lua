local vn = require "vn"
local portrait = require "portrait"

local achack = {}

local function make_fct ()
   local f1 = faction.dynAdd( "Sirius", "achack_sirius", _("Sirius"), {
      ai="sirius_norun",
      clear_enemies=true,
      clear_allies=true,
   } )
   local f2 =  faction.dynAdd( nil, "achack_thugs", _("Thugs"), {
      ai="baddie_norun",
   } )
   f2:dynEnemy( f1 )
   return f1, f2
end

function achack.fct_sirius ()
   local f, _f = make_fct()
   return f
end

function achack.fct_thugs ()
   local _f, f = make_fct()
   return f
end

achack.harja = {
   name = _("Harja"),
   portrait = "harja.webp",
   image = portrait.getFullPath("sirius/unique/harja.webp"),
}
achack.joanne = {
   name = _("Joanne"),
   portrait = "sirius/uinque/joanne.webp",
   image = portrait.getFullPath("sirius/uinque/joanne.webp"),
}

function achack.vn_harja( params )
   return vn.Character.new( achack.harja.name,
         tmerge( {
            image = achack.harja.image,
         }, params) )
end

function achack.vn_joanne( params )
   return vn.Character.new( achack.joanne.name,
         tmerge( {
            image = achack.joanne.image,
         }, params) )
end

return achack
