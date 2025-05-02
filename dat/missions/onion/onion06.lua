--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 06">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <done>Onion Society 05</done>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
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
   Onion 06

   Emergency Conclave Time
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local tut = require "common.tutorial"
local lg = require "love.graphics"

local title = _("Emergency Conclave")

-- Create the mission
function create()
   misn.finish() -- Not ready yet

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_onion() )
   misn.setNPC( _("Onion Society Conclave"), prt.t.tex, _([[It seems like your Ship AI is able to set you up with a connection to the Onion Society Conclave.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(_([[Join the emergency Onion Society conclave.]]))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[Your ship AI {shipai} materializes in front of you.]]),
      {shipai=tut.ainame()}))
   -- Is it truly your SAI, or is it... DOG!??!
   sai(_([["I'll spare you the details, but I believe it may be of your interest to join an emergency Onion Society conclave to keep an eye out on l337_b01."
The hologram slightly flickers.
"I can arrange a connection if you so desire."]]))
   vn.label("menu")
   vn.menu{
      {_([[Establish the connection.]]), "accept"},
      {_([[Maybe later.]]), "decline"},
      {_([[How can you do this?]]), "explain"},
   }

   vn.label("decline")
   vn.na(_([[You think it's maybe best to not join a risky Onion Society Conclave. Maybe later.]]))
   vn.done( tut.shipai.transition )

   vn.label("explain")
   sai(_([["I hope you are not offended, I was able to record the protocol last time you joined, and should be able to run the connections synchronously to join like before."]]))
   sai(_([["I've done some modifications so that your presence to be less noticeable, but it is likely you will be noticed."]]))
   vn.jump("menu")

   vn.label("accept")
   vn.func( function ()
      accepted = true
   end )
   vn.scene()
   local bg_cyberspace = love_shaders.shader2canvas( love_shaders.cyberspace() )
   vn.setBackground( function ()
      lg.setColour( 1, 1, 1, 1 )
      bg_cyberspace:draw( 0, 0 )
   end )
   vn.transition("fadeup")
   vn.na(_([[You accept and strap into a full sensorial holodeck. Once again you are thrust into an uncanny feeling as you plunge into the Nexus.]]))
   if var.peek( "nexus_sickness" ) then
      vn.na(_([[At least this time you seem to be much less affected by the Nexus syndrome.]]))
   end

   vn.scene()
   local offset = 1/7
   local l337 = vn.newCharacter( onion.vn_nexus_l337b01{pos=1*offset} )
   local underworlder = vn.newCharacter( onion.vn_nexus_underworlder{pos=2*offset, flip=true} )
   local puppet = vn.newCharacter( onion.vn_nexus_notasockpuppet{pos=3*offset, flip=true} )
   -- Trixie would be at 4*offset, but we keep it empty on purpose
   local dog = vn.newCharacter( onion.vn_nexus_dog{pos=5*offset, flip=false} )
   local lonewolf4 = vn.newCharacter( onion.vn_nexus_lonewolf4{pos=6*offset, flip=false} )
   vn.music( onion.loops.circus )
   vn.transition()
   vn.na(_([[You join what seems like an already started Conclave.]]))
   l337(_([["And that's it? TRIXIE WAS DAMN PEELED, YOU HEARTLESS BASTARDS!"]]))
   puppet(_([["Oh please, just because your protégé screwed up, it's no need to cause a scene. They weren't even a keeper!"]]))
   lonewolf4(_([["Trixie's untimely demised is another grave misfortune, yet another in this cosmic tragedy. We shall spare some tears, but the Onion Society shall persist, such as it always has, in defiance of annihilation."]]))
   dog(_([["..."]]))
   l337(_([["You MONSTERS! All you think about is power! We're supposed to be a SOCIETY!"]]))
   puppet(_([["We're a society of MONSTERS! Ha ha ha"]]))
   underworlder(_([["And what would you have us do? Risk everything to save Trixie? We can't afford more losses, only 4 keepers are left. And you know what that means."]]))
   puppet(_([["4 keepers! 4 monster keepers! Ha ha ha"]]))
   lonewolf4(_([["Silence! Begone is the time of buffoonery! Are naught the outcomes of these perilous times decided by the council?"]]))
   vn.musicPitch( nil, 1.1 ) -- Music should be faster and more chaotic if possible
   l337(_([["THERE WOULD BE 5 KEEPERS STILL IF WE HAD SAVED TRIXIE!"]]))
   puppet(_([["5 keepers! 5 keepers! Ha ha ha"]]))
   vn.na(_([[lonewolf4 mutters something under their breath and puppet stiffens up.]]))
   underworlder(_([["C'mon, no rough play in the council."]]))
   puppet(_([[notasockpuppet flickers back and frowns at lonewolf4.
"Bad wolf."]]))
   lonewolf4(_([["Shallst thus endure, then it will be up to each one to fond for naught other than themselves. Woe is such an end of the mighty Onion Society!"]]))
   l337(_([["I've had enough of this THEATRICAL BULLSHIT. Why'd you do it lonewolf4, huh? Why!?"]]))
   lonewolf4(_([["What vile thoughts dost thou put forth, wretch?"]]))
   vn.musicPitch( nil, 1.2 ) -- Music should be faster and more chaotic if possible
   l337(_([["After T-Trixie got peeled, I found a packet with audio. YOUR NAME WAS ON IT LONEWOLF4!"]]))
   puppet(_([[notasockpuppet seems to be enjoying this.]]))
   dog(_([[...]]))
   lonewolf4(_([["Lo, each accusation is but the echo of one's own sin. You speak as though you are special, as though you are different, but hesitated not to kill everyone, including keeper v3c70r on Tenebros Station to save your own hide! Wolf forgets not!"]]))
   l337(_([["THAT'S NOT... Not... not..."
Their voice fades out.]]))
   lonewolf4(_([["Thou canst not deny the weight of this evidence! Thou hast slain v3c70r as thou hast now slain Trixie, and seekest to lay the blame at mine own feet, that thou might further thy own wicked designs!"]]))
   l337(_([[...]]))
   lonewolf4(_([["Behold ye all! Silence! They offer no rebuttal for their exposed sins! Perhaps the peeling of DEADBEEF was also of thy doing!"]]))
   l337(_([[The name DEADBEEF seems to resonate in l337_b01.
"YOU! YOU DON'T KNOW ANYTHING! GO TO HELL!"]]))
   vn.disappear( l337 )
   vn.na(_([[l337_b01 drops the connection.]]))
   lonewolf4(_([["And thus truth exposed."]]))
   lonewolf4(_([[They suddenly focus their gaze on you.
"And what do we have here?"]]))

   vn.scene()
   vn.musicStop()
   vn.transition("fadedown")
   vn.na(_([[You abruptly are dropped out of the Nexus. Ugh, and there goes your last meal with it.]]))
   vn.na(_([[That did not go very well, maybe you should follow up on l337_b01 once you feel a bit better.]]))
   vn.done()
   vn.run()

   if not accepted then return end

   misn.finish(true)
end
