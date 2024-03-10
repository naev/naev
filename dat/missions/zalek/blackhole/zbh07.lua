--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 7">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 6</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 07

   Just a cut scene about the surgery, nothing fancy
]]--
local vn = require "vn"
local zbh = require "common.zalek_blackhole"
local zpp = require "common.zalek_physics"
local lg = require "love.graphics"

function create ()
   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local completed = false
   local state = 0
   local drones = 0

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   local icarus = zbh.vn_icarus()
   vn.transition( zbh.zach.transition )
   vn.na(_([[You meet with Zach who seems to be doing the last preparations for the surgery.]]))
   z(_([["Heya. I think I got this finally figured out. So most of the anatomy is like that of an arthropod, however, the inner scaffolding follows ship designs. It's incredibly well thought out the entire layout, and it also guarantees that no two ships will ever be exactly alike!"
He seems fairly excited about the entire prospect.]]))
   z(_([["There are still some rough details here and there, but I feel confident enough to try to improvise live as necessary. No matter how many simulations I run, I don't think it's going to be anything like the real thing. Although Icarus isn't really showing signs of stress, my scans indicate that it's probably better to try to operate sooner than later. Would you be willing to assist me?"]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([["Great! So let's get preparations started then. The tricky part will be getting Icarus all set up. I think it is more trusting of us, so hopefully we'll be able do it without a hitch. You seem to get along pretty well with Icarus, I still have to do some final runs on the surgery software and prepare the drones. Could you try to get Icarus into the docks and calm enough for surgery? I'll be back soon."
Before you have a chance to respond, Zach leaves to do the final preparations.]]))
   vn.disappear( z )
   vn.appear( icarus )
   vn.na(_([[You head to the docks, where you can see Icarus lazily floating around at a distance. Now the hard part begins, how to get Icarus to come inside and be docile enough for getting ready for surgery. What do you do?]]))

   vn.label("menu")
   vn.menu( {
      {_("Send drones to get Icarus"), "drones"},
      {_("Use snacks to attract Icarus"), "snacks"},
      {_("Send a translated 'Come.' message"), "come"},
      {_("Wait for Icarus"), "wait"},
      {_("Give up (#rFails Mission#0)"), "giveup"},
   }, function( i )
      if i=="giveup" then
         vn.jump("giveup")
         return
      elseif i=="drones" then
         state = -1
         drones = drones+1
         if drones>=3 then
            vn.jump("toomanydrones")
         else
            vn.jump("drones")
         end
         return
      end

      -- Two routes to beat this
      --    come -> wait -> wait
      --    snacks -> come -> wait
      if state==-1 then
         if i=="wait" then
            state = 0
            vn.jump("calmdown")
         else
            vn.jump("toogiddy")
         end
      elseif state==0 then
         if i=="come" then
            state = 1
            vn.jump("aftercome")
         elseif i=="snacks" then
            state = 3
            vn.jump("snacks")
         elseif i=="wait" then
            vn.jump("idle")
         end
      elseif state==1 then
         if i=="wait" then
            state = 2
            vn.jump("aftercome")
         elseif i=="come" then
            state = 1
            vn.jump("aftercome")
         elseif i=="snacks" then
            state = 3
            vn.jump("snacks")
         end
      elseif state==2 then
         if i=="wait" then
            vn.jump("completed")
         elseif i=="come" then
            state = 1
            vn.jump("aftercome")
         elseif i=="snacks" then
            state = 3
            vn.jump("snacks")
         end
      elseif state==3 then
         if i=="come" then
            state = 4
            vn.jump("aftercome")
         elseif i=="snacks" then
            vn.jump("snacks")
         elseif i=="wait" then
            vn.jump("idle")
         end
      elseif state==4 then
         if i=="wait" then
            vn.jump("completed")
         elseif i=="snacks" then
            state = 3
            vn.jump("snacks")
         elseif i=="come" then
            vn.jump("aftercome")
         end
      end
   end )

   vn.label("idle")
   icarus(_("Icarus keeps on lazily floating at a distance without a care in the world."))
   vn.jump("menu")

   vn.label("aftercome")
   icarus(_("Icarus keeps on floating at a distance. However, you could swear they are starting to get closer."))
   vn.jump("menu")

   vn.label("calmdown")
   icarus(_("You stand there in silence, locked in a staring contest with Icarus. Eventually, as time passes, Icarus seems to relax a bit."))
   vn.jump("menu")

   vn.label("snacks")
   icarus(_([[You grab one of the snacks that Zach had you bring over to the station. Nothing like tasty "Liquified Bioship Organic Slush Compounds"? That doesn't sound tasty… You toss the heavy cylinder over in Icarus's direction.]]))
   vn.sfx( zbh.sfx.bite )
   icarus(_([[They seem to ignore it for a while, until they suddenly swoop in and devour it in an impressive bite, before continuing to float nearby.]]))
   vn.jump("menu")

   vn.label("toogiddy")
   icarus(_("Icarus is too nervous to pay attention to you and keeps on eyeing you from a distance."))
   vn.jump("menu")

   vn.label("drones")
   icarus(_("The moment Icarus sees the drones he runs away to a safe distance from the station, eyeing you warily. It looks they will need to be calmed down first."))
   vn.jump("menu")

   vn.label("completed")
   vn.func( function () completed = true end )
   icarus(_([[Finally, Icarus seems to respond to your request and they slowly float into the space docks. This is the first time Icarus has come so close and they seem fairly relaxed. You feel like you've finally managed to bond with Icarus.]]))
   vn.na(_([[While you are sharing your tender moment with Icarus, you suddenly notice movement out of the corner of your eye. It seems like Zach is staring down at the docks from the elevated observation back. He signals for you to back up a bit and you do.]]))
   vn.scene()
   vn.music()
   vn.func( function ()
      local lw, lh = lg.getDimensions()
      vn.setBackground( function ()
         lg.setColour( 1, 1, 1, 1 )
         lg.rectangle( "fill", 0, 0, lw, lh )
      end )
      music.stop()
   end )
   vn.transition()
   vn.na(_([[Suddenly, you hear a high pitch drone of an energy capacitor draining. Icarus who hears that to and scrambles to get out of the docks, however, before he can, you are blinded by a flash and knocked off to the side.]]))
   vn.na(_([[You struggle to stand groggily as your eyes start recovering and colour returns to the world. You find yourself amidst a flurry of drones led by Zach who have quickly started to perform the surgery. Given that you are still completely useless in your current state, you drag yourself out of the way and lean against a wall. Time seems to fly by without you noticing, and you close your eyes.]]))
   vn.na(_("…"))
   vn.music( "snd/sounds/beam_fuzzy.ogg" )
   vn.na(_("You nod off and begin to have concussion-induced dreams where you are surrounded by mosquitoes that are buzzing around and making lots of noise. Every so often one of the mosquito says an obscenity, while you sort of float around with them."))
   vn.na(_("…"))
   vn.scene()
   vn.music()
   local shd_nebula = zpp.shader_nebula()
   shd_nebula.progress = 1
   shd_nebula:send("u_mode", 1)
   vn.func( function ()
      vn.setBackground( function ()
         shd_nebula:render()
      end )
      vn.setUpdateFunc( function( dt )
         shd_nebula:update( dt )
      end )
      music.stop()
   end )
   vn.transition( "dreamy" )
   vn.na(_("Now suddenly the mosquito buzz fades away and you see yourself in the middle of a vortex of red, which spins you around and around, and fills up the entire space around you. You fidget and struggle, but you can't go anywhere or move, you are lost in the vastness."))
   vn.scene()
   vn.func( function ()
      vn.setBackground()
      vn.setUpdateFunc()
      music.play()
   end )
   vn.newCharacter( z )
   vn.transition( "circleopen" )
   z(_([[You are awoken by a pat on your shoulder and you open your eyes to a triumphant Zach you lends you his hand, and pulls you up.
"You doing alright? Great job out there. Sorry I had to keep you in the cold on this, but I had a feeling you wouldn't have gone with the plan. Given that we have to move fast, a large EMP pulse was the best option to knock out so we could do the surgery on Icarus. By the way, the surgery went much better than expected, we can expect Icarus to be stronger, faster, and better than ever!"]]))
   vn.menu{
      {_([["Glad to hear that!"]]), "cont01a"},
      {_([["Next time warn me."]]), "cont01b"},
      {_([["You're an asshole!"]]), "cont01c"},
      {_([["You could have killed Icarus!"]]), "cont01d"},
   }

   vn.label("cont01a")
   z(_([["Nothing like a plan well executed!"
He grins happily.]]))
   vn.jump("cont01")

   vn.label("cont01b")
   z(_([["Well the entirely plan relied on your natural reactions. If I had warned you, Icarus would have likely sensed that and we would have had to gone with plan B."
The way he says plan B makes you feel glad plan A worked.]]))
   vn.jump("cont01")

   vn.label("cont01c")
   z(_([["I get told that disturbingly often. However, I would like to point out it was all perfectly calculated and will have no long-term effects on Icarus. In fact, Icarus's response was much better than I originally anticipated!"
Zach's words are unsurprisingly non-reassuring.]]))
   vn.jump("cont01")

   vn.label("cont01d")
   z(_([["It's impossible, I had computed and simulated all the possible outcomes even considering a 100% error in all parameters! There was no way anything… wait a second…"
He busts out his cyberdeck and starts running some computations. You see the colour flush out of his face.
"Well, damn. All's well that ends well?"]]))
   vn.jump("cont01")

   vn.label("cont01")
   z(_([["Anyway, my drones were able to realign the internal support beams with minimal damage to the organic tissue. My simulation of Icarus indicates it'll be recovered completely within a couple of periods at most. Furthermore, I took my time to add some Za'lek drone technology and materials to replace the primitive Soromid underlying structure, which should only make Icarus better in every sense. It's now an amalgamate of top Soromid and Za'lek technology, and one of a kind!"
He seems overly pleased with himself.]]))
   vn.na(_("You languorously look towards Icarus, which seems to be tucked away in the corner of the docks. You can see some simple monitoring drones camouflaged nearby controlling their vitals. You can't really tell that much has changed other than some extremely large bandage-like sheets wrapped around several parts of their body."))
   z(_([["I'll be keeping an eye on Icarus. It might be a bit confused, so I'll try to make sure it can't hurt itself. Once you feel a bit better, come meet me at the spaceport bar, I have a task I would love you to help me with."
He skips towards the inside of the station, leaving you to recover yourself.]]))
   vn.jump("done")

   vn.label("toomanydrones")
   vn.na(_([[You see Icarus seems to get fed up with the drones chasing it around and flies away.

#rMISSION FAILED"0]]))
   vn.jump("done")

   vn.label("giveup")
   vn.na(_([[You lower your head in resignation and give up. This is worse than cat herding.

#rMISSION FAILED"0]]))

   vn.label("done")
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be completed beyond this point
   if not completed then return end

   misn.accept()
   zbh.log(_("You helped Zach perform surgery on the feral bioship Icarus. The surgery was a success and Icarus is expected to make a speedy recovery."))
   misn.finish(true)
end
