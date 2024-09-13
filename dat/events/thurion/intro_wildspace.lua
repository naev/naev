--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Wild Space Thurion Intro">
 <unique/>
 <location>enter</location>
 <chance>100</chance>
 <system>Maron</system>
 <cond>not faction.get("Thurion"):known()</cond>
</event>
--]]
--[[
   Introduction to the Thurion

   Player gets hailed and told to land on FD-24
--]]
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"
local tut = require "common.tutorial"
local strmess = require "strmess"
--local ws = require "common.wildspace"

local landspb, landsys = spob.getS("FD-24")
local fthurion = faction.get("Thurion")

function create ()
   if not evt.claim( system.cur() ) then evt.finish(false) end

   pilot.clear()
   pilot.toggleSpawn(false)

   hook.timer(15, "timer")
   hook.enter("enter")

   evt.save(true)
end

function timer ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[As you are flying through {sys}, your comm systems flash briefly. Could this be a power failure?]]),
      {sys=system.cur()}))

   local sai = tut.vn_shipai()
   vn.appear( sai, "electric" )
   sai(fmt.f(_([[Your ship AI {shipai} materializes before you.
"{player}, it seems like we are currently undergoing a strong electromagnetic attack. I have rerouted power to cybernetic defenses for now. It is puzzling as we should be in uninhabited area."]]),
      {shipai=tut.ainame(), player=player.name()}))
   vn.menu{
      {_([["Scan for anomalies!"]]), "cont01_scan"},
      {_([["Do a counter electromagnetic attack!"]]), "cont01_counter"},
      {_([["Shut down communication systems!"]]), "cont01_shutdown"},
   }

   vn.label("cont01_scan")
   sai(_([["Scan is negative. No anomalies detected with heavy interference."]]))
   vn.jump("cont01")

   vn.label("cont01_counter")
   sai(_([["A lock-on is necessary first for countermeasures. Scans show no anomalies."]]))
   vn.jump("cont01")

   vn.label("cont01_shutdown")
   sai(_([["I would warn against a full shutdown as it will hamper our detection capabilities."]]))
   vn.jump("cont01")

   vn.label("cont01")
   sai(_([["Wait, I am getting some sort of transmission. Let me attempt to decode the signal."]]))

   vn.move( sai, "left" )
   local signal = vni.textonly(p_("signal","0"), {pos="right"})
   vn.appear( signal )

   signal(strmess.generate( {"0","1"}, 128 ))
   sai(_([["It seems to be using a more archaic encoding. Let try some other decoders."]]))
   signal( strmess.generate( {"0","1"}, rnd.rnd(16,64)).."\n"
         ..strmess.generate( {"0","1"}, rnd.rnd(16,64)).."\n"
         ..strmess.generate( {"0","1"}, rnd.rnd(16,64)).."\n"
         ..strmess.generate( {"0","1"}, rnd.rnd(16,64)) )
   sai(_([["..."]]))
   signal(strmess.generate( {"0","1"}, 32 ).._([["P01EAS101110100DENTIFY YOURSELF OR WE WILL OPEN FIRE. THIS IS YOUR LAST WARNING."]]))
   vn.me(fmt.f(_([["This is the {shipname}. We wish no hostilities, please identify yourself."]]),
      {shipname=player.pilot():name()}))
   signal(_([["YOU HAVE"]]))

   --local c = vn.newCharacter( _(""), { image=nil } )
   --c(_([[]]))

   vn.run()

   if fthurion:playerStanding() < 0 then
      return
   end

   naev.missionStart("Wild Space Thurion Intro - Helper")
   hook.land("land")
end

function land ()
   if spob.cur()==landspb then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.run()

      fthurion:setKnown(true)
      evt.finish(true)
   end
end

function enter ()
   if system.cur()==landsys then
      -- Let the player land on the spob
      landspb:landAllow( true )
   end
end
