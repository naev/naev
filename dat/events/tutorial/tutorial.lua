-- Master tutorial script.
-- This script allows the player to choose a tutorial module to run, or return to the main menu.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    menutitle = "Tutorial Menu"
    menutext = "Welcome to the Naev tutorial menu. Please select a tutorial module from the list below:"

    menubasic = "Tutorial: Basic Operation"
    menuinterstellar = "Tutorial: Interstellar Flight"
    menubasiccombat = "Tutorial: Basic Combat"
    menumisscombat = "Tutorial: Missile Combat"
    menudisable = "Tutorial: Disabling"
    menuplanet = "Tutorial: The Planetary Screen"
    menutrade = "Tutorial: Trade"
    menumissions = "Tutorial: Missions and Events"
    menucomms = "Tutorial: Communications"
    menux = "Quit to Main Menu"
end

function create()
    -- Set defaults just in case.
    local pp = player.pilot()
    player.teleport("Mohawk")
    player.msgClear()
    player.swapShip("Llama", "Tutorial Llama", "Paul 2", true, true)

    pp:setPos(vec2.new(0, 0))
    pp:setVel(vec2.new(0, 0))
    pp:setHealth(100, 100)
    pp:control(false)
    pp:setNoLand(false)
    pp:setNoJump(false)
    
    system.get("Mohawk"):setKnown(false)
    system.get("Cherokee"):setKnown(false)
    system.get("Iroquois"):setKnown(false)
    system.get("Navajo"):setKnown(false)

    -- Create menu.
    _, selection = tk.choice(menutitle, menutext, menubasic, menuinterstellar, menucomms, menubasiccombat, menumisscombat, menudisable, menuplanet, menumissions, menux)
    
    startModule(selection)
end

-- Helper function for starting the tutorial modules
function startModule(module)
    if selection == menux then -- Quit to main menu
        tut.main_menu()
    end
    naev.eventStart(module)
    evt.finish(true) -- While the module is running, the event should not.
end
