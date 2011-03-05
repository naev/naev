-- This is the tutorial: interstallar flight.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Interstellar flight"
    message1 = [[Welcome to tutorial: Interstellar flight.

In this tutorial you will learn how to travel from one star system to another, and how to use the galaxy map.]]
    message2 = [[As our first act in this tutorial, we will leave the Mohawk system and travel to one of the neighboring ones.

Each star system is connected to one or more other star systems by means of "jump points". These jump points are the ONLY places where you may enter hyperspace, and once you do you will appear near the jump point leading the other way in the destination system.

Let's select a jump point now. You can press %s to do this.]]
    message3 = [[Good. Your HUD has changed to show that you have selected a hyperspace target. Currently, the target is unknown, because you don't know anything about it yet. Press %s twice more to cycle through the jump points and clear your hyperspace target again.]]
    
    omsgthyper = "Press %s to target a jump point"
end

function create()
    misn.accept()
    
    -- Set up the player here.
    player.teleport("Mohawk")
    
    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))

    tk.msg(title1, message1)
    tk.msg(title1, message2:format(tutGetKey"thyperspace"))
    omsg = player.omsgAdd(omsgthyper:format(tutGetKey"thyperspace"))
    
    waitthyper = 0
    hook.input("input")
    -- TODO: Disable everything except thyperspace.
end

-- Input hook.
function input(inputname, inputpress)
    if inputname == "thyperspace" and waitthyper < 3 then
        waitthyper = waitthyper + 1
        if waitthyper == 1 then
            tk.msg(title1, message3:format(tutGetKey"thyperspace"))
        elseif waitthyper == 3 then
            player.omsgRm(omsg)
            
        end
    else
        tk.msg("test", "received " .. inputname .. "!")
    end
end

-- Capsule function for naev.getKey() that adds a color code to the return string.
function tutGetKey(command)
    return "\027b" .. naev.getKey(command) .. "\0270"
end

-- Abort hook.
function abort()
    cleanup()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    -- Function to return to the tutorial menu here
end