--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Judgement">
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 5</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</event>
--]]

--[[
   * Player's testimony gets called into question if killed harper or let scavengers live
   * Kex deciphers final stuff and blurts out important thing to get CEO convicted at the end, but by doing that shows off he is an intelligent being and gets sentenced to dissection by the Za'lek.
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
--local vni = require 'vnimage'
local fmt = require "format"
--local lmisn = require "lmisn"
--local love_shaders = require "love_shaders"

local trialspb, trialsys = spob.getS("Jade Court")

local title = _("Minerva Judgement")

-- Mission states:
--  nil: mission not accepted yet
--    1. fly to Jade Court

function create()
   misn.finish(false)

   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   misn.setDesc(_("Zuri wants you do help a Za'lek General and Dvaered Warlord finish each other off."))
   misn.setReward(_("The future of Minerva Station!"))
   misn.setTitle( title )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   vn.na(_([[]]))
   zuri(_([[""]]))

   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   misn.osdCreate( title, {
      fmt.f(_("Go to {spb} ({sys} system)"),{spb=trialspb, sys=trialsys}),
   } )
   mem.mrk = misn.markerAdd( trialspb )

   hook.land("land")
   hook.enter("enter")
end

-- Make sure can land on the Jade Court
function enter ()
   if system.cur()~=trialsys then return end
   trialspb:landOverride(true)
end

function land ()
   if spob.cur() == trialspb then
      misn.npcAdd( "trial_start", minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   end
end

function trial_start ()
--[[
- maikki_gave_drink (true, nil)
- maikki_response ("yes", "no", nil)
- maikki_scavengers_alive (true, nil)
- harper_ticket ("credits", "tokens", "free", "stole" )
- strangelove_death ("unplug", "comforted", "shot", nil)
--]]
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   vn.na(_([[]]))
   zuri(_([[""]]))
   vn.menu{
      {_([[Go to the courtroom.]]), "01_start"},
      {_([[Maybe later.]]), "01_later"},
   }

   vn.label("01_later")
   vn.na(_([[You decide to post-pone deciding the future of Minerva Station.]]))
   vn.done()

   vn.label("01_start")

   vn.run()
end
