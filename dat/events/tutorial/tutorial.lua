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
    menuadvcombat = "Tutorial: Advanced Combat"
    menuplanet = "Tutorial: The Planetary Screen"
    menutrade = "Tutorial: Trade"
    menumissions = "Tutorial: Missions and Events"
    menucomms = "Tutorial: Communications"
    menux = "Quit to Main Menu"
end

function create()
    -- Set defaults just in case.
    player.teleport("Mohawk")
    player.pilot():setPos(vec2.new(0, 0))
    player.pilot():setVel(vec2.new(0, 0))
    player.msgClear()
    
    system.get("Mohawk"):setKnown(false)
    system.get("Cherokee"):setKnown(false)
    system.get("Iroquois"):setKnown(false)
    system.get("Navajo"):setKnown(false)

    player.pilot():setNoLand(false)
    player.pilot():setNoJump(false)

    -- Create menu.
    _, selection = tk.choice(menutitle, menutext, menubasic, menuinterstellar, menucomms, menubasiccombat, menuadvcombat, menuplanet, menutrade, menumissions, menux)
    
    if selection == menubasic then
        startModule(menubasic)
    elseif selection == menuinterstellar then
        startModule(menuinterstellar)
    elseif selection == menucomms then
        startModule(menucomms)
    elseif selection == menubasiccombat then
        startModule(menubasiccombat)
    elseif selection == menuadvcombat then
        placeholder()
    elseif selection == menuplanet then
        startModule(menuplanet)
    elseif selection == menutrade then
        placeholder()
    elseif selection == menumissions then
        startModule(menumissions)
    elseif selection == menux then -- Quit to main menu
        tut.main_menu();
    else
        -- This point should never be reached!
        tk.msg("Error", "You've apparently selected an invalid menu option. This shouldn't happen. Please report.")
        create()
    end
end

-- Placeholder not-implemented function
function placeholder()
    tk.msg("Not Implemented", "This tutorial has not been implemented yet because the subject matter is still in development.")
    create()
end

-- Helper function for starting the tutorial modules
function startModule(module)
    naev.eventStart(module)
    evt.finish(true) -- While the module is running, the event should not.
end
