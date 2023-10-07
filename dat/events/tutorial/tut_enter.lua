--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Enter Tutorial Event">
 <location>enter</location>
 <chance>100</chance>
 <cond>
   local tut = require "common.tutorial"
   return not tut.isDisabled()
 </cond>
</event>
--]]
--[[

   Enter Tutorial Event

--]]
local fmt = require "format"
local tut = require "common.tutorial"
local vn  = require "vn"

local function check_unused_oufits ()
   local pp = player.pilot()
   local o = {}
   for k,v in ipairs(pp:outfits()) do
      if v and v:toggleable() then
         o[k] = true -- mark slot as needs checking
      end
   end
   for wid=1,10 do
      for k,v in ipairs(pp:weapsetList(wid)) do
         o[v] = nil -- unmark as it is in weapon set
      end
   end
   return o
end

function create ()
   local enter_delay = 5

   hook.land( "evt_done" )

   -- TODO we should probably allow looking at any faction not just empire
   if not var.peek("tut_illegal") and player.pilot():hasIllegal("Empire") then
      hook.timer( enter_delay, "tut_illegal" )
      return
   end

   --  If the player has not set up an outfit in a weapon set we have to warn them
   if not var.peek("tut_weapset") then
      -- Warn about remaining ones
      local o = check_unused_oufits ()
      local hasunused = (next(o)~=nil)
      if hasunused then
         hook.timer( enter_delay, "tut_weapset" )
         return
      end
   end

   local sys = system.cur()
   local _nebu_dens, nebu_volat = sys:nebula()
   if not var.peek( "tut_nebvol" ) and nebu_volat > 0 then
      hook.timer( enter_delay, "tut_volatility" )
      return
   end
end

function evt_done ()
   evt.finish()
end

function tut_illegal ()
   local pp = player.pilot()
   local badstuff = {}
   for k,o in ipairs(pp:outfitsList()) do
      if #o:illegality() > 0 then
         table.insert( badstuff, o )
      end
   end
   for k,v in ipairs(pp:cargoList()) do
      local c = commodity.get(v.name)
      if #c:illegality() > 0 then
         table.insert( badstuff, c )
      end
   end
   if #badstuff<=0 then
      warn("tut_illegal called, but can't find badstuff!")
      return
   end

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(fmt.f(_([[Just as after taking off, {ainame} materializes in front of you.
"I don't know how to say this to you, but it seems like you have acquired some illegal items. Your {item} is on the rather dubious side of the law, and you will run into trouble if you are scanned by patrols. Sometimes, right after you are discovered, you may get away with paying a small fine in the form of a bribe, but that may not always be the case."]]),
      {item=badstuff[ rnd.rnd(1,#badstuff) ], ainame=tut.ainame()} ))
   sai(fmt.f(_([["If you have to deal with illegal goods, I would recommend you to try to maximize the stealth functionality of your ship, that you can enable with {stealthkey}. As ship detection in general is tied to ship mass, you will most likely have best result using small and agile ships to avoid detection. Remember to stay off patrol routes and avoid crowded systems for highest chance of success."]]),
      {stealthkey=tut.getKey("stealth")} ))
   sai(fmt.f(_([["You can check to see if your commodities or outfits are illegal from the #bInfo Menu#0 which you can open with {infokey}, and then looking at your ship outfits or commodities. Note that illegality is determined on a per-faction basis instead of globally."]]),
      {infokey=tut.getKey("info")}))
   vn.na(fmt.f(_([[{ainame} vanishes and you are left wondering why they are so knowledgable about illegal activities.]]),{ainame=tut.ainame()}))
   vn.done( tut.shipai.transition )
   vn.run()

   var.push( "tut_illegal", true )
   evt.finish()
end

function tut_weapset ()
   local unused = check_unused_oufits()
   local hasunused = (next(unused)~=nil)
   if not hasunused then
      evt.finish()
      return
   end
   local unused_outfit =player.pilot():outfitGet(next(unused))
   unused_outfit = "#o"..unused_outfit:name().."#0"
   local skipped

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(fmt.f(_([[Just as after taking off, {ainame} materializes in front of you.
"I was reviewing the ship loadout to keep the urges away, and noticed that even though {item} is equipped on the ship, it is not part of any weapon set!"]]),
      {item=unused_outfit, ainame=tut.ainame()} ))
   vn.menu{
      {_([[Continue]]), "cont01"},
      {_([["Urges?!"]]), "cont01_urges"},
      {_([[Skip tutorial]]), "skip"},
   }

   vn.label("cont01_urges")
   sai(fmt.f(_([["Um, that's not important right now, what is important is that if you do not set {item} in a weapon set, you won't be able to use it when it becomes necessary!"]]),
      {item=unused_outfit}))
   vn.jump("cont01")

   vn.label("cont01_cont")
   sai(fmt.f(_([["If you do not set {item} in a weapon set, you won't be able to use it when it becomes necessary!"]]),
      {item=unused_outfit}))
   vn.jump("cont01")

   vn.label("cont01")
   sai(fmt.f(_([["To be able to use your outfits, you have to open the info menu with {infokey}, and from there access the weapons tab. From there you will be able to configure how you use the different outfits. Let me open it up for you."]]),
      {infokey=tut.getKey("info")}))
   vn.done( tut.shipai.transition )

   vn.label("skip")
   vn.na(fmt.f(_([[You tell {shipai} that you will solve it out yourself.]]),
      {shipai=tut.ainame()}))
   vn.func( function () skipped = true end )
   vn.done( tut.shipai.transition )
   vn.run()

   if skipped then
      var.push( "tut_weapset", true )
      evt.finish()
      return
   end

   naev.menuInfo( "weapons" )

   vn.clear()
   vn.scene()
   vn.transition( tut.shipai.transition )
   sai = vn.newCharacter( tut.ainame(), {
      color=tut.shipai.colour
   } )
   sai(_([["A large part of combat is decided ahead of time by the ship classes and their load out. However, good piloting can turn the tables easily. It is important to assign weapon sets to be easy to use. You can set weapon sets from the '#oWeapons#0' tab of the information window. You have 10 different weapon sets that can be configured separately for each ship."]]))
   sai(_([["There are three different types of weapon sets:
- #oSwitch#0: activating the hotkey will set your primary and secondary weapons
- #oToggle#0: activating the hotkey will toggle the outfits between on/off states
- #oHold#0: holding the hotkey will turn the outfits on]]))
   sai(_([["By default, the weapon sets will be automatically managed by me, with forward bolts in set 1 (switch), turret weapons in set 2 (switch), and both turret and forward weapons in set 3 (switch). Seeker weapons are in set 4 (hold), and fighter bays in set 5 (hold). However, you can override this and set them however you prefer. Just remember to update them whenever you change your outfits."]]))
   sai(fmt.f(_([["Go ahead and try to add {item} to a weapon set! First click on an empty set on the list, then click on the icon of {item} to add it to the weapon set. Finally, click on the #bCycle Mode#0 button as necessary to change the type of weapon set."]]),
      {item=unused_outfit}))
   vn.done( tut.shipai.transition )
   vn.run()

   var.push( "tut_weapset", true )
   evt.finish()
end

function tut_volatility ()
   local sys = system.cur()
   local _nebden, nebvol = sys:nebula()

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[As you jump the system you notice a small alarm lights up in the control panel:
#rWARNING - Volatile nebula detected in {sys}! Taking {nebvol:.1f} MW damage!#0]]),{sys=sys, nebvol=nebvol}))
   sai(fmt.f(_([[{ainame} materializes in front of you.
"It looks like we entered part of the volatile nebula. The instability here causes heavy damage to any ships that enter. If our shield regeneration surpasses the volatility damage, we should be fine. However, if the volatility gets any stronger, it could be fatal to the {ship}. Going deeper into the nebula could prove to be a very risky endeavour."]]), {ship=player.ship(), ainame=tut.ainame()} ) )
   vn.done( tut.shipai.transition )
   vn.run()

   var.push( "tut_nebvol", true )
   evt.finish()
end
