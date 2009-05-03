--[[
-- Derelict Event
--
-- Creates a derelict ship.
--
-- @todo Have derelict ship spawn missions, trigger pirate traps, etc...
--]]


function create ()

   -- Get the derelict's ship.
   r = rnd.rnd()
   if r > 0.95 then
      ship = "Trader Gawain"
   elseif r > 0.8 then
      ship = "Trader Mule"
   elseif r > 0.5 then
      ship = "Trader Koala"
   else 
      ship = "Trader Llama"
   end

   -- Create the derelict.
   angle = rnd.rnd() * 2 * math.pi
   dist  = rnd.rnd(400, 1000)
   pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
   p     = pilot.add(ship, "dummy", pos, false)
   for k,v in ipairs(p) do
      v:setFaction("Derelict")
      v:disable()
   end

end
