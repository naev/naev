--[[
-- Common data for the Frontier War campaign
--]]
local ai_setup = require "ai.core.setup"

local fw = {}

-- All the rewards:
fw.credits_00 = 400e3 -- Paid in Gauss Guns
fw.credits_01 = 400e3
fw.credits_02 = 500e3
fw.credits_03 = 700e3
fw.credits_04 = 700e3

fw.pirate_price = 10e6

fw.wlrd_planet = "Buritt"

fw.lords = {
   _("Lord Jim"),
   _("Lady Bitterfly"),
   _("Lady Pointblank"),
   _("Lord Chainsaw"),
   _("Lord Painbishop"),
   _("Lord Kriegsreich Hundertfeuer"),
   _("Lady Blackswan"),
   _("Lady Killington"),
   _("Lord Richthofen"),
   _("Lady Dewinter"),
   _("Lord Easytrigger"),
   _("Lady Sainte-Beuverie"),
   _("Lord Louverture"),
   _("Lord Abdelkiller"),
   _("Lady Proserpina")
}

-- Character's portraits. TODO: create unique portraits for them
fw.portrait_tam     = "major_tam.webp" -- Major Tam: main mission giver of this campaign
fw.portrait_leblanc = "dvaered/dv_military_f8.webp" -- Captain Leblanc: secondary mission giver and partner for the second half of the campaign
fw.portrait_klank   = "dvaered/dv_general1.webp"    -- Genenral Klank: boss of Major Tam, has few interactions with the player, excepted for the last mission
fw.portrait_hamfresser = "dvaered/dv_military_m7.webp" -- Captain Hamfresser: player's sidekick for some of the missions
fw.portrait_strafer = "lieutenant_stafner.webp" -- Lieutenant Stafer: partner of the player for the first half of the campaign, and subordinate of Captain Leblanc
fw.portrait_nikolov = "sergeant_nikolov.webp" -- Sergeant Nikolov: second in command of Captain Hamfresser
fw.portrait_therus  = "dvaered/dv_military_f1.webp" -- Caporal Therus: subordinate of Captain Hamfresser
fw.portrait_tronk   = "dvaered/dv_military_m5.webp" -- Private Tronk: subordinate of Captain Hamfresser


-- When an escort is hailed
function fw.escort_hailed( pilot )
   local c = tk.choice(_("Instructions to escort"), _("What do you want the escort to do?"), _("Attack nearby enemies"), _("Follow me"), _("Never mind"))
   if c == 1 then
      pilot:control(false)
   elseif c == 2 then
      pilot:control(true)
      pilot:taskClear()
      pilot:follow( player.pilot(), true )
   end
   player.commClose()
end

-- Test if an element is in a list
function fw.elt_inlist( elt, list )
   for i, elti in ipairs(list) do
      if elti == elt then
         return i
      end
   end
   return 0
end

-- Gets someone to make a message (useful for hook.timer)
function fw.message( arg )
   local pilot = arg.pilot
   local msg = arg.msg
   if pilot:exists() then
      pilot:broadcast( msg )
   end
end

-- Equips a Vendetta with full mace rockets
function fw.equipVendettaMace( pilot )
   pilot:cargoRm( "all" )
   pilot:outfitRm("all")
   pilot:outfitRm("cores")
   pilot:outfitAdd("S&K Light Combat Plating")
   pilot:outfitAdd("Tricon Zephyr II Engine")
   pilot:outfitAdd("Milspec Orion 3701 Core System")
   pilot:outfitAdd("Shield Capacitor I")
   pilot:outfitAdd("Milspec Impacto-Plastic Coating")
   pilot:outfitAdd("TeraCom Mace Launcher", 6)
   ai_setup.setup(pilot)

   pilot:setHealth(100,100)
   pilot:setEnergy(100)
   pilot:setFuel(true)
end

local function make_fct ()
   local f1 = faction.dynAdd( "Dvaered", "fw_warlords", _("Warlords"), {
      ai="baddie",
      clear_enemies=true,
      clear_allies=true,
      player=-10,
   } )
   local f2 =  faction.dynAdd( "Dvaered", "fw_dhc", _("Thugs"), {
      longname=_("Dvaered High Command"),
      ai="baddie_norun",
      player=10,
      -- Don't clear enemies/allies so they behave like Dvaered
   } )
   f2:dynEnemy( f1 )
   return f1, f2
end

function fw.fct_warlords ()
   local f, _f = make_fct()
   return f
end

function fw.fct_dhc ()
   local _f, f = make_fct()
   return f
end

return fw
