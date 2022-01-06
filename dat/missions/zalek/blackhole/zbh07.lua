--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 7">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <spob>Research Post Sigma-13</spob>
  <location>Bar</location>
  <done>Za'lek Black Hole 6</done>
 </avail>
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
--local fmt = require "format"
local zbh = require "common.zalek_blackhole"

function create ()
   misn.finish()
   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You meet with Zach who semes to be doing the last preparations for the surgery.]]))
   z(_([["Heya. I think I got this finally figure out. So most of the anatomy is like that of an arthropod, however, the inner scaffolding follows ship designs. It's incredibly well thought out the entire layout, and it also guarantees that no two ships will ever be exactly alike!"
He seems fairly excited about the entire prospect.]]))
   z(_([["There are still some rough details here and there, but I feel confident enough to try to improvise live as necessary. No matter how many simulations I run, I don't think it's going to be anything like the real thing. Although Icarus isn't really showing signs of stress, my scans indicate that it's probably better to try to operate sooner than later. Would you be willing to assist me?"]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   z(_([["Great! So let's get preparations started then."]]))

   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()
   misn.finish(true)
end
