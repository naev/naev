--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Totoran Station Events">
 <location>land</location>
 <chance>100</chance>
 <spob>Totoran</spob>
</event>
--]]

--[[
-- Event handling NPCs and such at Totoran
--]]

local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"
local gauntlet = require 'common.gauntlet'

-- Unsaved global tables
local bgnpcs

local spectator_names = {
   _("Spectator"),
   _("Aficionado"),
   _("Gauntlet Fan"),
}
local spectator_descriptions = {
   _("A person enjoying their time at the station."),
   _("An individual taking a break from viewing the action."),
   _("A spectator that looks strangely out of place."),
   _("A person enjoying some drinks."),
}
local spectator_messages = {
   _([["Sometimes when watching some of the competitions, I forget it is all just virtual reality."]]),
   _([["The realism of virtual reality here is impressive! It almost feels like it's real!"]]),
   function () return fmt.f(
      _([["I came all the way from {pnt} to be here! We don't have anything like this back at home."]]),
      {pnt=spob.get( {faction.get("Za'lek"), faction.get("Empire"), faction.get("Soromid")} )} -- No Dvaered
   ) end,
   _([["The Dvaered sure know how to put on a good show. I love seeing it rain Mace Rockets!"]]),
   _([["It's a shame that they require you to own the ship you want to use to enter the virtual reality competitions. I would love to try fly one of those majestic Dvaered Goddards."]]),
   _([["I like watching the competitions between fighters, it's incredible all the moves they can pull off."]]),
   _([["I tried to compete in my Llama, but it doesn't stand a chance against even a single Hyena."]]),
   _([["I was always told that Dvaered technology was primitive, but the virtual reality of the Crimson Gauntlet is incredible!"]]),
   _([["There's nothing quite like seeing two capital ships duke it out. I love watching railguns blasting away."]]),
   _([["I used to think the Za'lek virtual games were great, but this is so much better!"]]),
}
local pilot_names = {
   _("Gauntlet Pilot"),
   _("Resting Pilot"),
   _("Tired Pilot"),
}
local pilot_descriptions = {
   _("A pilot enjoying some downtime between competitions."),
   _("A tired pilot taking a break."),
   _("A pilot lounging around."),
}
local pilot_messages = {
   _([["Some people say that the Crimson gauntlet encourages and promotes violence, but I've been destroying ships long before I started participating in the Crimson Gauntlet!"]]),
   function () return fmt.f(
      _([["Hey, didn't I see you flying a {ship} in the Crimson Gauntlet? Nice flying."]]),
      {ship=player.pilot():ship()}
   ) end,
   _([["I was doing so well in my Hyena, but it just lacks the firepower to take on larger ships. Maybe I should upgrade to a Vendetta."]]),
   _([["The Crimson Gauntlet has really taught me to appreciate the small things in life, you know, blowing up your enemies with mace rockets and such."]]),
   _([["I used to be a pretty sloppy pilot before participating in the Crimson Gauntlet. I still am, but I used to be too."]]),
   _([["Sometimes I get motion sickness from the virtual reality. What's more troublesome is it also happens when I fly my real ship!"]]),
   _([["Sometimes when I get blown up in Crimson Gauntlet, it takes me a while to realize I haven't actually been blown up to smithereens."]]),
}

local function hasIntrinsic( p, o )
   for k,v in ipairs( p:outfitsList("intrinsic") ) do
      if v==o then
         return true
      end
   end
   return false
end

local guide_priority = 6

function create()
   bgnpcs = {}
   local function create_npc( names, descriptions, msglist, i )
      local name  = names[ rnd.rnd(1, #names) ]
      local img, prt = vni.generic()
      local desc  = descriptions[ rnd.rnd(1, #descriptions) ]
      local msg   = msglist[i]
      local id    = evt.npcAdd( "approach_bgnpc", name, prt, desc, 10 )
      local npcdata = { name=name, image=img, message=msg }
      bgnpcs[id]  = npcdata
   end

   -- Create random noise NPCs while avoiding duplicate messages
   local spectator_msglist = rnd.permutation( spectator_messages )
   local pilot_msglist = rnd.permutation( pilot_messages )
   for i = 1,rnd.rnd(3,5) do
      if rnd.rnd() < 0.6 then
         create_npc( spectator_names, spectator_descriptions, spectator_msglist, i )
      else
         create_npc( pilot_names, pilot_descriptions, pilot_msglist, i )
      end
   end

   -- Custom NPCs
   evt.npcAdd( "approach_guide", gauntlet.guide.name, gauntlet.guide.portrait, gauntlet.guide.desc, guide_priority )

   hook.takeoff( "leave" )
end

-- Just do random noise
function approach_bgnpc( id )
   local npcdata = bgnpcs[id]
   vn.clear()
   vn.scene()
   local spectator = vn.newCharacter( npcdata.name, { image=npcdata.image } )
   vn.transition()
   spectator( npcdata.message )
   vn.run()
end

function approach_guide ()
   vn.clear()
   vn.scene()
   local guide = vn.newCharacter( gauntlet.vn_guide() )
   vn.transition()
   vn.na(_("You approach the Crimson Gauntlet guide."))
   vn.label("menu_main")
   guide( function () return fmt.f(_([["Hello and welcome to the Crimson Gauntlet! You have {emblems}. What would you like to do?"]]), {emblems=gauntlet.emblems_str( gauntlet.emblems_get())}) end )
   vn.menu{
      {_("Information"), "information"},
      {_("Trade Emblems"), "trade_menu"},
      {_("Leave"), "leave"},
   }

   vn.label("information")
   guide(_([["What would you like to know about?"]]))
   vn.label("menu_info_raw")
   vn.menu{
      { _("Crimson Gauntlet"), "info_overview" },
      { _("Gauntlet History"), "info_gauntlet" },
      { _("Crimson Challenge"), "info_challenge" },
      { _("Totoran Emblems"), "info_emblems" },
      { _("Nothing."), "menu_main" },
   }

   vn.label("info_overview")
   guide(_([["The Crimson Gauntlet allows you to experience combat with your ships in virtual reality, without having to worry about any real damage. All you do is scan your ship and its outfits, and you will be ready to participate in all the challenges."]]))
   vn.jump("menu_info")

   vn.label("info_gauntlet")
   -- incident is 593:3726.4663
   guide(_([["The Crimson Gauntlet was founded originally in mid-UST 568, as a program for training Dvaered Military. Of course, back then they did not have such good virtual reality technology and instead relied on actual combat. However, as the number of accidents and casualties grew, they ended up starting a move to virtual reality, and now the Crimson Gauntlet boasts some of the best full immersion virtual reality experiences in the Universe."]]))
   guide(_([["Eventually, to encourage the warrior spirit among the general population, in UST 588, 20 cycles after its creation, the Crimson Gauntlet was open to the general public as a virtual reality experience. This is one of the few places in the universe where you can experience hard space combat without having a fear of dying. If your ship is destroyed, you are only dropped out of the virtual reality environment."]]))
   guide(_([["Many famous pilots have had their formation here, and it is also frequented by scouting agencies from across the Universe to find good pilots. Not to mention all the prizes that can be won from competing in the tournaments."]]))
   vn.jump("menu_info")

   vn.label("info_challenge")
   guide(_([["The Crimson Gauntlet Challenge is a set of challenges split into three types: skirmisher, for small ships like fighters and bombers; warrior, for ships like corvettes and destroyers; and warlord, for the larger ships such as cruisers or carriers. Once you enter a specific challenge, you will face waves of increasingly hard opponents which you must defeat."]]))
   guide(_([["You get bonus points depending on your ship class with respect to the category. Using smaller ships will give you a bonus in general. You also get a bonus for clearing the waves faster. If you are defeated, the total score up until your loss will be used to compute your rewards. As this is all done in virtual reality, you don't have to worry about any damage to your real ships!"]]))
   vn.jump("menu_info")

   vn.label("info_emblems")
   guide(_([["The Totoran Emblems represent the honour you have gained in the Crimson Gauntlet Arena, and can be used to unlock harder challenges and even special upgrades for your ships! The harder the challenges you undertake, the more Emblems you can obtain, allowing to truly test the limits of your piloting skills."]]))
   guide(_([["The Emblems come from the traditional Dvaered practices of rewarding feats of skilled captains on the battlefield. Where not only was the accumulation of Emblems a symbol of status, but it could also be used for bartering and adorning one's ships. Although this practice has fallen a bit out of favour with the current economy, it still lives on in the Crimson Gauntlet!"]]))
   vn.jump("menu_info")

   vn.label("menu_info")
   guide(_([["Is there anything else you would like to know about?"]]))
   vn.jump("menu_info_raw")

   vn.label("trade_menu")
   guide( function () return fmt.f(_([["You have {emblems}. What would you like to trade for?"]]), {emblems=gauntlet.emblems_str( gauntlet.emblems_get())}) end )
   local trades = {
      {name=_("Unlock Warrior Challenge"), cost=500, type="var", var="gauntlet_unlock_warrior",
         description=_("Unlocks participation in the difficult Warrior Challenge meant for Corvette and Destroyer-class ships.")},
      {name=_("Unlock Warlord Challenge"), cost=2000, type="var", var="gauntlet_unlock_warlord",
         description=_("Unlocks participation in the most difficult Warlord Challenge meant for Cruiser, Carrier, and Battleship-class ships."),
         test=function () return var.peek("gauntlet_unlock_warrior") end },
      {name=_("Unlock Double Damage Enemies Perk"), cost=2500, type="var", var="gauntlet_unlock_doubledmgtaken",
         description=_("Unlocks the Double Damage Enemies Perk, which causes all enemies to double the amount of damage. While this increases the challenge difficulty, it also immensely increases the rewards.")},
      {name=_("Unlock No Healing Perk"), cost=2500, type="var", var="gauntlet_unlock_nohealing",
         description=_("Unlocks the No Healing Perk, which makes it so you don't get healed between waves. While this increases the challenge difficulty, it also increases the rewards.")},
      {name=_("Gauntlet Deluxe (Ship Upgrade)"), cost=2500, type="intrinsic", outfit=outfit.get("Gauntlet Deluxe")},
   }
   local tradein_item = nil
   local handler = function (idx)
      -- Jump in case of 'Back'
      if idx=="menu_main" then
         vn.jump(idx)
         return
      end

      local t = trades[idx]
      local emblems = t.cost
      tradein_item = t
      if emblems > gauntlet.emblems_get() then
         -- Not enough money.
         vn.jump( "trade_notenough" )
         return
      end

      if t.type == "var" then
         tradein_item.description = t.description
      elseif t.type == "intrinsic" then
         tradein_item.description = t.outfit:summary().."\n"..t.outfit:description()
      else
         error(_("unknown tradein type"))
      end
      vn.jump( "trade_confirm" )
   end
   vn.label("trade_menu_raw")
   vn.menu( function ()
      local opts = {}
      for k,v in ipairs(trades) do
         local toadd = true
         if v.test and not v.test() then
            toadd = false
         end
         if v.type=="var" and var.peek(v.var) then
            toadd = false
         end
         if v.type=="intrinsic" and hasIntrinsic( player.pilot(), v.outfit ) then
            toadd = false
         end
         if toadd then
            table.insert( opts, {string.format(_("%s (%s)"), v.name, gauntlet.emblems_str(v.cost)), k} )
         end
      end
      table.insert( opts, {_("Back"), "menu_main"} )
      return opts
   end, handler )

   vn.label("trade_notenough")
   guide( function() return fmt.f(_([["You only have {emblems}, you would need {emblems_more} more to purchase the '{item}'.
Is there anything else you would like to purchase?"]]), {
         emblems = gauntlet.emblems_str(gauntlet.emblems_get()),
         emblems_more = gauntlet.emblems_str(tradein_item.cost - gauntlet.emblems_get()),
         item = tradein_item.name })
   end )
   vn.jump("trade_menu_raw")

   vn.label("trade_confirm")
   guide( function () return fmt.f(
      _([["Are you sure you want to trade in for the '#w{name}#0'? The description is as follows:"
#w{description}#0"]]),
      tradein_item)
   end )
   vn.menu{
      {_("Trade"), "trade_consumate"},
      {_("Cancel"), "trade_menu" },
   }

   vn.label("trade_consumate")
   vn.func( function ()
      local t = tradein_item
      gauntlet.emblems_pay( -t.cost )
      if t.type == "var" then
         var.push( t.var, true )
      elseif t.type == "intrinsic" then
         player.pilot():outfitAddIntrinsic( t.outfit )
      else
         error(_("unknown tradein type"))
      end
   end )
   vn.sfxMoney()
   guide(_([["Hope you enjoy your purchase!"]]))
   vn.jump("trade_menu")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

--[[
-- Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
