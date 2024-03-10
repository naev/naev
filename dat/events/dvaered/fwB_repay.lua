--[[
<?xml version='1.0' encoding='utf8'?>
 <event name="Repay General Klank">
  <location>land</location>
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
--Event for Frontier Invasion campaign. The player must help general Klank getting extra-funding (just a text-completion minigame)
--]]

local portrait = require "portrait"
local fw       = require "common.frontier_war"
local fmt      = require "format"
local vn       = require 'vn'

local npc_name = { _("Major Tam"), _("Captain Leblanc"), _("Lieutenant Strafer") }

local portraits = { fw.portrait_tam, fw.portrait_leblanc, fw.portrait_strafer }
local who = rnd.rnd(1,#npc_name)

local npc_desc = {}
npc_desc[1] = _("In the Dvaered military reserved bar, where you now have access, you see the major, apparently busy with a bundle of paper.")
npc_desc[2] = _("Leblanc is alone at a table, typing on her laptop.")
npc_desc[3] = _([[Strafer has put heaps of papers and datapads on his table. He is starring at it as someone who has something tedious to do, but does not want to start yet.]])

local fillIn


-- Each time the player lands, he meets a member of the team
function create()
   evt.npcAdd("pay", npc_name[who], portraits[who], npc_desc[who])
   hook.takeoff("takeoff")
end

function pay()
   if tk.yesno(_("We need more credits from the DHC"), fmt.f(_("I need to proofread the letter we are going to send to the High Command in order to obtain an extra funding of {credits} to continue with our mission. Do you want to help me?"), {credits=fmt.credits(fw.pirate_price)})) then
      fillIn()
   end
end

function fillIn()
   local failed = false

   vn.clear()
   vn.scene()
   local sol = vn.newCharacter( npc_name[who], { image=portrait.getFullPath(portraits[who]) } )
   sol(_([["Due to a data compression problem, some words of our letter have been replaced by random letters. I need to proofread it and re-write the correct words. I let you take care of the letter itself, while I correct the annexes."]]))

   vn.scene()
   vn.na(_([[From: General #ogrjhtgrfd#0, Task force 5613, Special Operations Force

To: General Mmosd, Central Headquarters

Subject: Extra funding request]]))
   vn.menu{
      {_("Tam"), "wrong"},
      {_("Leblanc"), "wrong"},
      {_("Klank"), "r0"},
      {_("Battleaddict"), "wrong"},
   }
   vn.label("r0")
   vn.na(_([[General context: Task force 5613 was created in anticipation of the planned liberalization of Dvaered north-eastern space, commonly referred as "#ooikyujhfbgv#0". It is led by Major General (3rd class) Caribert Klank (violence index: 267), assisted by Major (2nd class) Archibald Tam (violence index: 173).]]))
   vn.menu{
      {_("The Frontier"), "r1"},
      {_("House Sirius"), "wrong"},
      {_("The Milky Way"), "wrong"},
      {_("House Goddard"), "wrong"},
   }
   vn.label("r1")
   vn.na(_([[Particuliar context: Task force 5613 has recently suffered a consequent series of issues that include (but are not limited to) the firm opposition of Lord #owxft#0 (see Annexes A and B), ambushes by private pilots paid by an unidentified organization (see Annexes C, D and E),]]))
   vn.menu{
      {_("Hamelsen"), "wrong"},
      {_("Battleaddict"), "r2"},
      {_("Sauterfeldt"), "wrong"},
      {_("Oftherings"), "wrong"},
   }
   vn.label("r2")
   vn.na(_([[and an unfortunate altercation with House #opokjngf#0 (see Annexes F and G). These issues have been addressed by appropriate means ie. strength, violence and bestiality (see all above mentioned Annexes). However, this comes at a certain pecuniary cost, that had not been accounted for when the initial budget of Task force 5613 was decided]]))
   vn.menu{
      {_("of cards"), "wrong"},
      {_("Sirius"), "wrong"},
      {_("Goddard"), "wrong"},
      {_("Za'lek"), "r3"},
   }
   vn.label("r3")
   vn.na(_([[Request: For accomplishing its mission, Task force 5613 requires the extra amount of 10M credits.

In behalf of all the personal detached to Task force 5613, who work day after day to make House Dvaered closer to its goals, I thank you for your time and consideration, and wish you to remain right, loyal and #obfsrthgb#0.

Major General Klank]]))
   vn.menu{
      {_("brutal"), "wrong"},
      {_("barbaric"), "wrong"},
      {_("lovely"), "wrong"},
      {_("strong"), "r4"},
   }
   vn.label("r4")
   sol = vn.newCharacter( npc_name[who], { image=portrait.getFullPath(portraits[who]) } )
   sol(_([["Very well. Now, I can send it to the Headquarters. Thank you! We should have more missions for you soon."]]))
   vn.done()

   vn.label("wrong")
   sol = vn.newCharacter( npc_name[who], { image=portrait.getFullPath(portraits[who]) } )
   sol(_([["Are you serious? You are obviously writing nonsense. I think you should rest for some time and come back later. See you soon!"]]))
   vn.func( function () failed = true end )
   vn.done()
   vn.run()

   if failed then
      evt.finish(false)
   else
      var.push("dv_pirate_debt", false)
      evt.finish(true)
   end
end

function takeoff()
   evt.finish(false)
end
