-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function getNextSystem(nowsys, finalsys)
    if nowsys == finalsys then
        return nowsys
    else
        local neighs = nowsys:adjacentSystems()
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