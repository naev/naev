include("ai/tpl/generic.lua")

-- Settings
aggressive = true
safe_distance = 300
armour_run = 100
shield_return = 20
land_planet = false


function create ()
   ai.setcredits(ai.shipprice()/1000 , ai.shipprice()/100 )
   attack_choose()
end


function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.int(0,1) == 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            "For the Frontier!",
            "You'll make great target practice!",
            "Purge the oppressors!"
      }
   else
      taunts = {
            "You are no match for the FLF.",
            "I've killed scum much more dangerous then you."
      }
   end

   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end

