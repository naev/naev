--[[
-- example calling
--
--    planet = ai.rndplanet()
--    -- planet must exist
--    if planet == nil then
--       ai.pushtask(0, "hyperspace")
--    else
--       ai.pushtask(0, "goto", planet)
--    end
--]]

-- flies to the target planet
function goto ()
   target = ai.target()
   dir = ai.face(target)
   dist = ai.dist( target )
   bdist = ai.minbrakedist()
   if dir < 10 and dist > bdist then
      ai.accel()
   elseif dir < 10 and dist < bdist then
      ai.poptask()
      ai.pushtask(0,"stop")
   end
end

-- brakes
function land ()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
      ai.settimer(0, ai.rnd(8000,15000))
      ai.pushtask(0,"landed")
   else
      ai.brake()
   end
end

-- waits
function landed ()
   if ai.timeup(0) then
      -- Run X task (usually hyperspace
      ai.pushtask(0,"hyperspace")
   end
end
