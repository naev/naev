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
    
    -- Create menu.
    selection = tk.choice(menutitle, menutext, menu1, menu2, menu3, menu4, menu5, menu6, menu7, menux)
    
    if selection == 1 then
        placeholder()
    elseif selection == 2 then
        placeholder()
    elseif selection == 3 then
        placeholder()
    elseif selection == 4 then
        placeholder()
    elseif selection == 5 then
        placeholder()
    elseif selection == 6 then
        placeholder()
    elseif selection == 7 then
        placeholder()
    elseif selection == 8 then -- Quit to main menu
        cleanup()
    else
        -- This point should never be reached!
        print("Tutorial: Error - Invalid menu selected!\n")
        cleanup()
    end
end

-- Placeholder not-implemented function
function placeholder()
    tk.msg("Placeholder", "Not implemented yet!")
    create()
end

-- Cleanup function. Should be the exit point for the script.
function cleanup()
    -- Function to return to the main menu here
    tk.msg("TODO", "I can't return you to the main menu yet in Lua, so I'm going to kill you now.")
    player.pilot():setHealth(0, 0)
end