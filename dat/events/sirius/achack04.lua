--[[
-- This is a helper event for the fourth mission in the Academy Hack minor campaign.
--]]

function create()
   delay = rnd.rnd(10000, 40000) -- 10-40s
   hook.timer(delay, "startMission")
   hook.land("cleanup")
   hook.jumpout("cleanup")
end

function startMission()
   naev.missionStart("Sirian Truce")
   cleanup()
end

function cleanup()
   evt.finish()
end