--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 7">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Antlejos V</spob>
 <cond>require('common.antlejos').unidiffLevel() &gt;= 7</cond>
 <done>Terraforming Antlejos 6</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Try to create alliance with Gordon's Exchange, but PUAAA don't like that
--]]
local vn = require "vn"
local fmt = require "format"
local ant = require "common.antlejos"
local fleet = require "fleet"
local pilotai = require "pilotai"

local reward = ant.rewards.ant07

local retpnt, retsys = spob.getS("Antlejos V")
local mainpnt, mainsys = spob.getS("Gordon's Exchange")


function create ()
   if not misn.claim{mainsys,retsys} then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, ant.verner.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_("You meet up with Verner at the spaceport bar."))
   v(fmt.f(_([["How's it going? {pnt} is starting to take off, but the PUAAA don't cut us any slack. I tried to see if the Empire would do anything about it, and they just send me running around filing paperwork to no avail. Damn useless. However, I've started getting in contact with other independent worlds, and it seems like we could form a trade alliance, hopefully bringing stability to {pnt}. Would you be willing to go to {destpnt} in the {destsys} system to seal an agreement for me?"]]),{pnt=retpnt, destpnt=mainpnt, destsys=mainsys}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(fmt.f(_([["Glad to hear that. So, I've been talking with the traders running {destpnt}, and it seems like they would be willing to enter a trade deal where we would provide them with some processed goods for stuff that we can't easily get here. Especially stuff that requires high gravity for processing is our weakness. The deal is almost set, but as a sign of good faith I just need someone to go and do the final signing ceremony. It should be fairly straight-forward."]]),{destpnt=mainpnt}))
   v(fmt.f(_([["I would want to go do the signing myself, but with the latest incidents and the workload I have here, my hands are all tied. Now, with the last hit you did on the PUAAA supply ship, I'm not expecting any trouble, but you never know. If they knew we were about to strike a trade deal, I'm sure they'd try to stop it. Just in case, make sure to go to {destpnt} armed. Best of luck!"]]), {destpnt=mainpnt}))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Antlejos V Trade Deal") )
   misn.setDesc(fmt.f(_("Go to {pnt} in the {sys} system to try to forge an alliance with the government at {pnt} to support {retpnt}."),{pnt=mainpnt, sys=mainsys, retpnt=retpnt}))
   misn.setReward(reward)
   misn.osdCreate(_("Antlejos V Trade Deal"), {
      fmt.f(_("Go to {pnt} ({sys} system)"),{pnt=mainpnt, sys=mainsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=retpnt, sys=retsys}),
   })
   mem.mrk = misn.markerAdd( mainpnt )
   mem.state = 0

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==0 and spob.cur() == mainpnt then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_("You land and head towards the planet hall to do the signing."))
      vn.na(_("After confirming your appointment at the reception, you are led to an extravagant waiting room filled with famous craftsmanship from all over the universe. There are elegant carpets, ceremonial pottery, and lavish holoart displays. You are ogling them when an assistant comes to take you to another room."))
      vn.na(fmt.f(_("You meet up with a trader who is surprised you aren't Verner, but quickly adjusts to the situation. They discuss trivial topics with you at first before entering the trade details. They seem to be all in order, and you sign as a representative of {pnt}. After shaking hands, you are sent on your way. Dealing with traders is much more efficient than the Empire bureaucracy."),{pnt=retpnt}))
      vn.na(_("As you leave the planet hall, you notice some figures that seem to be trailing you. It looks like you might have company."))
      vn.run()

      -- Advance mission
      mem.state = 1
      misn.markerMove( mem.mrk, retpnt )
      misn.osdActive(2)

   elseif mem.state==1 and spob.cur() == retpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(_("You land and find Verner waiting for you right outside the space docks."))
      v(_([["How did it go? I see, the PUAAA haven't been discouraged by their supply ship getting taken out… This must mean they have someone or something else supporting them… It might also be worth trying to set up our own defence force. As the stakes get higher we can't keep on letting the PUAAA boss us around. I'll have to think of what we can do. In the meantime, we still have lots of things to do."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Done
      player.pay( reward )
      ant.log(fmt.f(_("You eliminated a PUAAA supply ship bringing more security to {pnt}."),{pnt=retpnt}))
      misn.finish(true)
   end
end

local function spawn_protestors( pos, ships )
   local puaaa = ant.puaaa()
   local f = fleet.add( 1, ships, puaaa, pos, _("PUAAA Protestor"), {ai="baddiepos"} )
   for k,p in ipairs(f) do
      p:setHostile(true)
   end
   return f
end

function enter ()
   if mem.state==0 and system.cur()==mainsys then
      spawn_protestors( vec2.new( 3000, 4000 ), {"Lancelot", "Shark", "Shark"} )

   elseif mem.state==1 and system.cur()==mainsys then
      spawn_protestors( vec2.new( -10000, 8700 ), {"Admonisher", "Hyena", "Hyena"} )

   elseif mem.state==1 and system.cur()==retsys then
      local f = spawn_protestors( vec2.new( -10000, 8700 ), {"Pacifier", "Lancelot", "Lancelot"} )
      for k,p in ipairs(f) do
         p:changeAI( "baddiepatrol" )
         pilotai.patrol( p, {
            vec2.new(    0, -2000 ),
            vec2.new( -4000, 4000 ),
            vec2.new( -8000, 2000 ),
         } )
      end
   end
end
