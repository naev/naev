--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 2">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 1</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 02

   Player has to bring back supplies to Zach, first encounter with evil PI's lackeys and feral ship
]]--
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local lmisn = require "lmisn"


local reward = zbh.rewards.zbh02
local cargo_name = _("Repair Supplies")
local cargo_amount = 83 -- Amount of cargo to take

local retpnt, retsys = spob.getS("Research Post Sigma-13")

function create ()
   if not misn.claim( retsys ) then
      misn.finish()
   end

   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 3, 8, "Za'lek", false, function( p )
      return p:tags().industrial
   end )
   if not mem.destpnt then
      misn.finish()
      return
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You approach Zach who seems a bit tired out from all his work on repairing the station.]]))
   z(fmt.f(_([["Hey, I got most of the station working, but to get the rest, including the shipyard, I'm going to need some additional supplies that I can't improvise my way out of. If I could get {amount} of {cargo}, I should be able to get it all operational. I was able to arrange a pick-up at {pnt} in the {sys} system, however, I have to stay here and watch {curpnt}. Would you be willing to bring the {cargo} from {pnt} for {credits}?"]]),
      {pnt=mem.destpnt, sys=mem.destsys, credits=fmt.credits(reward), cargo=cargo_name, amount=fmt.tonnes(cargo_amount), curpnt=retpnt}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([["Thanks again. Once we get this station fully up and running we should be able to access all the memory logs and recorded data and try to make sense of what the hell happened here."]]))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Repairing Sigma-13") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Pick up the necessary supplies at {pnt} in the {sys} system and bring them back to Zach at {retpnt}."),
      {pnt=mem.destpnt, sys=mem.destsys, retpnt=retpnt} ))

   mem.mrk = misn.markerAdd( mem.destpnt )
   mem.state = 1

   misn.osdCreate( _("Repairing Sigma-13"), {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==1 and spob.cur() == mem.destpnt then
      local fs = player.pilot():cargoFree()
      if fs < cargo_amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
            {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
         return
      end

      local lied = false
      local pd = vn.Character.new( _("Za'lek Individual"), {image="zalek_thug1.png"} )
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(fmt.f(_([[You land and quickly the automated loading drones begin to pack the {amount} of {cargo} onto your ship. While you are waiting, someone comes up to you.]]),{amount=fmt.tonnes(cargo_amount), cargo=cargo_name}))
      vn.appear( pd )
      pd(_([[They have an odd grin on their face and you can't help but to get a bad feeling about them.
"Say, you wouldn't be headed to the Anubis Black Hole? Lots of scary stuff there."]]))
      vn.menu{
         {p_("to Evil PI 1", [["Yes."]]), "01yes"},
         {p_("to Evil PI 1", [["No." (Lie)]]), "01no"},
      }

      vn.label("01yes")
      pd(_([["I see. Scary place, lots of things can go wrong there."
They seem to mentally take note of something.]]))
      vn.jump("02")

      vn.label("01no")
      vn.func( function () lied = true end )
      pd(_([["I see. Where are you going to?"
You say a random Za'lek planet but it doesn't seem like they are very interested in your answer.]]))

      vn.label("02")
      pd(_([["You wouldn't happen to be interested in research. You know, helping unravel the secrets of the universe, one step at a time."]]))
      vn.menu{
         {p_("to Evil PI 2", [["Yes."]]), "02cont"},
         {p_("to Evil PI 2", [["No."]]), "02no"},
      }

      vn.label("02no")
      vn.func( function () lied = true end )
      vn.label("02cont")
      pd(_([[Suddenly the grin disappears from their face leaving a scowl in its place.
"When you see Zach, tell him to get away from the black hole, or he won't be going on to a third post-doc."]]))
      pd( function ()
         if lied then
            return _([["Oh, and you're a shitty liar."
They walk away as you stand there dumbstruck.]])
         else
            return _([[They walk away as you stand there dumbstruck.]])
         end
      end )
      vn.run()

      local c = commodity.new( N_("Repair Supplies"), N_("A miscellaneous assortment of Za'lek equipment that seems like it would be pretty useful if needing to perform makeshift repairs on a space station.") )
      misn.cargoAdd( c, cargo_amount )
      misn.osdActive(2)
      mem.state = 2
      misn.markerMove( mem.mrk, retpnt )

   elseif mem.state==2 and spob.cur() == retpnt then

      vn.clear()
      vn.scene()
      local z = vn.newCharacter( zbh.vn_zach() )
      vn.transition( zbh.zach.transition )
      vn.na(fmt.f(_("The first thing you notice as your ship approaches the station is the lack of debris floating around, which allows you to make a landing unhindered. You land and are first greeted by drones that quickly begin to unpack the {cargo} from your ship. Zach appears shortly after the unloading process is underway,"),{cargo=cargo_name}))
      z(fmt.f(_([["I see you were able to bring everything I asked for."
They then notice your worried face and ask what is wrong. You explain the encounter with the individual at {pnt} with the thinly veiled threat made at Zach, and the weird ship you saw outside.]]),
         {pnt=mem.destpnt}))
      z(_([[As you explain the details his expression darkens and he goes quiet for a while before speaking.
"This makes it clear that what happened here was not an accident, but a deliberate act of destruction. This doesn't clear things up, but it is obvious that we must continue to investigate until the end. I have to find out what happened to my colleague and find out who did this."
He trembles slightly with barely contained anger.]]))
      z(_([[He closes his eyes, mutters something you can't make out, and lets out a deep breath.
"I must not let myself get carried away in rage. We must analyse this and proceed methodologically. There is no room for emotion-caused errors. Science guides to glory."
He looks more determined than ever.]]))
      z(_([["We must proceed carefully. It is clear that someone or something does not want us to be here. We also don't know what is going on around here and we're finding more questions than answers. However, as I was told by my first supervisor, to find answers you must first ask the right questions. I will analyse our options, meet me up at the bar once you are ready for the next steps."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      zbh.unidiff( "sigma13_fixed2" )

      faction.modPlayer("Za'lek", zbh.fctmod.zbh02)
      player.pay( reward )
      zbh.log(fmt.f(_("You helped Zach get more supplies to repair {pnt}, but were threatened by an unknown individual, and also ran into an unknown ship leading to more questions than answers."),{pnt=retpnt}))
      misn.finish(true)
   end
end

-- Set up seeing the feral bioship on the way back
local firsttime = true
function enter ()
   if mem.state~=2 or system.cur() ~= retsys or not firsttime then
      return
   end

   firsttime = false
   hook.timer( 3, "heartbeat" )
end

function heartbeat ()
   local pp = player.pilot()
   local d = retpnt:pos():dist( pp:pos() )
   if mem.state==2 and d < 10e3 then
      local pos = player.pos() + vec2.new( 3e3, 9e3 )
      local feral = zbh.plt_icarus( pos )
      feral:setInvisible(true)
      feral:control(true)
      feral:hyperspace( system.get("NGC-2601") )
      zbh.sfx.spacewhale1:play()
      camera.set( feral, false, 4000 )

      player.autonavAbort(_("You thought you saw something!"))

      hook.timer( 10, "cutscene_done" )
      return
   end
   hook.timer( 3, "heartbeat" )
end

function cutscene_done ()
   local pp = player.pilot()
   pp:control(false)
   camera.set()
   hook.timer( 5, "welcome_back")
end

function welcome_back ()
   pilot.comm( _("Sigma-13"), fmt.f(_("Zach: Welcome back {playername}."), {playername=player.name()}) )
end
