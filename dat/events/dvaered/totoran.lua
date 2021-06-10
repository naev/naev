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

spectator_names = {
   _("Spectator"),
}
spectator_descriptions = {
   _("A spectator enjoying their time at the station."),
   _("A spectator taking a break from viewing the action."),
   _("A spectator that looks strangely out of place."),
   _("A spectator enjoying some drinks."),
}
spectator_messages = {
   _([["Sometimes when watching some of the competitions, I forget it is all just virtual reality."]]),
   _([["The realism of virtual reality coliseum is impressive! It almost feels like it's real!"]]),
   function () return string.format(
      _([["I came all the way from %s to be here! We don't have anything like this back at home."]]),
      planet.get( {faction.get("Za'lek"), faction.get("Empire"), faction.get("Soromid")} ):name() -- 
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

--[[
-- Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
