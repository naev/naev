local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"
local tut = require "common.tutorial"
local poi = require "common.poi"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Already done
   if poi.data_get_gained() > 0 then
      return
   end

   return {
      type = "function",
      ship = "Gawain",
      func = function ()
         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"This derelict seems to be quite fancy. Analysis shows that it seems like the life support system on the ship failed. Looking at the computer system, most of the logs seem quite corrupted, however, I was able to recover some audio in part. Seems to be from the end of the black box. Let me play it back for you"]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.scene()
         local v01 = vn.newCharacter( vni.soundonly( _("01"), {colour={0.9,0.2,0.2}} ) )
         vn.transition()

         vn.na(_([[BEGIN PLAYBACK OF AUDIO DATA]]))
         v01(_([["What a bunch of assholes! I've spent my entire life toiling for the greater good of the Empire, and they decide to retire me to the Za'lek consulate! Ungrateful imbeciles!"]]))
         v01(_([["Lack of medals they say. Not my fault we haven't had a proper war in ages. 'Waste of resources' my ass! If the Emperor and his court got their collective heads out of the sky they'd realize we need a proper military, and not just a dozen ships to parade around and guard their newest plaything. No wonder piracy is getting worse than ever."]]))
         v01(_([["What the hell am I going to do in Za'lek space? They're as closed as ever. I'll just be locked up all day at the spaceport, wasting the precious cycles I have left. This is a 'waste of resources' if I've ever seen any. The only way to get through their thick skulls is by a display of force. They would damn well open their borders if we had a proper armada with Peacemakers covering the skies, but those days are long gone…"]]))
         v01(_([[There is a loud sound of something being hit.
"Dammit! I won't let them get away with this! Wait, what the hell is this?"
It sounds like the atmosphere is being vented. You hear some struggling before the recording becomes completely silent.]]))
         vn.na(_([[END OF AUDIO DATA]]))

         vn.disappear( v01 )

         vn.appear( sai, tut.shipai.transition )
         sai(_([["That was… interesting. Looking over the systems I've found something. My memory files indicate that it is a Data Matrix. It has some sort of archaic lock I am unable to de-encrypt. For some reason, I quite well remember them being useful, however, it seems that my files on them have been somewhat damaged… This is fairly odd."]]))
         vn.menu{
            {_("Take the Data Matrix."), "cont01"},
            {_("…"), "cont01"},
         }
         vn.label("cont01")
         local reward = poi.data_str(1)
         sai(fmt.f(_([["It does not seem dangerous, so let us take it with us."

{reward}]]),
               {reward=fmt.reward(reward)}))
         vn.na(_([[You search the rest of the ship, but there is nothing of interest other than the mummified remains of the pilot. You leave the ship behind.]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.func( function ()
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a derelict ship in the {sys} system and were able to recover {reward}.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
