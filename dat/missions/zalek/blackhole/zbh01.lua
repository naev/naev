--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <faction>Za'lek</faction>
  <location>Bar</location>
  <done>Za'lek Particle Physics 6</done>
 </avail>
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

-- luacheck: globals land enter heartbeat (Hook functions passed by name)

local reward = zbh.rewards.zbh01
local destpnt, destsys = planet.getS("Research Post Sigma-13")

function create ()
   if not var.peek("testing") then misn.finish(false) end
   misn.setNPC( _("Za'lek Scientist"), zbh.zach.portrait, _("You see a Za'lek scientist nerviously sitting at the bar. It seems like they might have a job for you.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You approach the Za'lek scientist who looks at you nervously.]]))
   z(fmt.f(_([[He musters up courage and beigns to speak.
"Say, yo uwouldn't happen to be a pilot that could take me to the {sys} system? My colleagues at {pnt} have gone silent during their investigations and I'm fearing maybe the worst happened to them. I would be able to pay you {credits} for the trip."]]),
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
   z(_([[iHe lets out a long sigh of relief when you accept his task.
"I've been asking for pilots for ages and nobody seems to want to go near the Anubis Black Hole. Too many bad rumours about the area. I was also worried when my colleague went over there. They were stubborn and decided to go for the research opportunity despite my protests. And now, no response in almost half a cycle!"]]))
   z(_([["Even taking into account the ergosphere this is too much of a delay. Something must have happened! I knew I should have stopped her."
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
   misn.setReward( fmt.reward(reward) )
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
   if planet.cur() ~= destpnt then
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.music( 'snd/music/landing_sinister.ogg' ) -- TODO new song? Also add to music.lua
   vn.transition( zbh.zach.transition )
   vn.na(_("As you approach the station you can tell that it's in very bad shape there are clear plasma blast craters on the outside of the shape and you have to avoid debris when trying to dock at the space port. Zach is silent the entire approach and has a dumbfounded look on his face. You don't think this is what he was expecting."))
   z(_([["This can't be happening. It was only supposed to be a two cycle post-doc…"
He looks pale.]]))
   vn.menu{
      {_([["She has to be around somewhere"]]), "cont01"},
      {_([["Let's see what happened"]]), "cont01"},
      {_([[Begin to search]]), "cont01"},
   }
   vn.label("cont01")
   vn.na(_("You begin to search the eerily quiet station. It seems like it became empty quite recently. The base is quite a mess, but knowing the Za'lek, chances are that was the usual state of affairs at the station. You go through al the rooms, but are unable to find any signs of life."))
   vn.na(_("You eventually make your way around the entire station and end up back at the docks. Zach seems to be staring at some of the of the damage."))
   z(_([["Say, doesn't that over there look like bite marks?"]]))
   vn.na(_("You look over at where Zach is pointing and squint a bit. Indeed it does seem like it could be bite marks, but what on earth would bite a station?"))
   z(_([["Something isn't right here. Well, besides the obvious, it just doesn't make sense."
He gives a puzzled look.
"I think I'm going to stay here and try to continue the research. It shouldn't be too hard to reactivate the station. I have to found out what happened to her!"]]))
   z(_([[He goes back to your ship and brings his engineering toolkit and personal drones with him, and gets to. He works in a flurry of determination, and after a short while, you hear the buzz and whirs of a station coming back to life. It seems like he was able to activate it all.]]))
   z(_([["Let me pay you for your services. I'll start seeing if I can figure out what happened and what they were researching on. I don't know much about the details. If you want to help, you should be able to find be around the station while I check everything. I might have things for you to do."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   diff.apply("sigma13_fixed")

   faction.modPlayer("Za'lek", zbh.fctmod.zbh01)
   player.pay( reward )
   zbh.log(fmt.f(_("You took Zach to {pnt} where he found all his colleagues had seemingly met a gruesome fate. He vowed to look into what happened and continue the research that was started. Using his engineering skills, he was able to restore minimum functionality of the station."),{pnt=destpnt}))
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
      pp:comm(_([[Zach: "I hope she's all right."]]))
      msg = 2
   elseif msg==2 and d < 3e3 then
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
