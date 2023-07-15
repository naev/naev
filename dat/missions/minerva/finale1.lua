--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Finale 1">
 <unique />
 <priority>1</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Jade Court</spob>
 <done>Minerva Judgement</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]
--[[
   Escape the Jade Court and rendezvous with the Pink Demon.
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
--local vni = require 'vnimage'
local fmt = require "format"
--local lmisn = require "lmisn"
--local equipopt = require "equipopt"
--local pilotai = require "pilotai"
--local love_shaders = require "love_shaders"
local love_audio = require 'love.audio'
local reverb_preset = require 'reverb_preset'

local _trialspb, trialsys = spob.getS("Jade Court")
local badsys = system.get("Kobopos")
local destsys = system.get("Logania")
local title = _("Escape the Courts")

-- Mission states:
--  nil: mission not accepted yet
--    1. escape jade court
mem.state = nil

function create ()
   misn.finish(false)
   if not misn.claim{trialsys, badsys, destsys} then
      misn.finish( false )
   end
   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, _("Meet up with Zuri at her hiding spot?") )
   misn.setDesc(fmt.f(_("A wounded Zuri needs to rendezvous with her colleagues in the {sys} system."),
      {sys=destsys}))
   misn.setReward(_("??"))
   misn.setTitle( title )
end

function accept ()
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   love_audio.setEffect( "reverb_sad", reverb_preset.drugged() )
   vn.music( minerva.loops.pirate, {pitch=0.6, effect="reverb_sad"} )
   vn.transition()

   vn.na(_([[You find Zuri clutching her wound in one hand and holding Kex with the other. Although she's in really rough shape she flashes you a grin as you approach.]]))
   zuri(fmt.f(_([["Hey {player}, ready to get out of here? My colleague should be waiting for us in the {sys} system."
She coughs a bit, and although she tries to hide it, you notice speckles of blood.]]),
      {player=player.name(), sys=destsys}))
   vn.menu( {
      {_("Save Zuri"), "accept"},
      {_("Prepare more"), "decline"},
   } )

   vn.label("decline")
   zuri(_([["I'll be waiting."
She sort of slumps at the wall, you're not sure if she's still concious.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () mem.state=0 end )
   vn.na(_([[You manage to help get Zuri up to her feet, and begin to drag her to the spaceport.]]))
   vn.na(_([[With all the commotion going on the Jade Court, most of the security is preoccupied and barely give you a second glance as they run around following orders.]]))
   vn.na(_([[Eventually you make it unnoticed to the spaceport and manage sneak both Zuri, who seems to be breathing very hard, and Kex into ship. As soon as you help her into a bunk bed, her body seems to lose tension as her conciousness fades away.]]))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.na(_([[Suddenly, an alarm starts blaring. It seems like the situation is getting worse and the station is headed for a lock down.]]))
   vn.na(_([[As the station gates begin to close, you deftly activate your ship, hit the accelerator and aim for free space. It's a close call with the spaceport almost clamping down on your ship, but you break through.]]))
   vn.na(_([[You almost begin to celebrate, but it seems like it a short while you won't be alone anymore. There are bogeys on your tail!]]))

   vn.run()

   -- If not accepted, mem.state will still be nil
   if mem.state==nil then
      return
   end

   local c = commodity.new( N_("Zuri and Kex"), N_("A heavily wounded Zuri holding Kex. They are both unconscious."))
   misn.cargoAdd( c, 0 )

   misn.accept()
   misn.osdCreate( title, {
      fmt.f(_("Go to the {sys} system"),{sys=destsys}),
      _("Rendezvous with Zuri's Colleague"),
   } )
   mem.mrk = misn.markerAdd( destsys )

   player.takeoff()
   hook.enter("enter")
end

function enter ()
   --[[
   local scur = system.cur()
   if scur==trialsys then
   elseif scur==badsys then
   elseif scur==destsys then
   end
   --]]
end
