local luaspob = require "spob.lua.lib.spob"
local vn = require "vn"
local ccomm = require "common.comm"
local fmt = require "format"
local love_shaders = require "love_shaders"

local contact_var = "protera_nova_first_contact"


function init( spb )
   return luaspob.init( spb, {
      std_bribe = 100,
      msg_granted = {
         _([["You may land on the Directrix."]]),
      },
      msg_notyet = {
         _([["You may not approach the Directrix."]]),
      }
   } )
end

load = luaspob.load
unload = luaspob.unload

function can_land()
   if not var.peek(contact_var) then
      return false, mem.msg_notyet
   else
      return true, mem.msg_granted
   end
end

function comm()
   vn.clear()
   vn.scene()
   if var.peek(contact_var) then
   if faction.playerStanding("Proteron")<0 then
      ccomm.newCharacterSpob( vn, mem.spob, {faction_str="Neutral", name_colour=colour.new("Yellow")} )
   else
      ccomm.newCharacterSpob( vn, mem.spob)
   end
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with the ensigns at {spb}, who chatter excitedly at you."),
      {spb=mem.spob}))

   vn.label("menu")
   vn.menu( function ()
      return {
         { _("Close"), "leave" }
      }
   end )
   vn.label("leave")
   vn.run()

   mem.spob:canLand() -- forcess a refresh of condition
   else
   local spb = ccomm.newCharacterSpob( vn, mem.spob )
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with {spb}. Some opera seems to be playing in the background with someone shouting at someone else to turn it off. \nYou wonder what base would be this poorly managed."),
      {spb=mem.spob}))

   vn.menu( function ()
      return {
         { _("Close"), "leave" },
         { _("Bribe"), "bribe" }
      }
   end )

   vn.label("bribe")
   vn.na(_([["How dare you try to bribe the Sovereign Proteron Autarchy!? We would certainly never take a bribe! I have never taken one and I don't know who would either! I -"]]))
   vn.na(_([["bob, have you started pretending to uphold the ideals of the Proteron to imaginary enemies again?"]])) --bob is intentionally smallcase, do not attempt to change.
   vn.na(_([["Wait, why isn't the video on? Turn the video on! I want to see which holo you spawned this time!"]]))

   vn.disappear(spb) --It's no longer a comm channel, it's now a regular conversation
   local bob = vn.newCharacter(_("bobblehat"), {image="neutral/male2n.webp", color=nil, shader=love_shaders.hologram(), pos="left"})
   local Jen = vn.newCharacter(_("Jennet"), {image="neutral/female1n.webp", color=nil, shader=love_shaders.hologram(), pos="right"})
   Jen(_([[Oh. This isn't a holo, is it? This is real. You weren't playing around this time.]])) --Double quotes dropped to highlight the context change (usual comm -> convo)
   bob(_([[For your information, ensign Jennet, I was never "playing around". I was practicing in order to better defend our glorious Autarchy.]]))
   Jen(_([[Riight, of course... "Practising", huh? I'll keep that in mind.]]))
   vn.menu( function ()
      return {
         { _("Cough loudly"), "cont" },
         { _("..."), "cont" }
      }
   end )
   vn.label("cont")
   Jen(_([[Anyway, back to our visitor! Who are you, how did you get in here, and what is your rank?]]))

   vn.menu( function ()
      local opts = {
         {_("What rank?"), "clueless"}
      }
      if faction.playerStanding("Proteron")>=30 then
         opts = {
         { _("Tell me your rank and name, now, and I won't have you executed for \ninsubordination and disorderly conduct!"), "threat"},
         { _("State your name and rank to me, the senior officer."), "cont2"}
         }
      end
      return opts
   end )
   vn.label("clueless")
   bob(_([[Your rank in the SPA, soldier!]]))
   Jen(_([[bob, I don't think they're from the SPA. They seem to be outsiders.]]))
   bob(_([[Well, no harm in introducing ourselves, I suppose. We're stuck here anyhow and so are they.]]))
   vn.jump("cont2")

   vn.label("threat")
   Jen(_([[Sir, sorry, sir! Will not happen again, sir!]])) --Sir here being a gender-neutral term of command; feel free to change should the need arise.
   --Technically speaking, you never proved your rank, but I suppose an ensign is easily intimidated.
   vn.jump("cont2")

   vn.label("cont2")
   Jen(_([[I am Ensign Jennet of the CCS Directrix, long may she live!]])) --Circle Carrier Ship = CCS; unique ship class (manufacture is just Watson)
   bob(_([[I am Ensign bobblehat, id-code SPA-3216AF6649392F, serving on the CCS Directrix since UST 593:1100.0000.]])) --ID meant to be like an SSN, no sanity checks performed on necessary size
   bob(_([[We've been stuck here since the Incident, and all the senior officers died in the shock or the crash.]]))
   bob(_([[We still remain ready to aid the Autarchy against the Empire in any way we can!]]))
   bob(_([[For now, land on the planet should you wish... We're all stuck here.]]))
   vn.func(function ()
	var.push(contact_var, true)
	end )

   vn.label("leave")
   vn.run()

   mem.spob:canLand() -- forcess a refresh of condition
   end
   return true
end
