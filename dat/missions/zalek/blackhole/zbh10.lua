--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 10">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <chance>100</chance>
  <spob>Research Post Sigma-13</spob>
  <location>Bar</location>
  <done>Za'lek Black Hole 9</done>
 </avail>
 <tags>
  <tag>zlk_cap_ch01_lrg</tag>
 </tags>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 10

   FInal showdown vs evil PI
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"
local love_shaders = require "love_shaders"
local lmisn = require "lmisn"

-- luacheck: globals land enter heartbeat feral_hail feral_check (Hook functions passed by name)

local reward = zbh.rewards.zbh10
local title = _("Sigma-13 Showdown")

local mainpnt, mainsys = spob.getS("Research Post Sigma-13")
local jumpsys = system.get("NGC-23")
local feraljumpsys = system.get("NGC-13674")

function create ()
   misn.finish()
   if not misn.claim( mainsys ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = true -- autoaccepts

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[TODO]]))
   z(_([["TODO"]]))
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
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[TODO]]))
   z(_([["TODO"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   faction.modPlayer("Za'lek", zbh.fctmod.zbh10)
   player.pay( reward )
   zbh.log(fmt.f(_([[You defended {pnt} from a hostile attack. TODO]]),{pnt=mainpnt}))
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
   if player.pilot():ship():size() >= 5 then
      create_fleet{"Za'lek Mephisto", "Za'lek Diablo", "Za'lek Demon", "Za'lek Demon" }
      create_fleet{"Za'lek Demon", "Za'lek Demon", "Za'lek Heavy Drone", "Za'lek Heavy Drone"}
      create_fleet{"Za'lek Sting", "Za'lek Sting", "Za'lek Light Drone", "Za'lek Light Drone"}
      create_fleet{"Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone"}
   else
      create_fleet{"Za'lek Mephisto", "Za'lek Demon", "Za'lek Demon" }
      create_fleet{"Za'lek Demon", "Za'lek Heavy Drone"}
      create_fleet{"Za'lek Sting", "Za'lek Light Drone"}
      create_fleet{"Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone"}
   end

   -- Main boss gets hilighted
   local l = badguys[1]
   l:setVisplayer(true)
   l:setHilight(true)

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
      vn.transition("electric")
      vn.na(_([[TODO]]))
      pi(_([["TODO"]]))
      z(_([["TODO"]]))
      vn.done("electric")
      vn.run()

      -- TODO have Zach deploy friendly drones

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
               zach_say( _("Deploying defense drones!") )
               local fgoodguys = zbh.fzach()
               local fbadguys = zbh.evilpi()
               fgoodguys:dynEnemy( fbadguys )
               defense_drones = fleet.add( 5, {"Za'lek Light Drone"}, fgoodguys, mainpnt, _("Defense Drone"), {ai="guard"} )
               for k,p in ipairs(defense_drones) do
                  p:setFriendly(true)
                  p:memory().guardpos = mainpnt:pos() + vec2.newP(200*rnd.rnd(), rnd.angle() )
               end
               drones_deployed = true
            end
         end

         local bl = badguys[1]
         local ba = bl:health()
         if not feralpack and (ba < 90 or bl:pos():dist( mainpnt:pos() ) < 3000 or naev.ticksGame()-fightstart > 300) then
            local fferals = zbh.feralbioship()
            local fbadguys = zbh.evilpi()
            fferals:dynEnemy( fbadguys )

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
            icarus:control(true)
            icarus:follow( l )
            table.insert( feralpack, icarus )

            hook.timer( 0.3, "feral_check" )
         end

      else
         for k,p in ipairs(feralpack) do
            if p:exists() then
               p:setInvincible(true)
               p:setInvisible(false)
               p:control(true, true)
               p:moveto( mainpnt:pos() + vec2.newP( 200+200*rnd.rnd(), rnd.angle() ) )
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
         misn.osdActive(2)
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
