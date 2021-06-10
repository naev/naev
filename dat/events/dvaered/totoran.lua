--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Totoran Station Events">
 <trigger>land</trigger>
 <chance>100</chance>
 <cond>planet.cur()==planet.get("Totoran")</cond>
</event>
--]]

--[[
-- Event handling NPCs and such at Totoran
--]]

local vn = require 'vn'
local portrait = require 'portrait'
local totoran = require 'totoran'

spectator_names = {
   _("Spectator"),
   _("Aficionado"),
   _("Coliseum Fan"),
}
spectator_descriptions = {
   _("A person enjoying their time at the station."),
   _("An individual taking a break from viewing the action."),
   _("A spectator that looks strangely out of place."),
   _("A person enjoying some drinks."),
}
spectator_messages = {
   _([["Sometimes when watching some of the competitions, I forget it is all just virtual reality."]]),
   _([["The realism of virtual reality coliseum is impressive! It almost feels like it's real!"]]),
   function () return string.format(
      _([["I came all the way from %s to be here! We don't have anything like this back at home."]]),
      planet.get( {faction.get("Za'lek"), faction.get("Empire"), faction.get("Soromid")} ):name() -- No Dvaered
   ) end,
   _([["The Dvaered sure know how to put on a good show. I love seeing it rain Mace Rockets!"]]),
   _([["It's a shame that they require you to own the ship you want to use to enter the virtual reality competitions. I would love to try fly one of those majestic Dvaered Goddards."]]),
   _([["I like watching the competitions between fighters, it's incredible all the moves they can pull off."]]),
   _([["I tried to compete in my Llama, but it doesn't stand a chance against even a single Hyena."]]),
   _([["I was always told that Dvaered technology was primitive, but the virtual reality coliseum is incredible!"]]),
   _([["There's nothing quite like seeing two capital ships duke it out. I love watching railguns blasting away."]]),
   _([["I used to think the Za'lek virtual games were great, but this is so much better!"]]),
}
pilot_names = {
   _("Coliseum Pilot"),
}
pilot_descriptions = {
   _("A coliseum pilot enjoying some downtime between competitions."),
   _("A tired coliseum pilot taking a break."),
   _("A coliseum pilot lounging around."),
}
pilot_messages = {
   _([["Some people say that the coliseum encourages and promotes violence, but I've been destroying ships long before I started participating in the coliseum!"]]),
   function () return string.format(
      _([["Hey, didn't I see you flying a %s in the Coliseum? Nice flying."]]),
      player.pilot():ship():name()
   ) end,
   _([["I was doing so well in my Hyena, but it just lacks the firepower to take on larger ships. Maybe I should upgrade to a Vendetta."]]),
   _([["The coliseum has really taught me to appreciate the small things in life, you know, blowing up your enemies with mace rockets and such."]]),
   _([["I used to be a pretty sloppy pilot before participating in the coliseum. I still am, but I used to be too."]]),
   _([["Sometimes I get motion sickness from the virtual reality. What's more troublesome is it also happens when I fly my real ship!"]]),
   _([["Sometimes when I get blown up in Coliseum, it takes me a while to realize I haven't actually been blown up to smithereens."]]),
}

guide_priority = 6

function create()
   bgnpcs = {}
   local function create_npc( names, descriptions, msglist, i )
      local name  = names[ rnd.rnd(1, #names) ]
      local img   = portrait.get()
      local desc  = descriptions[ rnd.rnd(1, #descriptions) ]
      local msg   = msglist[i]
      local id    = evt.npcAdd( "approach_bgnpc", name, img, desc, 10 )
      local npcdata = { name=name, image=portrait.getFullPath(img), message=msg }
      bgnpcs[id]  = npcdata
   end

   -- Create random noise NPCs while avoiding duplicate messages
   local spectator_msglist = rnd.permutation( spectator_messages )
   local pilot_msglist = rnd.permutation( pilot_messages )
   for i = 1,rnd.rnd(3,5) do
      if rnd.rnd() < 0.6 then
         create_npc( spectator_names, spectator_descriptions, spectator_msglist, i )
      else
         create_npc( pilot_names, pilot_descriptions, pilot_msglist, i )
      end
   end

   -- Custom NPCs
   npc_guide = evt.npcAdd( "approach_guide", totoran.guide.name, totoran.guide.portrait, totoran.guide.desc, guide_priority )

   hook.takeoff( "leave" )
end

-- Just do random noise
function approach_bgnpc( id )
   local npcdata = bgnpcs[id]
   vn.clear()
   vn.scene()
   local spectator = vn.newCharacter( npcdata.name, { image=npcdata.image } )
   vn.transition()
   spectator( npcdata.message )
   vn.run()
end

function approach_guide ()
   vn.clear()
   vn.scene()
   local guide = vn.newCharacter( totoran.vn_guide() )
   vn.transition()
   vn.na(_("You approach the Totoran Coliseum guide."))
   vn.label("menu_main")
   guide(string.format(_([["Hello and welcome to the Totoran Coliseum! You have %s. What would you like to do?"]]), totoran.emblems_str( totoran.emblems_get())))
   vn.menu{
      {_("Information"), "information"},
      {_("Leave"), "leave"},
   }

   vn.label("information")
   guide(_("What would you like to know about?"))
   vn.label("menu_info_raw")
   vn.menu{
      { _("Totoran Coliseum"), "info_coliseum" },
      { _("Totoran Emblems"), "info_emblems" },
      { _("Nothing."), "menu_main" },
   }

   vn.label("info_coliseum")
   -- incident is 593:3726.4663
   guide(_("The Totoran Coliseum was founded originally in mid-UST 568, as a program for training Dvaered Military. Of course, back then they did not have such good virtual reality technology and instead relied on actual combat. However, as the number of accidents and casualties grew, they ended up starting a move to virtual reality, and now the Totoran Coliseum boasts some of the best full immersion virtual reality experiences in the Universe."))
   guide(_("Eventually, to encourage the warrior spirit among the general population, in UST 588, 20 cycles after its creation, the Totoran Coliseum was open to the general public as a virtual reality experience. This is one of the few places in the universe where you can experience hard space combat without having a fear of dying. If your ship is destroyed, you are only dropped out of the virtual reality environment."))
   guide(_("Many famous pilots have had their formation here, and it is also frequented by scouting agencies from across the Universe to find good pilots. Not to mention all the prizes that can be won from competing in the tournaments."))
   vn.jump("menu_info")

   vn.label("info_emblems")
   guide("TODO")
   vn.jump("menu_info")

   vn.label("menu_info")
   guide(_("Is there anything else you would like to know about?"))
   vn.jump("menu_info_raw")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

--[[
-- Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
