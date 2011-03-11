-- This is tutorial: the planetary screen.

include("dat/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: The planetary screen"
    message1 = [[Welcome to tutorial: The planetary screen.

In this tutorial you will learn about what you can expect on a planet or station once you land on it. We'll start by landing on Paul 2. Landing is covered in another tutorial, so for now we'll land automatically.]]
    message2 = [[Once landed, you will be presented with the landing screen. This is the default screen for all planets or stations you land on. The landing screen gives you information about the planet or station in general and lets you refuel.

To access the more interesting facilities on Paul 2, you will need to click on the tabs at the bottom of the planet menu. Each tab represents a different area of the spaceport. Some spaceports may not have all of the facilities you see here, and the things on offer will vary from planet to planet. Sometimes you will have to fly to another planet to get what you need.

Select any tab now to view a short explanation on what you can do there. Once you have seen enough, press the takeoff button to end this tutorial.]]
    message3 = [[This is the outfit seller. Here you may buy and sell things such as weapons, ship upgrades and ammunition. Note that you need to visit the equipment facility to actually install outfits into your ship.

A particular item of interest that most outfit sellers will carry is the star map. Buying a star map will give you information about surrounding systems, so it's a useful item when you're just starting out.]]
    message4 = [[This is the spaceport bar. Here you may find people who are interested in giving you missions. You will also see a news feed on the right of the screen. You can approach patrons by clicking on their portrait and then clicking on the "approach" button.

There are no mission givers here at the moment. Missions are explained in more detail in another tutorial.]]
    message5 = [[This is the mission computer. This is where you can find work if you're in need of credits. Most missions here will initially involve ferrying passengers or cargo, but you may be able to unlock more mission types during the course of the game.

There is no work available right now. Missions are explained in more detail in another tutorial.]]
    message6 = [[This is the equipment screen, where you can customize your ship, as well as switch to another ship if you own more than one.

There's currently nothing for you to do here. The equipment screen is explained in greater detail in another tutorial.]]
    message7 = [[This is the shipyard. You can buy new ships and sell ships you currently own here.

There are no ships for sale here, but generally you will always find at least a modest selection of ships in any shipyard you encounter.]]
    message8 = [[This is the commodity exchange. Here you can buy and sell goods.

The trade system is currently in an early stage of development. Once the system is more developed, it will be explained in a separate tutorial.]]
end

function create()
    misn.accept()
    
    -- Set up the player here.
    player.teleport("Mohawk")
    player.pilot():setPos(planet.get("Paul 2"):pos())
    player.msgClear()
    
    enable = {"menu"}
    
    tkMsg(title1, message1, enable)
    
    player.pilot():rmOutfit("all")
    player.pay(-player.credits()) -- The player gets nothing at all.
    player.pilot():control()
    player.pilot():land(planet.get("Paul 2")) -- Only one planet in the system anyway.
    
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
    tkMsg(title1, message2, enable)
    hook.rm(mainland)
end

function outfits()
    tkMsg(title1, message3, enable)
    hook.rm(outfitsland)
end

function bar()
    tkMsg(title1, message4, enable)
    hook.rm(barland)
end

function mission()
    tkMsg(title1, message5, enable)
    hook.rm(missionland)
end

function equipment()
    tkMsg(title1, message6, enable)
    hook.rm(equipmentland)
end

function shipyard()
    tkMsg(title1, message7, enable)
    hook.rm(shipyardland)
end

function commodity()
    tkMsg(title1, message8, enable)
    hook.rm(commodityland)
end

function takeoff()
    cleanup()
end

-- Abort hook.
function abort()
    cleanup()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    naev.keyEnableAll()
    -- Function to return to the tutorial menu here
end