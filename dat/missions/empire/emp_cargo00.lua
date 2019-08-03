--[[

   Simple cargo mission that opens up the Empire cargo missions.

   Author: bobbens
      minor edits by Infiltrator

]]--

include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

bar_desc = _("You see an Empire Lieutenant who seems to be looking at you.")
misn_title = _("Empire Recruitment")
misn_reward = _("%s credits")
misn_desc = _("Deliver some parcels for the Empire to %s in %s.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Empire Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[You approach the Empire Lieutenant.
"Hello, I'm Lieutenant Czesc from the Empire Armada Shipping Division. We're having another recruitment operation and would be interested in having another pilot among us. Would you be interested in working for the Empire?"]])
text[2] = _([["Welcome aboard," says Czesc before giving you a firm handshake. "At first you'll just be tested with cargo missions while we gather data on your flying skills. Later on, you could get called upon for more important missions. Who knows? You could be the next Yao Pternov, greatest pilot we ever had in the armada."
    He hits a couple buttons on his wrist computer, which springs into action. "It looks like we already have a simple task for you. Deliver these parcels to %s. The best pilots started delivering papers and ended up flying into combat against gigantic warships with the Interception Division."]])
text[3] = _([[You deliver the parcels to the Empire Shipping station at the %s spaceport. Afterwards, they make you do some paperwork to formalise your participation with the Empire. They tell you to keep an eye out for missions labeled ES, which stands for Empire Shipping, in the mission computer, to which you now have access.
    You aren't too sure of what to make of your encounter with the Empire. Only time will tell...]])


function create ()
   -- Note: this mission does not make any system claims.
   local landed, landed_sys = planet.cur()

   -- target destination
   local planets = {} 
   getsysatdistance( system.cur(), 1, 6,
       function(s)
           for i, v in ipairs(s:planets()) do
               if v:faction() == faction.get("Empire") and v:canLand() then
                   planets[#planets + 1] = {v, s}
               end
           end 
           return false
       end ) 
   if #planets == 0 then abort() end -- Sanity in case no suitable planets are in range. 
   local index = rnd.rnd(1, #planets)
   dest = planets[index][1]
   sys = planets[index][2]

   misn.setNPC( _("Lieutenant"), "empire/unique/czesc" )
   misn.setDesc( bar_desc )
end


function accept ()
   misn.markerAdd( sys, "low" )

   -- Intro text
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission details
   reward = 30000
   misn.setTitle(misn_title)
   misn.setReward( string.format(misn_reward, numstring(reward)) )
   misn.setDesc( string.format(misn_desc,dest:name(),sys:name()))

   -- Flavour text and mini-briefing
   tk.msg( title[2], string.format( text[2], dest:name() ))
   misn.osdCreate(title[2], {misn_desc:format(dest:name(),sys:name())})

   -- Set up the goal
   parcels = misn.cargoAdd("Parcels", 0)
   hook.land("land")
end


function land()

   local landed = planet.cur()
   if landed == dest then
      if misn.cargoRm(parcels) then
         player.pay(reward)
         -- More flavour text
         tk.msg(title[3], string.format( text[3], dest:name() ))
         var.push("es_cargo", true)
         faction.modPlayerSingle("Empire",3);
         misn.finish(true)
      end
   end
end

function abort()
   misn.finish(false)
end
