--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Kidnapped">
 <location>enter</location>
 <chance>15</chance>
 <cond>player.misnDone("Kidnapped") == false and player.misnActive("Kidnapped") == false and system.cur() == system.get("Arcturus") and player.outfitNum("Mercenary License") &gt; 0</cond>
 <notes>
  <campaign>Kidnapping</campaign>
  <tier>3</tier>
 </notes>
</event>
--]]
--[[
   Event for kidnapped mission.
--]]
local ccomm = require "common.comm"
local vn = require "vn"
local fmt = require "format"

local pnt1, sys3 = spob.getS("Praxis")

local panma, yohail -- Non-persistent state

--Create Mom and Dad in their spaceship, and have them come from the planet Brooks in Arcturus system, following the player.
function create ()
   -- Test for claim here
   if not naev.claimTest( {system.get("Goddard")}, true) then
      return
   end

   local pnt = spob.get("Brooks")
   panma = pilot.add( "Llama", "Independent", pnt )
   panma:control()
   panma:follow( player.pilot() )
   hook.pilot(panma, "jump", "finish")
   hook.pilot(panma, "death", "finish")
   hook.land("finish")
   hook.jumpout("finish")

   yohail = hook.timer( 6, "hailme" );
end

--Pa and Ma are hailing the player!
function hailme()
   panma:hailPlayer()
   hook.pilot(panma, "hail", "hail")
end

--Pa and Ma have been hailed. The mission can begin, and panma should land on the planet Brooks
function hail()
   local accepted = false

   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, panma )
   vn.transition()

   p(fmt.f(_([["Hello {player}, thank you so much for answering our hail! We really could use your help," says a haggard sounding man over the comm. "It's about our children, my wife's and mine. We were out on a family vacation to Antica, you see, and we were attacked by a gang of pirates…"
"And he thought he could out-fly them," a woman's voice pipes up. "My husband used to be a bit of a pilot, but that was back when we were still dating. He would fly all the way to the Apez system to see me! Before we had children…" The woman trails off.]]),
      {player=player.name()}))
   p(_([[The man quickly speaks up again. "The pirates disabled our ship and we thought we were goners, but when they boarded us, they took our three children and left us! I tried to fight them, but they had my children with knives to their necks… what was I supposed to do? So we got a tow back to Brooks, but now we need to find someone who will rescue our children. We've heard of your skills; will you please help us?"]]))
   vn.menu{
      {_("Rescue those children!"), "accept"},
      {_("Politely refuse"), "refuse"},
   }

   vn.label("refuse")
   p(_([[You can hear that the the man is quite disappointed. "I'm sure you have a good reason to not want to help us. Perhaps you have something else more pressing…" Before the comm is cut you can hear the woman beginning to sob and the man consoling her.]]))
   vn.done()

   vn.label("accept")
   p(fmt.f(_([[The two parents immediately begin thanking you quite profusely, spending a few hectoseconds simply telling you how much they truly appreciate your assistance. After a while, you realize that if these children are going to be rescued this cycle, you are going to need to get started sooner rather than later. "Yes, quite right," the father replies. "No need to delay any longer than absolutely necessary. I don't know a whole lot, but you should be able to eavesdrop on some pirates at a bar. The bar on {pnt} in the {sys} system has been known to serve pirates occasionally, so stopping there would be a good course of action. We will anticipate your return. Again, this means so much to us." Before you know it, the two parents are at it again, thanking you like it's all they know how to do. Before it gets really bad, you bid farewell, break communication, and get on your way.]]),
      {sys=sys3, pnt=pnt1}))
   vn.func( function () accepted = true end )

   vn.run()

   panma:control(false)
   player.commClose()

   if not accepted then
      evt.finish(false)
   end

   naev.missionStart("Kidnapped")
   evt.finish(true)
end

function finish()
   hook.rm(yohail)
   evt.finish()
end
