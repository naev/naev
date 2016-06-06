include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true


function create ()

   -- Not too many credits.
   ai.setcredits( rnd.rnd(ai.pilot():ship():price()/300, ai.pilot():ship():price()/70) )

   -- Get refuel chance
   p = player.pilot()
   if p:exists() then
      standing = ai.getstanding( p ) or -1
      mem.refuel = rnd.rnd( 2000, 4000 )
      if standing < 20 then
         mem.refuel_no = "\"The warriors of Sorom are not your personal refueller.\""
      elseif standing < 70 then
         if rnd.rnd() > 0.2 then
            mem.refuel_no = "\"The warriors of Sorom are not your personal refueller.\""
         end
      else
         mem.refuel = mem.refuel * 0.6
      end
      -- Most likely no chance to refuel
      mem.refuel_msg = string.format( "\"I suppose I could spare some fuel for %d credits.\"", mem.refuel )
   end

   -- Handle bribing
   if rnd.int() > 0.4 then
      mem.bribe_no = "\"I shall especially enjoy your death.\""
   else
      bribe_no = {
            "\"Snivelling waste of carbon.\"",
            "\"Money won't save you from being purged from the gene pool.\"",
            "\"Culling you will be doing humanity a service.\"",
            "\"We do not consort with vermin.\"",
			"\"Who do you take us for, the Empire?\""
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
            "Culling you will be doing humanity a service.",
            "Enjoy your last moments, worm!",
			"Time for a little natural selection!",
			"Might makes right!",
			"Embrace your weakness!"
      }
   else
      taunts = {
            "Cunning, but foolish.",
            "Ambush! Defend yourselves!",
            "You should have picked easier prey!",
            "You'll regret that!",
            "That was a fatal mistake!"
      }
   end

   ai.pilot():comm(target, taunts[ rnd.rnd(1,#taunts) ])
end


