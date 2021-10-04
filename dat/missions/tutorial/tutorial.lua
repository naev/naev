--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Tutorial">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>1</priority>
  <location>None</location>
 </avail>
</mission>
--]]
--[[

   Initial Tutorial Mission

--]]
local tut = require "common.tutorial"
local neu = require "common.neutral"
local vn = require "vn"
local vntk = require "vntk"

function create ()
   missys = system.get( "Delta Polaris" )
   destsys = system.get( "Jade" )
   start_planet = planet.get( "Bolero" )
   start_planet_r = 200
   dest_planet = planet.get( "Benteen" )
   dest_planet_r = 200

   if not misn.claim( missys ) then
      print(fmt.f(_( "Warning: 'Tutorial' mission was unable to claim system {sysname}!"), {sysname=missys:name()} ) )
      misn.finish( true ) -- We mark as clear anyway, but this is not good
   end

   misn.setTitle( _("Tutorial") )
   misn.setDesc( _("Your ship AI has offered to teach you how to fly your ship.") )
   misn.setReward( _("Skills to play the game!") )

   local dotut = false

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(_("As you are admiring the view from your cockpit, suddenly a holographic projection appears in front of you."))
   sai(fmt.f(_([["Congratulations on your first space ship, {playername}! You made an excellent decision to purchase from Melendez Corporation! Our ships are prized for their reliability and affordability. I promise you won't be disappointed!"
You are skeptical of the sales pitch, of course; you really only bought this ship because it was the only one you could afford. Still, you tactfully thank the hologram.]]),{playername=player.name()}))
   sai(_([["I am your ship AI, but don't worry if you get a new ship. I can be transferred over without an issue. If you have any question or comment about how your ship works or how to do things, I am always ready to help. As your new Ship AI, would you like to give me a name?"]]))
   vn.label("rename")
   local ainame
   vn.func( function ()
      -- TODO integrate into vn
      ainame = tk.input( _("Name your Ship AI"), 1, 16, _("SAI") )
      if ainame then
         var.push("shipai_name",ainame)
         sai:rename( ainame )
         vn.jump("gavename")
         return
      end
      vn.jump("noname")
   end )
   vn.label("gavename")
   sai(fmt.f(_([["You have given me the name of '{ainame}', is this correct?"]]),{ainame=ainame}))
   vn.menu{
      {fmt.f(_("Continue with '{ainame}'"),{ainame=ainame}), "renamedone"},
      {_("Rename"), "rename"},
   }
   vn.jump("mainmenu")

   vn.label("noname")
   sai(_([["You haven't given me a name. I will be continued to be called Ship AI or SAI for short, is that OK?"]]))
   vn.menu{
      {_([["Ship AI is fine"]]), "renamedone"},
      {_("Rename"), "rename"},
   }

   vn.label("renamedone")
   sai(fmt.f(_([["OK! Pleased to meet you {playername}. If you change your mind about my name or want to change tutorial settings, you can always do so from the info menu which you can open with {infokey}."]]),{playername=player.name(), infokey=tut.getKey("info")}))

   vn.label("mainmenu")
   sai(_([["Now that we have you out in space for the first time, how about I go over your new ship's controls with you real quick? No charge!"]]))
   vn.menu{
      {_("Do the tutorial"), "dotut"},
      {_("Skip the tutorial"), "skiptut"},
      {_("Turn off all tutorials"), "offtut"},
   }

   vn.label("skiptut")
   sai(_([["Are you sure you want to skip the initial tutorial? You will not be able to do it again without starting a new game. You will still get prompts for advanced in-game mechanics as you encounter them."]]))
   vn.menu{
      {_("Skip the tutorial"), "skiptut_yes"},
      {_("Nevermind"), "mainmenu"},
   }
   vn.done( tut.shipai.transition )

   vn.label("offtut")
   sai(_([["Are you sure you want to disable all the tutorials? This includes explanation of advanced in-game mechanics you will meet as you play the game."]]))
   vn.menu{
      {_("Disable all tutorials"), "offtut_yes"},
      {_("Nevermind"), "mainmenu"},
   }
   vn.func( function ()
      var.push( "tut_disable", true )
   end )
   vn.done( tut.shipai.transition )

   vn.label("dotut")
   vn.func( function ()
      dotut = true
   end )
   sai(_([["Alrght, let's go over how to pilot your new state-of-the-art ship from Melendez Corporation, then!"
You resist the urge to roll your eyes.]]))
   sai(fmt.f(_([["Moving is pretty simple: rotate your ship with {leftkey} and {rightkey}, and thrust to move your ship forward with {accelkey}! You can also use {reversekey} to rotate your ship to the opposite direction you are moving, or to reverse thrust if you purchase and install a reverse thruster onto your Melendez Corporation starship. Give it a try by flying over to {destpnt}! You see it on your screen, right? It's the planet right next to you."]]),{leftkey=tut.getKey("left"), rightkey=tut.getKey("right"), accelkey=tut.getKey("accel"), reversekey=tut.getKey("reverse"), destpnt=dest_planet:name()}))
   vn.done( tut.shipai.transition )
   vn.run()

   if not dotut then
      misn.finish( true ) -- Mark os completed
      return
   end
   misn.accept()

   local posd = { startpnt=startplanet:name(), destpnt=dest_planet:name(), sysname=missys:name() }
   misn.osdCreate( _("Tutorial"), {
      fmt.f(_("Fly to {startpnt} in the {sysname} system with the movement keys"),posd),
      fmt.f(_("Land on {startpnt} in the {sysname} system by double-clicking on it"),posd),
      fmt.f(_("Go to {startpnt} in the {sysname} system by right-clicking it on the overview map"),posd),
      fmt.f(_("Destroy the practice drone near {destpnt} in the {sysname} system"),posd),
      _("Jump to a nearby system by using your starmap"),
   } )

   timer_hook = hook.timer( 5.0, "timer" )
   hook.land("land")
   hook.takeoff("takeoff")
   hook.enter("enter")

   -- Set stuff known as necessary
   dest_planet:setKnown(true)
   jump.get( system.cur(), destsys ):setKnown(true)

   stage = 1
end

function timer ()
   if timer_hook ~= nil then
      hook.rm( timer_hook )
      timer_hook = nil
   end
   timer_hook = hook.timer( 5.0, "timer" )

   -- Have to be in the mission system
   if not system.cur() == missys then return end

   if stage==1 and player.pos():dist( start_planet:pos() ) <= start_planet_r then
      stage = 2
      misn.osdActive( 2 )

      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Perfect! That was easy enough, right? We at Melendez Corporation recommend this manner of flight, which we call 'keyboard flight'. However, there is one other way you can fly if you so choose: press {mouseflykey} on your console and your Melendez Corporation ship will follow your #bmouse pointer#0 automatically! It's up to you which method you prefer to use."]]),{mouseflykey=tut.getKey("mousefly")}))
      sai(_([["Ah, you may also have noticed the mission on-screen display on your monitor! As you can see, you completed your first objective of the Tutorial mission, so the next objective is now being highlighted."]]))
      sai(fmt.f(_([["On that note, let's go over landing! All kinds of actions, like landing on planets, hailing ships, boarding disabled ships, and jumping to other systems can be accomplished by #bdouble-clicking#0 on an applicable target, or alternatively by pressing certain buttons on your control console. How about you try landing on {pntname}?"]]),{pntname=start_planet:name()}))
      sai(fmt.f(_([["To land on {pntname}, you'll need to first slow down your ship so that the landing procedure can be carried out safely; you can do this either with your regular movement controls or, if you prefer, by pressing {autobrakekey}, which will bring your Melendez Corporation ship to a complete stop. You can then land either by #bdouble-clicking#0 on the planet, or by targeting the planet with {targetplanetkey} and then pressing {landkey}. Give it a try!"]]),{pntname=start_planet:name(), autobrakekey=tut.getKey("autobrake"), targetplanetkey=tut.getKey("target_planet"), landkey=tut.getKey("land")}))
      vn.done( tut.shipai.transition )
      vn.run()

   elseif stage==4 and player.pos():dist(dest_planet:pos()) <= dest_planet_r then
      stage = 5
      misn.osdActive( 4 )

      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(_([["Great job! As you can see, by using Autonav, the perceived duration of your trip was cut substantially. You will grow to appreciate this feature in your Melendez Corporation ship in time, especially as you travel from system to system delivering goods and such."]]))
      sai(_([["Let's now practice combat. You won't need this if you stick to the safe systems in the Empire core, but sadly, we are likely to encounter pirate scum if you venture further out, so you need to know how to defend yourself. Fortunately, your ship comes pre-equipped with a state-of-the-art laser cannon for just that reason!"]]))
      sai(fmt.f(_([["I will launch a combat practice drone off of {pntname} now for you to fight. Don't worry; our drone does not have any weapons and will not harm you. Target the drone by clicking on it or by pressing {targethostilekey}, then use your weapons, controlled with {primarykey} and {secondarykey}, to take out the drone!"
   "Ah, yes, one more tip before I launch the drone: if your weapons start losing their accuracy, it's because they're becoming overheated. You can remedy that by pressing {autobrakekey} twice to engage active cooling."]]),{pntname=dest_planet:name(), targethostilekey=tut.getKey("target_hostile"), primarykey=tut.getKey("primary"), secondarykey=tut.getKey("secondary"), autobrakekey=tut.getKey("autobrake")}))
      vn.done( tut.shipai.transition )
      vn.run()

      spawn_drone()
   end
end

function land ()
   if timer_hook ~= nil then
      hook.rm(timer_hook)
      timer_hook = nil
   end
   if stage == 2 then
      stage = 3
      msg_info{_([["Excellent! The landing was successful. Melendez Corporation uses advanced artificial intelligence technology so that you never have to worry about your ship crashing. It may seem like a small thing, but it wasn't long ago when pilots had to land manually and crashes were commonplace! We at Melendez Corporation pride ourselves at protecting the safety of our valued customers and ensuring that your ship is reliable and resilient."]]),
      _([["When you land, your ship is refueled automatically and you can do things such as talk to civilians at the bar, buy new ship components, configure your ship, and most importantly, accept missions from the Mission Computer. Why don't we look around? As you can see, we are currently on the Landing Main tab, where you can learn about the planet and buy a local map. Click all the other tabs below and I'll give you a tour through what else you can do on a planet. When you are done, click the 'Take Off' button so we can continue."]])
      }

      bar_hook       = hook.land("land_bar", "bar")
      mission_hook   = hook.land("land_mission", "mission")
      outfits_hook   = hook.land("land_outfits", "outfits")
      shipyard_hook  = hook.land("land_shipyard", "shipyard")
      equipment_hook = hook.land("land_equipment", "equipment")
      commodity_hook = hook.land("land_commodity", "commodity")
   end
end

function msg_info( msg )
   vntk.msg( _("Tutorial"), msg )
end

function land_bar ()
   if bar_hook ~= nil then
      hook.rm(bar_hook)
      bar_hook = nil
   end
   msg_info(_([["This is the Spaceport Bar, where you can read the latest news (as you can see on your right at the moment), but more importantly, you can meet other pilots, civilians, and more! Click on someone at the bar and then click on the Approach button to approach them. I recommend regularly talking to bar patrons, at least early on. There may be pilots among them who may have useful tips for you!"]]))
end

function land_mission ()
   if mission_hook ~= nil then
      hook.rm(mission_hook)
      mission_hook = nil
   end
   msg_info{_([["This is the Mission Computer, where you can find basic missions in the official mission database. Missions are how you make your living as a pilot, so I recommend you check this screen often to see where the money-making opportunities are! You can see that each mission is given a brief summary, and by clicking them, you will be able to see more information about the mission. Since many missions involve cargo, you can also see how much free space is available in your ship in the top-right."]]),
    fmt.f(_([["When picking missions, pay attention to how much they pay. You'll want to strike a balance of choosing missions that you're capable of doing, but getting paid as much as possible to do them. Once you've chosen a mission, click the 'Accept Mission' button on the bottom-right and it will be added to your active missions, which you can review via the Info screen by pressing {infokey}."]],{infokey=tut.getKey("info")})),
   }
end

function land_outfits ()
   if outfits_hook ~= nil then
      hook.rm(outfits_hook)
      outfits_hook = nil
   end
   msg_info{_([["This is the Outfitter, where you can buy new outfits to make your Melendez Corporation starship even better! You can fit your ship with new weapons, extra cargo space, more powerful core systems, and more! Regional maps which can help you explore the galaxy more easily can also be purchased here, as well as licenses required for higher-end weaponry and starships (for example, you will require a Large Civilian Vessel License to purchase our top-of-the-line Melendez Mule Bulk Cargo Starhip, widely sought after for its unmatched cargo capacity)."]]),
   _([["As you can see, a series of tabs at the top of your screen allow you to filter outfits by type: 'W' for weapons, 'U' for utilities, 'S' for structurals, 'Core' for cores, and 'Other' for anything outside of those categories (most notably, regional maps and licenses). When you see an outfit that interests you, click on it to see more information about it, then either click on the 'Buy' button to buy it or click on the 'Sell' button to sell it (if you have one in your possession). Different planets have different outfits available; if you don't see a specific outfit you're looking for, you can search for it via the 'Find Outfits' button."]]),
   }
end

function land_shipyard ()
   if shipyard_hook ~= nil then
      hook.rm(shipyard_hook)
      shipyard_hook = nil
   end
   msg_info{_([["This is the Shipyard, where you can buy new starships to either replace the one you've got, or to add to your collection! On the left of this screen, you will see ships available on the planet you're currently on. Click on a ship you're interested in learning more about. You can then either buy the ship with the 'Buy' button, or trade your current ship in for the new ship with the 'Trade-In' button. Different planets have different ships available; if you don't see a specific ship you're looking for, you can search for it via the 'Find Ships' button."]]),
   _([["You have no need for this screen right now, but later on, when you've saved up enough, you'll likely want to upgrade your ship to an even better one, depending on what kinds of tasks you will be performing. Melendez Corporation specializes primarily in cargo ships. I highly recommend either our Melendez Quicksilver Rush Cargo Starship for cargo missions that have a time limit, or our Melendez Koala Compact Bulk Cargo Starship for missions that require carrying large amounts of cargo. We sincerely thank you for shopping with Melendez Corporation!"]])
   }
end

function land_equipment ()
   if equipment_hook ~= nil then
      hook.rm(equipment_hook)
      equipment_hook = nil
   end
   msg_info{_([["This is the Equipment screen, which is available only on planets which have either an outfitter or a shipyard. Here, you can equip your ship with any outfits you have bought at the Outfitter. If and only if the current planet has a shipyard, you can also do so with any other ship you own, and you can swap which ship you are currently piloting by selecting another ship and clicking the 'Swap Ship' button. You can also sell those other ships (but not your current ship) with the 'Sell Ship' button, if you decide that you no longer need them. Selling a ship that still has outfits equipped will also lead to those outfits being sold along with the ship, so do keep that in mind if there's an outfit you need to keep."]]),
   _([["If you make any changes to your ship now, please ensure that you still have two weapons equipped, as you will need those later for practicing combat and flying around space without any weapons can be very risky anyway."]]),
   }
end

function land_commodity ()
   if commodity_hook ~= nil then
      hook.rm(commodity_hook)
      commodity_hook = nil
   end
   msg_info{_([["This is the Commodity screen, where you can buy and sell commodities. Commodity prices vary from planet to planet and even over time, so you can use this screen to attempt to make money by buying low and selling high. Click on a commodity to see information about it, most notably its average price per tonne, and click on the 'Buy' and 'Sell' buttons to buy or sell some of the commodity, respectively. Here, it's useful to hold the Shift and/or Ctrl keys to adjust the modifier of how many tonnes of the commodity you're buying or selling at once. This will reduce the number of times you have to click on the Buy and Sell buttons."]]),
   fmt.f(_([["If you're unsure what's profitable to buy or sell, you can press {starmapkey} to view the star map and then click on the 'Mode' button for various price overviews. The news terminal at the bar also includes price information for specific nearby planets."]],{starmapkey=tut.getKey("starmap")}))
   }
end

function takeoff ()
   if bar_hook ~= nil       then hook.rm(bar_hook) end
   if mission_hook ~= nil   then hook.rm(mission_hook) end
   if outfits_hook ~= nil   then hook.rm(outfits_hook) end
   if shipyard_hook ~= nil  then hook.rm(shipyard_hook) end
   if equipment_hook ~= nil then hook.rm(equipment_hook) end
   if commodity_hook ~= nil then hook.rm(commodity_hook) end
   bar_hook       = nil
   mission_hook   = nil
   outfits_hook   = nil
   shipyard_hook  = nil
   equipment_hook = nil
   commodity_hook = nil
end

function enter ()
   if timer_hook ~= nil then
      hook.rm( timer_hook )
      timer_hook = nil
   end
   timer_hook = hook.timer( 5.0, "timer" )
   hook.timer( 2.0, "enter_timer" )
end

function enter_timer ()
   if stage == 3 then
      stage = 4
      misn.osdActive( 3 )

      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["Welcome back to space, {playername}! Let's continue discussing moving around in space. As mentioned before, you can move around space manually, no problem. However, you will often want to travel large distances, and navigating everywhere manually could be a bit tedious. That is why we at Melendez Corporation always require the latest Autonav technology with all of our ships!"]]),{playername=player.name()}))
      sai(fmt.f(_([["Autonav is simple and elegant. Simply press {overlaykey} to open your ship's overlay map, then simply #bright-click#0 on any location, planet, ship, or jump point to instantly take your ship right to it! The trip will take just as long, but advanced Melendez Corporation technology allows you to step away from your controls, making it seem as though time is passing at a faster rate. And don't worry; if any hostile pilots are detected, our Autonav system automatically alerts you so that you can observe the situation and respond in whatever fashion is deemed necessary. This can be configured from your ship's Options menu, which you can access by pressing {menukey}.]]),{overlaykey=tut.getKey("overlay"), menukey=tut.getKey("menu")}))
      sai(fmt.f(_([["Why don't you try using Autonav to fly over to {pntname}? You should be able to see it on your overlay map."]]),{pntname=dest_planet:name()}))
      vn.done( tut.shipai.transition )
      vn.run()

   elseif stage == 5 and system.cur() == missys then
      spawn_drone()

   elseif stage == 6 and system.cur() == destsys then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      sai(fmt.f(_([["You have done very well, {playername}! As you can see, the trip consumed fuel. You consume fuel any time you make a jump and can refuel by landing on a friendly planet. If you find yourself in a pinch, you may also be able to buy fuel from other pilots in the system; hail a pilot by #bdouble-clicking#0 on them, or by selecting them with {targetnextkey} and then pressing {hailkey}."]]),{playername=player.name(), targetnextkey=tut.getKey("target_next"), hailkey=tut.getKey("hail")}))
      sai(_([["Ah, that reminds me: you can also attempt to bribe hostile ships, such as pirates, by hailing them. Bribes work better on some factions than on others; pirates will happily take your offer and may even sell you fuel afterwards, but many other factions may be less forthcoming."]]))
      sai(_([["And I think that's it! I must say, you are a natural-born pilot and your new Melendez ship suits you well! I wish you good luck in your travels. Thank you for shopping with Melendez Corporation!" Captain T. Practice ceases contact and you finally let out a sigh of relief. You were starting to think you might lose your mind with all of the marketing nonsense being poured out at you. At least you learned how to pilot the ship, though!"]]))
      vn.done( tut.shipai.transition )
      vn.run()

      -- Normal finish of the tutorial
      tut.log(_([[Captain T. Practice, the Melendez employee who sold you your first ship, gave you a tutorial on how to pilot it, claiming afterwards that you are "a natural-born pilot".]]))
      misn.finish( true )
   end
end

function pilot_death ()
   hook.timer( 2.0, "pilot_death_timer" )
end

function pilot_death_timer ()
   stage = 6
   misn.osdActive( 5 )
   misn.markerAdd( destsys )

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(_([["Excellent work taking out that drone! As you may have noticed, shield regenerates over time, but armor does not. This is not universal, of course; some ships, particularly larger ships, feature advanced armor repair technology. But even then, armor regeneration is usually much slower than shield regeneration."]]))
   sai(fmt.f(_([["You may have also noticed your heat meters going up and your weapons becoming less accurate as your ship and weapons got hot. This is normal, but too much heat can make your weapons difficult to use, which is why we at Melendez Corporation recommend using active cooling when it is safe to do so. As I said before, you can engage active cooling by pressing {autobrakekey} twice. Alternatively, you can cool off your ship instantly by landing on any planet or station."]]),{autobrakekey=tut.getKey("autobrake")}))
   sai(fmt.f(_([["It is also worth noting that you can configure the way your weapons shoot from the Info screen, which can be accessed by pressing {infokey} or through the button on the top of your screen. The Info screen also lets you view information about your ship, cargo, current missions, and reputation with the various factions. You will likely be referencing it a lot."]]),{infokey=tut.getKey("info")}))
   sai(_([["I think we should try venturing outside of this system! There are many systems in the universe; this one is but a tiny sliver of what can be found out there!"]]))
   sai(_([["Traveling through systems is accomplished through jump points. Like planets, you usually need to find these by exploring the area, talking to the locals, or buying maps. Once you have found a jump point, you can use it by right-clicking on it."]]))
   sai(fmt.f(_([["But there is yet a better way to navigate across systems! By pressing {starmapkey}, you can open your starmap. The starmap shows you all of the systems you currently know about. Through your starmap, you can click on a system and click on the Autonav button to be automatically transported to the system! Of course, this only works if you know a valid route to get there, but you will find that this method of travel greatly simplifies things."]]),{starmapkey=tut.getKey("starmap")}))
   sai(fmt.f(_([["Why don't you give it a try and jump to the nearby {sysname} system? You should see an indicator blip on your map; missions often use these blips to show you where to go next. You will have to make two jumps and may have to do some exploration to find the second jump point. Let's see what you've learned!"]]),{sysname=destsys:name()}))
   vn.done( tut.shipai.transition )
   vn.run()
end

function spawn_drone ()
   local p = pilot.add( "Hyena", "Dummy", dest_planet, _("Captain T. Practice"), {ai="baddie_norun"} )
   p:outfitRm( "all" )
   p:outfitRm( "cores" )
   p:outfitAdd( "Previous Generation Small Systems" )
   p:outfitAdd( "Patchwork Light Plating" )
   p:outfitAdd( "Beat Up Small Engine" )

   p:setHealth( 100, 100 )
   p:setEnergy( 100 )
   p:setTemp( 0 )
   p:setFuel( true )

   p:setHostile()
   p:setVisplayer()
   p:setHilight()
   hook.pilot( p, "death", "pilot_death" )
end

function abort ()
   misn.finish( true ) -- Aborting finishes the mission properly
end
