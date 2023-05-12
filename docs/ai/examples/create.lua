--[[
-- Gives the pilot it's initial stuff, money and loot generally
--]]
function create ()
   ai.setcredits( rnd.rnd(200, ai.pilot():ship():price()/100) )

   -- What about the other goodies? Well, nowadays the mission or factions.spawn script tends to handle equipment and cargo.
end
