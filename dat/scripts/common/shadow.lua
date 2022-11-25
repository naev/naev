--[[

   Shadow Common Functions

--]]
local shadow = {}

function shadow.addLog( text )
   shiplog.create( "shadow", _("Shadow"), _("Shadow") )
   shiplog.append( "shadow", text )
end

shadow.rewards = {
   shadowrun = 400e3,
   shadowvigil = 700e3,
   darkshadow = 1e6,
}

local function make_fct ()
   local f1 = faction.dynAdd( "Mercenary", "shadow_fourwinds", _("Four Winds"), {
      clear_allies=true,
      clear_enemies=true,
      ai="baddie",
   })
   local f2 = faction.dynAdd( "Pirate", "shadow_pirates", _("Pirate"), {
      clear_allies=true,
      clear_enemies=true,
      ai="baddie_norun",
      player=-100,
   })
   local f3 = faction.dynAdd( "Mercenary", "shadow_rogue", _("Four Winds"), {
      clear_allies=true,
      clear_enemies=true,
      ai="baddie",
   })
   local f4 = faction.dynAdd( "Independent", "shadow_diplomatic", _("Diplomatic"), {
      clear_allies=true,
      clear_enemies=true,
      ai="baddie",
      colour="blue",
   })
   f1:dynEnemy(f2)
   f1:dynEnemy(f3)
   f4:dynEnemy(f2)
   return f1, f2, f3, f4
end

function shadow.fct_fourwinds ()
   local f1, _f2, _f3, _f4 = make_fct()
   return f1
end

function shadow.fct_pirates ()
   local _f1, f2, _f3, _f4 = make_fct()
   return f2
end

function shadow.fct_rogues ()
   local _f1, _f2, f3, _f4 = make_fct()
   return f3
end

function shadow.fct_diplomatic ()
   local _f1, _f2, _f3, f4 = make_fct()
   return f4
end

return shadow
