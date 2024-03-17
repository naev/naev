--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 10">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 9</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 10

   Final showdown vs evil PI
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"
local love_shaders = require "love_shaders"
local lmisn = require "lmisn"

local reward = zbh.rewards.zbh10
local title = _("Sigma-13 Showdown")
local pi_shipname = _("Godheart")

local mainpnt, mainsys = spob.getS("Research Post Sigma-13")
local jumpsys = system.get("NGC-23")
local feraljumpsys = system.get("NGC-13674")

function create ()
   if not misn.claim( mainsys ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = true

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You find Zach completely absorbed in thought at the bar. He feels really distant despite being physically close.]]))
   z(_([[You can catch him muttering something under his breath. "Wet… what could that mean."]]))
   vn.na(_([[You clear your throat to catch his attention, and after your third try, he finally seems to recognize your presence.]]))
   z(_([["Hey, how's it going? Wait… didn't I have something for you? One second."
He fumbles with his cyberdeck looking for something.
"I've been looking over the scan analysis of the area and haven't really found anything, however, the signal is weak towards the Anubis Black Hole. It's a bit of a shot in the dark, but I think we should try to…"]]))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.na(fmt.f(_([[He gets suddenly cut off by the blaring siren.
"ALERT: Large hostile ships detected inbound from {sys}."]]),{sys=jumpsys}))
   z(fmt.f(_([["Shit! Not again. At least they didn't catch us with our pants down. I've prepared some defence drones, but I don't think they'll be much help. We must not let {pnt} fall! Try to defend the station and I'll see what I can do over here!"]]),{pnt=mainpnt}))
   vn.na(_([[Zach heads to the command centre, and you make your way to your ship while mentally preparing you for the challenge to come..]]))
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle(title)
   misn.setReward( fmt.f(_("The safety of {pnt}"), {pnt=mainpnt} ) )
   misn.setDesc(fmt.f(_("Defend {pnt} from hostiles inbound from {sys}."),{pnt=mainpnt,sys=jumpsys}))

   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 1

   misn.osdCreate( title, {
      fmt.f(_("Eliminate hostiles coming from {sys}!"), {sys=jumpsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=mainpnt, sys=mainsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state~=2 or spob.cur() ~= mainpnt then
      lmisn.fail(_("You were supposed to eliminate the hostiles!"))
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   local i = zbh.vn_icarus{pos="farleft"}
   vn.transition( zbh.zach.transition )
   vn.na(_([[You land amidst a swarm of feral bioships circling the station. Zach seems a bit distraught at the enormous behemoths visible from the docks.]]))
   z(_([["I had somehow gotten used to Icarus, but this is a bit too much."
He laughs nervously.
"I really hope they don't chew on the station."]]))
   vn.na(_([[You follow Zach inside so he calms down a bit.]]))
   z(_([[With the bioships out of sight he seems to relax a bit.
"That was some great flying out there! It all worked out much better than expected. Sure beats ending up as part of a hunk of molten metal which all my simulations predicted…"]]))
   z(_([["While I was preparing to evacuate with the notes left, I found something that might shine the light a bit on who they were. I believe that was Dr. Slorn and their laboratory, who have been working on ethereal material theory. My colleagues and I have always been pretty sceptical of their findings, and no other group has been able to replicate any experiments from what I know. In general, just pretty bad science if you ask me."]]))
   z(_([["However, Dr. Slorn's group has always been very good at getting funding. Suspiciously good to a point that some of us have doubted about their authenticity of the work. Even with all the ethics counsels and obligatory ethics courses, lots of shit gets through if you know what I mean."]]))
   z(_([["So, this is just my working hypothesis, but they must have found something when doing research that casts doubts on Dr. Slorn's research, and probably submitted it to the Journal on Advanced Physics, and I wouldn't be surprised if someone in Dr. Slorn's group caught a whiff of it and tracked them down. This all happens more than you would wish it did."]]))
   z(_([["What still is weird is how they went through all the effort to destroy the research station, instead of a more indirect approach."
He lets out a sigh.
"If it weren't for Icarus, we would have also joined my late colleagues as research martyrs…"]]))
   vn.sfx( zbh.sfx.spacewhale1 )
   vn.na(_([[Suddenly, you are surprised by a large commotion coming from the docks. You and Zach rush out to see what is going on.]]))
   z(_([["Oh shit. Are they nibbling on the station?"]]))
   vn.appear( i )
   i(_([[You enter the docks again and are met with an incredibly close Icarus. After the initial surprise, you can see they are carrying something in their mouth and offering it to you. They seem oddly proud of themselves.]]))
   z(_([["Is that… a drone?"]]))
   vn.na(_([[Icarus gently releases the drone and a cautious Zach approaches it, like one would approach a dead jellyfish on the beach.]]))
   z(_([["Wait, this isn't one of my drones… could this be?…"
Zach grabs the drone and runs into the station, leaving you alone face to face with Icarus.]]))
   vn.disappear( z, zbh.zach.transition )
   vn.sfx( zbh.sfx.spacewhale2 )
   i(_([[Icarus seems to nod at you before taking off to rejoin his tribe. They let out a strong electromagnetic impulse that resonates inside the space station as they head off to the depths of space.]]))
   vn.disappear( i )
   vn.na(_("You are left alone to reflect on all the recent happenings. When you have time you feel like you should probably check up on Zach to see how he is doing."))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   faction.modPlayer("Za'lek", zbh.fctmod.zbh10)
   player.pay( reward )
   zbh.log(fmt.f(_([[You defended {pnt} from a hostile attack with the help of Icarus and their kin.]]),{pnt=mainpnt}))
   misn.finish(true)
end

local function zach_say( msg )
   pilot.comm( _("Sigma-13"), _("Zach: ")..msg )
end

-- Set up the enemies
local heartbeat_state = 0
local badguys, fightstart
function enter ()
   if system.cur() ~= mainsys then
      if mem.state==1 and heartbeat_state > 0 then
         lmisn.fail(_("You were supposed to eliminate the hostiles!"))
      end
      return
   end

   local fbadguys = zbh.evilpi()
   local jp = jump.get( system.cur(), jumpsys )
   badguys = {}
   local function create_fleet( ships )
      local plts = fleet.add( 1, ships, fbadguys, jp, nil, {ai="baddie"} )
      for k,p in ipairs(plts) do
         p:setHostile(true)
         table.insert( badguys, p )
      end
      -- Make leader head towards planet
      local l = plts[1]
      l:changeAI("baddiepos")
      l:memory().guardpos = mainpnt:pos()
   end

   -- Fleets should have leaders with different speeds or they clump together
   local ppss = player.pilot():ship():size()
   if ppss >= 5 then
      create_fleet{"Za'lek Mephisto", "Za'lek Mephisto" }
      create_fleet{"Za'lek Demon", "Za'lek Demon", "Za'lek Heavy Drone"}
      create_fleet{"Za'lek Sting", "Za'lek Sting", "Za'lek Light Drone"}
      create_fleet{"Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone"}
   else
      create_fleet{"Za'lek Mephisto", "Za'lek Demon", "Za'lek Demon" }
      create_fleet{"Za'lek Demon", "Za'lek Heavy Drone"}
      create_fleet{"Za'lek Sting", "Za'lek Light Drone"}
      create_fleet{"Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone"}
   end

   -- Main boss gets hilighted
   local l = badguys[1]
   l:rename( pi_shipname )
   l:setVisplayer(true)
   l:setHilight(true)
   l:setNoDeath(true)
   hook.pilot( l, "death", "pi_death" )

   fightstart = naev.ticksGame()

   hook.timer( 15, "heartbeat" )
end

local drones_deployed = false
local feralpack
local defense_drones = {}
function heartbeat ()
   heartbeat_state = heartbeat_state + 1
   if heartbeat_state == 1 then
      vn.clear()
      vn.scene()
      local pi = vn.newCharacter( zbh.vn_pi{ pos="left", shader=love_shaders.hologram() } )
      local z = vn.newCharacter( zbh.vn_zach{ pos="right", shader=love_shaders.hologram() } )
      pi:rename(fmt.f(_("Captain of the {shipname}"),{shipname=pi_shipname}))
      vn.transition("electric")
      vn.na(fmt.f(_([[You suddenly get a read-only transmission from {pnt}. You accept and a hologram of Zach and an individual appears on your screen.]]),
         {pnt=mainpnt}))
      pi(_([["You should have stayed out of this when you had a chance Zach."]]))
      z(_([["What the hell?… wait… I know you… You're that professor working on the ethereal matter hypothesis…"]]))
      pi(_([["You won't be needing my name where you are going."]]))
      z(_([["Why the hell are you doing this? Did you kill them all? This is just a research post!"]]))
      pi(_([[He suddenly furrows his brows.
"You either are playing a fool or a lot more obtuse than I originally thought."]]))
      z(p_("Zach to Evil PI", [["What?"]]))
      pi(_([["Even if you don't know anything, it is too late now. Prepare to be eviscerated. For science!"]]))
      vn.disappear(pi, "electric")
      vn.na(_([[After the individual disappears, the transmission switches to read-write mode.]]))
      z(_([[Zach turns to you.
"Shit, we're pretty outgunned. He's flying a Mephisto… That's going to wreck you pretty bad if you get close to it. I have no idea how we're going to pull this off. At least try to buy me some time while I try to save what I can from the station!"]]))
      vn.done("electric")
      vn.run()

      misn.osdCreate( title, { _("Defend Sigma-13 and buy Zach some time"), } )

   else
      local alive = false
      for k,p in ipairs(badguys) do
         if p:exists() then
            alive = true
            break
         end
      end

      if alive then
         if not drones_deployed then
            -- Get nearest enemy
            local d = math.huge
            for k,p in ipairs(badguys) do
               if p:exists() then
                  d = math.min( d, p:pos():dist( mainpnt:pos() ) )
               end
            end

            -- Deploy drones if close to the planet
            if d < 3000 then
               player.autonavReset( 6 )
               zach_say( _("Deploying defence drones!") )
               local fgoodguys = zbh.fzach()
               local fbadguys = zbh.evilpi()
               fgoodguys:dynEnemy( fbadguys )
               defense_drones = fleet.add( 5, {"Za'lek Light Drone"}, fgoodguys, mainpnt, _("Defence Drone"), {ai="guard"} )
               for k,p in ipairs(defense_drones) do
                  p:setFriendly(true)
                  p:memory().guardpos = mainpnt:pos() + vec2.newP(200*rnd.rnd(), rnd.angle() )
               end
               drones_deployed = true
            end
         end

         local bl = badguys[1]
         local ba = (bl:exists() and bl:health()) or -1
         if not feralpack and (ba < 90 or bl:pos():dist( mainpnt:pos() ) < 3000 or naev.ticksGame()-fightstart > 300) then
            local fferals = zbh.feralbioship()
            local fbadguys = zbh.evilpi()
            fferals:dynEnemy( fbadguys )
            bl:setNoDeath(false)

            zach_say( _("I'm detecting incoming ships… Wait, are those Icarus' kin?") )

            local ships = { "Kauweke", "Taitamariki", "Taitamariki" }
            local jp = jump.get( system.cur(), feraljumpsys )
            feralpack = fleet.add( 2, ships, fferals, jp )
            for k,p in ipairs(feralpack) do
               p:rename(_("Feral Bioship"))
               p:setNoDeath()
               p:setFriendly(true)
               hook.pilot( p, "hail", "feral_hail" )
            end
            local l = feralpack[1]
            l:changeAI("guard")
            l:memory().guardpos = mainpnt:pos() + vec2.newP(200+200*rnd.rnd(), rnd.angle() )

            local icarus = zbh.plt_icarus( jp )
            icarus:setVisplayer(true)
            icarus:setInvincible(true)
            icarus:setFriendly(true)
            icarus:setHilight(true)
            icarus:control(true)
            icarus:follow( l )
            table.insert( feralpack, icarus )

            hook.timer( 0.3, "feral_check" )
         end

      else
         local pp = player.pilot()
         for k,p in ipairs(feralpack) do
            if p:exists() then
               p:setInvincible(true)
               p:setInvisible(false)
               p:control(true, true)
               p:moveto( mainpnt:pos() + vec2.newP( 200+200*rnd.rnd(), rnd.angle() ) )
               p:face( pp )
            end
         end

         for k,d in ipairs(defense_drones) do
            if d:exists() then
               d:control(true)
               d:land( mainpnt )
            end
         end

         -- All gone
         player.autonavReset( 6 )
         zach_say( fmt.f(_("Looks like the job is done. Come back to {pnt}."), {pnt=mainpnt} ) )
         misn.osdCreate( title, {
            fmt.f(_("Return to {pnt}"),{pnt=mainpnt}),
         } )
         mem.state = 2
         misn.markerMove( mem.mrk, mainpnt )
         return
      end
   end

   hook.timer( 10, "heartbeat" )
end

local sfx_spacewhale = {
   zbh.sfx.spacewhale1,
   zbh.sfx.spacewhale2,
}
function feral_hail ()
   local sfx = sfx_spacewhale[ rnd.rnd(1,#sfx_spacewhale) ]
   sfx:play()
   player.commClose()
end

function feral_check ()
   if mem.state==2 then return end
   local jp = jump.get( system.cur(), feraljumpsys )
   for k,p in ipairs(feralpack) do
      if not p:flags("invincible") then
         local pa = p:health()
         if pa < 70 then
            p:setInvincible(true)
            p:setInvisible(true)
            p:control(true)
            p:moveto( jp:pos() + vec2.newP( 300*rnd.rnd(), rnd.angle() ) )
         end
      end
   end
   hook.timer( 0.3, "feral_check" )
end

function pi_death( p )
   p:broadcast(_("Aaaargggh…"))
end
