--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>10</chance>
  <faction>Za'lek</faction>
  <location>Bar</location>
  <done>Za'lek Particle Physics 6</done>
 </avail>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 01

   Introductory mission that sets the tone where you bring Zack to Ceres Station.
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"

-- luacheck: globals land enter (Hook functions passed by name)

local reward = zbh.rewards.zpp01
local destpnt, destsys = planet.getS("Ceres Station")

function create ()
   misn.finish(false)
   misn.setNPC( _("Za'lek Scientist"), zbh.zack.portrait, _("You see a Za'lek scientist nerviously sitting at the bar. It seems like they might have a job for you.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[]]))
   z(_([[""]]))
   z(fmt.f(_([[""]]),
      {pnt=destpnt, sys=destsys}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(fmt.f(_([[""]]),{}))
   vn.done( zbh.zack.transition )

   vn.label("accept")
   z(_([[""]]))
   vn.func( function () accepted = true end )
   vn.done( zbh.zack.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   local c = commodity.new( N_("Zach"), N_("A worried Za'lek scientist.") )
   misn.cargoAdd(c, 0)

   -- mission details
   misn.setTitle( _("Black Hole Research") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc( fmt.f(_("Take Zach to see what happened to his colleagues at {pnt} in the {sys} system."),
      {pnt=destpnt, sys=destsys} ))

   misn.markerAdd( destpnt )

   misn.osdCreate( _("Black Hole Research"), {
      fmt.f(_("Take Zach to {pnt} ({sys} system)"), {pnt=destpnt, sys=destsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if planet.cur() ~= destpnt then
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zack() )
   vn.transition( zbh.zack.transition )
   vn.na(_(""))
   z(_([[""]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zack.transition )
   vn.run()

   diff.apply( "ceres_online" )

   player.pay( reward )
   zbh.log(fmt.f(_("You took Zach to {pnt} where he found all his colleagues had seemingly met a gruesome fate. He vowed to look into what happened and continue the research that was started. Using his engineering skills, he was able to restore minimum functionality of the station."),{pnt=destpnt}))
   misn.finish(true)
end

local firsttime = true
function enter ()
   if system.cur() ~= destsys or not firsttime then
      return
   end

   firsttime = false
end
