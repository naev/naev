include("ai/tpl/merchant.lua")


function sos ()
   msg = {
      "Mayday! We are under attack!",
      "Requesting assistance.  We are under attack!",
      "Merchant vessel under attack! Requesting help!",
      "Help! Ship under fire!",
      string.format("Mayday! Merchant %s being assaulted!", ai.shipclass())
   }
   ai.broadcast(msg[ rnd.int(1,#msg) ])
end


function create ()
   ai.setcredits( rnd.int(100, ai.shipprice()/25) )

   mem.bribe_no = "\"The Space Traders do not negotiate with criminals.\""

   -- Some stuff has more chance then others
   num = rnd.int(12)
   if num < 5 then
      cargo = "Food"
   elseif num < 8 then
      cargo = "Ore"
   elseif num < 10 then
      cargo = "Industrial Goods"
   elseif num < 12 then
      cargo = "Luxury Goods"
   else
      cargo = "Medicine"
   end
   ai.setcargo( cargo, rnd.int(0, ai.cargofree() ) )
end
