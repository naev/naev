include("ai/tpl/generic.lua")
include("ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true


function create ()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ai.shipprice()/300, ai.shipprice()/70) )

   -- Get refuel chance
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing < 20 then
         mem.refuel_no = "\"My fuel isn't for sale.\""
      elseif standing < 70 then
         if rnd.rnd() > 0.2 then
            mem.refuel_no = "\"Sorry, my fuel isn't for sale.\""
         end
      else
         mem.refuel = mem.refuel * 0.6
      end
      -- Most likely no chance to refuel
      mem.refuel_msg = string.format( "\"I can transfer some fuel for %d credits.\"", mem.refuel )
   end

   -- See if can be bribed
   if rnd.rnd() > 0.6 then
      mem.bribe = math.sqrt( ai.shipmass() ) * (500. * rnd.rnd() + 1750.)
      mem.bribe_prompt = string.format("\"House Proteron can always use some income. %d credits and you were never here.\"", mem.bribe )
      mem.bribe_paid = "\"Get lost before I have to dispose of you.\""
   else
     bribe_no = {
            "\"You won't buy your way out of this one.\"",
            "\"House Proteron likes to make examples out of scum like you.\"",
            "\"You've made a huge mistake.\"",
            "\"Bribery carries a harsh penalty, scum.\"",
            "\"I'm not interested in your blood money!\"",
            "\"All the money in the world won't save you now!\""
     }
     mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
     
   end

   -- Finish up creation
   create_post()
end

-- taunts
function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            "There is no room in this universe for scum like you!",
            "House Proteron will enjoy your death!",
            "Your head will make a fine addition to my collection!",
            "None survive the wrath of House Proteron!",
            "Enjoy your last moments, criminal!"
      }
   else
      taunts = {
            "You dare attack me!",
            "You are no match for House Proteron!",
            "The Empire will have your head!",
            "You'll regret that!",
            "That was a fatal mistake!"
      }
   end

   ai.comm(target, taunts[ rnd.rnd(1,#taunts) ])
end


