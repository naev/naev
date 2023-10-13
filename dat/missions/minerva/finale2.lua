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
local ccomm = require "common.comm"
--local lmisn = require "lmisn"
--local der = require 'common.derelict'
--local tut = require 'common.tutorial'

local mainspb, mainsys = spob.getS("Minerva Station")
local returnspb, returnsys = spob.getS("Shangris Station")
local title = _("Minerva Station Redux")

-- Mission states:
--  nil: mission not accepted yet
--    1. Get to Minerva Station
--    2. Hack the gibson
--    3. Won fight
--    4. On way to darkshed
mem.state = nil

function create ()
   misn.finish(false)
   if not misn.claim( mainspb ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.maikki.name, minerva.maikki.portrait, _("Maikki seems to be waiting for you in her regular clothes.") )
   misn.setDesc(fmt.f(_([[Maikki has entrusted you with raiding the weapons laboratory at Minerva Station to obtain anything that can help Kex, while also plundering anything you find of value. Afterwards she has told you to meet her up at {returnspb} in the {returnsys} system.]]),
      {returnspb=returnspb, returnsys=returnsys}))
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

      vn.na(_([[You join Maikki at her table. Although she has changed into her civilian clothes, her composure and facial expression is quite unlike when you first met her.]]))
      maikki(_([["I've talked with the pirate head surgeon, and there's both good news and bad news."]]))
      vn.menu{
         {[["Good news?"]], "01_good"},
         {[["Bad news?"]], "01_bad"},
      }

      -- Just change the order she says stuff in
      local vn01good, vn01bad
      vn.label("01_good")
      maikki(_([["The good news is that Zuri is somewhat stable. It's still not clear if she's going to pull it off, but we have to trust her fortitude. We'll have to fly her back to get some more proper follow-up care. However, at least the worst has been avoided for now."]]))
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
      maikki(_([["Damn it, I should have brought my squad with me, but I erred on the side of cautious. How unlike me! But with Zuri and Kex's state, we're going to have to take them to a proper medical facility. That doesn't mean we can leave Minerva Station as it is, after all we've been through!"]]))
      local winner = var.peek("minerva_judgement_winner")
      local msg
      if winner=="dvaered" then
         msg = _([["Minerva Station is going to be swarming with Dvaereds brutes in no time when it gets handed over. ]])
      elseif winner=="zalek" then
         msg = _([["Minerva Station is going to be swarming with Za'leks and their pesky drones in no time when it gets handed over. ]])
      else -- "independent"
         msg = _([["We don't know what's going to happen to the station after the trial ended as it did. I wouldn't be surprised if it would be flooded with Imperials in a short time. ]])
      end
      maikki(msg.._([[We have to infiltrate the weapons facility at the station and not only plunder some nice weapon prototypes and blueprints, but now we also have to see if we can find anything that can help my father! It's a bit of a long shot, but it's our best bet."]]))
      vn.menu( {
         {_([["Wait, you knew about the station?"]]), "02_plan"},
         {_([["Anything for Kex!"]]), "02_kex"},
      } )

      vn.label("02_plan")
      maikki(_([["A pirate has to be efficient: finding my father, and infiltrating a weapons facility. Two birds with one stone! I wasn't entirely sure, but I had a good lead on the fact that the Minerva CEO was doing quite a bit more than gambling."]]))
      vn.jump("02_cont")

      vn.label("02_kex")
      maikki(_([["Yes. You have no idea how much shit I've had to put up with to find him. There is no way I'm going to lose him now! The rest can wait!"]]))
      vn.jump("02_cont")

      vn.label("02_cont")
      maikki(_([["Although I'm really tempted to storm Minerva Station myself, and plunder the weapons facility like any pirate dreams of, but it pisses me off that I'm going to have to leave it to you. Damn responsibilities."]]))
      maikki(_([["Zuri got most of the information and should be the one to brief you on the weapons facility, but that's not going to work out now."
She lets out a sigh.
"I'll try to give you a short rundown on what we know."]]))
      maikki(_([[She clears her throat.
"So, our intel hints that they are working on an experimental energy weapon of some time at the station. Should be quite preliminary design, but we don't have much info on the current state of development. Either way, it's going to be useful and/or worth a fortune!"]]))
      maikki(_([["Apparently, the laboratory is located in a penthouse to not raise suspicion. Security is quite tight around the area, but we've got the likely location narrowed down. I forget where it was, but I'll send you the documents we have on it."]]))
      maikki(_([["We don't really know what we'll find there, but I guess you'll have to improvise? It's pretty much now or never, and we won't get a second chance once the station gets locked down."]]))
      maikki(_([["You'll have to make it to the station on your own, but once you get there, some pirate operatives should be there to help you ransack the place. You won't have too much time, so just try to grab whatever you can and get out of there. Make sure to keep an eye out for any sort of thing that can help Kex. I'm not sure our engineers will be able to figure it out by themselves."]]))
      maikki(_([[She gives a solemn nod.
"Are you ready to infiltrate Minerva Station one last time?"]]))
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
   vn.func( function () mem.state=1 end )
   maikki(_([["Great! I knew I could count on you."]]))
   maikki(fmt.f(_([["I want to take Kex and Zuri to {spb} in the {sys} system, where I know a gal that should help patch them up. Not as good as the surgeons back at New Haven, but it'll have to do. We don't have the time to make the full trip at the moment, once you join us, we can figure out if we can make the trip."]]),
      {spb=returnspb, sys=returnsys}))
   maikki(fmt.f(_([["You infiltrate the station, get what you can, and meet up with us at {spb}. We'll then figure out what to do. Best of luck!"
Maikki gives you a weird two finger salute and takes off to her ship, leaving you to do your part on your own.]]),
      {spb=returnspb}))

   vn.run()

   -- If not accepted, mem.state will still be nil
   if mem.state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( title, {
      fmt.f(_("Land on {spb} ({sys} system)"),{spb=mainspb, sys=mainsys}),
      _("Ransack the weapon laboratory"),
      fmt.f(_("Meet up with Maikki at {spb} ({sys} system)"),{spb=returnspb, sys=returnsys}),
   } )
   mem.mrk = misn.markerAdd( mainspb )

   hook.enter("enter")
end

local boss, hailhook
function enter ()
   local scur = system.cur()
   if scur==mainsys and mem.state==1 then
      -- TODO add some enemies or something
      pilot.clear()
      pilot.toggleSpawn(false)

      local pos = mainspb:pos() + vec2.new( 100+200*rnd.rnd(), rnd.angle() )
      boss = pilot.add( "Empire Pacifier", "Empire", pos, nil, {ai="guard"} )
      for k,v in ipairs{"Empire Lancelot", "Empire Lancelot"} do
         local p = pilot.add( v, "Empire", pos+rnd.rnd(50,rnd.angle()), nil, {ai="guard"} )
         p:setLeader( boss )
      end

      hailhook = hook.hail_spob( "comm_minerva" )
   elseif scur==mainsys and mem.state==2 then
      -- Set up fight
      pilot.clear()
      pilot.toggleSpawn(false)
   else
      -- Clean up some stuff if applicable
      if hailhook then
         hook.rm( hailhook )
         hailhook = nil
      end
   end
end

function comm_minerva( commspb )
   if commspb ~= mainspb then return end

   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, mainspb )
   vn.transition()
   spb(_([[]]))

   vn.run()

   player.commClose()
end
