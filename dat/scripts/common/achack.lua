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

return achack
