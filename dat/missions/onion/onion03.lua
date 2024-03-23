--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 03">
 <unique />
 <priority>3</priority>
 <chance>20</chance>
 <location>Bar</location>
 <done>Onion Society 02</done>
 <cond>
   if spob.cur() == spob.get("Tepadania Prime") then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion03
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local love_shaders = require "love_shaderS"

-- Action happens in the jump from Tepadania (Empire) to Ianella (Dvaered)
-- l337_b01 lives in Anubis (Scorpius)?
local spb1, sys1 = spob.getS("Tepdania Prime")
--local jp = jump.get( sys1, "Ianella" )

--local money_reward = onion.rewards.misn03

local title = _("Enter the Nexus")

--[[
   Mission States
   0: mission accepted
   1: plugged into grid
   2: defended the relay
--]]
mem.state = 0

-- Create the mission
function create()
   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_onion() )

   misn.setNPC( _("l337_b01"), prt.t.tex, _([[You seem to have an incoming connection from the Onion Society.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Help l337_b01 short the Empire-Dvaered Nexus connection near the {sys} system.]]),
      {sys=sys1}))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = vn.newCharacter( onion.vn_l337b01() )
   vn.transition("electric")
   vn.na(_([[You answer the incoming connection and a familiar hologram appears on-screen.]]))
   l337(fmt.f(_([["Hey {player}, how's it going? Anyway, enough small chat. One of my relay backdoors seems to be down, and I need you to short the Empire-Dvaered connection for me in {sys}. You up for the task, right?"]]),
      {player=player.name(), sys=sys1}))
   vn.menu{
      {_([["What does that even mean?"]]), "01_meaning"},
      {_([["Shouldn't you flim-flam the wurglebump?"]]), "01_flimflam"},
      {_([["Count on me!"]]), "01_ok"},
      {_([["Maybe next time."]]), "decline"},
   }

   vn.label("01_ok")
   vn.func( function () accepted = true end )
   vn.jump("01_cont")

   vn.label("01_flimflam")
   l337(_([["Flim-flam the wurglebump? Wait... I see. It all flew over your head, didn't it? Cute, but you could have just said you understood jack shit."]]))
   vn.label("01_meaning")
   l337(_([["Let me see if I can explain this for you to understand. It's quite simple, actually."]]))
   vn.label("01_cont")
   l337(_([["You familiar with the Nexus right? It's the digital hodgepodge that connects everyone together. Lets you send messages and data all over a galaxy, it's how news and information gets around. Really nice to get in touch with people, as long as you can afford it."]]))
   l337(_([["The only way to go faster than the speed of light and actually make the Nexus possible is to use jumpgate Nexus relays, that accumulate local data, and then jump through the gate to transmit it to the other side. Voilà, super light speed communication."]]))
   l337(_([["Of course, everyone wants in with this, so naturally, unless you pay, you can spend ages trying to go through a relay. And normally, you want to go through more than one relay to get anywhere. Meant to connect humanity, it's now just another extension of our lovely class system."]]))
   l337(_([["That's the explanation for normal people anyway, Onion Society doesn't really play by the rules, and we just short the relays to communicate with everyone, all the time, as it should be. In practice, it's not that pretty though. Poorly maintained hardware is constantly failing, and nobody seems to give a rat's ass about reliability these days."]]))
   l337(_([["An Onion Society meeting is coming up in the Nexus, and just my bad luck, the Empire-Dvaered connection seems to have had some critical hardware repaired. I need you to be my hands in the, er, non-digital realm, and help me short it so that all is good and information flows free."]]))
   vn.func( function ()
      if accepted then
         vn.jump("accepted")
      end
   end )
   l337(fmt.f(_([["Should be an easy job in the {sys} system. You in?"]]),
      {sys=sys1}))
   vn.menu{
      {_([[Accept the job.]]), "accepted"},
      {_([[Decline for now.]]), "decline"},
   }

   vn.labelb("decline")
   vn.na(_([[You decline the work for now, and the hologram fades away.]]))
   vn.done("electric")

   vn.label("accepted")
   vn.func( function () accepted = true end )
   l337(fmt.f(_([["I've hooked up into your ship, so you don't have to worry about anything, just get me close and I'll jump through your ship systems. I should have enough bandwidth for that. Anyway, head to {spob} in the {sys} system, and we can fix this."]]),
      {spob=spb1, sys=sys1}))
   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.markerAdd( spb1 )
   mem.state = 0
   hook.land("land")
   hook.enter("enter")
end

function land ()
   if spob.cur()==spb1 and mem.state==0 then
      vn.clear()
      vn.scene()
      local l337 = vn.newCharacter( onion.vn_l337b01() )
      vn.transition("electric")
      vn.na()
      l337()
      vn.done("electric")
      vn.run()

      mem.state = 1
   end
end

function enter ()
end
