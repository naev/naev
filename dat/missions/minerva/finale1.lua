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
local vne = require "vnextras"
local fmt = require "format"
local pilotai = require "pilotai"
local love_audio = require 'love.audio'
local reverb_preset = require 'reverb_preset'
local ccomm = require "common.comm"
local lmisn = require "lmisn"
local der = require 'common.derelict'
local tut = require 'common.tutorial'

-- Assumes the trialsys -> Kobopos -> Logania systems are connected
local trialspb, trialsys = spob.getS("Jade Court")
local badsys = system.get("Kobopos")
local destsys = system.get("Logania")
local recoupspob, recoupsys = spob.getS("Regensburg")
local title = _("Escape the Courts")

-- Mission states:
--  nil: mission not accepted yet
--    1. escape jade court
mem.state = nil

function create ()
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
   c:illegalto( "Empire" )
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

local pinkdemon, baddie_ships, emp_boss
function enter ()
   local scur = system.cur()
   if scur==trialsys and mem.state==0 then
      local fct = faction.get("Empire")

      -- Soft clearing should make things feel a bit more alive
      pilotai.clear()
      pilot.toggleSpawn(false)

      -- No relanding
      player.landAllow( false, _("You can't land right now!") )

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

      hook.timer( 3, "player_warning" )
      hook.timer( 11, "spawn_bogeys" )
      mem.state=1
   elseif scur==badsys then
      -- Just some random patrols here and there, player can stealth or brute force through
      -- TODO probably too similar to pirate5 that was quite recent, maybe make it less of an overt stealth mission
      pilot.clear()
      pilot.toggleSpawn(false)

      local routeA = {
         spob.get("Kayel"):pos(),
         jump.get( badsys, "Jade" ):pos(),
      }
      local posB = vec2.new( 1e3, 1.6e3 )
      local routeC = {
         vec2.new( 5e3, 2e3 ),
         vec2.new( 12e3, 3e3 ),
         vec2.new( 10e3, 11e3 ),
         vec2.new( 3e3, 8e3 ),
      }
      local routeD = {
         vec2.new( 3e3, 2.6e3 ),
         vec2.new( -7.5e3, 14e3 ),
      }
      local posE = vec2.new( 18e3, 6e3 )

      baddie_ships = {}
      local function fuzz_pos( pos, max_offset )
         max_offset = max_offset or 100
         return vec2.newP(max_offset*rnd.rnd(), rnd.angle()) + pos
      end
      local function spawn_baddie( shipname, pos )
         local p = pilot.add( shipname, "Empire", fuzz_pos(pos) )
         -- We are nice and make the ships easier to see for this mission
         p:intrinsicSet( "ew_hide", 100 )
         table.insert( baddie_ships, p )
         return p
      end
      local function add_patrol_group( route, ships, start )
         start = start or rnd.rnd(1,#route)
         local pos = route[ start ]
         local l
         for k, s in ipairs( ships ) do
            local p = spawn_baddie( s, pos )
            if k==1 then
               l = p
               pilotai.patrol( p, route )
            else
               p:setLeader( l )
            end
         end
         return l
      end
      local function add_guard_group( pos, ships )
         local l
         for k, s in ipairs( ships ) do
            local p = spawn_baddie( s, pos )
            if k==1 then
               l = p
               p:changeAI("guard")
               local aimem = p:memory()
               aimem.guardpos = pos
               aimem.guarddodist = 6e3
               aimem.guardreturndist = 12e3
            else
               p:setLeader( l )
            end
         end
         return l
      end

      local pacifier = ship.get("Empire Pacifier" )
      local admonisher = ship.get("Empire Admonisher")
      local lancelot = ship.get("Empire Lancelot")
      local shark = ship.get("Empire Shark")
      add_patrol_group( routeA, { admonisher, shark, shark } )
      emp_boss = add_guard_group( posB, { "Empire Peacemaker", pacifier, pacifier, lancelot, lancelot, lancelot, lancelot, lancelot, lancelot } )
      add_patrol_group( routeC, { admonisher, shark, shark, shark, shark } )
      add_patrol_group( routeD, { admonisher, shark, shark } )
      add_guard_group( posE, { "Empire Rainmaker", admonisher, admonisher, lancelot, lancelot, lancelot, lancelot, lancelot, lancelot } )

      -- Tell the player to f off
      hook.timer( 5.0,  "message_first" )
      hook.timer( 15.0, "message_hostile" )

   elseif scur==destsys then
      -- All's quiet on the front
      pilot.clear()
      pilot.toggleSpawn(false)
      misn.osdActive( 2 )

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

function player_warning ()
   pilot.broadcast( trialspb:name(), fmt.f(_("Unauthorized departure. Fleets engage {player}!"),{player=player.pilot():name()}) )
end

local bogey_spawner = 0
local bogeys = {
   { "Empire Shark", "Empire Shark" },
   { "Empire Lancelot", "Empire Lancelot" },
   { "Empire Admonisher", "Empire Admonisher" },
   { "Empire Pacifier", "Empire Lancelot", "Empire Lancelot" },
   { "Empire Hawking", "Empire Admonisher", "Empire Admonisher" },
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
      local p = pilot.add( s, fct, trialspb, nil, {ai="baddiepatrol"} )
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

   -- Spam stuff everytime they spawn
   local pp = player.pilot()
   local inr, nofuz = l:inrange( pp )
   if inr and nofuz then
      l:broadcast(fmt.f(_("Engaging {player}!"),{player=pp:name()}))
   end

   -- They keep on coming!
   hook.timer( 7+3*bogey_spawner, "spawn_bogeys" )
end

function message_first ()
   emp_boss:broadcast( fmt.f(_("Following Imperial Decree ED-17838, {sys} is under lockdown for military exercises. All non-affiliated personal must evacuate the system."), {sys=badsys}), true )
end

function message_hostile ()
   for k,p in pairs(baddie_ships) do
      p:setHostile(true)
   end
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

   p(fmt.f(_([["I see you made it past the patrols in {sys}. Talk about bad timing for them to be doing military exercises. Luck is not in our favour."]]),
      {sys=badsys}))
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
   misn.osdCreate( title, {
      fmt.f(_("Board the {ship}"),{ship=pinkdemon}),
   } )
   player.commClose()
end

function maikki_board ()
   player.unboard()
   hook.safe( "maikki_board_safe" )
end

function maikki_board_safe ()
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
   local maikki = vn.newCharacter( minerva.vn_maikkiP() )
   vn.transition( "hexagon" )
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
   maikki(_([[She abruptly changes back to her piratey self.
"I knew you were thick, but you're thicker than molasses!"]]))
   maikki(_([["The unlucky fowl is my father! Of course he wasn't always this way, but you should know that better than I."]]))
   maikki(_([["Just my luck to cross the entire galaxy chasing my fool of a father, and when I finally find them, they've been transformed into a fowl, and, and, to make things WORSE, I can't even talk to them because they seem be offline or some shit."]]))
   maikki(_([["What's a girl gotta do to bloody enjoy a normal family, just for ONCE!"
She closes her eyes and rubs her temples.]]))
   maikki(_([["And Zuri, my poor, dear, Zuri, is yet again under life-saving surgery. At this rate, I don't think she's going to have a single one of her original organs..."]]))
   maikki(_([["This whole ordeal has become quite a mess. I'm really glad I was finally able to find my father, but things haven't really been going our way."]]))
   local winner = var.peek("minerva_judgement_winner")
   if winner=="independent" then
      maikki(_([["At least the judge didn't assign Minerva Station to the Dvaered brutes or Za'lek popsicles, however, with the current situation, I don't think it helps much."]]))
   else
      local badguys
      if winner=="zalek" then
         badguys = _("Za'lek")
      else
         badguys = _("Dvaered")
      end
      maikki(fmt.f(_([["Not to mention the bloody judge assigned Minerva Station to the damn {badguys}. Not that it matters much, but it's just salting the wounds."]]),
         {badguys=badguys}))
   end
   vn.label("05_menu")
   vn.menu{
      {_("Ask about Zuri's health"), "05_zuri"},
      {_("Ask about her father"), "05_father"},
      {_("Ask about next plans (continue)"), "05_cont"},
   }

   vn.label("05_zuri")
   maikki(_([["She's a tough cookie to crack. We've been together through much harder times. Pirate doctors are second to none when it comes to experience."
You sense a some worry and uneasiness despite the strong words.]]))
   maikki(_([["One time, we were doing your standard hit operation in Delta Pavonis, some Empire Combat Bureaucrat had a bounty on the head, and I figure we might as well cash in. Sources said they would be flying a Pacifier, no deal, right? Turns out it was a bloody Peacemaker with a full entourage."]]))
   maikki(_([["Was about to back off when a second Peacemaker jumped in behind us. The bastards set us up for a trap. With no way out, we charged the Bureaucrat asshole and smashed the captain's bridge right into the fighter bay, and went to fight our way through the ship."]]))
   maikki(_([["While we were trying to take hold of the hangar, Zuri grabbed a bunch of weapons and charged basically charged through the ship. When we found her in the bridge, she had somehow managed to single-handedly cross the entire ship and capture it by herself!"]]))
   maikki(_([["Of course, she had nearly blown herself into two, and was riddled with laser holes, but we were lucky that the field surgeon was with us and was able to somehow stabilize her while I rammed the peacemaker into the second one."]]))
   maikki(_([["We ended up escaping with 17 people crammed in a Lancelot, and somehow made it back to New Haven. It took Zuri nearly a cycle to recover, but then she was better than ever. She's got be fine this time too!"]]))
   vn.jump("05_menu")

   vn.label("05_father")
   maikki(_([["He wasn't really with us when I was a child. He'd always be searching for artefacts in the nebula or whatnot. But, he would have the best stories when he would come back. He always was the life of the party. It's what made me become a pilot."]]))
   maikki(_([["Then, one day, he didn't come back. Eaten by the Nebula, killed by scavengers, taken prisoner by the Dvaered, blah blah blah. The rumours were awful."]]))
   maikki(_([["My mother took it really poorly, her health deteriorated fast. When she died, I decided I would try to find my father and find out why he did that, you know?"]]))
   maikki(_([["Took me long enough, but I never expected to find him like this..."]]))
   vn.jump("05_menu")

   vn.label("05_cont")
   maikki(_([["I guess you can sort of expect that whatever plans I had are probably moot at this point. Too many unknowns at this point, we should head to a haven and recoup a bit. Have to figure out what happened to my father and also wait until Zuri wakes up."]]))
   maikki(fmt.f(_([["For now, we should get out of here, lest the Imperials come down from {badsys}. I know some people at {spob} in the {sys} system. We can head there for now."]]),
      {spob=recoupspob, sys=recoupsys, badsys=badsys}))

   local log = vne.flashbackTextStart()
   log(fmt.f(_([[You tell {shipai} to set your ship's autonav to follow the Pink Demon and ride with Maikki to {spob}.

During the ride, you talk with her about the entire experience up until now, without missing any details of your interactions with Kex and the dealings with Dr. Strangelove. She is enthralled in all talk of Kex and inquisitive asking many questions the entire time while flying the ship.

The ride is fairly smooth, surprising you with how effortlessly Maikki seems to avoid patrols on the way to your destination.]]),
      {spob=recoupspob, shipai=tut.ainame()}))
   vn.func( function ()
      -- Simulate the elapsed time for moving over
      local dist, jumps = lmisn.calculateDistance( system.cur(), player.pos(), recoupsys, recoupspob:pos() )
      local elapsed = dist / pinkdemon:speedMax() + #jumps * pinkdemon:stats().jump_delay
      time.inc( time.new( 0, 0, elapsed ) )
      -- Land on the spob
      player.land( recoupspob )
   end )
   vne.flashbackTextEnd{ notransition=true }
   maikki = minerva.vn_maikkiP()
   vn.transition()

   vn.na(fmt.f(_([[You land with Makki on {spob} through what seems to be a hidden landing pad, away from the main spaceport.]]),
      {spob=recoupspob}))
   maikki(_([[Despite being quite flustered with the situation, Maikki seems intent on trying to manage the situation.
"I'm going to get a full briefing and diagnostic on both Zuri and Kex. Since this will take a while, meet up with me at the bar and I'll fill you out with the important details."]]))
   vn.run()

   misn.finish(true)
end
