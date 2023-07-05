--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 1">
 <unique />
 <priority>4</priority>
 <chance>30</chance>
 <faction>Za'lek</faction>
 <cond>
   if faction.playerStanding("Za'lek") &lt; 0 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <location>Bar</location>
 <done>Za'lek Particle Physics 6</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 01

   Introductory mission that sets the tone where you bring Zach to Research Post Sigma-13
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"


local reward = zbh.rewards.zbh01
local destpnt, destsys = spob.getS("Research Post Sigma-13")

function create ()
   misn.setNPC( _("Za'lek Scientist"), zbh.zach.portrait, _("You see a Za'lek scientist nervously sitting at the bar. It seems like they might have a job for you.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You approach the Za'lek scientist who looks at you nervously.]]))
   z(fmt.f(_([[He musters up courage and begins to speak.
"Say, you wouldn't happen to be a pilot that could take me to the {sys} system? My colleagues at {pnt} have gone silent during their investigations and I'm fearing maybe the worst happened to them. I would be able to pay you {credits} for the trip."]]),
      {pnt=destpnt, sys=destsys, credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(fmt.f(_([["Thanks anyways."
He lets out a sigh and goes back to nervously looking for a pilot to take him to the {sys} system.]]),
      {sys=destsys}))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([[He lets out a long sigh of relief when you accept his task.
"Thanks, my name is Zack Xiao. I've been asking pilots for ages and nobody seems to want to go near the Anubis Black Hole. Too many bad rumours about the area. I was also worried when my colleague went over there. They were stubborn and decided to go for the research opportunity despite my protests. And now, no response in almost half a cycle!"]]))
   z(_([["Even taking into account the ergosphere this is too much of a delay. Something must have happened! I knew I should have stopped them."
He seems visibly distraught and you try to soothe him.]]))
   z(fmt.f(_([[He goes on in a smaller shaky voice.
"Please take me to {pnt}, so we can see it's all fine, and they just got absorbed in their research again…"]]),
      {pnt=destpnt}))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   local c = commodity.new( N_("Zach"), N_("A worried Za'lek scientist.") )
   misn.cargoAdd(c, 0)

   -- mission details
   misn.setTitle( _("Black Hole Research") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Take Zach to see what happened to his colleagues at {pnt} in the {sys} system."),
      {pnt=destpnt, sys=destsys} ))

   misn.markerAdd( destpnt )

   misn.osdCreate( _("Black Hole Research"), {
      fmt.f(_("Take Zach to {pnt} ({sys} system)"), {pnt=destpnt, sys=destsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if spob.cur() ~= destpnt then
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.music( 'snd/music/landing_sinister.ogg' ) -- TODO new song? Also add to music.lua
   vn.transition( zbh.zach.transition )
   vn.na(_("As you approach the station you can can make out clear plasma blast craters on the outer hull. Your ship has to push away floating debris as you make a landing approach at the space docks while Zach is silent the entire time with a dumbfounded look on his face. You don't think this is what he was expecting. After what seems like an eternity, the ship's magnetic docking clamps make contact with the station."))
   z(_([["This can't be happening. It was only supposed to be a two cycle post-doc…"
He looks pale.]]))
   vn.menu{
      {_([["They have to be around somewhere"]]), "cont01"},
      {_([["Let's see what happened"]]), "cont01"},
      {_([[Begin to search]]), "cont01"},
   }
   vn.label("cont01")
   vn.na(_("You and Zach don your space suits and begin to search the atmosphere-less breached station in eerie silence. A floating spoon, some dirty boots… It seems like the station had inhabitants in it until quite recently. Although the base is quite a mess, knowing the Za'lek, chances are that was the usual state of affairs at the station. As you go through all the rooms one by one, but are unable to find any signs of life. During the whole ordeal, Zach remains silent and concentrated."))
   vn.na(fmt.f(_("You eventually make your way around the entire station and end up back at the docks, near your ship, {shipname}. Zach seems to be staring at some of the of the damage. As you struggle with how to break the silence, you notice that Zach has become fixed on some structural damage."), {shipname=player.pilot()} ))
   z(_([["Say, doesn't that over there look like bite marks?"]]))
   vn.na(_([[You look over at where Zach is pointing, but can't really make out what he's getting at. However, after a while of staring and squinting, you seem to make out some damage that could indeed be bite marks, but what on earth would bite a station?]]))
   z(_([["Something isn't right here."
He stops one second realizing what he just said and gives a puzzled look.
"Well, besides the obvious, it just doesn't make sense. Why would anyone want to do this to a basic research laboratory? There is nothing over here other than the black hole…"]]))
   z(_([[He goes silent and seems to be lost in thought. Finally, he looks up and looks around the ship, his gaze becoming determined. He then turns to you.
"I think I'm going to stay here and try to continue the research. It shouldn't be too hard to reactivate basic station functionality. I would never forgive myself if I went back without finding out what happened to them."]]))
   z(_([[Zach goes back to your ship and brings his engineering toolkit and personal drones with him. He starts floating around scanning the hull panels of the station, apparently looking for something. Eventually, he seems to have found what he was looking for, and he rips off a panel. He motions to the drones and disappears with them into the depths of the station internals.]]))
   vn.na(_([[You float close by and peer through but can't make much out in the dark tangle of cables and electronics. Only occasional flashes of light indicate the presence of Zach and his drones, with a drone occasionally coming out to drop damaged electronics and procure new supplies. Knowing you won't be of help, you wait at the docks wondering how long it'll take him, and begin to nod off.]]))
   vn.na(_([[Suddenly, you are awoken by the buzz, whirs, and groans of a station coming back to life. Alarms blaze as the station begins to generate a breathable atmosphere and weak gravitational field. While you are distracted by all the commotion, you feel a tap on your shoulder and turn around to see a grinning Zach, helmet in hand.]]))
   z(_([["The gravitational flux quantifier was bust, but I managed to hot-wire it with a reverse Fleuret capacitor. The station isn't in great shape, but it's a good starting point to get things working like they should be. You can take off your helmet now."]]))
   vn.na(_([[You unlatch your helmet, and take a breath of the new atmosphere. It smells heavily of ionization but has a weird musty tang to it which makes you grimace slightly.]]))
   z(_([["You smell that too? I have no idea what it is, but it should go away once the filters get cycled. Here, let me pay you for your services. I'll start seeing if I can figure out what happened to this station and what exactly they were researching. I'm afraid I'm currently in the dark and don't really know much about the details. If you want to help, you should be able to find me around the station while I finish up the repairs. I might have things for you to help with."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   zbh.unidiff( "sigma13_fixed1" )

   faction.modPlayer("Za'lek", zbh.fctmod.zbh01)
   player.pay( reward )
   zbh.log(fmt.f(_("You took Zach to {pnt} where he found that his colleagues had seemingly met a gruesome fate. He vowed to look into what happened and continue the research that was started. Using his engineering skills, he was able to restore minimum functionality of the station."),{pnt=destpnt}))
   music.play()
   misn.finish(true)
end

local firsttime = true
function enter ()
   if system.cur() ~= destsys or not firsttime then
      return
   end

   firsttime = false
   hook.timer( 3, "heartbeat" )
end

local msg = 0
function heartbeat ()
   local pp = player.pilot()
   local d = destpnt:pos():dist( pp:pos() )
   if msg==0 then
      player.autonavReset(3)
      pp:comm(fmt.f(_([[Zach: "{pnt} should be towards the black hole."]]),{pnt=destpnt}))
      msg = 1
   elseif msg==1 and d < 10e3 then
      player.autonavReset(3)
      pp:comm(_([[Zach: "I hope they're all right."]]))
      msg = 2
   elseif msg==2 and d < 3e3 then
      music.stop()
      player.autonavReset(3)
      pp:comm(_([[Zach: "Everything looks too quiet."]]))
      msg = 3
   elseif msg==3 and d < 1e3 then
      player.autonavReset(3)
      pp:comm(_([[Zach: "Are those blast marks?"]]))
      return
   end
   hook.timer( 3, "heartbeat" )
end
