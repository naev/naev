include("ai/tpl/generic.lua")
include("ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true


function create ()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ai.shipprice()/300, ai.shipprice()/70) )

   -- Lines to annoy the player. Shouldn't be too common or Gamma Polaris and such get inundated.
   r = rnd.rnd(0,20)
   if r == 0 then
     ai.broadcast("The Empire is watching you.")
    elseif r == 1 then
      ai.broadcast("The Emperor sees all.")
   end

   -- Get refuel chance
   p = ai.getPlayer()
   if ai.exists(p) then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing < 20 then
         mem.refuel_no = "\"My fuel is property of the Empire.\""
      elseif standing < 70 then
         if rnd.rnd() > 0.2 then
            mem.refuel_no = "\"My fuel is property of the Empire.\""
         end
      else
         mem.refuel = mem.refuel * 0.6
      end
      -- Most likely no chance to refuel
      mem.refuel_msg = string.format( "\"I suppose I could spare some fuel for %d credits.\"", mem.refuel )
   end

   -- See if can be bribed
   if rnd.rnd() > 0.7 then
      mem.bribe = math.sqrt( ai.shipmass() ) * (500. * rnd.rnd() + 1750.)
      mem.bribe_prompt = string.format("\"For some %d credits I could forget about seeing you.\"", mem.bribe )
      mem.bribe_paid = "\"Now scram before I change my mind.\""
   else
     bribe_no = {
            "\"You won't buy your way out of this one.\"",
            "\"The Empire likes to make examples out of scum like you.\"",
            "\"You've made a huge mistake.\"",
            "\"Bribery carries a harsh penalty, scum.\"",
            "\"I'm not interested in your blood money!\"",
            "\"All the money in the world won't save you now!\""
     }
     mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
     
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

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
            "The Empire will enjoy your death!",
            "Your head will make a fine gift for the Emperor!",
            "None survive the wrath of the Emperor!",
            "Enjoy your last moments, criminal!"
      }
   else
      taunts = {
            "You dare attack me!",
            "You are no match for the Empire!",
            "The Empire will have your head!",
            "You'll regret that!",
            "That was a fatal mistake!"
      }
   end

   ai.comm(target, taunts[ rnd.rnd(1,#taunts) ])
end


