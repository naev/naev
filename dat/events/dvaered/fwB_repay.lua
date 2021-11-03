--[[
<?xml version='1.0' encoding='utf8'?>
 <event name="Repay General Klank">
  <trigger>land</trigger>
  <chance>100</chance>
  <cond>var.peek("dv_pirate_debt") == true</cond>
  <notes>
   <campaign>Frontier Invasion</campaign>
   <done_evt name="Betray General Klank">If you don't betray</done_evt>
   <requires name="General Klank wants his 10M back"/>
  </notes>
 </event>
 --]]
--[[
--Event for Frontier Invasion campaign. The player must repay general Klank
--]]

local fw = require "common.frontier_war"
local fmt = require "format"

local npc_name = { _("Major Tam"), _("Captain Leblanc"), _("Lieutenant Strafer") }

local portraits = { fw.portrait_tam, fw.portrait_leblanc, fw.portrait_strafer }

local npc_desc = {}
npc_desc[1] = _("In the Dvaered military reserved bar, where you now have access, you see the major, sitting at a table with a few High Command officers that you don't happen to know.")
npc_desc[2] = _("Leblanc is relaxing at a table, with a group of Dvaered pilots.")
npc_desc[3] = _([[Strafer plays cards with a few other soldiers. Of course, they don't use real money because "Dvaered Warriors never seize money of others, except the dead bodies of defeated enemies." As it is written on the Dvaered etiquette manual Strafer gave to you recently.]])

-- Each time the player lands, he meets a member of the team who proposes to pay
function create()
   local who = rnd.rnd(1,#npc_name)
   evt.npcAdd("pay", npc_name[who], portraits[who], npc_desc[who])
   hook.takeoff("takeoff")
end

function pay()
   if tk.yesno(_("Do you want to repay DHC?"), fmt.f(_("You still owe {credits} to the High Command. Do you want to repay now?"), {credits=fmt.credits(fw.pirate_price)})) then
      if player.credits() >= fw.pirate_price then
         player.pay(-fw.pirate_price)
         shiplog.create( "frontier_war", _("Frontier War"), _("Dvaered") )
         shiplog.append( "frontier_war", _("You repaid the Dvaered High Command. For now, your collaboration with House Dvaered has not been lucrative AT ALL.") )
         var.push("dv_pirate_debt", false)
         evt.finish(true)
      else
         tk.msg(_("Do you want to repay DHC?"),_("You don't have enough money."))
         evt.finish(false)
      end
   end
end

function takeoff()
   evt.finish(false)
end
