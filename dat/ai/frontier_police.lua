include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive = true


-- Create function
function create ()

   -- Credits.
   ai.setcredits( rnd.int(ai.pilot():ship():price()/300, ai.pilot():ship():price()/100) )

   -- Handle bribing
   if rnd.int() > 0.4 then
      mem.bribe_no = "\"I shall especially enjoy your death.\""
   else
      bribe_no = {
            "\"You insult my honour.\"",
            "\"I find your lack of honour disturbing.\"",
            "\"You disgust me.\"",
            "\"Bribery carries a harsh penalty.\"",
            "\"The Frontier does not lower itself to common scum.\""
     }
     mem.bribe_no = bribe_no[ rnd.rnd(1,#bribe_no) ]
   end

   -- Handle refueling
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 1000, 3000 )
      if standing < 50 then
         mem.refuel_no = "\"Mare magno turbantibus. That means that I don't care about your problems.\""
      else
         mem.refuel_msg = string.format("\"For you I could make an exception for %d credits.\"", mem.refuel)
      end
   end

   -- Handle misc stuff
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   create_post()
end

-- taunts
function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   -- Offense is not actually used
   taunts = {
       "Alea iacta est!",
       "Morituri te salutant!",
       "Your head will make a great trophy!",
       "Cave canem!",
       "Death awaits you!"
   }
   ai.pilot():comm( target, taunts[ rnd.int(1,#taunts) ] )
end

