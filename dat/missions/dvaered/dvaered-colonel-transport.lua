--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Escort a Dvaered colonel to a system">
 <unique />
 <priority>1</priority>
 <chance>24</chance>
 <location>Bar</location>
 <spob>Dvaer Prime</spob>
</mission>
--]]
--[[

   MISSION: ESCORT TO A PLANET
   DESCRIPTION: SMALL MISSION WHERE YOU ESCORT AN ARSENAL TO A PLANET: WIP

--]]

local fmt = require "format"


function create ()
   mem.talked = false
   misn.setNPC( _("A Dvaered colonel"), "devaered/dv_military_f6.webp", _("This soldier seems to be a colonel.") )
end

function accept ()
   local text
   else
      text = fmt.f(_([[You approach the Dvaered, who seems to be a soldier, probably one of the colonel rank. Arriving at their table, you are greeted, "Hello! Could you escort my ship, an Arsenal, to {pnt} in the {sys} system? I'll give you {rwd} for it, but I can't tell you why. Well, what do you say?"]]),
         {pnt=misplanet, sys=missys, rwd=reward_text})
      mem.talked = true
   end
   if vntk.yesno( _("Escort Agreed"), text ) then
      vntk.msg( _("Escort Agreed"), _([["Perfect! I'll pay you as soon as we get there."]]) )
      misn.accept()
      misn.osdCreate(_("Dvaered colonel escort"), {
         fmt.f(_("Escort a Dvaered colonel flying an Arsenal to {pnt} in the {sys} system. You haven't been told why, but there may be a large payment.")), {pnt=mem.destspob, sys=mem.destsys}),
      }
      misn.markerAdd( mem.destspob )
      hook.land( "land" )
   end
   local colonel_ship
      colonel_ship = trepeat{Arsenal}
   end
   escort.init ( colonel_ship, {
   })
end

function land ()
   if spob.cur() == misplanet then
      vntk.msg( fmt.f(_([[As you land on {pnt} with the Arsenal close behind, you receive an intercom message. "Thank you for bringing me here!" says the colonel. "Here is {reward}, as we agreed. Have safe travels!"]]), {pnt=misplanet, reward=reward_text}) )
      player.pay( credits )
      neu.addMiscLog( fmt.f(_([[You escorted a Dvaered colonel who was flying an Arsenal to {pnt}. For some reason, they didn't tell you why.]]), {pnt=misplanet} ) )
      misn.finish( true )
   end
end
