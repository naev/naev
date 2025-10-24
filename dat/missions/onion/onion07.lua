--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 07">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <done>Onion Society 06</done>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion 07

   Simple mission mainly there to set up some flavour and story stuff.

   Player sets up honeypot, does some scans, and beats up mercenaries.
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local trigger = require "trigger"
local fleet = require "fleet"
local lmisn = require "lmisn"
local pilotai = require "pilotai"
--local tut = require "common.tutorial"

-- Reference to honeypot (trap)
local title = _("Onion and Honey")
local reward = onion.rewards.misn07

-- Mission states
local STATE_TALKED_TO_DOG = 1
local STATE_SET_UP_HONEYPOT = 2
local STATE_FINISH_SCANS = 3
local STATE_BEAT_MERCENARIES = 4
mem.state = nil

-- Candidates are somewhat uninhabited spobs along the main trade lanes
local TARGETSYS_CANDIDATES = {
   system.get("Gremlin"),
   system.get("Overture"),
   system.get("Daan"),
   system.get("Santoros"),
   system.get("Fidelis"),
}

local SHIPS_TO_SCAN = 10 -- Ships it says to scan
local SHIPS_TO_SCAN_REAL = 3 -- The true amount after which the player is attacked

function create()
   -- Try to find a closeby acceptable target
   local targets = {}
   for k,t in ipairs(TARGETSYS_CANDIDATES) do
      if naev.claimTest( t ) and t:jumpDist() < 6 then
         table.insert( targets, t )
      end
   end
   if #targets <= 0 then return misn.finish(false) end
   mem.targetsys = targets[ rnd.rnd(1, #targets) ]

   -- Need to soft claim
   if not misn.claim( mem.targetsys, false ) then misn.finish(false) end

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )
   misn.setNPC( _("l337_b01"), prt.t.tex, _([[Try to get in touch with l337_b01.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Help l337_b01 find out who the instigator is by setting up a honeypot and monitoring communications in the {sys} system.]]),
      {sys=mem.targetsys}))
end

local function reset_osd()
   misn.markerRm()
   misn.markerAdd( mem.targetsys )
   misn.osdCreate( title, {
      fmt.f(_("Go to the location in the {sys} system"),
         {sys=mem.targetsys}),
      _("Scan ships in the system"),
   } )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01()

   if not mem.talked then
      vn.transition()

      vn.na(_([[You send a connection request to the Nexus address of l337_b01.]]))
      vn.na(_([[There is no response, but you keep on trying.]]))
      vn.na(_([[...]]))
      vn.na(_([[Maybe a bit more?]]))
      vn.na(_([[...]]))
      vn.na(_([[You're about to give up for now when the connection gets through.]]))

      vn.scene()
      vn.newCharacter( l337 )
      vn.music( onion.loops.hacker ) -- TODO different music
      vn.transition("electric")

      l337(_([[l337_b01's avatar appears in front of you.]]))
      vn.menu{
         {_([["Yo."]]), "01_cont"},
         {_([["Heyo."]]), "01_heyo"},
         {_([["l337_b01, you alright?"]]), "01_alright"},
      }

      vn.label("01_heyo")
      vn.na(_([[There is a long pause.]]))
      vn.jump("01_cont")

      vn.label("01_alright")
      l337(_([["Not really, no."]]))
      vn.jump("01_cont")

      vn.label("01_cont")
      l337(_([["I guess I made a mess out there."]]))
      vn.menu{
         {_([["It was already a mess."]]), "02_cont"},
         {_([["Can't be helped."]]), "02_cont"},
         {_([["I barely followed."]]), "02_followed"},
      }

      vn.label("02_followed")
      l337(_([[You hear a long sigh.
"It's a long story. Sorry, I'm not in the mood to remember it now."]]))
      vn.jump("02_cont")

      vn.label("02_cont")
      l337(_([["Something feels wrong. Almost like a dream."]]))
      vn.menu{
         {_([["You're in shock."]]), "03_shock"},
         {_([["Something is definitely fishy."]]), "03_cont"},
      }

      vn.label("03_shock")
      l337(_([["And you aren't!?!"
They go quiet for a second.]]))
      vn.jump("03_cont")

      vn.label("03_cont")
      l337(_([["It was all going so well, this couldn't... shouldn't have happened..."]]))
      vn.menu{
         {_([["What do you mean?"]]), "04_mean"},
         {_([["Is the Nexus IT is behind this?"]]), "04_nexus"},
         {_([["You think someone in the Onion society did this?"]]), "04_insider"},
      }

      vn.label("04_mean")
      l337(_([["I think someone is working against us. Maybe someone has been tracking us the entire time."]]))
      vn.menu{
         {_([["Is the Nexus IT is behind this?"]]), "04_nexus"},
         {_([["You think someone in the Onion society did this?"]]), "04_insider"},
      }

      vn.label("04_nexus")
      l337(_([["That's very unlikely. The Nexus IT don't have the skills or authority to pull something like this off. They would have left a trail several light years wide."]]))
      l337(_([["It has to be someone much more knowledgeable about this."]]))
      vn.menu{
         {_([["You think someone in the Onion society did this?"]]), "04_insider"},
      }

      vn.label("04_insider")
      l337(_([["Is there any other way someone could have tracked us without noticing? They have to have serious skills."]]))
      l337(_([["Who do you think it is?"]]))
      vn.menu( function ()
         -- Randomize order of main 4 targets
         local opts = rnd.permutation{
            {_([[underworlder]]), "05_underworlder"},
            {_([[DOG]]), "05_dog"},
            {_([[notasockpuppet]]), "05_notasockpuppet"},
            {_([[lonewolf4]]), "05_lonewolf4"},
         }
         table.insert( opts, {_([[Ogre]]), "05_ogre"} )
         table.insert( opts, {_([[Trixie]]), "05_trixie"} )
         return opts
      end )

      vn.label("05_underworlder")
      vn.func( function() var.push("onion_guess_insider", "underworlder") end )
      l337(_([["I thought about them, but they don't seem to have that much to gain in this. They always dislike change, and want to keep things as stable as they can be."]]))
      vn.jump("05_thoughts")

      vn.label("05_dog")
      vn.func( function() var.push("onion_guess_insider", "dog") end )
      l337(_([["Let me think about it."
They go silent for a bit.
"I guess DOG is a possibility, however, it is unlikely. They don't have much to gain in this as they are no longer a keeper of the secrets."]]))
      l337(_([["Ah, I'll explain what keeper of the secrets is later, just trust me here."]]))
      vn.jump("05_thoughts")

      vn.label("05_notasockpuppet")
      vn.func( function() var.push("onion_guess_insider", "notasockpuppet") end )
      l337(_([["notasockpuppet? They're an asshole, but I don't think they have the patience to do something like this. They seem to only be able to focus on things for a couple of periods before they get bored."]]))
      vn.jump("05_thoughts")

      vn.label("05_lonewolf4")
      vn.func( function() var.push("onion_guess_insider", "lonewolf4") end )
      l337(_([["That's exactly what I was thinking! You can see them trying to pin the blame on me! It's almost as like they are trying to escalate the situation to their own game."]]))
      l337(_([["Good to see we are on the same page."]]))
      vn.jump("05_cont")

      vn.label("05_ogre")
      vn.func( function() var.push("onion_guess_insider", "ogre") end )
      l337(_([["You serious? Ogre is long gone. They barely had the skills to tie their own shoelaces let alone do anything serious on the Nexus."]]))
      vn.jump("05_thoughts")

      vn.label("05_trixie")
      vn.func( function() var.push("onion_guess_insider", "trixie") end )
      l337(_([["Don't joke about that."
They let out a sigh.]]))
      l337(_([["I guess it's not entirely out of the question, but why would they peel themselves? Or was it all a ruse to cover their tracks?"]]))
      l337(_([["It just sounds like a wild conspiracy theory. I don't think it makes sense to go down that train of thought."]]))
      vn.jump("05_thoughts")

      vn.label("05_thoughts")
      l337(_([["I was thinking that it is probably lonewolf4, you can see them trying to pin the blame on me. It's almost as like they are trying to escalate the situation to their own game!"]]))

      vn.label("05_cont")
      l337(_([["I've always got the creeps from them. With their entire weird role-playing and funny talk."]]))
      l337(_([["We can't sit back and let them win. Trixie would..."
They have to take a deep breath.
"Trixie would never quit! And I'm not going to either!"]]))
      l337(fmt.f(_([["I've got an idea. We can set up a honeypot and then see if we can trap the bastard and get their general location. One second, it seems like the {sys} system would be a good place to set this up. You in?"]]),
         {sys=mem.targetsys}))
      vn.func( function () mem.talked = true end )
   else
      vn.newCharacter( l337 )
      vn.music( onion.loops.hacker )
      vn.transition("electric")

      vn.na(_([[You open a channel with l337_b01.]]))
      l337(fmt.f(_([["You ready to set up the honeypot to trap the bastard. The {sys} system would be a good place to set this up. You in?"]]),
         {sys=mem.targetsys}))
   end

   vn.menu{
      {_([["I'm in!"]]), "agree"},
      {_([["Let me get ready."]]), "wait"},
   }

   vn.label("wait")
   l337(_([["OK, get in touch with me when you are ready to do this."]]))
   vn.done("electric")

   vn.label("agree")
   l337(fmt.f(_([["Great! You leave the specifics to me, I'll be proxying through your ship again. Get us to the {sys} system and I'll handle the hacking."]]),
      {sys=mem.targetsys}))
   vn.func( function() accepted = true end )

   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()

   mem.state = 0
   reset_osd()
   hook.enter("enter")
end

local ships_scanned = {}
function enter ()
   local scur = system.cur()
   if mem.state<STATE_TALKED_TO_DOG then
      hook.timer( 8, "dog" )
   elseif scur==mem.targetsys and mem.state<=STATE_TALKED_TO_DOG then

      -- Try to get a good position
      local rep = 0
      local good
      local position
      local function good_position( pos )
         for k,s in ipairs(scur:spobs()) do
            if s:pos():dist2(pos) < 2500^2 then
               return false
            end
         end
         local mindist = math.huge
         for k,j in ipairs(scur:jumps()) do
            mindist = math.min( j:pos():dist2(pos), mindist )
         end
         return mindist > 5000^2
      end
      repeat
         position = vec2.newP( 2/3*scur:radius(), rnd.angle() )
         good = good_position(position)
         rep = rep+1
      until good or rep > 100

      mem.honeypot = position
      mem.sysmarker = system.markerAdd( position, _("Honeypot") )

      trigger.distance_player( position, 2000, function ()

         player.msg(_([[l337_b01: I've set up the honeypot. Start scanning ships!]]),true)

         mem.state = STATE_SET_UP_HONEYPOT
         ships_scanned = {}
         system.markerRm( mem.sysmarker )

         hook.pilot( player.pilot(), "scan", "scan" )
         scan() -- set up OSD and such
         hook.timer( 5, "scan_start" )
      end )
   elseif mem.state==STATE_BEAT_MERCENARIES then
      -- Make random mercenaries attack the player
      hook.timerClear()
      hook.timer( 30*rnd.rnd(), "mercenaries_gone_bad" )
   else
      -- Reset state
      mem.state = nil
      hook.timerClear()
      reset_osd()
   end
end

function scan_start()
   player.msg(_([[l337_b01: OK, I need you to scan some ships. Any will do.]]),true)
end

-- Small chat with dog
function dog()
   vn.clear()
   vn.scene()
   local dog = vn.newCharacter( onion.vn_dog() )
   vn.transition("electric")

   vn.na(_([[Your systems flicker for a second, before a familiar hologram appears.]]))
   dog(fmt.f(_([["Pardon the intrusion, {name}. I do not have much time, so I will keep this short."]]),
      {name=player.name()}))
   dog(_([["I worry you are getting too deep. l337_b01 is drawn more towards passion than reason, and I worry about their safety."]]))
   dog(_([["You should not proceed further, as such recklessness may endanger us all."]]))
   vn.na(_([[The hologram fades as your systems flicker once more and everything returns to normal. What was that all about?]]))

   vn.done("electric")
   vn.run()

   mem.state = STATE_TALKED_TO_DOG
end

-- Make natural spawned mercenaries become hostile to the player if they see them
function mercenaries_gone_bad()
   -- Stop if the system gets claimed
   if not naev.claimTest( system.cur(), true ) then
      return
   end

   -- Go over all the mercenaries
   local pp = player.pilot()
   for k,p in ipairs(pilot.get( { faction.get("Mercenary") } )) do
      if p:memory().natural then
         local ir, fuz = p:inrange(pp)
         if ir and fuz then
            p:setHostile(true)
         end
      end
   end
   hook.timer( 15+30*rnd.rnd(), "mercenaries_gone_bad" )
end

local function spawn_baddies()
   -- Clear pilots
   pilotai.clear()

   -- Spawn new ones and send them towards the player
   local baddies = fleet.spawn({
      "Pacifier",
      "Admonisher",
      "Admonisher",
   }, "Mercenary", lmisn.nearestJump() )
   for k,p in ipairs(baddies) do
      p:setHostile(true)
   end
   pilotai.patrol( baddies, {player.pos(), mem.honeypot} )
   pilotai.setTaunt( baddies, _("That's the ship!") )
   baddies[1]:setHilight(true)
   baddies[1]:setVisplayer(true)
   baddies[1]:rename(_("Mercenary Boss"))

   -- Finish when beaten
   trigger.pilots_defeated( baddies, function ()
      mem.state = STATE_BEAT_MERCENARIES
      player.msg(_("l337_b01: OK, we have enough data. Land somewhere with a Nexus connection."),true)
      misn.osdCreate( title, {
         _("Land to speak with l337_b01."),
      } )
      hook.land( "land" )
   end  )

   trigger.timer_chain{
      { 5, _([[l337_b01: What is this? Shit, it seems like someone put a bounty on your ship!]]) },
      { 5, _([[l337_b01: Wait, we can probably use this. Take the mercenaries out!]]) },
      { 1, function ()
         misn.osdCreate( title, {
            _("Defeat the mercenaries!"),
         } )
      end },
   }
end

function scan( _pp, tgt )
   if mem.state~=STATE_SET_UP_HONEYPOT then
      return
   end

   if not inlist( ships_scanned, tgt ) then
      table.insert( ships_scanned, tgt )
      player.msg(fmt.f(_("Ship '{name}' was scanned. {left} ships left to scan."),
         {name=tgt:name(), left=SHIPS_TO_SCAN-#ships_scanned}))
   end
   misn.markerRm()
   misn.osdCreate( title, {
      fmt.f(_("Scan {n} ships in the system by targeting them until the scan is complete ({left} left)"),
         {n=SHIPS_TO_SCAN, left=SHIPS_TO_SCAN-#ships_scanned}),
   } )
   if #ships_scanned >= SHIPS_TO_SCAN_REAL then
      spawn_baddies()
      mem.state = STATE_FINISH_SCANS
   end
end

function land ()
   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01()
   vn.newCharacter( l337 )
   vn.music( onion.loops.hacker )
   vn.transition("electric")

   vn.na(_([[You land and are promptly greeted by l337_b01's hologram.]]))
   l337(_([["Putting a fake bounty on your ship was unexpected, but thanks to that, I was able to get more information."]]))
   l337(_([["I had to clear the fake bounty, so I'm still a bit behind on analysing the collected data, but I should be able to churn through it in a bit."]]))
   l337(_([["Get in touch with me in a bit, and we'll finally unmask who is behind everything!"]]))
   vn.na(_([[The connection closes as l337_b01 focuses computational resources on signal processing.]]))

   vn.scene()
   vn.transition("electric")
   vn.na(_([[As you recline in your captain's chair you notice you got an incoming transfer from some anonymous account, but you can guess who sent it to you.]]))
   vn.sfxVictory()
   vn.func( function () player.pay( reward ) end )
   vn.na(fmt.reward(reward))

   vn.run()

   onion.log(_([[You helped l337_b01 set up a honeypot to intercept communication and try to unmask whoever is behind the recent incidents. A fake bounty was set up on your ship, but you were able to overcome mercenaries set on you.]]))

   misn.finish(true)
end
