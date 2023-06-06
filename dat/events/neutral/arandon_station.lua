--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Arandon Station Vault">
 <location>land</location>
 <chance>100</chance>
 <spob>Arandon Station</spob>
 <unique />
 <tags>
  <tag>fleetcap_10</tag>
 </tags>
</event>
--]]
--[[
   Some kids sneak onto your ship if you land on Majesteka II.
--]]
local neu = require "common.neutral"
local vn = require "vn"
local tut = require "common.tutorial"
local fmt = require "format"
local poi = require "common.poi"

local poi_needed = 1

function create ()
   local inserted = false
   local sai = tut.vn_shipai()

   vn.clear()
   vn.scene()
   vn.transition()

   if var.peek("arandon_station_intro") then
      vn.na(_([[You make your way back to the crusty old terminal.]]))
      vn.appear( sai, tut.shipai.transition )

   else
      var.push("arandon_station_intro",true)

      vn.na(_([[Your ship docks against the derelict station, and don your spacesuit in preparation for entering the unpressurised structure. The hull of the station groans and rings, almost as if it were alive and trying to communicate with you as you begin to explore the wreckage.]]))
      vn.na(_([[You walk through the eerie station, surprised at how nebula crystals are growing into it, penetrating completely the walls, and make your way through the hallways. The station is picked clean, and you feel like you are wasting your time.]]))
      vn.na(_([[Eventually, you find a narrow passage, hidden between some nebula crystals and you manage to barely squeeze through after a couple of intents. You find what seems to be a crusty old terminal, you brush some of the dust and debris off of it and are surprised to see that it seems to still work.]]))

      vn.appear( sai, tut.shipai.transition )
      sai(_([[Your Ship AI materializes in front of you.
"Wow, I haven't seen a terminal like this since the war. Does it have a working socket interface?"]]))
      vn.na(fmt.f(_([[You fumble around and eventually seem to identify what {shipai} was referring to.]]),
         {shipai=tut.ainame()}))
   end
   vn.func( function ()
      if poi.data_get() < poi_needed then
         vn.jump("notenough_sai")
         return
      end
   end )
   sai(_([["You still have data matrices handy, right? If I recall correctly, they should be able to activate the terminal. It may be worth a try."]]))
   vn.menu{
      {_([[Insert the Data Matrix]]),"insert"},
      {_([[Leave]]),"leave"},
   }

   vn.label("insert")
   vn.func( function ()
      poi.data_take( poi_needed )
      inserted = true
   end )
   vn.na(_([[You carefully insert an encrypted data matrix into the socket, which it seems to fit perfectly, and the old machine seems to whir into life.]]))
   sai(_([["Let us see what is available. Ooooh. I didn't know that trick. Ooh Aaah. That's quite nifty."]]))
   vn.na(fmt.f(_([[{shipai} keeps going on for a while, you assume they are processing the data, however, since the holoscreen does not seem to be in working condition, you have no choice but to wait until {shipai} explains it to you.]]),
      {shipai=tut.ainame()}))
   sai(_([["That was quite enlightening, yet inconsistent. It must be the damage, but it seems like the system clock is off by almost a few hundred cycles. Most of the logs of the station seem corrupted, however, I was able to recover some interesting data on fleet formations."]]))
   if player.chapter()=="0" then
      sai(_([["I'm not sure how the information may be useful to us at the current moment, but it may come useful in the near future."]]))
   else
      sai(_([["With this newfound knowledge, it should be possible to control slightly larger fleets under your command."]]))
   end
   sai(_([["I do not think we can get anything else from this terminal, maybe there is something else left on the station?"]]))
   vn.disappear( sai, tut.shipai.transition )
   vn.sfxVictory()
   vn.na(_([[You explore the rest of the station, but other than the eerie nebula crystals piercing and growing throughout the hull, you find nothing else of interest.]]))
   vn.done()

   vn.label("notenough_sai")
   sai(_([["Let me see if I remember what went into it. It was a data-whatyacallem… … …data matrices! That is it. We'll need data matrices if we wish to activate the terminal."]]))
   vn.na(_([[Given that you have no data matrices available, it seems like you have no other option that to head back to your ship and try again later.]]))
   vn.done( tut.shipai.transition )

   vn.label("leave")
   vn.na(_([[You leave the terminal behind. It is best to let sleeping dogs lie.]]))
   vn.done( tut.shipai.transition )

   vn.run()

   if inserted then
      var.pop("arandon_station_intro") -- clean up vars we don't care about anymore
      neu.addMiscLog(fmt.f(_([[You found a crusty old terminal on {spob}. You were able to activate it using an encrypted data matrix and your Ship AI was able to recover some information on fleet formations.]]),
         {spob=spob.cur()}))
      evt.finish(true)
   end
end
