--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Durea Pirates Epilogue">
 <unique/>
 <location>land</location>
 <chance>100</chance>
 <spob>Durea</spob>
 <cond>player.evtDone("Durea Pirates")</cond>
</event>
--]]
--[[
   Epilogue for saving Durea.
--]]
local vn = require 'vn'
local fmt = require 'format'
local neu = require "common.neutral"

local mainspb, mainsys = spob.cur()

local articles = {
   {
      faction = "Generic",
      head = fmt.f(_("Pirate Blockade Lifted at {sys}"),{sys=mainsys}),
      body = fmt.f(_("A pirate blockade was lifted in the {sys} system. Locals said they never noticed anything weird happen during the entire time."),{sys=mainsys}),
   },
}

function create ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(_([[You land on the newly opened space port which is surprisingly empty.]]))
   vn.na(_([[Expecting to be greeted with open arms, you walk around until you eventually find a lone individual who seems to be mopping the floor.]]))
   vn.na(_([[You ask him what is going on and explain that you lifted the blockade. He seems unfazed and comments that that's why there were slightly fewer ships than usual coming it before going back to his duties.]]))
   vn.na(fmt.f(_([[After putting your life in peril for the people of {spb}, it seems like nobody really noticed that the system was blockaded in the first place. Giving up on a potential reward, you head back to your ship.]]),
      {spb=mainspb}))
   vn.run()

   -- Add some news
   articles[1].date_to_rm = time.get()+time.new(0,20,0)
   news.add( articles )

   neu.addMiscLog(fmt.f(_([[You helped clear a pirate blockade at the {sys} system, and were met with indifference from the local population.]]),
      {sys=mainsys}))

   evt.finish(true)
end
