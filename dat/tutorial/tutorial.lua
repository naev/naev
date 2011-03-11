-- Master tutorial script.
-- This script allows the player to choose a tutorial module to run, or return to the main menu.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    menutitle = "Tutorial menu"
    menutext = "Welcome to the Naev tutorial menu. Please select a tutorial module from the list below."

    menu1 = "Tutorial: Basic operation"
    menu2 = "Tutorial: Interstellar flight"
    menu3 = "Tutorial: Basic combat"
    menu4 = "Tutorial: Advanced combat"
    menu5 = "Tutorial: The planetary screen"
    menu6 = "Tutorial: Trade"
    menu7 = "Tutorial: Missions and events"
    menux = "Quit to main menu"
end

function create()
    -- Set defaults just in case.
    player.teleport("Mohawk")
    player.pilot():setPos(vec2.new(0, 0))
    player.msgClear()
    
    -- Create menu.
    selection = tk.choice(menutitle, menutext, menu1, menu2, menu3, menu4, menu5, menu6, menu7, menux)
    
    if selection == 1 then
        startModule(menu1)
    elseif selection == 2 then
        startModule(menu2)
    elseif selection == 3 then
        startModule(menu3)
    elseif selection == 4 then
        placeholder()
    elseif selection == 5 then
        startModule(menu5)
    elseif selection == 6 then
        placeholder()
    elseif selection == 7 then
        startModule(menu7)
    elseif selection == 8 then -- Quit to main menu
        tut.main_menu();
    else
        -- This point should never be reached!
        tk.msg("Error", "You've apparently selected an invalid menu option. This shouldn't happen. Please report.")
        create()
    end
end

-- Placeholder not-implemented function
function placeholder()
    tk.msg("Not implemented", "This tutorial has not been implemented yet because the subject matter is still in development.")
    create()
end

-- Helper function for starting the tutorial modules
function startModule(module)
    evt.misnStart(module)
    evt.finish(true) -- While the module is running, the event should not.
end