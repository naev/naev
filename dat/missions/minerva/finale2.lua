--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Finale 2">
 <unique />
 <priority>1</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Regensburg</spob>
 <done>Minerva Finale 2</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]
--[[
   Final mission having to revisit and break into Minerva Station
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
--local vni = require 'vnimage'
--local vne = require "vnextras"
local fmt = require "format"
--local pilotai = require "pilotai"
--local love_audio = require 'love.audio'
--local reverb_preset = require 'reverb_preset'
--local ccomm = require "common.comm"
--local lmisn = require "lmisn"
--local der = require 'common.derelict'
--local tut = require 'common.tutorial'

local mainspb, mainsys = spob.getS("Minerva Station")
local title = _("Minerva Station Redux")

-- Mission states:
--  nil: mission not accepted yet
--    1. get to Minerva Station
--    2. hack the gibson
--    3.
mem.state = nil

function create ()
   misn.finish(false)
   if not misn.claim( mainspb ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.maikki.name, minerva.maikki.portrait, _("Maikki seems to be waiting for you in her regular clothes.") )
   misn.setDesc(_([[TODO]]))
   misn.setReward(_("Saving Kex!"))
   misn.setTitle( title )
end

local talked -- Reset when loads
function accept ()
   vn.clear()
   vn.scene()
   local maikki =vn.newCharacter( minerva.vn_maikki() )
   vn.music( minerva.loops.maikki )
   vn.transition()

   if not talked then
      talked = true

      vn.na(_([[You join Maikki at her table. Although she has changed into her civilian clothes, her composture and facial expression is quite unlike when you first met her.]]))
      maikki(_([["I've talked with the pirate head surgeon, and there's both good news and bad news."]]))
      vn.menu{
         {[["Good news?"]], "01_good"},
         {[["Bad news?"]], "01_bad"},
      }

      -- Just change the order she says stuff in
      local vn01good, vn01bad
      vn.label("01_good")
      maikki(_([["The good news is that Zuri is somewhat stable. It's still not clear if she's going to pull it off, but we have to trust her fortitude. We'll have to fly her back to New Haven to get some more proper follow-up care. However, at least the worst has been avoided for now."]]))
      vn.func( function ()
         vn01good = true
         if not vn01bad then
            vn.jump("01_bad")
         else
            vn.jump("01_cont")
         end
      end )

      vn.label("01_bad")
      maikki(_([["The bad news is that we don't know what the deal is with my father. He seems to be alive, however, the surgeon had no idea what to do. I guess this is a first for them. We got a second look by the lead engineer who told us that it seems like some of his circuitry was damaged."]]))
      vn.func( function ()
         vn01bad = true
         if not vn01good then
            vn.jump("01_good")
         else
            vn.jump("01_cont")
         end
      end )

      vn.label("01_cont")
      maikki(_([["Damn it, I should have brought my squad with me, but I erred on the side of cautious. How unlike me. We have no choice but to bring Zuri and my father to a New Haven. However, we can't leave Minerva Station as it is, after all we've been through!"]]))
      local winner = var.peek("minerva_judgement_winner")
      local msg
      if winner=="dvaered" then
         msg = _([["Minerva Station is going to be swarming with Dvaereds brutes in no time when it gets handed over. ]])
      elseif winner=="zalek" then
         msg = _([["Minerva Station is going to be swarming with Za'leks and their pesky drones in no time when it gets handed over. ]])
      else -- "independent"
         msg = _([["We don't know what's going to happen to the station after the trial ended as it did. I wouldn't be surprised if it would be flooded with Imperials in a short time. ]])
      end
      maikki(msg.._([[We have to infiltrate the weapons facility at the station and not only extract vital weapon prototypes and blueprints, but now we also have to see if we can find anything that can help my father!"]]))
      vn.menu( {
         {_([["Wait, you knew about the station?"]]), "02_plan"},
         {_([["Anything for Kex!"]]), "02_kex"},
      } )

      vn.label("02_plan")
      maikki(_([["A pirate has to be efficient: finding my father, and infiltrating a weapons facility. Two birds with one stone! I wasn't entirely sure, but I had a good lead on the fact that the Minerva CEO was doing quite a bit more than gambling."]]))
      vn.jump("02_cont")

      vn.label("02_kex")
      maikki(_([["Yes. You have no idea how much shit I've had to put up with to find him. There is no way I'm going to lose him now!"]]))
      vn.jump("02_cont")

      vn.label("02_cont")
      maikki(_([[""]]))
   else
      maikki(_([["Are you ready now to infiltrate Minerva Station?"]]))
   end

   vn.menu( {
      {_("Accept the Undertaking"), "accept"},
      {_("Prepare more"), "decline"},
   } )

   vn.label("decline")
   vn.na(_([[Maikki gives you some time to prepare more. Return to her when you are ready.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () mem.state=0 end )

   vn.run()

   -- If not accepted, mem.state will still be nil
   if mem.state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( title, {
      fmt.f(_("Land on {spb} ({sys} system)"),{spb=mainspb, sys=mainsys}),
      _("Break into the Station"),
   } )
   mem.mrk = misn.markerAdd( mainspb )

   hook.enter("enter")
end

function enter ()
end
