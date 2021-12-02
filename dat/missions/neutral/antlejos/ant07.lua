--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 7">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Antlejos V</planet>
  <cond>require('common.antlejos').unidiffLevel() &gt;= 7</cond>
  <done>Terraforming Antlejos 6</done>
 </avail>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Try to create alliance with Gordon's Exchange, but PUAAA don't like that
--]]
local vn = require "vn"
local fmt = require "format"
local ant = require "common.antlejos"
local fleet = require "fleet"

local reward = ant.rewards.ant07

local retpnt, retsys = planet.getS("Antlejos V")
local mainpnt, mainsys = planet.getS("Gordon's Exchange")

-- luacheck: globals approaching enter land (Hook functions passed by name)

function create ()
   misn.finish()
   if not misn.claim{mainsys,retsys} then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, _("Verner seems to be taking a break from all the terraforming and relaxing at the new spaceport bar.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   v(_([[""]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([[""]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Independent Alliance") )
   misn.setDesc(fmt.f(_("Go to {pnt} in the {sys} system to try to forge an alliance with the government at {pnt} to support {retpnt}."),{pnt=mainpnt, sys=mainsys, retpnt=retpnt}))
   misn.setReward( fmt.credits(reward) )
   misn.osdCreate(_("Independent Alliance"), {
      fmt.f(_("Go to {pnt} ({sys} system)"),{pnt=mainpnt, sys=mainsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=retpnt, sys=retsys}),
   })
   mem.mrk = misn.markerAdd( mainpnt )
   mem.state = 0

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==0 and planet.cur() == mainpnt then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_(""))
      vn.run()

      mem.state = 1
      misn.markerMove( mem.mrk, retpnt )
      misn.osdActive(2)

   elseif mem.state==1 and planet.cur() == retpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(_(""))
      v(_([[""]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      player.pay( reward )
      ant.log(fmt.f(_("You eliminated a PUAAA supply ship bringing more security to {pnt}."),{pnt=retpnt}))
      misn.finish(true)
   end
end

local function spawn_protestors( pos, ships )
   local puaaa = ant.puaaa()
   local f = fleet.add( 1, ships, puaaa, pos, _("PUAAA Protestors"), {ai="baddiepos"} )
   for k,p in ipairs(f) do
      p:setHostile(true)
   end
end

function enter ()
   if mem.state==0 and system.cur()==mainsys then
      spawn_protestors( vec2.new( 3000, 4000 ), {"Lancelot", "Shark", "Shark"} )

   elseif mem.state==1 and system.cur()==mainsys then
      spawn_protestors( vec2.new( -10000, 8700 ), {"Admonisher", "Hyena", "Hyena"} )

   end
end
