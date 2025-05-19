--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Data Matrix Intro">
 <location>load</location>
 <chance>100</chance>
 <unique />
 <priority>99</priority>
</event>
--]]
local poi = require "common.poi"
local tut = require "common.tutorial"
local vn = require "vn"
local fmt = require "format"

function create ()
   if poi.data_known() then
      return evt.finish(true)
   end

   hook.mission_done( "check_matrices" )
   hook.event_done( "check_matrices" )
end

function check_matrices ()
   if poi.data_known() then
      return evt.finish(true)
   end

   if poi.data_get_gained() <= 0 then
      return
   end

   hook.timer( "delay", rnd.rnd(3,6) )
end

function delay ()
   if poi.data_known() then
      return evt.finish(true)
   end

   vn.reset()
   vn.scene()

   local sai = tut.vn_shipai()
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[You ship AI {ainame} appears before you.]]),
      {ainame=tut.ainame()}))
   sai(_([["I have noticed you have acquired a Data Matrix. It has some sort of archaic lock I am unable to decrypt. For some reason, my memory banks them being useful, however, it seems that my files on them have been somewhat damaged… This is fairly odd."]]))
   sai(_([["Keeping on to them seems like a reasonable course of action. You may possibly find what my memory banks have forgotten, that is, some use for them."]]))
   vn.done( tut.shipai.transition )

   vn.run()

   poi.data_set_known()
   return evt.finish(true)
end
