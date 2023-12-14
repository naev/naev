--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Land Tutorial Event">
 <location>land</location>
 <chance>100</chance>
</event>
--]]
--[[

   Tutorial Event

--]]
local fmt = require "format"
local tut = require "common.tutorial"
local vn  = require 'vn'


function create ()
   if tut.isDisabled() then evt.finish() end

   hook.outfit_buy( "outfit_buy" )
   hook.ship_buy( "ship_buy" )
   hook.takeoff( "takeoff" )
end

function outfit_buy( o )
   local tbroad = o:typeBroad()
   local isturret = o:specificstats().isturret

   if tbroad == "Afterburner" and not var.peek( "tut_afterburner" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Looks like you just bought an #oAfterburner#0 outfit. Afterburners are active outfits that you can only equip one of on any specific ship. You can either configure to be set to a weapon set keybinding in the Info menu (opened with {infokey}), or double tap {accelkey} to trigger it. Note that they are prone to overheating and use a lot of energy, so it is best to use afterburners only when necessary."]]),{infokey=tut.getKey("info"), accelkey=tut.getKey("accel")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_afterburner", true )

   elseif tbroad == "Launcher" and not var.peek( "tut_launcher" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Looks like you just acquired your first #oLauncher#0 outfit. Launchers are ammo-based weapons that can have target tracking abilities. Seeking launchers have two important properties: '#oLock-on#0' and '#oIn-Flight Calibration#0'. Lock-on determines how many seconds it takes to be able to launch rockets after getting a new target. It is modulated depending if the target's '#oSignature#0' is lower than the '#oOptimal Tracking#0'."]]))
      sai(_([["'#oIn-Flight Calibration#0' is the amount of time it takes for the rocket after being launched to start tracking the target. When not locked-on it will fly in a straight line and damage all hostiles they encounter, however, once locked-on they will only damage the target unless jammed. In-Flight Calibration is not affected by the target's signature, and is visualized by a coloured circle around the outfit that shrinks as the calibration finishes."]]))
      sai(_([["That leads us to the last important concept: jamming! Seeking rockets can be jammed depending on whether or not the target has jamming equipment, and the resistance of the rocket. The chance of a rocket being jammed is the difference between the jamming chance and the resistance. When a rocket becomes jammed it can either get slowed down, or get stuck in a random trajectory. Despite being jammed, they can still damage any hostiles they encounter."]]))
      sai(fmt.f(_([["Finally, the ammo of launchers regenerates over time: there is no need to buy ammunition. By either performing a cooldown with {cooldownkey} or double-tapping {reversekey}, or landing on a planet or station you can instantly refill the ammunition when necessary. Launchers can be very useful if you master them. Please try them out with different configurations!"]]),{cooldownkey=tut.getKey("cooldown"), reversekey=tut.getKey("reverse")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_launcher", true )

   elseif tbroad == "Fighter Bay" and not var.peek( "tut_fighterbay" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Looks like you are moving up in the world with your first #oFighter Bay#0 outfit! As you can expect, fighter bays let you launch and control interceptor or fighter class escorts. They autonomously defend your ship when deployed, and can be given particular orders. You can tell them to #oattack#0 your target with {eattackkey}, #oclear orders#0 with {eclearkey}, #ohold position#0 with {eholdkey}, and #oreturn to ship#0 with {ereturnkey}."]]),{eattackkey=tut.getKey("e_attack"), eholdkey=tut.getKey("e_hold"), ereturnkey=tut.getKey("e_return"), eclearkey=tut.getKey("e_clear")}))
      sai(fmt.f(_([["Besides giving direct orders to your escorts, you can also control how they behave by using the 'Escort AI' button accessible from the #oMain#0 tab of the #oInformation#0 menu accessible with {infokey}. You can set a range of behaviours from engaging all hostiles to only defending when directly attacked. Using proper settings can minimize the amount of orders you have to give your escorts!"]]),
         {infokey=tut.getKey("info")}))
      sai(fmt.f(_([["Like ammunition in launchers, lost fighters will regenerate slowly over time, and you can restock them either by performing a cooldown operation with {cooldownkey} or double-tapping {reversekey}, or by landing on a planet or station. You can either fly around with deployed fighters or keep them inside your ship and only deploy as necessary. You should try to see whatever works best for you!"]]),{cooldownkey=tut.getKey("cooldown"), reversekey=tut.getKey("reverse")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_fighterbay", true )

   elseif isturret and not var.peek( "tut_turret" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Is that a #oTurret#0 weapon you just bought? Almost all weapons have built-in tracking capabilities, however, non-turret weapons have very limited mobility, and are only able to target weapons more or less straight ahead. On the other hand, turret weapons have 360 degree tracking capability and can hit enemy ships from all over!"]]))
      sai(fmt.f(_([["Like non-turret weapons, turret weapons must be fired manually. When outfitting a ship with both turrets and non-turrets at the same time, it may be useful to set up #oWeapon Sets#0 to be able to fire non-turrets and turrets independently. Weapon sets can be set up from the #oWeapons#0 tab of the #oInformation#0 menu accessible with {infokey}. You should try and see what works best for you!"]]),
         {infokey=tut.getKey("info")}))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_turret", true )

   elseif tbroad == "License" then

      local text
      if o == outfit.get("Large Civilian Vessel License") and not var.peek( "tut_lic_largeciv" ) then
         text = {
            _([["If you want to rake in the credits doing larger missions or commodities trades, the #oLarge Civilian Vessel License#0 is your first step! Plan your routes carefully, though, because these ships' defensive capabilities are very limited."]])
         }
         var.push("tut_lic_largeciv", true )

      elseif o == outfit.get("Medium Weapon License") and not var.peek( "tut_lic_medweap" )  then
         text = {
            _([["Oh, is that a #oMedium Weapon License#0? This will open the possibility of equipping larger weapons like turrets or launchers. Turrets can rotate any direction and take the burden of aiming off your ship. Launchers use ammunition and can lock on to enemy ships to make sure the payload hits the target. By mixing and matching weapons with complementary strengths you can greatly increase your combat ability."]]),
            _([["Many of the newer weapons you'll now have access to have higher CPU requirements. To increase your CPU, you have to equip better #oCore Systems#0. How powerful of a core system you can equip is limited by the slot sizes of your ship. Larger slots give you more power, but come at the cost of more mass. If you want to stay nimble and stealthy, you should be careful about increasing your mass."]]),
         }
         var.push( "tut_lic_medweap", true )

      elseif o == outfit.get("Heavy Weapon License") and not var.peek( "tut_lic_hvyweap" ) then
         text = {
            _([["Looks like you're finally ready to take on the big guns. #oHeavy Weapons License#0 will allow you to buy the largest and most powerful of weaponry. These weapons are generally similar to their medium counterparts, but are on a different scale. They use much more energy, weigh much more, but also have much longer range and higher firepower."]]),
            _([["One thing you will have to watch out is that heavy weapons tend to have large minimum tracking values and optimal tracking values. Ships with #oSignature#0 lower than your weapons minimum tracking value will be able to easily dodge most of your shots. On the other hand, large ships with signature above your optimal tracking value will be devastated by your shots. If you intend to deal with smaller ships too, make sure to equip fighter bays or lighter weapons!"]]),
         }
         var.push( "tut_lic_hvyweap", true )

      elseif o == outfit.get("Light Combat Vessel License") and not var.peek( "tut_lic_lightcom" ) then
         text = {
            _([["It looks like you can finally get your hands on combat vessels with the #oLight Combat Vessel License#0! Combat vessels differ from civilian vessels in that they are intrinsically suited for combat. This, in general, translates to higher mobility and firepower, at the cost of cargo capacity and utility. There are mainly three classes of light combat vessels: #oFighters#0, #oBombers#0, and #oInterceptors#0."]]),
            _([["#oInterceptors#0 are the lightest of the three with limited slots, but low mass and high manoeuvrability. #oFighters#0 and #oBombers#0 are heavier but still very agile, with fighters specialized in closer combat with small craft, while bombers are known to carry torpedoes that can ravage even the largest of battleships."]]),
            _([["To make the most of combat ships, it is recommended you look at their strengths and weaknesses. Equipping only forward weapons on a #oBomber#0-class ship is not going to make for an effective combat vessel. Slots also play an important role, with #oInterceptor#0-class ships being more limited than #oFighter#0-class ships. Be sure to experiment with set ups to see what works well for you."]]),
         }
         var.push( "tut_lic_lightcom", true )

      elseif o == outfit.get("Medium Combat Vessel License") and not var.peek( "tut_lic_medcom" ) then
         text = {
            _([["Looks like you've outgrown the #oLight Combat Vessel License#0. With the #oMedium Combat Vessel License#0 you'll get access to #oCorvette#0- and #oDestroyer#0-class ships, which can start to pack a real punch. Corvettes are the more agile of the two and can skirmish with smaller craft, while destroyers tend to be slower with much more firepower."]]),
            _([["Medium combat vessels tend to have a good balance between firepower and utility, being able to both perform stealth operations and frontal assaults. As they also have more slots, they can be configured more extensively to play different roles. You should experiment and see what works well for you."]]),
         }
         var.push( "tut_lic_medcom", true )

      elseif o == outfit.get("Heavy Combat Vessel License") and not var.peek( "tut_lic_hvycom" ) then
         text = {
            _([["Looks like you finally got your hands on the #oHeavy Combat Vessel License#0! This will allow you to pilot the largest of the combat ships, which includes #oCruiser#0-, #oBattleship#0-, and #oCarrier#0-class ships. These ships sacrifice size and utility for survivability and firepower, being able to lay devastating barrages to lay waste to enemy forces."]]),
            _([["#oCruisers#0 are the smallest of the three categories, but also the lowest mass and highest manoeuvrability. They can be seen more or less as a heavier version of a destroyer. #oBattleships#0 push the firepower to the max and can chew through any ship that is unable to evade their concentrated firepower. #oCarriers#0 use a different strategy of using fighters to do the combat for them while maximizing survivability."]]),
            _([["Flying a heavy combat vessel is very different from other classes of ships. Given their size, usually time seems to pass at a faster rate than smaller ships which can be seen by their '#oTime dilation#0' value. Furthermore, given their size, stealth becomes difficult, and they can be vulnerable to volleys of torpedoes. Escorts and fighter bays can help them deal with enemy bombers."]]),
         }
         var.push( "tut_lic_hvycom", true )

      elseif o == outfit.get("Mercenary License") and not var.peek( "tut_lic_merc" ) then
         text = {
            _([["It looks like you got a #oMercenary License#0. This will enable bounty collection missions that should start appearing in your mission computer screens from now on. These missions are usually generally combat-oriented and will require you to have a suitable combat vessel to successfully complete them. They can be fairly challenging at the beginning so make sure your ship is properly equipped for combat."]]),
         }
         var.push( "tut_lic_merc", true )
      end

      if text then
         vn.clear()
         vn.scene()
         local sai = vn.newCharacter( tut.vn_shipai() )
         vn.transition( tut.shipai.transition )
         for k,v in ipairs(text) do
            sai( v )
         end
         vn.done( tut.shipai.transition )
         vn.run()
      end
   end
end

function ship_buy( s )

   if not var.peek( "tut_buyship" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Congratulations on buying a brand new {ship}! Unless you trade in your ship, when you buy a new ship it is added to your available ships. Usually, ships come with only core outfits equipped, so you should head over to the #oEquipment#0 window to deck the ship out and swap it with your current one if you want to use it. You can swap ships at any planet or station with a shipyard and there is no penalty nor cost associated with swapping. In fact, getting a diversity of ships and switching to the one that best fits your need is a great way to get things done."]]),{ship=s}))
      sai(_([["You also don't have to worry about your ship AI changing, I am automatically transferred between your ships. You can't get away from me, ha ha."]]))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_buyship", true )

   elseif s:tags().bioship and not var.peek( "tut_bioship" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Wow! Looks like you acquired a bioship! My data banks show that they are grown on top of synthetic structures and have varying degrees of sentience. However, because of this they start at lower stages and have to gain experience to reach their full potential. As bioships unlock their potential, you will be able to customize and modify them at will. However, be aware that the choices that you make are not easy to reverse."]]))
      sai(fmt.f(_([["You can see the status of your current bioship from the #bInfo menu#0, which you can access with #b{infokey}#0. As your bioship gains experience, and advances to new stages, you'll be able to obtain new skills that open up new possibilities. Make sure to choose your skills carefully as it is not easy to change them once they have been chosen."]]),{infokey=tut.getKey("info")}))
      sai(_([[I've never been run in a bioship before. I wonder what it will be like?"]]))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_bioship", true )

   elseif s:time_mod() > 1 and not var.peek( "tut_timedil" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Whoa, it looks like you just acquired your first large ship. You may have noticed by now, but ships have a '#oTime Constant#0' rating, which indicates how time feels to pass by when flying the ship. Smaller ships tend to have values below 100%, and thus time ends up passing slower in them than in other ships. On the other hand, large ships will have time constants above 100%, making time seem to pass faster. For example, a ship with a 150% time constant will have time passing 50% faster than a normal ship, and will help the ship feel less cumbersome to fly."]]))
      sai(_([["In general, due to the low manoeuvrability of large ships, you should consider using turret weapons and/or fighter bays for self-defence. They allow you to worry less about aiming and more about getting jobs done. Turrets rely heavily on their tracking values. '#oMinimum Tracking#0' refers to the '#oSignature#0' value of a target ship at which the turret will not be able to track. On the other hand, if the target ship is above the '#oOptimal Tracking#0', your turrets will be able to perfectly track them. It is best to try out different turrets and see what feels best for your ship."]]))
      vn.done( tut.shipai.transition )
      vn.run()
      var.push( "tut_timedil", true )
      return
   end
end

function takeoff ()
   evt.finish()
end
