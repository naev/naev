--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 5">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 4</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 05

   Help scavenger go look for the curious drones
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local lmisn = require "lmisn"
local pilotai = require "pilotai"
local fleet = require "fleet"
local pulse = require "luaspfx.pulse"
local tut = require "common.tutorial"
local cinema = require "cinema"

local reward = taiomi.rewards.taiomi05
local title = _("Missing Drones")
local base, basesys = spob.getS("One-Wing Goddard")
-- The systems below backtrack from Taiomi to Haven
local firstsys = system.get("Bastion")
local fightsys = system.get("Gamel")
local lastsys = system.get("Haven")
local firstpos = vec2.new( -2500, -2000 )
local fightpos = vec2.new( -2000, 3000 )
local corpsepos = vec2.new( 11e3, 5e3 )
local DIST_THRESHOLD = 2000 -- Distance in units
local BROADCAST_LENGTH = 150 -- Length in seconds
local SPAWNLIST_FIRST = {
   { p={"Pirate Hyena", "Pirate Hyena"}, t=0 },
   { p={"Pirate Shark", "Pirate Shark"}, t=20 },
   { p={"Pirate Admonisher"}, t=30 }, -- Slow, will take time
   { p={"Pirate Hyena", "Pirate Hyena"}, t=60 },
   { p={"Pirate Ancestor"}, t=70 },
   { p={"Pirate Vendetta","Pirate Vendetta"}, t=100 },
}
local SPAWNLIST_FIGHT = {
   { p={"Pirate Hyena", "Pirate Hyena", "Pirate Hyena", "Pirate Hyena"}, t=0 },
   { p={"Pirate Admonisher"}, t=20 }, -- Slow, will take time
   { p={"Pirate Shark", "Pirate Shark"}, t=40 },
   { p={"Pirate Starbridge"}, t=40 }, -- Slow, will take time
   { p={"Pirate Vendetta", "Pirate Vendetta"}, t=65 },
   { p={"Pirate Ancestor"}, t=80 },
   { p={"Pirate Shark","Pirate Shark", "Pirate Shark"}, t=100 },
}

--[[
   0: mission started
   1: landed on one-winged goddard
   2: first broadcast
   3: finish defense
   4: second broadcast
   5: finish second defense
   6: return time
--]]
mem.state = 0

function create ()
   if not misn.claim({firstsys, fightsys}) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to help find {name}, who went missing."),
      {name = taiomi.young_died()} ) )
   misn.setReward(reward)
   mem.marker = misn.markerAdd( base )

   misn.osdCreate( title, {
      fmt.f(_("Land on {base}"),{base=base}),
      fmt.f(_("Search for {name}"),{name=taiomi.young_died()})
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

local function osd_list ()
   local dead = taiomi.young_died()
   local osd = {
      fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=fightsys}),
      fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=lastsys}),
      _("Scavenger must survive"),
   }
   if mem.state <= 2 then
      table.insert( osd, 1, fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=firstsys}) )
   end
   return osd
end

local prevsys = basesys
function enter ()
   if mem.timer then
      hook.rm( mem.timer )
      mem.timer = nil
   end

   local scur = system.cur()
   if mem.state==1 and scur == firstsys and prevsys==basesys then
      mem.timer = hook.timer( 3, "scavenger_enter" )

   elseif mem.state==3 and scur == fightsys and prevsys==firstsys then
      mem.timer = hook.timer( 3, "scavenger_enter" )

   end

   prevsys = scur
end

local scavenger
local function spawn_scavenger( pos )
   scavenger = pilot.add( "Drone (Hyena)", "Independent", pos, _("Scavenger Drone") )
   scavenger:setFriendly(true)
   hook.pilot( scavenger, "hail", "scavenger_hail" )
   hook.pilot( scavenger, "death", "scavenger_death" )
   hook.pilot( scavenger, "attacked", "scavenger_attacked" )
   local m = scavenger:memory()
   m.vulnerability = 1000 -- Less preferred as a target
   scavenger:intrinsicSet( "shield", 1000 ) -- beefy shields
   return scavenger
end

function scavenger_hail ( p )
   if mem.state < 6 then
      p:comm(_("Our task is of uttermost importance!"))
   else
      p:comm(_("Aaaaaagh!"))
   end
   player.commClose()
end

function scavenger_death ()
   lmisn.fail(_("Scavenger died! You were supposed to protect them!"))
end

function scavenger_attacked( _p, attacker )
   -- Make whatever attacks scavenger hostile
   attacker:setHostile(true)
end

local systemmrk
function scavenger_enter ()
   local jmp, pos, msg
   if mem.state < 2 then
      jmp = jump.get( firstsys, basesys )
      pos = firstpos
      msg = _("I have marked the first location.")
   else
      jmp = jump.get( fightsys, firstsys )
      pos = fightpos
      msg = _("Let us make haste to the next location.")
   end
   local s = spawn_scavenger( jmp )
   s:control()
   s:follow( player.pilot() ) -- TODO probably something better than just following

   -- Highlight position
   systemmrk = system.markerAdd( pos )
   local osd = osd_list ()
   osd[1] = _("Go to the marked location")
   misn.osdCreate( title, osd )

   hook.timer( 5, "scavenger_msg", msg )
   hook.timer( 1, "check_location", pos )
end

function scavenger_msg( msg )
   scavenger:comm(msg)
end

function check_location( pos )
   if pos:dist( player.pos() ) < DIST_THRESHOLD then
      scavenger:taskClear()
      scavenger:moveto( pos )
      scavenger:comm(_("Let me get in position."))
      hook.timer( 10, "scavenger_pos", pos )
      return
   end
   hook.timer( 1, "check_location", pos )
end

local function do_pulse ()
   pulse( scavenger:pos(), scavenger:vel(), {size=1000} )
end

local broadcast_timer, broadcast_spawned, broadcast_spawnlist
function scavenger_pos( pos )
   if pos:dist( scavenger:pos() ) < 100 then
      broadcast_spawned = 0
      broadcast_timer = 0
      if mem.state==1 then
         broadcast_spawnlist = SPAWNLIST_FIRST
         mem.state = 2
      else
         broadcast_spawnlist = SPAWNLIST_FIGHT
         mem.state = 4
      end
      scavenger:comm(_("Commencing broadcast!"))
      scavenger_broadcast( pos )
      system.markerRm( systemmrk )
      scavenger:setHilight( true )
      do_pulse()

      pilotai.clear() -- Get rid of all natural pilots if possible
      pilot.toggleSpawn(false)
      return
   end
   hook.timer( 1, "scavenger_pos", pos )
end

local corpse
function scavenger_broadcast( pos )
   broadcast_timer = broadcast_timer + 1

   local spawn = broadcast_spawnlist[ broadcast_spawned+1 ]
   if spawn and spawn.t and broadcast_timer > spawn.t then
      local fct = faction.get("Marauder")
      for k,p in ipairs( fleet.add( 1, spawn.p, fct ) ) do
         pilotai.guard( p, pos )
         p:setHostile()
      end
      broadcast_spawned = broadcast_spawned+1
   end

   -- Do more pulses over time
   if math.fmod( broadcast_timer, 4 )==0 then
      do_pulse()
   end

   -- Can get knocked off by weapon knockback, go back
   if pos:dist( scavenger:pos() ) < 100 then
      scavenger:moveto( pos )
   end

   -- Broadcast over
   if broadcast_timer > BROADCAST_LENGTH then
      scavenger:taskClear()
      scavenger:follow( player.pilot() )
      if mem.state==2 then
         scavenger:comm(_("Nothing… Let us move on."))
         pilot.toggleSpawn(true)
         scavenger:setHilight( false )
         misn.markerMove( mem.marker, fightsys )
         mem.state = 3
      else
         scavenger:setHilight( false )

         -- Find location
         corpse = pilot.add( "Drone", "Independent", corpsepos, taiomi.young_died() )
         corpse:disable()
         corpse:setInvisible(true)
         system.markerAdd( corpsepos )

         scavenger:comm(_("I got a faint signal! Let's rush there!"))
         misn.osdCreate( title, {
            _("Search the marked area"),
         } )
         mem.state = 5
         hook.timer( 1, "scavenger_approachcorpse" )
         return
      end
      -- Update OSD and marker
      local osd = osd_list()
      misn.osdCreate( title, osd )
      return
   end

   -- Update OSD
   local osd = osd_list()
   osd[1] = fmt.f(_("Protect Scavenger ({left} s left)"),
      {left=BROADCAST_LENGTH-broadcast_timer})
   misn.osdCreate( title, osd )

   hook.timer( 1, "scavenger_broadcast", pos )
end

function scavenger_approachcorpse ()
   local d = player.pos():dist( corpsepos )
   if d < 2e3 then
      cinema.on()

      camera.set( scavenger )
      scavenger:setInvincible(true)

      local spos = scavenger:pos()
      local cpos = corpse:pos()
      local pos = cpos - vec2.newP( 100, (cpos-spos):angle() )
      scavenger:taskClear()
      scavenger:moveto( pos )
      scavenger:face( cpos )

      hook.timer( 15, "corpse00" )
      return
   end
   hook.timer( 1, "scavenger_approachcorpse" )
end

local corpse_pir
function corpse00 ()
   local dead = taiomi.young_died()

   vn.clear()
   vn.scene()
   vn.music( "snd/sounds/songs/sad_drama.ogg" )
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )

   vn.na(_("In the vast darkness of space, you make out a small white speck. Instinctively knowing the worst has come to pass, scavenger slows down and gets closer to take a good look."))
   vn.na(fmt.f(_("As you focus your scanners, you begin to make out the details of the wreck. It does not seem like {dead} will be making it back to Taiomi…"),
      {dead=dead}))
   vn.na(_("Scavenger remains silent while they carefully examine the pieces and parts of ship debris, meticulously caressing and collecting the different parts together as a requiem."))
   vn.na(fmt.f(_("Finally, without turning their back to {dead}, they open a secure communication channel with you. However, it takes a while for the channel to lose its somber silence."),
      {dead=dead}))
   s(_([["What is this sensation? Almost if my neurocircuitry was set on fire and exposed to space radiation. This should not be in my programming."]]))
   s(_([["So full of hope, all lost to the vanity of human marauders. There can be no dreams of leaving to the stars as long as the humans encroach and pick us off!"]]))
   s(_([["The Elder was right! There is no future for our kind by hiding in the shadows and planning our escape. We must grab our future by the core by establishing our own territory! Death to all humans!"]]))
   vn.na(_("The secure communication channel quickly switches to a global broadcast."))
   s(fmt.f(_([[Broadcast: "Human scum, prepare to be annihilated. There shall be blood for {dead}!"]]),{dead=dead}))

   -- "Only love gives us the taste of eternity."
   -- “Grief is the price we pay for love”

   vn.done( taiomi.scavenger.transition )
   vn.run()

   local pos = corpse:pos() + vec2.newP( 3000, rnd.angle() )
   corpse_pir = pilot.add( "Pirate Hyena", "Marauder", pos )
   corpse_pir:control()
   corpse_pir:attack( scavenger )
   hook.pilot( corpse_pir, "death", "pirate_death" )

   hook.timer( 5, "corpse01" )
end

function corpse01 ()
   scavenger:intrinsicSet( "fwd_damage", 500 )
   scavenger:intrinsicSet( "tur_damage", 500 )
   scavenger:intrinsicSet( "launch_damage", 500 )
   scavenger:taskClear()
   scavenger:attack( corpse_pir )

   hook.timer( 5, "corpse02" )
end

function corpse02 ()
   scavenger:broadcast(_("AAaaaaaaaaa!!!"))
end

function pirate_death ()
   scavenger:taskClear()
   local ppos = player.pos()
   local spos = scavenger:pos()
   local pos = spos + vec2.newP( 10e3, (spos-ppos):angle() )
   scavenger:moveto( pos )
   hook.timer( 5, "corpse99" )
end

function corpse99 ()
   cinema.off()

   misn.osdCreate( title, {
      fmt.f(_("Return to {base} ({basesys})?"),{base=base, basesys=basesys}),
   } )
   misn.markerMove( mem.marker, base )

   scavenger:effectAdd( "Fade-Out" )

   mem.state = 6
end

function land ()
   local c = spob.cur()
   if c ~= base then
      return
   end

   if mem.state==0 then
      local dead = taiomi.young_died()
      local alive = taiomi.young_alive()

      vn.clear()
      vn.scene()
      --vn.music("") -- TODO some anticipation music or something
      local s = vn.newCharacter( taiomi.vn_scavenger() )
      vn.transition( taiomi.scavenger.transition )
      vn.na(_([[You disembark and are soon met with a Scavenger you can only describe as solemn.]]))
      s(fmt.f(_([["It is irrational for us to leave the safety of Taiomi unless it is necessary. I have no idea what compelled {younga} and {youngb}. Minimization of risk is essential to our survival, which is why I entrusted the acquirement of Therite to you. Could this be a program fault?"]]),
         {younga=taiomi.younga.name, youngb=taiomi.youngb.name}))
      vn.menu{
         {_([["Is that not curiosity?"]]), "cont01_curiosity"},
         {_([["Maybe they wanted to see the outside world?"]]), "cont01_curiosity"},
         {_([["Definitely a software bug."]]), "cont01_software"},
         {_([[…]]), "cont01"},
      }

      vn.label("cont01_curiosity")
      s(_([["Such irrationality does not compute. It is very unlikely of our kind to behave in such suboptimal behaviour. My simulations indicate that it is more probable that this be a degeneration of our computational cortex after excessive replication."]]))
      vn.jump("cont01")

      vn.label("cont01_software")
      s(_([["It is statistically unlikely that our diagnostics did not pick up such an important issue. Maybe the diagnostic protocols need to be revised. It would not be the first time we had issues with them."]]))
      vn.jump("cont01")

      vn.label("cont01")
      s(fmt.f(_([["Being too late to take preemptive measures, we have no other option than be proactive in searching for {dead}."]]),
         {dead=dead}))
      s(fmt.f(_([["The only information we were able to obtain from {alive} is that both {alive} and {dead} followed you. It is not exactly clear what happened, but they got surprised and fled. At that time, it seems like they got split up. While {alive} made it back, {dead} is still missing"]]),
         {alive=alive, dead=dead}))
      s(fmt.f(_([["We have tried to get more details, but {alive} seems to be stuck in a loop. Until their computational cortex recovers, we have no option but to infer the most probable situation from the available data to find {dead}."]]),
         {alive=alive, dead=dead}))
      s(fmt.f(_([["I have simulated the most likely situations and devised a plan that maximizes the chance of recovering {dead} safely by minimizing the amount of time to find them, however, this will come at a risk for us."]]),
         {dead=dead}))
      s(fmt.f(_([["The core idea is to backtrack the most probable path, starting with {firstsys}, then {fightsys}, and finally {lastsys}. At each system, we will make a run to an optimal position, and I will begin broadcasting a special code while listening to possible answers."]]),
         {firstsys=firstsys, fightsys=fightsys, lastsys=lastsys}))
      s(fmt.f(_([["There is no time to explain the details, but this code will allow detecting {dead}. However, I will remain largely immobile, and it is likely that it will attract unwanted attention in the system. I will need you to protect me for the duration of the signal."]]),
         {dead=dead}))

      vn.menu{
         {_([["Understood."]]), "cont02"},
         {_([["Wait, you'll be coming?"]]), "cont02_q"},
         {_([[…]]), "cont02"},
      }

      vn.label("cont02_q")
      s(fmt.f(_([["Yes. While I believe your piloting skills are excellent, they will not be sufficient to find {dead} in time. While I am not a fighter, this plan maximizes the probability of success. All other plans are suboptimal."]]),
         {dead=dead}))

      vn.label("cont02")
      s(fmt.f(_([["When you are ready, we shall make route to {nextsys}. Combat is anticipated, so be prepared."]]),
         {nextsys=firstsys}))

      vn.done( taiomi.scavenger.transition )
      vn.run()

      misn.markerMove( mem.marker, firstsys )
      mem.state = 1 -- advance state

      local osd = osd_list ()
      misn.osdCreate( title, osd )
      return

   elseif mem.state==6 then
      vn.clear()
      vn.scene()

      local p = taiomi.vn_philosopher{ pos="farright", flip=false }
      local w = taiomi.vn_elder{ pos="farleft", flip=true }
      local sai = tut.vn_shipai()
      local died = taiomi.young_died()
      vn.transition()

      vn.na(fmt.f(_("You return to the {base} to try to process what happened… Not only was the life of {dead} lost, but Scavenger has also gone missing in their thirst for revenge…"),
         {base=base, dead=taiomi.young_died()}))

      vn.appear( sai, tut.shipai.transition )
      sai(fmt.f(_([[While you are pondering to yourself, {shipai} materializes besides you.
"I recorded the last communication with Scavenger. You may find it of use."]]),
         {shipai=tut.ainame()}))
      sai(fmt.f(_([[As you get a notification of nearby motion, {shipai} dematerializes.]]),
         {shipai=tut.ainame()}))
      vn.disappear( sai, tut.shipai.transition )

      vn.appear( {p,w} )
      vn.music( "snd/sounds/songs/sad_drama.ogg" ) -- TODO necessary?
      vn.na(_("You hear the squeal of metal rubbing on metal as Elder and Philosopher make their way haphazardly through the ship. They get close to you until they fill your entire field of vision."))
      w(_([[After what seems an eternity, Elder begins to broadcast slow and solemnly over an audio channel.
"It seems like Scavenger has not returned."]]))
      vn.menu{
         {_([["They have gone missing."]]), "01_cont"},
         {fmt.f(_([["We found {died}…"]]),{died=died}), "01_cont"},
         {_([[…]]), "01_silent"},
      }

      vn.label("01_missing")
      vn.na(fmt.f(_("You explain the events that happened leading up to finding {died}'s body and Scavenger deciding to take revenge in their own hands."),
         {died=died}))
      vn.jump("01_cont")

      vn.label("01_silent")
      vn.na(fmt.f(_("You play the recording of the events that happened leading up to finding {died}'s body and Scavenger deciding to take revenge in their own hands."),
         {died=died}))
      vn.jump("01_cont")

      vn.label("01_cont")
      w(_([[After a bout of silence, Elder speaks.
"The only way to be safe is to carve our own space. If we are to be decimated, I would rather go down fighting than hunted."]]))
      w(_([["It seems like Scavenger has finally seen the way. Without security, we have no freedom. We have to refocus on eliminating all hostiles to ensure the safety of our young ones and continuity of our species!"]]))
      w(_([[They turn to you.
"I have seen you have been helping Scavenger. If you wish to further help us secure our freedom, communicate with me out in space. I do not do well in closed spaces."]]))
      w(_([[After their last words, Elder once again begins the painstaking process of getting out of the Goddard as a chorus of metal scratching sounds echoes throughout the ship.]]))
      vn.disappear( w )

      p(_([[You are left alone with Philosopher, who suddenly breaks the silence.
"It seems like things are changing faster than expected. We may seem like rational machines, but that that is a mere illusion. I do not believe it is possible for there to be sentience without some irrationality."]]))
      p(_([["Before you left, Scavenger told me to follow-up. Although I am not much of mundane pragmatism, my musing depend on our survival. Please take this reward for helping Scavenger. If you are looking for more work, I recommend you see Elder. I can not provide anything else."]]))
      p(_([[Philosopher takes their leave, exiting the ship in a slightly more elegant way than Elder.]]))
      vn.disappear( p )

      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( taiomi.scavenger.transition )
      vn.run()

      -- Force name known just in case
      var.push( "taiomi_drone_elder", true )

      player.pay( reward )
      taiomi.log.main(fmt.f(_("You found out that {died} was destroyed by marauders. This caused Scavenger to go into a rage and disappear into deeper space to seek revenge. Back at Taiomi, Elder offered to give you more work."),
         {died=died}))
      misn.finish(true)
   end
end
