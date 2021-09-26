--[[
-- Common data for the Frontier War campaign
--]]

-- All the rewards:
credits_00 = 100e3
credits_01 =  50e3
credits_02 = 200e3
credits_03 =  50e3
credits_04 = 150e3

pirate_price = 10e6

wlrd_planet = "Buritt"

instr_title = _("Instructions to escort")
instr_text = _("What do you want the escort to do?")
in_free = _("Attack nearby enemies")
in_follow = _("Follow me")
in_nevermind = _("Never mind")

lords = { _("Lord Jim"),
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
          _("Lady Proserpina") }


-- Character's portraits. TODO: create unique portraits for them
portrait_tam        = "dvaered/dv_military_m3.webp" -- Major Tam: main mission giver of this campaign
portrait_leblanc    = "dvaered/dv_military_f8.webp" -- Captain Leblanc: secondary mission giver and partner for the second half of the campaign
portrait_klank      = "dvaered/dv_general1.webp"    -- Genenral Klank: boss of Major Tam, has few interactions with the player, excepted for the last mission
portrait_hamfresser = "dvaered/dv_military_m7.webp" -- Captain Hamfresser: player's sidekick for some of the missions
portrait_strafer    = "dvaered/dv_military_m1.webp" -- Lieutenant Stafer: partner of the player for the first half of the campaign, and subordinate of Captain Leblanc
portrait_nikolov    = "dvaered/dv_military_f7.webp" -- Sergeant Nikolov: second in command of Captain Hamfresser
portrait_therus     = "dvaered/dv_military_f1.webp" -- Caporal Therus: subordinate of Captain Hamfresser
portrait_tronk      = "dvaered/dv_military_m5.webp" -- Private Tronk: subordinate of Captain Hamfresser


-- When an escort is hailed
function escort_hailed( pilot )
   local c = tk.choice(instr_title, instr_text, in_free, in_follow, in_nevermind)
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
function elt_inlist( elt, list )
   for i, elti in ipairs(list) do
      if elti == elt then
         return i
      end
   end
   return 0
end

-- Gets someone to make a message (useful for hook.timer)
function message( arg )
   local pilot = arg.pilot
   local msg = arg.msg
   if pilot:exists() then
      pilot:broadcast( msg )
   end
end

-- Equips a Vendetta with full mace rockets
function equipVendettaMace( pilot )
   pilot:cargoRm( "all" )
   pilot:outfitRm("all")
   pilot:outfitRm("cores")
   pilot:outfitAdd("S&K Light Combat Plating")
   pilot:outfitAdd("Tricon Zephyr II Engine")
   pilot:outfitAdd("Milspec Orion 3701 Core System")
   pilot:outfitAdd("Shield Capacitor I")
   pilot:outfitAdd("Milspec Impacto-Plastic Coating")
   pilot:outfitAdd("Unicorp Mace Launcher", 6)

   pilot:setHealth(100,100)
   pilot:setEnergy(100)
   pilot:setFuel(true)
end
