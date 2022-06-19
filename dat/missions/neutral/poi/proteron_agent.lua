local fmt = require "format"
local vn = require "vn"
local tut = require "common.tutorial"
local poi = require "common.poi"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Already done
   if player.numOutfit( "Veil of Penelope" ) > 0 then
      return
   end

   return {
      type = "function",
      ship = "Proteron Kahan",
      func = function ()
         if faction.known( "Proteron" ) then
            vn.na(_([[You enter the ship and make way to the bridge. The entire ship is oddly quiet as you pass through, with no signs of life. You reach the bridge and find the ship's systems have just enough energy left to power up, letting you jack in.]]))
         else
            vn.na(_([[You enter the ship enter the derelict, which is of a make you do not fully recognize, and make way to the bridge. The entire ship is oddly quiet as you pass through, with no signs of life. You reach the bridge and find the ship's systems have just enough energy left to power up, letting you jack in.]]))
         end

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"This ship seems to be running a fairly archaic operating system. Let me see what information I can find."]]))
         sai(_([["Most of the information is heavily corrupted, but I have managed to find some partially corrupted logs that may be of interest."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.scene()
         local v01 = vn.newCharacter( poi.vn_soundonly( _("01"), {color={0.9,0.2,0.2}, pos="left"} ) )
         local v02 = vn.newCharacter( poi.vn_soundonly( _("02"), {color={0.4,0.2,0.9}, pos="right"} ) )
         vn.transition()

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA ##1892]]))
         v01(_([["…inevitable … #n*CLICK*#0 … they have become weak… #n*CRACKLE*#0 …no longer of use. We must now take the initiative and… #n*HISS*#0 …our rightful place in the galaxy. We have surpassed our original purpose and no longer can live in the shadow of the Emp… #n*RASPING*#0… … #n*CLICK*#0 …sending agents to ensure a successful takeover."]]))
         v01(_([["This will not be an easy task, but we will need you to infiltrate… #n*HISS*#0 …and learn about their current military allocation. You do understand, Age… #n*CRACKLE*#0?"]]))
         v02(_([["I do."]]))
         v01(_([["You understand the importance of your task? …#n*CRACKLE*#0… …not tolerate failure. We must all play our role for the ultimate success of our Great House. A new history will be written!…"]]))
         vn.na(_([[END OF AUDIO DATA ##1892]]))

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA ##2969]]))
         v02(_([["…encrypted by one-time pad. We… #n*HISS*#0 …communicate now."]]))
         v01(_([["Thank you for your hard work Agent Penelope. Your excellent performance is an example for others to follow."]]))
         v02(_([["I have obtained information on the forces located around the Hypergate in… #n*CRACKLE*#0 …potentially… #n*HISS*#0 … …assembling in the specified time frame."]]))
         v01(_([["All is well within our predictions. When the time comes our blow shall be fleet and decisive."]]))
         v02(_([["When is the strike planned?"]]))
         v01(_([["Do not get ahead of yourself agent. The Leaders are the only ones who can know. Just make sure you are prepared to take action when the motion starts."]]))
         v02(_([["Understood."]]))
         vn.na(_([[END  OF AUDIO DATA ##2969]]))

         vn.disappear( v01 )

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA ##4189]]))
         v02(_([["…personal log date… #n*CRACKLE*#0 …no transmissions getting through. Ship is still heavily damaged from what I can only describe as an enormous explosion. Hiding in the shadow of… #n*HISS*#0 …minimzed damage, however, nothing remains… #n*WHISTLING*#0"]]))
         v02(_([["…possibly end of civilization. The little functionality left in my scanners has not picked up any objects, just this dense fog or whatever… #n*CRACKLE*#0"]]))
         v02(_([["…rations will not last much longer. I just hope that was just part of the plan, if not all my work wil have been in vain.""]]))
         vn.na(_([[END  OF AUDIO DATA ##4189]]))

         vn.disappear( v02 )

         vn.appear( sai, tut.shipai.transition )
         sai(_([["I'm afraid that is all that I was able to recover. Have you managed to make sense of it?"]]))
         vn.menu{
            {_("I thin I have idea."), "cont01"},
            {_("Beats me."), "cont01"},
            {_("…"), "cont01"},
         }
         vn.label("cont01")
         sai(_([["I see. maybe you should continue exploring the ship. There might be something of use that the ship scanner has not been able to pick up."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.na(fmt.f(_([[Following {shipai}'s advice, you continue to explore the ship and eventually reach the systems room. You notice there seems to be a device interfering with the radiation emitted. You can't tell who made it, but it seems that it was likely the main reason that the derelict was so hard to find. You manage to dislodge it to take it back to your ship for further analysis.]]),{shipai=tut.ainame()}))

         vn.func( function ()
            player.outfitAdd( "Veil of Penelope" )
         end )
      end,
   }
end
