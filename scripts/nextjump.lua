-- Choose the next system to jump to on the route from the current system to the argument system.
function getNextSystem(finalsys)
    local mysys = system.cur()
    if mysys == finalsys then
        return mysys
    else
        local neighs = mysys:adjacentSystems()
        local nearest = -1
        local mynextsys = finalsys
        for _, j in pairs(neighs) do
            if nearest == -1 or j:jumpDist(finalsys) < nearest then
                nearest = j:jumpDist(finalsys)
                mynextsys = j
            end
        end
        return mynextsys
    end
end