--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Ship AI">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[
   Handles the Ship AI (tutorial-ish?) being triggered from the info menu
--]]
local fmt = require "format"
local tut = require "common.tutorial"
local vn  = require "vn"
local bioship = require "bioship"

-- Functions:
local advice
local gave_advice

local function clicked ()
   gave_advice = false

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.label("mainmenu")
   sai(fmt.f(_([["Hello {playername}! Is there anything you want to change or brush up on?"]]),{playername=player.name()}))
   vn.menu( function ()
      local opts = {
         {_("Get Advice"), "advice"},
         {_("Get Information"), "tutorials"},
         {_("Tutorial/Ship AI Options"), "opts"},
         {_("Close"), "close"},
      }
      return opts
   end )

   vn.label("advice")
   sai( advice )
   vn.jump("mainmenu")

   vn.label("opts")
   vn.menu( function ()
      local opts = {
         {_("Reset all tutorial hints"), "reset"},
         {_("Rename Ship AI"), "rename"},
         {_("Close"), "mainmenu"},
      }
      if tut.isDisabled() then
         table.insert( opts, 1, {_("Enable tutorial hints"), "enable"} )
      else
         table.insert( opts, 1, {_("Disable tutorial hints"), "disable"} )
      end
      return opts
   end )

   vn.label("enable")
   vn.func( function () var.pop("tut_disable") end )
   sai(_([["In-game hints are now enabled! If you want to revisit old tutorial hints, make sure to reset them."]]))
   vn.jump("mainmenu")

   vn.label("disable")
   vn.func( function () var.push("tut_disable", true) end )
   sai(_([["In-game tutorial hints are now disabled."]]))
   vn.jump("mainmenu")

   vn.label("reset")
   sai(_([["This will reset and enable all in-game tutorial hints, are you sure you want to do this?"]]))
   vn.menu{
      {_("Reset tutorial hints"), "resetyes"},
      {_("Nevermind"), "opts"},
   }
   vn.label("resetyes")
   vn.func( function () tut.reset() end )
   sai(_([["All in-game tutorial hints have been set and enabled! They will naturally pop up as you do different things in the game."]]))
   vn.jump("mainmenu")

   vn.label("rename")
   local ainame
   vn.func( function ()
      -- TODO integrate into vn
      ainame = tk.input( _("Name your Ship AI"), 1, 16, _("SAI") )
      if ainame then
         var.push("shipai_name",ainame)
         sai.displayname = ainame -- Can't use rename here

         if tut.specialnames[ string.upper(ainame) ] then
            vn.jump("specialname")
            return
         end
      end
      vn.jump("gavename")
   end )
   vn.label("specialname")
   sai( function () return tut.specialnames[ string.upper(ainame) ] end )

   vn.label("gavename")
   sai( function () return fmt.f(_([[Your Ship AI has been renamed '{ainame}'.]]),{ainame=tut.ainame()}) end )
   vn.jump("opts")

   vn.label("tutorials")
   sai(fmt.f(_([["Hello {playername}. What do you want to learn about?"]]),{playername=player.name()}))
   vn.menu( function ()
      local opts = {
         {_("Weapon Sets"), "tut_weaponsets"},
         {_("Electronic Warfare"), "tut_ewarfare"},
         {_("Stealth"), "tut_stealth"},
         {_("Asteroids and Mining"), "tut_mining"},
         {_("Nevermind"), "mainmenu"},
      }
      if var.peek( "tut_illegal" ) then
         table.insert( opts, #opts, {_("Illegality and Smuggling"), "tut_illegal"} )
      end
      if var.peek( "tut_bioship" ) then
         table.insert( opts, #opts, {_("Bioships"), "tut_bioship"} )
      end
      if player.fleetCapacity() > 0 then
         table.insert( opts, #opts, {_("Fleets"), "tut_fleets"} )
      end
      return opts
   end )

   vn.label("tut_weaponsets")
   sai(_([["A large part of combat is decided ahead of time by the ship classes and their load out. However, good piloting can turn the tables easily. It is important to assign weapon sets to be easy to use. You can set weapon sets from the '#oWeapons#0' tab of the information window. You have 10 different weapon sets that can be configured separately for each ship."]]))
   sai(_([["There are three different types of weapon sets:
- #oWeapons - Switched#0: activating the hotkey will set your primary and secondary weapons
- #oWeapons - Instant#0: activating the hotkey will fire the weapons
- #oAbilities - Toggled#0: activating the hotkey will toggle the state of the outfits]]))
   sai(_([["By default, the weapon sets will be automatically managed by me, with forward bolts in set 1 (switched), turret weapons in set 2 (switched), and both turret and forward weapons in set 3 (switched). Seeker weapons are in set 4 (instant), and fighter bays in set 5 (instant). However, you can override this and set them however you prefer. Just remember to update them whenever you change your outfits."]]))
   vn.jump("tutorials")

   vn.label("tut_ewarfare")
   sai(_([["Ship sensors are based on detecting gravitational anomalies, and thus the mass of a ship plays a critical role in being detected. Smaller ships like yachts or interceptors are inherently much harder to detect than carriers or battleship."]]))
   sai(p_("Ship AI", [["Each ship has three important electronic warfare statistics:
- #oDetection#0 determines the distance at which a ship appears on the radar.
- #oSignature#0 determines the distance at which a ship is fully detected, that is, ship type and faction are visible. It also plays a role in how missiles and weapons track the ship.
- #oStealth#0 determines the distance at which the ship is undetected when in stealth mode"]]))
   sai(_([["#oDetection#0 plays a crucial in how much attention you draw in a system, and detection bonuses can be very useful for avoiding lurking dangers. Furthermore, concealment bonuses will lower your detection, signature, and stealth ranges, making outfits that give concealment bonuses very useful if you get your hands on them."]]))
   sai(_([["#oSignature#0 is very important when it comes to combat as it determines how well weapons can track your ship. If your signature is below your enemies weapon's #ominimal tracking#0, their weapons will not be able to accurately target your ship at all. However, if your signature is above the #ooptimal tracking#0, you will be tracked perfectly. Same goes for your targets. This also has an effect on launcher lock-on time, with lower signature increasing the time it takes for rockets to lock on to you."]]))
   sai(fmt.f(_([["#oStealth#0 is a whole different beast and is only useful when entering stealth mode with {stealthkey}. If you want to learn more about stealth, please ask me about it."]]),{stealthkey=tut.getKey("stealth")}))
   vn.jump("tutorials")

   vn.label("tut_stealth")
   sai(fmt.f(_([["You can activate stealth mode with {stealthkey} when far enough away from other ships, and only when you have no missiles locked on to you. When stealthed, your ship will be completely invisible to all ships. However, if a ship gets within the #ostealth#0 distance of your ship, it will slowly uncover you."]]),{stealthkey=tut.getKey("stealth")}))
   sai(_([["Besides making your ship invisible to other ships, #ostealth#0 slows down your ship by 50% to mask your gravitational presence. This also has the effect of letting you jump out from jumpoints further away. There are many outfits that can change and modify this behaviour to get more out of stealth."]]))
   sai(_([["When not in stealth, ships can target your ship to perform a scan. This can uncover unwanted information, such as illegal cargo or outfits. The time to scan depends on the mass of the ship. If you don't want to be scanned, you should use stealth as much as possible. Enemy ships may also use stealth. Similarly to how you get uncovered when ships enter your #ostealth#0 range, you can uncover neutral or hostile ships by entering their #ostealth#0 range, however, you will not be able to know where they are until you are on top of them."]]))
   sai(_([["Finally, escorts and fighters will automatically stealth when their leader goes into stealth, so you don't have to worry giving stealth orders to ships you may be commanding. Friendly ships will also not uncover your stealth, so it is good to make as many friends as possible."]]))
   vn.jump("tutorials")

   vn.label("tut_mining")
   sai(_([["Asteroid mining can be a lucrative business. Asteroid field scan be found throughout the galaxy in many different shapes and sizes. They can be a good source of many precious materials that fetch a good price on the commodity exchange. However, not all asteroid fields are made equal. In general, the more easily accessible asteroid fields are generally over-mined, leading to low yields. On the other hand, hard to access asteroid fields in dangerous areas have not yet succumbed to the hordes of ravenous mining fleets. If you find a good mining spot, I recommend you add a note on the #bSystem Map#0 so you don't forget."]]))
   sai(_([["Standard ship sensors are not designed for prospecting asteroid fields, so if you want to be able to get a rough estimate of what materials are in each asteroid, you will need to get an asteroid scanner. They can be either sold as standalone outfits, or integrated into mining tools such as drills. With an asteroid scanner equipped, all you have to do is get close to asteroids and their material composition will be displayed. You can also equip multiple asteroid scanners at the same time to increase the scanning range."]]))
   sai(fmt.f(_([["Once you have found a good potential mining spot, mining is as straight forward as shooting asteroid with damaging weapons. However, with most weapons, this will only yield common materials and not be a very efficient way of mining. By using mining-specific weapons such as {tool1}, you can also obtain more expensive materials. Furthermore, integrated tools such as {tool2} can be used as a solution for both scanning and extraction. One important point is that asteroid mining is fairly noisy and can attract unwanted attention. One of my previous owners found that out the hard way."]]),
      {tool1=outfit.get("Mining Lance MK1"), tool2=outfit.get("S&K Plasma Drill")}))
   vn.jump("tutorials")

   vn.label("tut_bioship")
   -- TODO more text
   sai(fmt.f(_([["You can see the status of your current bioship from the #bInfo menu#0, which you can access with {infokey}. As your bioship gains experience, and advances to new stages, you'll be able to obtain new skills that open up new possibilities. Make sure to choose your skills carefully as it is not easy to change them once they have been chosen."]]),{infokey=tut.getKey("info")}))
   vn.jump("tutorials")

   vn.label("tut_illegal")
   sai(_([["As you explore, you'll find different commodities, outfits, and even ships themselves, can be outright banned and made illegal by different factions. Although you are able to transport, equip, or use them normally, if a pilot from a faction that considers them illegal scans you, you will be in hot water. Although sometimes you can bribe them right away to continue on your travels, it is better to not have this problem in the first place."]]))
   sai(fmt.f(_([["The best way to avoid detection is to use stealth which you can enable with {stealthkey}. By staying away from patrol routes and using stealth as much as possible, you can minimize the amount of encounters with patrols. The most tricky parts then become jumping and landing, where lots of ships can converge in tight spaces. Other than using stealth increasing outfits, using a smaller ship will lower the overall visibility of your ship."]]),
      {stealthkey=tut.getKey("stealth")}))
   sai(fmt.f(_([["You can check to see if your commodities or outfits are illegal from the #bInfo Menu#0 which you can open with {infokey}, and then looking at your ship outfits or commodities. Note that illegality is determined on a per-faction basis instead of globally."]]),
      {infokey=tut.getKey("info")}))
   vn.jump("tutorials")

   vn.label("tut_fleets")
   sai(_([["With my overriding of the disable routine on the fleet procedure, you can now deploy additional ships you own. You have a maximum amount of fleet capacity, indicated in the equipment tab when landed, and each ship consumes a fixed amount of fleet capacity."]]))
   sai(fmt.f(_([["For example, you have a total fleet capacity of {fleetcap} points, and your current ship takes {shipcap} points. Note that if you are flying a single ship with no additional deployed ships, you can go over the fleet capacity. However, if you deploy additional ships, you have to stay below the total fleet capacity."]]),
      {fleetcap=player.fleetCapacity(), shipcap=player.pilot():ship():points()}))
   sai(_([["It is important to note that fleet capacity depends exclusively on the ship, and is not affected by normal outfits. Make sure to equip your ships as best as possible!"]]))
   sai(_([["Your additional ships will behave as escorts and you can give them commands to perform actions just like deployed fighters. Furthermore, they are all insured so even if you lose any ships, they will respawn when you land."]]))
   sai(_([["Finally, you can toggle ships' deployment by #bright-clicking#0 on their icon. Try it out!"]]))
   vn.jump("tutorials")

   vn.label("close")
   vn.done( tut.shipai.transition )
   vn.run()
end

-- Tries to give the player useful contextual information
function advice ()
   local adv = {}
   local adv_rnd = {}
   local pp = player.pilot()
   local ppstats = pp:stats()

   local msg_fuel = _([["When out of fuel, if there is an inhabitable planet you can land to refuel for free. However, if you want to save time or have no other option, it is possible to hail passing ships to get refueled, or even take fuel by force by boarding ships. Bribing hostile ships can also encourage them to give you fuel afterwards."]])
   table.insert( adv_rnd, msg_fuel )
   if ppstats.fuel < ppstats.fuel_consumption then
      table.insert( adv, msg_fuel )
   end

   local hostiles = pp:getEnemies( nil, nil, true )
   local msg_hostiles = _([["When being overwhelmed by hostile enemies, you can sometimes get out of a pinch by bribing them so that they leave you alone. Not all pilots or factions are susceptible to bribing, however."]])
   table.insert( adv_rnd, msg_hostiles )
   if #hostiles > 0 then
      table.insert( adv, msg_hostiles )
   end

   local _hmean, hpeak = pp:weapsetHeat(true)
   local msg_heat = fmt.f(_([["When your ship or weapons get very hot, it is usually a good idea to perform an active cooldown when it is safe to do so. You can actively cooldown with {cooldownkey} or double-tapping {reversekey}. The amount it takes to cooldown depends on the size of the ship, but when done, not only will your ship be cool, it will also have replenished all ammunition and fighters."]]),{cooldownkey=tut.getKey("cooldown"), reversekey=tut.getKey("reverse")})
   table.insert( adv_rnd, msg_heat )
   if pp:temp() > 300 or hpeak > 0.2 then
      table.insert( adv, msg_heat )
   end

   local msg_asteroids = fmt.f(_([["Asteroid fields can be a good way to make credits. It is simple enough to shoot at asteroids to release minerals and then approach to collect them with your ship's scoop. However, most weapons destroy uncommon and rare materials, and are only suitable for mining common materials. If you want to mine for better materials, you have to use mining-specific weapons or tools such as the {tool1} or {tool2}. You do have to watch out though, as mining is generally a fairly noisy process and can attract unwanted attention."]]),
      {tool1=outfit.get("Mining Lance MK1"), tool2=outfit.get("S&K Plasma Drill")})
   table.insert( adv_rnd, msg_asteroids )
   if #system.cur():asteroidFields() > 0 then
      table.insert( adv, msg_asteroids )
   end

   local armour = pp:health()
   local msg_armour = _([["In general, ships are unable to regenerate armour damage in space. If you take heavy armour damage, it is best to try to find a safe place to land to get fully repaired. However, there exists many different outfits that allow you to repair your ship, and some ships have built-in  armour regeneration allowing you to survive longer in space."]])
   table.insert( adv_rnd, msg_armour )
   if armour < 80 and ppstats.armour_regen <= 0 then
      table.insert( adv, msg_armour )
   end

   if bioship.playerisbioship() then
      local msg_bioship = _([["Bioships gain experience over time, allowing them to advance to new stages and learn new abilities. Bioships must land to advance to new stages, and only while landed will they will be able to learn new skills. You can set the skills from the bioship interface which is accessible from the #bInfo window#0 which you can open with #b{infokey}#0."]],
         {infokey=tut.getKey("info")})
      table.insert( adv_rnd, msg_bioship )
      if bioship.skillpointsfree() > 0 then
         table.insert( adv, msg_bioship )
      end
   end

   -- Return important advice
   if not gave_advice and #adv > 0 then
      gave_advice = true
      return adv[ rnd.rnd(1,#adv) ]
   end

   -- Run random advice
   return adv_rnd[ rnd.rnd(1,#adv_rnd) ]
end

function create ()
   player.infoButtonRegister( _("Ship AI"), clicked, 1, "A" )
end
