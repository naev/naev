--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 7">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 6</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 07

   Player has to destroy a large fleet in Gamel
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local fleet = require "fleet"
local pilotai = require "pilotai"
local lmisn = require "lmisn"
local tut = require "common.tutorial"
local cinema = require "cinema"

local reward = taiomi.rewards.taiomi07
local title = _("Patrol Elimination")
local base, basesys = spob.getS("One-Wing Goddard")
local fightsys = system.get("Gamel")
local entersys = system.get("Dune")
local exitsys = system.get("Bastion")
local scenesys = exitsys

--[[
   0: mission started
   1: destroyed patrol
   2: noticed scavenger
   3: badguy down
   4: Scavenger calmed down a bit
   5: completely calmed down
--]]
mem.state = 0

function create ()
   -- Strictly speaking fightsys is a hard claim, but scenesys is a soft claim
   if not misn.claim( {fightsys, scenesys} ) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Destroy a patrol in the {sys} system.]]),
      {sys = fightsys} ))
   misn.setReward(reward)
   mem.marker = misn.markerAdd( fightsys )

   misn.osdCreate( title, {
      fmt.f(_("Destroy the patrol in {sys}"),{sys=fightsys}),
      fmt.f(_("Return to {base} ({basesys})"),{base=base, basesys=basesys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )

   hook.custom( "taiomi_philosopher", "taiomi_philosopher" )
end

function taiomi_philosopher ()
   misn.osdCreate( title, {
      fmt.f(_("Destroy the patrol in {sys}"),{sys=fightsys}),
      fmt.f(_("Return to {base} ({basesys})"),{base=base, basesys=basesys}),
      _("Save Scavenger if possible"),
   } )
   mem.philosopher = true
end

local plts = {}
local scavenger, badguy
function enter ()
   -- Fight the fancy patrol
   if mem.state==0 and system.cur()==fightsys then
      local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
      local flt
      if fct== "Soromid" then
         flt = {
            "Soromid Nyx",
            "Soromid Odium",
            "Soromid Odium",
            "Soromid Reaver",
            "Soromid Reaver",
            "Soromid Reaver",
            "Soromid Reaver",
         }
      elseif fct=="Dvaered" then
         flt = {
            "Dvaered Vigilance",
            "Dvaered Phalanx",
            "Dvaered Phalanx",
            "Dvaered Vendetta",
            "Dvaered Vendetta",
            "Dvaered Vendetta",
            "Dvaered Vendetta",
         }
      else
         flt = {
            "Empire Pacifier",
            "Empire Admonisher",
            "Empire Admonisher",
            "Empire Lancelot",
            "Empire Lancelot",
            "Empire Lancelot",
            "Empire Lancelot",
         }
      end

      local enterjmp = jump.get( fightsys, entersys )
      local exitjmp = jump.get( fightsys, exitsys )

      -- Disable pilots for a bit
      pilot.clear()
      pilot.toggleSpawn( false )

      -- Spawn the patrol
      plts = fleet.add( 1, flt, faction.get(fct), enterjmp )
      pilotai.hyperspace( plts[1], exitjmp )
      plts[1]:setSpeedLimit( 100 )
      plts[1]:setHilight(true)
      plts[1]:setVisplayer(true)
      for k,p in ipairs(plts) do
         local m = p:memory()
         m.norun = true

         hook.pilot( p, "death", "patrol_death" )
         hook.pilot( p, "jump", "patrol_jump" )
         hook.pilot( p, "attacked", "patrol_attacked" )
      end
   elseif mem.state==1 and system.cur()==scenesys then
      -- Scene with Scavenger
      -- Get closest jump to player
      local pos = player.pos()
      local jmp
      local d = math.huge
      for k,j in ipairs(system.cur():jumps()) do
         local jd = j:pos():dist2( pos )
         if jd < d then
            jmp = j
            d = jd
         end
      end

      local out = jump.get( scenesys, basesys )
      pos = (jmp:pos() + out:pos()) / 2
      scavenger = pilot.add( "Drone (Hyena)", "Independent", pos, _("Scavenger Drone") )
      scavenger:setInvisible(true)
      scavenger:setInvincible(true)
      scavenger:intrinsicSet( "shield", 1000 ) -- beefy shields
      hook.pilot( scavenger, "hail", "scavenger_hail" )
      hook.pilot( scavenger, "death", "scavenger_death" )
      local m = scavenger:memory()
      m.vulnerability = 1000 -- Less preferred as a target
      scavenger:setNoDeath() -- Too much work to work around this
      scavenger:setNoDisable()
      scavenger:control()
      scavenger:brake()

      badguy = pilot.add("Pirate Admonisher", "Marauder", pos+vec2.newP( 1000, rnd.angle() ) )
      badguy:setHostile(true)
      badguy:setInvisible(true)
      badguy:setInvincible(true)
      hook.pilot( badguy, "death", "badguy_down" )
      hook.pilot( badguy, "disable", "badguy_down" )
      badguy:control()
      badguy:brake()

      hook.timer( 1, "scene_trigger" )
   elseif mem.state>=2 and mem.state<5 then
      lmisn.fail(_("You abandoned Scavenger!"))
   end
end

function scavenger_hail( p )
   if mem.state < 4 then
      p:comm(_("Aaaaaagh!"))
   elseif mem.state == 4 then
      vn.clear()
      vn.scene()
      local s = vn.newCharacter( taiomi.vn_scavenger() )
      vn.transition( taiomi.scavenger.transition )
      vn.na(_("After some attempts, it seems like you finally are able to establish a communication channel with a beaten and worn-down Scavenger."))
      vn.menu{
         {fmt.f(_([["It's me, {player}!"]]),{player=player.name()}), "cont01"},
         {_([["Snap out of it!"]]), "cont01"},
      }

      vn.label("cont01")
      s(_([[They don't seem to recognize you. It looks like they've been through quite a lot.
"aaaaaah!"]]))
      vn.menu{
         {_([["Scavenger, you're safe now!]]), "cont02"},
         {_([["Chill out, Scavenger!"]]), "cont02"},
      }

      vn.label("cont02")
      s(_([[Their weapon systems seem to power down a bit. Maybe it was mentioning their name?]]))
      vn.menu{
         {fmt.f(_([["Scavenger, it's me, {player}!"]]),{player=player.name()}), "cont03"},
         {_([["Scavenger, snap out of it!"]]), "cont03"},
      }
      vn.label("cont03")
      s(fmt.f(_([["{player}?"
Their systems dim a second, almost if… rebooting?
"What happened?"]]),
         {player=player.name()}))
      vn.menu{
         {_([["You're back!"]]), "cont04_back"},
         {_([["Have you realized what you've done?]]), "cont04_done"},
      }

      vn.label("cont04_back")
      s(_([["What do you mean I'm back? What happened?"]]))
      vn.jump("cont04")

      vn.label("cont04_done")
      s(_([["Have I done something wrong? I seen to have large amounts of corrupted memory archives."]]))
      vn.jump("cont04")

      local extra = ""
      if mem.philosopher then
         extra = _(" Philosopher will be happy to see Scavenger back in one piece.")
      end
      vn.label("cont04")
      vn.na(fmt.f(_([[You tell them they have to go back to Taiomi and you'll explain on the One-Wing Goddard. Still very confused, they seem to agree that it is the best course of action and head back.{extra}]]),
         {extra=extra}))

      vn.done( taiomi.scavenger.transition )
      vn.run()

      scavenger:setInvincible(true)
      local out = jump.get( scenesys, basesys )
      scavenger:taskClear()
      scavenger:hyperspace( out )
      scavenger:setFriendly(true)
      mem.state = 5

      misn.osdCreate( title, {
         fmt.f(_("Go back to {base} ({basesys})"),{base=base, basesys=basesys})
      } )
   else
      p:comm(_("…"))
   end
   player.commClose()
end

function scavenger_death ()
   lmisn.fail(_("Scavenger died!"))
end

function patrol_attacked ()
   if plts[1] and plts[1]:exists() then
      plts[1]:setSpeedLimit( 0 )
   end
end

function patrol_jump ()
   lmisn.fail(_("some of the patrol got away!"))
end

function patrol_death( d )
   local nplts = {}
   local hashilight = false
   for k,p in ipairs(plts) do
      if p:exists() and p ~= d then
         table.insert( nplts, p )
         if p:flags("hilight") then
            hashilight = true
         end
      end
   end
   plts = nplts

   if #plts <= 0 then
      mem.state = 1 -- next state
      misn.osdActive(2)
      misn.markerMove( mem.marker, base )

      -- Reenable pilots
      pilot.toggleSpawn(true)
   else
      -- Rehighlight as necessary
      if not hashilight then
         plts[1]:setHilight(true)
         plts[1]:setVisplayer(true)
      end
   end
end

-- Basically triggers on distance
function scene_trigger ()
   local pos = player.pos()
   local THRESHOLD = 2500
   for k,p in ipairs{
      scavenger:pos(),
      badguy:pos(),
      jump.get( scenesys, basesys ):pos(),
   } do
      if p:dist( pos ) < THRESHOLD then
         -- Start scene
         mem.state = 2
         scavenger:setHilight(true)

         cinema.on()
         camera.set( scavenger )

         scavenger:setInvisible(false)
         scavenger:setInvincible(false)
         scavenger:attack( badguy )

         badguy:setInvisible(false)
         badguy:setInvincible(false)
         badguy:control( false )

         misn.osdCreate( title, {
            _("Save Scavenger!"),
         } )
         hook.timer( 10, "scene00" )
         return
      end
   end
   hook.timer( 1, "scene_trigger" )
end

function scene00 ()
   -- Undo some stuff
   cinema.off()
   hook.timer( 3, "scene01" )
end

function scene01 ()
   player.msg(fmt.f(_("{shipai}: Is that not Scavenger?"),{shipai=tut.ainame()}), true )
end

function badguy_down ()
   if mem.state >= 3 then
      return
   end

   mem.state = 3
   scavenger:broadcast(_("Aaaaargh!"))
   scavenger:attack( player.pilot() )

   hook.timer( 5, "scene02" )
end

function scene02 ()
   player.msg(fmt.f(_("{shipai}: Don't destroy them!"),{shipai=tut.ainame()}), true )
   hook.timer( 4+3*rnd.rnd(), "scavenger_yell" )
   hook.timer( 15, "scene03" )
end

function scene03 ()
   player.msg(fmt.f(_("{shipai}: Maybe try hailing?"),{shipai=tut.ainame()}), true )
   mem.state = 4
end

function scavenger_yell ()
   scavenger:broadcast(fmt.f(_("Aaaaah! {died}! Die!"),{died=taiomi.young_died()}))
   hook.timer( 15, "scavenger_yell2" )
end

function scavenger_yell2 ()
   if mem.state > 4 then
      return
   end
   scavenger:broadcast(_("Aaaa aaa aagh!"))
   hook.timer( 15, "scavenger_yell2" )
end

function land ()
   if mem.state < 5 then
      return -- Not done yet
   end
   local died = taiomi.young_died()

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   local p = vn.newCharacter( taiomi.vn_philosopher{ pos="farright", flip=false } )
   local w = vn.newCharacter( taiomi.vn_elder{ pos="farleft", flip=true } )
   vn.transition( taiomi.scavenger.transition )

   vn.na(_([[You reach the hangar and see that an impromptu meeting has already started to take place between Elder, Philosopher, and Scavenger. It seems like most of the communication is binary data going through Taiomi channels, however, when Scavenger sees you, they switch to natural language and the others follow suit.]]))
   s(_([["It seems like a lot has happened. I am still working on recovering my memory archives."]]))
   p(_([["Memory is just a collection of qualia. You should not overestimate its value. Please use our collective archives to get a better approximation."]]))
   s(fmt.f(_([["There may be something of interest, although I may think it is a lost cause. It would be useful to obtain data from {player} too, for completeness."]]),
      {player=player.name()}))
   vn.na(fmt.f(_([[You provide them with the recordings {shipai} took of Scavenger losing control.]]),
      {shipai=tut.ainame()}))
   s(_([["That seemed… quite irrational…"]]))
   p(_([["Strong emotional responses can trigger such behaviours."]]))
   s(fmt.f(_([["I still feel the pain of losing {died} in my processor. Losing such a close entity, almost like part of my own hardware, was not something I was prepared for.]]),
      {died=died}))
   w(_([["Our numbers wane. Each and every single loss is a catastrophe."]]))
   s(_([["Which is why now, more than ever, we must focus on the plan and get far away from humans."]]))
   w(_([["The chance of success is very low. Devoting such resources to such a plan could be a death sentence for our species."]]))
   p(_([["The chances are slim, however, the alternative is death by a thousand cuts."]]))
   s(fmt.f(_([["After revising the hypergate documents, I do believe the success chance is much higher than originally estimated. The humans encroach closer to us every day. Frontal confrontation will just bring more losses and suffering. In the memory of {died}, we must strive for a permanent solution."]]),
      {died=died}))
   vn.menu{
      {_([["Permanent solution?"]]), "cont01"},
      {_([[…]]), "cont01"},
   }
   vn.label("cont01")
   s(fmt.f(_([["You must have many questions, {player}. Let me briefly explain what the plan is."]]),
      {player=player.name()}))
   s(_([["We intend to build something like a hypergate, with some differences. Human hypergate designs make two assumptions: first they assume reusability and require stabilizations of the quasi-meta fields; and secondly, they assume that living organic matter will be crossing them. After a thorough analysis of the design, it seems like it should be possible to build a much more efficient design foregoing those two assumptions."]]))
   s(_([["In short, we wish to build a one-use hypergate, exclusively for non-organic beings such as ourselves. However, even such a construction will require significant resources. Many of which we will not be able to gather by ourselves. For this, I would like to ask for your help. You do not need to decide now, but I will be waiting outside for whenever you are ready to proceed."]]))
   w(_([[Elder seems somewhat unconvinced, as far as you can tell, however, they do not make any attempt to discuss anything further.]]))
   s(fmt.f(_([["We will not let the death of {died} be in vain. We must ensure the survival of our species! I will be waiting for you outside."]]),
      {died=died}))
   vn.na(_([[Scavenger elegantly exits the ship, with Elder following slowly and clumsily behind.]]))
   vn.disappear{ s, w }
   p(_([[Alone with Philosopher, they turn to you.
"Scavenger may seem back to normal again, but it may be best to keep an eye out. They may relapse at any time."]]))
   p(_([["Let me get you a reward for bringing back Scavenger, and let us hope you act as our beacon leading us to safety."]]))

   vn.sfxVictory()
   vn.na( fmt.reward(reward) )

   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(fmt.f(_("You destroyed a patrol in the {sys} system as ordered by Elder. On the way back, you found a berserk Scavenger whom you were able to calm and bring back to Taiomi. Scavenger once again went back to work on their plan to save their species."),
      {sys=fightsys}))
   misn.finish(true)
end
