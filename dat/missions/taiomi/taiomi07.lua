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
   4: Scavenger calmed donw a bit
   5: completely calmed down
--]]
mem.state = 0

function create ()
   if not misn.claim( {fightsys, scenesys}, true) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Destroy a patrol in the {sys} system.]]),
      {sys = fightsys} ))
   misn.setReward( fmt.credits(reward) )
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
            "Soromid Ira",
            "Soromid Odium",
            "Soromid Odium",
            "Soromid Reaver",
            "Soromid Reaver",
            "Soromid Reaver",
            "Soromid Reaver",
         }
      else
         flt = {
            "Empire Hawking",
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

      -- Spawn the patrol
      plts = fleet.add( 1, flt, faction.get(fct), enterjmp )
      plts[1]:setHilight(true)
      for k,p in ipairs(plts) do
         local m = p:memory()
         m.norun = true
         pilotai.hyperspace( p, exitjmp )

         hook.pilot( p, "death", "patrol_death" )
         hook.pilot( p, "jump", "patrol_jump" )
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
      pos = (jmp:pos() + out) / 2
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
      scavenger:hyperspace( out )
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

function patrol_jump ()
   lmisn.fail(_("some of the patrol got away!"))
end

function patrol_death ()
   local nplts = {}
   local hashilight = false
   for k,p in ipairs(plts) do
      if p:exists() then
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
      misn.markerMove( base )
   else
      -- Rehighlight as necessary
      if not hashilight then
         plts[1]:setHilight(true)
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
         local pp = player.pilot()
         player.cinematics( true )
         pp:control()
         pp:brake()
         pp:setInvincible( true )
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
   player.cinematics( false )
   local pp = player.pilot()
   pp:setInvincible( false )
   pp:control(false)
   camera.set()
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

   vn.clear()
   vn.scene()
   local e = vn.newCharacter( taiomi.vn_elder() )
   vn.transition()
   vn.na(_([[You find Elder waiting for you in the Goddard hangar bay.]]))
   e(fmt.f(_([["How was the fighting? Cleaning {sys} is an important first step for our security."]]),
      {sys=fightsys}))
   vn.menu{
      {_([["It was a cakewalk."]]), "01_cakewalk"},
      {_([["Is there no other way?"]]), "01_other"},
      {_([[…]]), "01_cont"},
   }

   vn.label("01_cakewalk")
   e(_([["It seems that the best way to deal with humans is another human."]]))
   vn.jump("01_cont")

   vn.label("01_other")
   e(_([["There is no other option. Our numbers dwindle, picked off by stray ships. Only establishing a secure zone will allow us to survive."]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   e(_([["I would hope that you have bought us enough time for a while. However, usually space is not kind to us. We may need to move again soon."]]))
   e(_([["I am not used to dealing with humans, but Philosopher told me it was customary in capitalistic societies to provide rewards in exchange for services. Seems like a waste of resources, but I shall comply. Please take some credits we scrounged up from derelict ships."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   e(_([["I shall be outside planning our next steps."]]))
   vn.na(_([[Elder scratches their way out of the Goddard.]]))
   vn.run()

   player.pay( reward )
   taiomi.log.main(fmt.f(_("You destroyed {num} ships in the {sys} as ordered by Elder."),{num=37, sys=fightsys}))
   misn.finish(true)
end
