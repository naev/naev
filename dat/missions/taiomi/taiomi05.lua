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
--local equipopt = require "equipopt"
--local luatk = require "luatk"

local reward = taiomi.rewards.taiomi05
local title = _("Missing Drones")
local base, basesys = spob.getS("One-Wing Goddard")
-- The systems below backtrack from Taiomi to Haven
local firstsys = system.get("Bastion")
local fightsys = system.get("Gamel")
local lastsys = system.get("Haven")
local firstpos = vec2.new( -2500, -2000 )
local fightpos = vec2.new( -2000, 3000 )
local DIST_THRESHOLD = 2000 -- Distance in units
local BROADCAST_LENGTH = 300 -- Length in seconds
local SPAWNLIST_FIRST = {
   { p={"Pirate Hyena", "Pirate Hyena"}, t=0 },
   { p={"Pirate Shark", "Pirate Shark"}, t=20 },
   { p={"Pirate Admonisher"}, t=40 },
   { p={"Pirate Hyena", "Pirate Hyena"}, t=60 },
   { p={"Pirate Ancestor"}, t=100 },
   { p={"Pirate Vendetta","Pirate Vendetta"}, t=120 },
   { p={"Pirate Starbridge"}, t=180 },
   { p={"Pirate Hyena", "Pirate Hyena", "Pirate Hyena"}, t=240 },
}
local SPAWNLIST_FIGHT = {
}

--[[
   0: mission started
   1: landed on one-winged goddard
   2: first broadcast
   3: second broadcast
   4: scavenger goes berserk
   5: return time
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
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( base )

   misn.osdCreate( title, {
      fmt.f(_("Land on {base}"),{base=base}),
      fmt.f(_("Search for {name}"),{name=taiomi.young_died()})
   } )

   hook.enter( "enter" )
   hook.land( "land" )
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

   elseif mem.state==2 and scur == fightsys and prevsys==firstsys then
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
   local mem = scavenger:memory()
   mem.vulnerability = 1000 -- Less preferred as a target
   return scavenger
end

function scavenger( p )
   if mem.state < 2 then
      p:comm(_("Our task is of uttermost importance!"))
   else
      p:comm(_("Aaaaaagh!"))
   end
   player.commClose()
end

function scavenger_death ()
   lmisn.fail( "Scavenger died! You were supposed to protect them!" )
end

function scavenger_enter ()
   local dead = taiomi.young_died()
   local osd = {
      fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=firstsys}),
      fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=fightsys}),
      fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=lastsys}),
      _("Scavenger must survive"),
   }

   local jmp, pos, msg
   if mem.state==0 then
      jmp = jump.get( firstsys, basesys )
      pos = firstpos
      msg = _("I have marked the first location.")
      osd[1] = _("Go to the marked location")
   else
      jmp = jump.get( firstsys, basesys )
      pos = fightpos
      msg = _("Let us make haste to the next location.")
      table.remove( osd, 1 )
      osd[1] = _("Go to the marked location")
   end
   local s = spawn_scavenger( jmp )
   s:control()
   s:follow( player.pilot() ) -- TODO probably something better than just following

   -- Highlight position
   system.mrkAdd( pos )
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

local broadcast_timer, broadcast_spawned, broadcast_spawnlist
function scavenger_pos( pos )
   if pos:dist( scavenger:pos() ) < 100 then
      mem.state = mem.state+1
      broadcast_spawned = 0
      broadcast_timer = 0
      if mem.state==2 then
         broadcast_spawnlist = SPAWNLIST_FIRST
      else
         broadcast_spawnlist = SPAWNLIST_FIGHT
      end
      scavenger:comm(_("Commencing broadcast!"))
      scavenger_broadcast( pos )

      pilotai.clear() -- Get rid of all natural pilots if possible
      pilot.toggleSpawn(false)
      return
   end
   hook.timer( 1, "scavenger_pos", pos )
end

local enemies = {}
function scavenger_broadcast( pos )
   broadcast_timer = broadcast_timer + 1

   local spawn = broadcast_spawnlist[ broadcast_spawned+1 ]
   if spawn and spawn.t and broadcast_timer > spawn.t then
      local fct = faction.get("Marauder")
      for k,p in ipairs( fleet.add( 1, spawn.p, fct ) ) do
         pilotai.guard( p, pos )
         p:setHostile()
         table.insert( enemies, p )
      end
      broadcast_spawned = broadcast_spawned+1
   end

   if broadcast_timer > BROADCAST_LENGTH then
      scavenger:taskClear()
      scavenger:follow( player.pilot() )
      if mem.state==2 then
         scavenger:comm(_("Nothing… Let us move on."))
         pilot.toggleSpawn(true)
         return
      --else
      end
      mem.state = mem.state + 1
      return
   end

   -- Update OSD
   local osdtitle, osd = misn.osdGet()
   osd[1] = fmt.f(_("Protect Scavenger ({left} s left)"),
      {left=BROADCAST_LENGTH-broadcast_timer})
   misn.osdCreate( osdtitle, osd )

   hook.timer( 1, "scavenger_broadcast", pos )
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
      s(fmt.f(_([["The core idea is to backtrack the most probably path, starting with {firstsys}, then {fightsys}, and finally {lastsys}. At each system, we will make a run to an optimal position, and I will begin broadcasting a special code while listening to possible answers."]]),
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

      mem.marker = misn.markerMove( mem.marker, firstsys )
      mem.state = 1 -- advance state

      misn.osdCreate( title, {
         fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=firstsys}),
         fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=fightsys}),
         fmt.f(_("Search for {name} ({sys})"),{name=dead, sys=lastsys}),
         _("Scavenger must survive"),
      } )
      return
   end

   -- TODO
   --[=[
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   --vn.na(fmt.f(_([[You bring the {resource} aboard the Goddard and find Scavenger waiting for you.]]),
   --   {resource=resource}))
   s(_([["I see you managed to bring all the needed resources. This will be enough enough for us to start working on our project. I will be outside getting things set up. We may still need something else so make sure to check in after you get some rest."
Scavenger backs out of the Goddard and returns to space.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You collected important resources for the inhabitants of Taiomi."))
   misn.finish(true)
   --]=]
end
