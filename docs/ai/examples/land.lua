--[[
-- example calling
--
--    planet = ai.rndplanet()
--    -- planet must exist
--    if planet == nil then
--       ai.pushtask("hyperspace")
--    else
--       ai.pushtask("moveto", planet)
--    end
--]]

-- flies to the target planet
function moveto( target )
   local dir = ai.face(target)
   local dist = ai.dist( target )
   local bdist = ai.minbrakedist()
   if math.abs(dir) < math.rad(10) and dist > bdist then
      ai.accel()
   elseif math.abs(dir) < math.rad(10) and dist < bdist then
      ai.poptask()
      ai.pushtask("stop")
   end
end

-- brakes
function land ()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
      ai.settimer(0, rnd.uniform(8.0, 15.0))
      ai.pushtask("landed")
   else
      ai.brake()
   end
end

-- luacheck: globals landed (A hook which waits)
function landed ()
   if ai.timeup(0) then
      -- Run X task (usually hyperspace)
      ai.pushtask("hyperspace")
   end
end
