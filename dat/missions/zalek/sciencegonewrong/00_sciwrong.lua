--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The one with the Shopping">
 <unique />
 <priority>4</priority>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Za'lek</faction>
 <cond>return require("misn_test").reweight_active()</cond>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Mission: The one with the Shopping

   Description: A Za'lek scientist asks the player to fetch some raw material that he needs to build his prototype.
   Multiple systems have to be visited and materials acquired from contact persons in the bar.

   Difficulty: Easy to Medium? Depending on whether the player lingers around too much and gets caught.

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]

local fmt = require "format"
require "proximity"
local sciwrong = require "common.sciencegonewrong"
local equipopt = require "equipopt"
local vn = require 'vn'
local ccomm = require "common.comm"
local lmisn = require "lmisn"

local adm1, lance1, lance2 -- Non-persistent state
local spwn_police -- Forward-declared functions

-- set mission variables
mem.t_sys = {}
mem.t_pla = {}
mem.t_pla[1], mem.t_sys[1] = spob.getS("Vilati Vilata")
mem.t_pla[2], mem.t_sys[2] = spob.getS("Waterhole's Moon")
-- t_x[3] is empty bc it depends on where the mission will start finally. (To be set in mission.xml and then adjusted in the following campaign missions)
--mem.t_pla[3], mem.t_sys[3] = spob.getS("Gastan")
local pho_mny = 50e3
local reward = 1e6

local trader1 = {
   name = _("Trader"),
   portrait = "neutral/scientist3.webp",
   image = "gfx/portraits/neutral/scientist3.webp",
   desc = _("A scientist conspicuously sits in the corner. Perhaps he might be the person you're supposed to get this stuff for."),
}
local trader2 = {
   name = _("Contact Person"),
   portrait = "neutral/unique/dealer.webp",
   image = "gfx/portraits/neutral/unique/dealer.webp",
   desc = _("You see a shifty-looking dealer of some kind. Maybe he has what you're looking for."),
}

function create ()
   -- Variable set up and clean up
   var.push( sciwrong.center_operations, spob.cur():nameRaw() )
   mem.t_pla[3], mem.t_sys[3] = sciwrong.getCenterOperations()

   -- Spaceport bar stuff
   misn.setNPC( _("A scientist"), sciwrong.geller.portrait, _("You see a scientist talking to various pilots. Perhaps you should see what he's looking for.") )
end
function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()

   -- Mission details:
   geller(_([["Oh, hello! You look like you're a pilot; is that right? I've got a job for you. Allow me to introduce myself; my name is Dr. Geller, and I am on the brink of revolutionizing science! I've basically already done it; there's just some minor fiddling to do. Would you like to help me out? I just need you to find some samples that I can study."]]))
   vn.menu( {
      { _("Help them out"), "accept" },
      { _("Decline to help"), "decline" },
   } )

   vn.label("decline")
   geller(_("I guess you don't care for science…"))
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   geller(_([["Excellent! Here is the list." He hands you a memory chip and turns away even before you can say anything and without giving you any cash to actually do his shopping. Once you check the list, you find that it contains not only a list of materials he needs, but also information where to retrieve them, as well as a list of traders to contact.]]))
   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   misn.osdCreate(_("The one with the Shopping"), {
      fmt.f(_("Go to the {sys} system and talk to the trader on {pnt}"), {sys=mem.t_sys[1], pnt=mem.t_pla[1]}),
   })
   misn.setDesc(_("You've been hired by Dr. Geller to collect some materials he urgently needs for his research."))
   misn.setTitle(_("The one with the Shopping"))
   misn.setReward(_("The gratitude of science and a bit of compensation"))
   mem.misn_mark = misn.markerAdd( mem.t_pla[1], "high" )
   mem.talked = false
   mem.lhook1 =  hook.land("land1", "land")
end

function land1()
   if spob.cur() == mem.t_pla[1] and not mem.traded1 then
      mem.bar1pir1 = misn.npcAdd("first_trd", trader1.name, trader1.portrait, trader1.desc )
   elseif spob.cur() == mem.t_pla[1] and mem.talked and mem.traded1 then
      mem.bar1pir1 = misn.npcAdd("third_trd", trader1.name, trader1.portrait, trader1.desc )
   end
end

function land2()
   if spob.cur() == mem.t_pla[2] and mem.talked and not mem.traded1 then
      mem.bar2pir1 = misn.npcAdd("second_trd", trader2.name, trader2.portrait, trader2.desc )
   end
end

-- first trade: send player 2 2nd system, if he goes back here, tell them to get going...
function first_trd()
   vn.clear()
   vn.scene()
   local trader = vn.newCharacter( trader1.name, {image=trader1.image} )
   vn.transition()

   if mem.talked then
      trader(_([["What are you still doing here? No phosphine, no trade."]]))
   else
      trader(_([["With what can I help you, my friend?" says the shifty figure. You hand him the memory chip the scientist handed you.]]))
      trader(fmt.f(_([["Of course I have what you need. I'll trade it for some 3t phosphine. You can find it on {pnt} in the {sys} system."]]), {pnt=mem.t_pla[2], sys=mem.t_sys[2]}))
      vn.func( function ()
         mem.talked = true
      end )
   end

   vn.run()

   misn.osdCreate(_("The one with the Shopping"), {
      fmt.f(_("Go to the {sys} system and talk to the contact person on {pnt}"), {sys=mem.t_sys[2], pnt=mem.t_pla[2]}),
   })

   misn.markerMove(mem.misn_mark, mem.t_pla[2])

   if not mem.lhook2 then
      mem.lhook2 = hook.land("land2", "land")
   end
end

-- 2nd trade: Get player the stuff and make them pay, let them be hunted by the police squad
function second_trd()
   local traded = false
   vn.clear()
   vn.scene()
   local trader = vn.newCharacter( trader2.name, {image=trader2.image} )
   vn.transition()

   trader(fmt.f(_([["You approach the dealer and explain what you are looking for. He raises his eyebrow. "It will be {credits}. But if you get caught by the authorities, you're on your own. Far as I'm concerned, I never saw you. Deal?"]]), {credits=fmt.credits(pho_mny)}))
   vn.menu{
      {fmt.f(_([[Pay {credits}.]]),{credits=fmt.credits(pho_mny)}), "accept"},
      {_("Decline."), "decline"},
   }

   vn.label("decline")
   trader(_([["Then we have nothing to to discuss."]]))
   vn.done()

   vn.label("broke")
   trader(_([["You don't have enough money. Stop wasting my time."]]))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.credits() < pho_mny then
         vn.jump("broke")
         return
      end
      player.pay(-pho_mny)
      local c = commodity.new(N_("Phosphine"), N_("A colourless, flammable, poisonous gas."))
      mem.carg_id = misn.cargoAdd(c, 3)
      traded = true
   end )
   trader(_([["Pleasure to do business with you."]]))
   vn.run()

   if not traded then
      return
   end

   misn.osdCreate(_("The one with the Shopping"), {
      fmt.f(_("Return to the {sys} system to the trader on {pnt}"), {sys=mem.t_sys[1], pnt=mem.t_pla[1]}),
   })
   -- create hook that player will be hailed by authorities bc of toxic materials
   misn.markerMove(mem.misn_mark, mem.t_pla[1])
   hook.rm(mem.lhook2)
   hook.enter("sys_enter")
   mem.traded1 = true
   misn.npcRm(mem.bar2pir1)
end

-- 3rd trade: Get the stuff the scientist wants
function third_trd()
   vn.clear()
   vn.scene()
   local trader = vn.newCharacter( trader1.name, {image=trader1.image} )
   vn.transition()
   trader(_([["Ah, yes indeed," he says as he inspects a sample in front of him. "That will do. And here, as promised: a piece of a ghost ship from the Nebula. 100% authentic! At least, according to my supplier."]]))
   vn.run()

   misn.npcRm(mem.bar1pir1)
   misn.cargoRm(mem.carg_id)
   player.msg(mem.t_sys[3]:name())
   misn.osdCreate(_("The one with the Shopping"), {
      fmt.f(_("Return to the {sys} system and deliver to Dr. Geller on {pnt}"), {sys=mem.t_sys[3], pnt=mem.t_pla[3]}),
   })

   misn.markerMove(mem.misn_mark, mem.t_pla[3])

   hook.rm(mem.lhook1)

   mem.traded2 = true
   mem.lhook1 = hook.land("fnl_ld", "land")
end

-- final land: let the player land and collect the reward
function fnl_ld ()
   if spob.cur() == mem.t_pla[3] and mem.traded2 then
      vn.clear()
      vn.scene()
      local geller = vn.newCharacter( sciwrong.vn_geller() )
      vn.transition()
      geller(_([[Dr. Geller looks up at you as you approach. "Do you have what I was looking for?" You present the ghost ship piece and his face lights up. "Yes, that's it! Now I can continue my research. I've been looking everywhere for a sample!" You ask him about the so-called ghost ships. He seems amused by the question. "Some people believe in ridiculous nonsense related to this. There is no scientific explanation for the origin of these so-called ghost ships yet, but I think it has to do with some technology involved in the Incident. Hard to say exactly what, but hey, that's why we do research!"]]))
      geller(_([[As he turns away, you audibly clear your throat, prompting him to turn back to you. "Oh, yes, of course you want some payment for your service. My apologies for forgetting." He hands you a credit chip with your payment. "I might need your services again in the future, so do stay in touch!"]]))
      vn.sfxVictory()
      vn.func( function ()
         player.pay(reward)
      end )
      vn.na(fmt.reward(reward))
      vn.run()

      sciwrong.addLog( fmt.f( _([[You helped Dr. Geller at {pnt} in the {sys} system to obtain a "ghost ship piece" for his research. When you asked about these so-called ghost ships, he seemed amused. "Some people believe in ridiculous nonsense related to this. There is no scientific explanation for the origin of these so-called ghost ships yet, but I think it has to do with some technology involved in the Incident. Hard to say exactly what, but hey, that's why we do research!"]]), {pnt=mem.t_pla[3], sys=mem.t_sys[3] } ) )
      misn.finish(true)
   end
end
-- when the player takes off the authorities will want them
function sys_enter ()
   if system.cur() == mem.t_sys[2] then
      hook.timer(7.0, "call_the_police")
   end
end
function call_the_police ()
   spwn_police()

   vn.reset()
   vn.scene()
   local p = ccomm.newCharacter( vn, adm1 )
   vn.transition()
   p(_([["We have reason to believe you are carrying controlled substances without a proper license. Please stop your ship and prepare to be boarded."]]))
   p(_([["Stand down for inspection."]]))
   vn.run()

   if mem.boardh == nil then
      mem.boardh = hook.pilot(player.pilot(), "disable", "go_board")
   end
end

function spwn_police () -- Get called to Waterhole
   local spwnsys = system.get("Holly")
   lance1 = pilot.add( "Empire Lancelot", "Empire", spwnsys, nil, {naked=true} )
   lance2 = pilot.add( "Empire Lancelot", "Empire", spwnsys, nil, {naked=true} )
   adm1 = pilot.add( "Empire Admonisher", "Empire", spwnsys, nil, {naked=true} )

   local eparams = {
      damage = 0, -- disable weapons only
      outfits_add = { -- Give them disable weapons (Emprie don't have by default!)
         "Ion Cannon",
         "Heavy Ion Cannon",
         "TeraCom Medusa Launcher",
         "EMP Grenade Launcher",
      },
   }

   -- Re-outfit the ships to use disable weapons.
   equipopt.empire( lance1, eparams )
   equipopt.empire( lance2, eparams )
   equipopt.empire( adm1, eparams )

   -- Hostile
   lance1:setHostile(true)
   lance2:setHostile(true)
   adm1:setHostile(true)
end
-- move to the player ship

function go_board ()
   if adm1:exists() then
      adm1:control()
      adm1:setHostile(false)
      adm1:moveto(player.pos())
      mem.admho = hook.pilot(adm1, "idle", "fine_vanish")
   end
   if lance1:exists() then
      lance1:control()
      lance1:setHostile(false)
      lance1:moveto(player.pos())
      mem.l1ho = hook.pilot(lance1, "idle", "fine_vanish")
   end
   if lance2:exists() then
      lance2:control()
      lance2:setHostile(false)
      lance2:moveto(player.pos())
      mem.l2ho = hook.pilot(lance2, "idle", "fine_vanish")
   end
end
-- display msgs and have the ships disappear and fail the mission...
function fine_vanish ()
   vn.reset()
   vn.scene()
   local p
   if adm1:exists() then
      p = ccomm.newCharacter( vn, adm1 )
   elseif lance1:exists() then
      p = ccomm.newCharacter( vn, lance1 )
   elseif lance2:exists() then
      p = ccomm.newCharacter( vn, lance2 )
   end
   vn.transition()

   local fine = 100e3
   p(_([["You are accused of violating regulations on the transport of toxic materials. Your ship will be searched now. If there are no contraband substances, we will be out of your hair in just a moment."]]))
   p(fmt.f(_([[The inspectors search through your ship and cargo hold. It doesn't take long for them to find the phosphine; they confiscate it and fine you {credits}.]]), {credits=fmt.credits(fine)}))
   vn.run()

   if player.credits() > fine then
      player.pay(-fine)
   else
      player.pay(-player.credits())
   end

   misn.cargoRm(mem.carg_id)
   if adm1:exists() then
      adm1:hyperspace()
   end
   if lance1:exists() then
      lance1:hyperspace()
   end
   if lance2:exists() then
      lance2:hyperspace()
   end
   lmisn.fail(_("the phosphine was confiscated!"))
end
