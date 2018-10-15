-- This is tutorial: the planetary screen.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Tutorial: The Planetary Screen")
message1 = _([[Welcome to the planetary screen tutorial.

In this tutorial you will learn about what you can expect on a planet or station once you land on it. We'll start by landing on Paul 2. Landing is covered in another tutorial, so for now we'll land automatically.]])
message2 = _([[Once landed, you will be presented with the landing screen. This is the default screen for all planets or stations you land on. The landing screen gives you information about the planet or station in general and lets you refuel.

To access the more interesting facilities on Paul 2, you will need to click on the tabs at the bottom of the planet menu. Each tab represents a different area of the spaceport. Some spaceports may not have all of the facilities you see here, and the things on offer will vary from planet to planet. Sometimes you will have to fly to another planet to get what you need.

Select any tab now to view a short explanation on what you can do there. Once you have seen enough, press the takeoff button to end this tutorial.]])
message3 = _([[This is the outfit seller. Here you may buy and sell things such as weapons, ship upgrades and ammunition. Note that you need to visit the equipment facility to actually install most outfits into your ship.

A particular item of interest that most outfit sellers will carry is the star map. Buying a star map will give you information on remote systems and their jump points, so it's a useful item when you're just starting out.]])
message4 = _([[This is the spaceport bar. Here you may find people who are interested in giving you missions. You will also see a news feed on the right of the screen. You can approach patrons by clicking on their portrait and then clicking on the "approach" button.

There are no mission givers here at the moment. Missions are explained in more detail in another tutorial.]])
message5 = _([[This is the mission computer. This is where you can find work if you're in need of credits. Most missions here will initially involve ferrying passengers or cargo, but you may be able to unlock more mission types during the course of the game.

There is no work available right now. Missions are explained in more detail in another tutorial.]])
message6a = _([[This is the equipment screen, where you can customize your ship, as well as switch to another ship if you own more than one.

You have been given a modest selection of equipment. Try installing it in your ship by right-clicking the item boxes in the inventory panel. You can uninstall them again by right-clicking on the ship slots they are installed in. There are a few things to keep in mind:

- Each outfit is either a Structure, Utility or Weapon outfit, and can ONLY be installed in an appropriate slot.

- Each outfit has a size. Possible sizes are small, medium and large. Sizes are currently indicated by yellow, green and blue, respectively. Outfits can ONLY be installed in a slot that's big enough.

- Each outfit uses up a certain amount of CPU. Your CPU capacity is shown as a green bar that turns red as CPU is used up. You can only take off if you haven't exceeded your CPU capacity.

- Each outfit has a mass that will be added to your ship when installed. If your ship's mass exceeds the engine's mass limit, your speed and thrust will be reduced.]])
message6b = _([[If you look at your ship's slots on the right hand side of the menu, you will see that three of them are different than the others. This is because these three slots are your ship's "core" slots.

Core slots are special slots that can only contain their corresponding core outfits. Engine slots can only hold engines, system slots can only hold systems and hull slots can only hold hull mods. The important thing to know is that if even one of these slots is empty, your ship can't take off!

Core outfits can be found in the Core tab of the equipment browser.]])

message7 = _([[This is the shipyard. You can buy new ships and sell ships you currently own here.

There are no ships for sale here, but generally you will always find at least a modest selection of ships in any shipyard you encounter.]])
message8 = _([[This is the commodity exchange. Here you can buy and sell goods.

The trade system is currently in an early stage of development. Once the system is more developed, it will be explained in a separate tutorial.]])

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    player.pilot():setPos(planet.get("Paul 2"):pos())
    player.msgClear()
    
    tk.msg(title1, message1)
    
    player.pay(-player.credits())
    player.addOutfit("Laser Cannon MK1", 2)
    player.addOutfit("Plasma Blaster MK1", 1)
    player.addOutfit("Engine Reroute", 1)
    player.addOutfit("Unicorp Scrambler", 1)
    player.addOutfit("Cargo Pod", 1)
    player.addOutfit("Milspec Prometheus 2203 Core System")
    player.addOutfit("Nexus Dart 150 Engine")
    player.addOutfit("S&K Light Stealth Plating")
    player.pilot():control()
    player.pilot():land(planet.get("Paul 2"))
    
    mainland = hook.land("land")
    outfitsland = hook.land("outfits", "outfits")
    barland = hook.land("bar", "bar")
    missionland = hook.land("mission", "mission")
    equipmentland = hook.land("equipment", "equipment")
    shipyardland = hook.land("shipyard", "shipyard")
    commodityland = hook.land("commodity", "commodity")
    hook.takeoff("takeoff")
end

-- Land hook.
function land()
    tk.msg(title1, message2)
    hook.rm(mainland)
end

function outfits()
    tk.msg(title1, message3)
    hook.rm(outfitsland)
end

function bar()
    tk.msg(title1, message4)
    hook.rm(barland)
end

function mission()
    tk.msg(title1, message5)
    hook.rm(missionland)
end

function equipment()
    tk.msg(title1, message6a)
    tk.msg(title1, message6b)
    hook.rm(equipmentland)
end

function shipyard()
    tk.msg(title1, message7)
    hook.rm(shipyardland)
end

function commodity()
    tk.msg(title1, message8)
    hook.rm(commodityland)
end

function takeoff()
    hook.safe("cleanup")
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
