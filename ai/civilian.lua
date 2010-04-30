include("ai/tpl/generic.lua")
include("ai/personality/civilian.lua")


mem.shield_run = 100
mem.armour_run = 100
mem.defensive  = false
mem.enemyclose = 500
mem.distressmsgfunc = sos


-- Sends a distress signal which causes faction loss
function sos ()
   msg = {
      "Mayday! We are under attack!",
      "Requesting assistance. We are under attack!",
      "Civilian vessel under attack! Requesting help!",
      "Help! Ship under fire!",
      "Taking hostile fire! Need assistance!",
      "We are under attack, require support!",
      "Mayday! Ship taking damage!",
      string.format("Mayday! Civilian %s being assaulted!", ai.shipclass())
   }
   ai.settarget( ai.target() )
   ai.distress( msg[ rnd.int(1,#msg) ])
end


function create ()
   -- TODO: Civilian specific comm replies etc. here
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end

