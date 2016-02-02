include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive     = true
mem.safe_distance  = 300
mem.armour_run     = 100
mem.shield_return  = 20
mem.land_planet    = false
mem.careful       = true


function create ()

   -- Give monies.
   ai.setcredits( rnd.int(ai.pilot():ship():price()/600 , ai.pilot():ship():price()/100) )

   -- Get standing.
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
   else
      standing = -1
   end

   -- Handle bribes.
   if standing < -30 then
      mem.bribe_no = "\"The only way to deal with scum like you is with cannons!\""
   else
      mem.bribe = math.sqrt( ai.pilot():stats().mass ) * (300. * rnd.int() + 850.)
      mem.bribe_prompt = string.format("\"It'll cost you %d credits for me to ignore your dirty presence.\"", mem.bribe)
      mem.bribe_paid = "\"Begone before I change my mind.\""
   end

   -- Handle refueling.
   if standing > 70 then
      mem.refuel = rnd.rnd( 1000, 2000 )
      mem.refuel_msg = string.format("\"I should be able to spare some fuel for %d credits.\"", mem.refuel)
   else
      mem.refuel_no = "\"I can't spare fuel for you.\""
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Finish up creation
   create_post()
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
            "I've killed scum far more dangerous than you.",
            "You'll regret that!",
            "Death to the enemies of the Frontier!"
      }
   end

   ai.pilot():comm(target, taunts[ rnd.int(1,#taunts) ])
end

