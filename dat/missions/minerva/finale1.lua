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
local vni = require 'vnimage'
local fmt = require "format"
--local lmisn = require "lmisn"
local pilotai = require "pilotai"
--local love_shaders = require "love_shaders"
local love_audio = require 'love.audio'
local reverb_preset = require 'reverb_preset'
local ccomm = require "common.comm"
local der = require 'common.derelict'

-- Assumes the trialsys -> Kobopos -> Logania systems are connected
local trialspb, trialsys = spob.getS("Jade Court")
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

local pinkdemon
function enter ()
   local scur = system.cur()
   if scur==trialsys and mem.state==0 then
      local fct = faction.get("Empire")

      -- Soft clearing should make things feel a bit more alive
      pilotai.clear()

      local function add_blockade( jp )
         local pos = jp:pos()
         local m, a = pos:polar()
         pos = vec2.newP( m-100, a ) -- Slightly towards the system center

         local l = pilot.add( "Empire Peacemaker", fct, pos, nil, {ai="guard"} )
         l:setHostile(true)
         for k,s in ipairs{ "Empire Pacifier", "Empire Pacifier", "Empire Admonisher", "Empire Admonisher", "Empire Shark", "Empire Shark", "Empire Shark" } do
            local p = pilot.add( s, fct, pos+vec2.newP(100,rnd.angle()), nil, {ai="guard"} )
            p:setLeader( l )
            p:setHostile(true)
         end
      end

      -- Create blockades on all jump points except the one we want
      for k,j in ipairs(scur:jumps()) do
         if not j:hidden() and not j:exitonly() and j:dest() ~= badsys then
            add_blockade( j )
         elseif j:dest() ~= badsys then
            -- Tiny blockade on the target system, player should be able to plow through
            local l
            for i=1,2 do
               local p = pilot.add( "Empire Shark", fct, j:pos() + vec2.newP( 50, rnd.angle() ), nil, {ai="guard"} )
               if not l then
                  l = p
               else
                  p:setLeader(l)
               end
               p:setHostile(true)
            end
         end
      end

      hook.timer( 7, "spawn bogeys" )
      mem.state=1
   elseif scur==badsys then
      -- Just some random patrols here and there, player can stealth or brute force through
      pilot.clear()
      pilot.toggleSpawn(false)


   elseif scur==destsys then
      -- All's quiet on the front
      pilot.clear()
      pilot.toggleSpawn(false)

      local pos = vec2.new( 5e3, 6e3 )
      pinkdemon = minerva.pink_demon( pos, {stealth=true} )
      pinkdemon:setFaction( minerva.fct_wildones() ) -- Non-hostile faction
      hook.pilot( pinkdemon, "board", "maikki_board" )
      hook.pilot( pinkdemon, "hail", "maikki_hail" )
      hook.pilot( pinkdemon, "discovered", "maikki_discovered" )
      pinkdemon:control(true)
      pinkdemon:stealth()
      pinkdemon:setInvincible(true)
   end
end

local bogey_spawner = 0
local bogeys = {
   { "Empire Shark", "Empire Shark", "Empire Lancelot" },
   { "Empire Admonisher", "Empire Admonisher" },
   { "Empire Pacifier", "Empire Lancelot", "Empire Lancelot" },
   { "Empire Hawking", "Empire Admonisher", "Empire Adominsher" },
   { "Empire Peacemaker", "Empire Pacifier", "Empire Pacifier" },
}
function spawn_bogeys ()
   bogey_spawner = bogey_spawner+1
   if bogey_spawner > #bogeys or system.cur()~=trialsys then
      return
   end

   local jmp = jump.get( system.cur(), badsys )

   local fct = faction.get("Empire")
   local l
   for k,s in ipairs(bogeys[ bogey_spawner ]) do
      local p = pilot.add( bogeys[1], fct, trialspb, nil, {ai="patrol"} )
      if not l then
         l = p
      else
         p:setLeader( l )
      end
      p:setHostile(true)
      -- They will naturally go to the jump point and attack the player
      local m = p:memory()
      m.guardpos = { trialspb:pos(), jmp:pos() }
   end

   -- They keep on coming!
   hook.timer( 7, "spawn_bogeys" )
end

function maikki_discovered ()
   hook.timer( 5, "maikki_hailPlayer" )
end

function maikki_hailPlayer ()
   pinkdemon:hailPlayer()
end

function maikki_hail ()
   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, pinkdemon )
   vn.transition()

   p(_([["I see you made it past the patrols in {sys}. Talk about bad timing for them to be doing military exercises. Luck is not in our favour."]]))
   vn.menu{
      {_([["Who are you?"]]), "01_cont"},
      {_([["It was a drag."]]), "01_cont"},
      {_([["You're Zuri's colleague?"]]), "01_cont"},
      {_([["Glad to be here."]]), "01_cont"},
      {_([["…"]]), "01_cont"},
   }

   vn.label("01_cont")
   p(_([["Come here and board and we can talk."]]))

   vn.run()

   pinkdemon:setActiveBoard(true)
   misn.osdCreate{
      fmt.f(_("Board the {ship}"),{ship=pinkdemon}),
   }
end

function maikki_board ()
   vn.clear()
   vn.scene()
   local pir1 = vn.newCharacter( _("Pirate A"), {image=vni.pirate(), pos="left"} )
   local pir2 = vn.newCharacter( _("Pirate B"), {image=vni.pirate(), pos="right"} )
   local unknown = vn.newCharacter( _("???"), {color=minerva.maikkiP.colour} )
   vn.sfx( der.sfx.board )
   vn.transition( "slideup" )

   vn.na(fmt.f(_([[You dock with the {ship}, and as soon as the airlock opens, you are greeted with a swarm of pirates with weapons in front of them.]]),
      {ship=pinkdemon}))
   vn.menu{
      {_([[Throw your hands in the air.]]), "01_cont"},
      {_([[Try to tackle the pirates.]]), "01_cont"},
      {_([[Draw your weapon.]]), "01_cont"},
   }

   vn.label("01_cont")
   --love_audio.setEffect( "reverb_sad", reverb_preset.drugged() )
   --vn.music( minerva.loops.pirate, {pitch=0.6, effect="reverb_sad"} )
   vn.music( minerva.loops.maikki ) -- TODO more aggressive
   unknown(_([["STOOOOOOOOOOOOOOOOOOOOP!"
A powerful booming voice echoes through your ship, instantly defusing the situation.]]))

   -- Move the pirates out of the way
   vn.move( pir1, -1 )
   vn.move( pir2, 2 )

   vn.scene()
   local maikki = minerva.vn_maikkiP()
   vn.transition( "slideup" )
   vn.na(_([[The pirates give way and the source of the powerful voice appears before you. It's a small recognizable figure that you know quite well.]]))
   vn.menu{
      {_([["Maikki?"]]), "02_cont"},
      {_([["Pirate?"]]), "02_cont"},
      {_([["…"]]), "02_cont"},
   }

   vn.label("02_cont")
   maikki(_([["Ah ha ha! Surprised to see me in my true form? Never thought a Pirate Lord could enjoy parfaits? Arrr!"
She winks at you.]]))
   maikki(_([["Speaking of which, where's Zuri?"]]))
   vn.menu{
      {_([["She's seen better days."]]), "03_cont"},
      {_([["She's bleeding out."]]), "03_cont"},
   }

   vn.label("03_cont")
   vn.na(fmt.f(_([[You quickly explain the situation and Maikki quickly orders her troops to pick up Zuri and Kex and take them to the infirmary aboard the {ship}, which seems to be surprisingly state of the art. The pirate head surgeon quickly gets Zuri prepped up and starts running analysis.]]),
      {ship=pinkdemon}))
    vn.na(_([[As Kex does not seem to have life threatening issues, you are left with him and Maikki in the waiting room.]]))
    maikki(_([[Maikki looks fondly at Kex and speaks softly, "Oh father, why did it have to be this way?"]]))
    vn.menu{
      {_([["You knew?"]]), "04_cont"},
      {_([["Father?"]]), "04_cont"},
    }

    vn.label("04_cont")
    maikki(_([[]]))

   vn.sfx( der.sfx.unboard )
   vn.run()
end
