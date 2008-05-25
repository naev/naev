--[[
-- Gives the pilot it's initial stuff, money and loot generally
--]]
function create ()
   ai.setcredits( ai.rnd(200, ai.shipprice()/100) )

   num = ai.rnd(0,1)
   if num == 0 then
      cargo = "Food"
   elseif num ==1 then
      cargo = "Ore"
   end
   ai.setcargo( cargo, ai.rnd(0, ai.cargofree() ) )
end

