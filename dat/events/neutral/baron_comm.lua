--[[
-- Comm Event for the Crazy Baron mission string
--]]


function create ()
    if not evt.claim(system.cur()) then
      evt.finish()
    end

    hyena = pilot.add("Civilian Hyena", nil, true)[1]

    hook.pilot(hyena, "jump", "finish")
    hook.pilot(hyena, "death", "finish")
    hook.land("finish")
    hook.jumpout("finish")

    -- Lower chance of getting this event. First three times are
    -- full chance; after that, chance decreases by half each time.
    -- Does not apply to the Toaxis system. (This is here to
    -- prevent annoyance from a player constantly being offered
    -- this mission when they have no interest in it.)
    local chance = var.peek( "baron_comm_chance" ) or 4
    var.push( "baron_comm_chance", chance / 2 )

    hailie = hook.timer( 3000., "hailme" );
end

-- Make the ship hail the player
function hailme()
    hyena:hailPlayer()
    hook.pilot(hyena, "hail", "hail")
end

-- Triggered when the player hails the ship
function hail()
    naev.missionStart("Baron")
    player.commClose()
    evt.finish(true)
end

function finish()
    hook.rm(hailie)
    evt.finish()
end
