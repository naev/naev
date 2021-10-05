--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Runaway">
  <flags>
    <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>11</chance>
   <location>Bar</location>
   <system>Gamma Polaris</system>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[
This is the "The Runaway" mission as described on the wiki.
There will be more missions to detail how you are perceived as the kidnapper of "Cynthia"
--]]

local fmt = require "format"
local neu = require "common.neutral"


npc_name = _("Young Teenager")
title = _("The Runaway")
cargoname = N_("Cynthia")
cargodesc = N_("A young teenager.")
misn_desc = _("Deliver Cynthia safely to %s in the %s system.")

osd_text = {}
osd_text[1] = _("Deliver Cynthia to Zhiru in the Goddard system")

log_text = _([[You gave a teenage girl named Cynthia a lift to Zhiru. When you got there, she suddenly disappeared, leaving behind a tidy pile of credit chips and a worthless pendant.]])


function create ()
   startworld, startworld_sys = planet.cur()
   targetworld_sys = system.get("Goddard")
   targetworld = planet.get("Zhiru")
   reward = 500e3

   misn.setNPC( npc_name, "neutral/unique/cynthia.webp", _("A pretty teenager sits alone at a table.") )
end


function accept ()
   --This mission does not make any system claims
   if not tk.yesno( title, string.format( _([[She looks out of place in the bar. As you approach, she seems to stiffen.
    "H..H..Hi", she stutters. "My name is Cynthia. Could you give me a lift? I really need to get out of here.
    I can't pay you much, just what I have on me, %s." You wonder who she must be to have this many credits on her person. "I need you to take me to Zhiru."
    You wonder who she is, but you dare not ask. Do you accept?]]), fmt.credits(reward), targetworld:name() ) ) then
      misn.finish()
   end

   --Our *cargo* weighs nothing
   --This will probably cause a mess if this fails
   if player.pilot():cargoFree() < 0 then
      tk.msg( title, _("Your cargo hold doesn't have enough free space.") )
      misn.finish()
   end

   misn.accept()

   misn.osdCreate(title,osd_text)
   misn.osdActive(1)

   local c = misn.cargoNew( cargoname, cargodesc )
   cargoID = misn.cargoAdd( c, 0 )

   misn.setTitle( title )

   misn.setReward( string.format( _("%s on delivery."), fmt.credits(reward) ) )

   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )
   misn.markerAdd( targetworld_sys, "high")

   tk.msg( title, _([["Thank you. But we must leave now, before anyone sees me."]]) )

   hook.land("land")
end

function land ()
  --If we land, check if we're at our destination
   if planet.cur() == targetworld then
      misn.cargoRm( cargoID )
      player.pay( reward )

      tk.msg( title, string.format( _([[As you walk into the docking bay, she warns you to look out behind yourself.
    When you look back to where she was, nothing remains but a tidy pile of credit chips and a worthless pendant.]]), fmt.number(reward) ) )

      neu.addMiscLog( log_text )

      misn.finish(true)
   end
end

