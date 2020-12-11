require("ai/collective")

mem.armour_run = 0

function donothing ()
    ai.brake()
end

function idle () 
    ai.pushtask("donothing") 
end