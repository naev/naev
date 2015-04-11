--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   This is a helper event for the fourth mission in the Academy Hack minor campaign.

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
