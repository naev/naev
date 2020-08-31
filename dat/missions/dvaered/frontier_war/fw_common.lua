--[[
-- Common data for the Frontier War campaign
--]]

-- All the rewards:
credits_00 = 100000
credits_01 = 50000
credits_02 = 200000
credits_03 = 50000

pirate_price = 10000000

wlrd_planet = "Buritt"

instr_title = _("Instructions to escort")
instr_text = _("What do you want the escort to do?")
in_free = _("Attack nearby enemies")
in_follow = _("Follow me")
in_nevermind = _("Nevermind")

-- Decides wether the player is stronger than a corvette
function playerMoreThanCorvette()
   local playerclass = player.pilot():ship():class()
   return (playerclass == "Destroyer" or playerclass == "Cruiser" or playerclass == "Carrier" or playerclass == "Armoured Transport")
end

-- When an escort is hailed
function escort_hailed(pilot)
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
