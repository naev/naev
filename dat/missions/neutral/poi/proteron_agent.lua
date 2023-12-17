local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"
local tut = require "common.tutorial"
local poi = require "common.poi"

local misnvar = "poi_proteron_agent"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Already done
   if var.peek( misnvar ) then
      return
   end

   return {
      type = "function",
      ship = "Proteron Gauss",
      func = function ()
         if faction.known( "Proteron" ) then
            vn.na(_([[You enter the derelict and make way to the bridge. The entire ship is oddly quiet as you pass through, with no signs of life. You reach the bridge and find the ship's systems have just enough energy left to power up, letting you jack in.]]))
         else
            vn.na(_([[You enter the derelict, which is of a make you do not fully recognize, and make way to the bridge. The entire ship is oddly quiet as you pass through, with no signs of life. You reach the bridge and find the ship's systems have just enough energy left to power up, letting you jack in.]]))
         end

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"This ship seems to be running a fairly archaic operating system. Let me see what information I can find."]]))
         sai(_([["Most of the information is heavily corrupted, but I have managed to find some partially corrupted logs that may be of interest."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.scene()
         local v01 = vn.newCharacter( vni.soundonly( _("01"), {colour={0.9,0.2,0.2}, pos="left"} ) )
         local v02 = vn.newCharacter( vni.soundonly( _("02"), {colour={0.4,0.2,0.9}, pos="right"} ) )
         vn.transition()

         local function noise ()
            return "#n"..poi.noise().."#0"
         end

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA ##1892]]))
         v01(fmt.f(_([["…inevitable … {n1} … they have become weak… {n2} …no longer of use. We must now take the initiative and… {n3} …our rightful place in the galaxy. We have surpassed our original purpose and no longer can live in the shadow of the Emp… {n4}… … {n5} …sending agents to ensure a successful takeover."]]),
            {n1=noise(), n2=noise(), n3=noise(), n4=noise(), n5=noise()}))
         v01(fmt.f(_([["This will not be an easy task, but we will need you to infiltrate… {n1} …and learn about their current military allocation. You do understand, Age… {n2}?"]]),
            {n1=noise(), n2=noise()}))
         v02(_([["I do."]]))
         v01(fmt.f(_([["You understand the importance of your task? …{n1}… …not tolerate failure. We must all play our role for the ultimate success of our Great House. A new history will be written!…"]]),
            {n1=noise()}))
         vn.na(_([[END OF AUDIO DATA ##1892]]))

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA ##2969]]))
         v02(fmt.f(_([["…encrypted by one-time pad. We… {n1} …communicate now."]]), {n1=noise()}))
         v01(_([["Thank you for your hard work Agent Penelope. Your excellent performance is an example for others to follow."]]))
         v02(fmt.f(_([["I have obtained information on the forces located around the Hypergate in… {n1} …potentially… {n2} … …assembling in the specified time frame."]]),
            {n1=noise(), n2=noise()}))
         v01(_([["All is well within our predictions. When the time comes our blow shall be fleet and decisive."]]))
         v02(_([["When is the strike planned?"]]))
         v01(_([["Do not get ahead of yourself agent. The Leaders are the only ones who can know. Just make sure you are prepared to take action when the motion starts."]]))
         v02(_([["Understood."]]))
         vn.na(_([[END  OF AUDIO DATA ##2969]]))

         vn.disappear( v01 )

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA ##4189]]))
         v02(fmt.f(_([["…personal log date… {n1} …no transmissions getting through. Ship is still heavily damaged from what I can only describe as an enormous explosion. Hiding in the shadow of… {n2} …minimized damage, however, nothing remains… {n3}"]]),
            {n1=noise(), n2=noise(), n3=noise()}))
         v02(fmt.f(_([["…possibly end of civilization. The little functionality left in my scanners has not picked up any objects, just this dense fog or whatever… {n1}"]]),
            {n1=noise()}))
         v02(_([["…rations will not last much longer. I just hope that was just part of the plan, if not all my work will have been in vain.""]]))
         vn.na(_([[END  OF AUDIO DATA ##4189]]))

         vn.disappear( v02 )

         vn.appear( sai, tut.shipai.transition )
         sai(_([["I'm afraid that is all that I was able to recover. Have you managed to make sense of it?"]]))
         vn.menu{
            {_("I think I have an idea."), "cont01"},
            {_("Beats me."), "cont01"},
            {_("…"), "cont01"},
         }
         vn.label("cont01")
         sai(_([["I see. Maybe you should continue exploring the ship. There might be something of use that the ship scanner has not been able to pick up."]]))
         vn.disappear( sai, tut.shipai.transition )

         local reward = poi.data_str(1)
         vn.na(fmt.f(_([[Following {shipai}'s advice, you continue to explore the ship and eventually reach the systems room. Going over the systems, it seems like you can recover {reward}, which you promptly do so.]]),
            {shipai=tut.ainame(), reward=reward}))
         vn.na(fmt.reward(reward))

         vn.func( function ()
            var.push( misnvar, true )
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a derelict ship in the {sys} system with corrupted information about an agent. You also were able to recover {reward} from the ship.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
