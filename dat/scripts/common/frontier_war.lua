--[[
-- Common data for the Frontier War campaign
--]]
local ai_setup = require "ai.core.setup"
local vn       = require "vn"
local portrait = require "portrait"

local fw = {}

-- All the rewards:
fw.credits_00 = 400e3 -- Paid in Gauss Guns
fw.credits_01 = 400e3
fw.credits_02 = 500e3
fw.credits_03 = 700e3
fw.credits_04 = 700e3

fw.pirate_price = 1e6

fw.wlrd_planet = "Buritt"

-- CHARACTERS --
-- Major Archibald Tam: main mission giver of this campaign
fw.tam = {
   portrait = "major_tam",
   image = "major_tam",
   name = _("Major Tam"),
   colour = nil,
}
-- Captain Leblanc: secondary mission giver and partner for the second half of the campaign
fw.leblanc = {
   portrait = "dvaered/unique/captain_leblanc",
   image = "dvaered/unique/captain_leblanc",
   name = _("Captain Leblanc"),
   colour = nil,
}
-- Genenral Caribert Klank: boss of Major Tam, has few interactions with the player, excepted for the last mission
fw.klank = {
   portrait = "dvaered/unique/general_klank",
   image = "dvaered/unique/general_klank",
   name = _("General Klank"),
   colour = nil,
}
-- Captain Leopold Hamfresser: player's sidekick for some of the missions
fw.hamfresser = {
   portrait = "dvaered/dv_military_m7", -- TODO: portrait needed
   image = portrait.getFullPath("dvaered/dv_military_m7"),
   name = _("Captain Hamfresser"),
   colour = nil,
}
-- Lieutenant Stafer: partner of the player for the first half of the campaign, and subordinate of Captain Leblanc
fw.strafer = {
   portrait = "lieutenant_stafner",
   image = "lieutenant_stafner",
   name = _("Lieutenant Stafer"),
   colour = nil,
}
-- Sergeant Nikolov: second in command of Captain Hamfresser
fw.nikolov = {
   portrait = "sergeant_nikolov",
   image = "sergeant_nikolov",
   name = _("Sergeant Nikolov"),
   colour = nil,
}
-- Caporal Therus: subordinate of Captain Hamfresser, medic
fw.therus = {
   portrait = "dvaered/dv_military_f1",
   image = portrait.getFullPath("dvaered/dv_military_f1"),
   name = _("Caporal Therus"),
   colour = nil,
}
-- Private Tronk: subordinate of Captain Hamfresser
fw.tronk = {
   portrait = "dvaered/dv_military_m5",
   image = portrait.getFullPath("dvaered/dv_military_m5"),
   name = _("Private Tronk"),
   colour = nil,
}
-- Colonel Hamelsen: main antagonist
fw.hamelsen = {
   portrait = "dvaered/dv_military_f5", -- TODO: portrait needed
   image = portrait.getFullPath("dvaered/dv_military_f5"),
   name = _("Colones Hamelsen"),
   colour = nil,
}

-- Generic function for everyone
function fw.vn_char( character, params )
   return vn.Character.new( character.name,
         tmerge( {
            image=character.image,
            colour=character.colour,
         }, params) )
end

-- Character's portraits. TODO: remove
fw.portrait_tam     = "major_tam" -- Major Tam: main mission giver of this campaign
fw.portrait_leblanc = "dvaered/unique/captain_leblanc" -- Captain Leblanc: secondary mission giver and partner for the second half of the campaign
fw.portrait_klank   = "dvaered/unique/general_klank"    -- Genenral Caribert Klank: boss of Major Tam, has few interactions with the player, excepted for the last mission
fw.portrait_hamfresser = "dvaered/dv_military_m7" -- Captain Hamfresser: player's sidekick for some of the missions
fw.portrait_strafer = "lieutenant_stafner" -- Lieutenant Stafer: partner of the player for the first half of the campaign, and subordinate of Captain Leblanc
fw.portrait_nikolov = "sergeant_nikolov" -- Sergeant Nikolov: second in command of Captain Hamfresser
fw.portrait_therus  = "dvaered/dv_military_f1" -- Caporal Therus: subordinate of Captain Hamfresser
fw.portrait_tronk   = "dvaered/dv_military_m5" -- Private Tronk: subordinate of Captain Hamfresser
--

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
   pilot:outfitAdd("S&K Skirmish Plating",2)
   pilot:outfitAdd("Tricon Zephyr Engine",2)
   pilot:outfitAdd("Milspec Orion 2301 Core System",2)
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
