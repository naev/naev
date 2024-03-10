--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 6">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 5</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 06

   Ward off bigger enemy attack from evil PI
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"
local lmisn = require "lmisn"


local reward = zbh.rewards.zbh06

local mainpnt, mainsys = spob.getS("Research Post Sigma-13")
local jumpsys = system.get("NGC-23")

function create ()
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
   vn.na(_([[You find Zach analyzing what seems to be a blueprint of Icarus, likely preparing for surgery.]]))
   z(_([["It looks like deep down inside, bioships are not that different from your standard ship. I've looked carefully over Icarus, and it seems like they have what you could call a 'fracture' on the left module, that has healed poorly. I'll have to cut through most of the armoured flesh to reach it and do some welding to set the injury. Other than that, it seems like bioships are really resilient, as most old wounds are just scars now."]]))
   z(_([["What's curious is that most of the scars seem to be from Za'lek weaponry. The fracture is also likely the result of a ferromagnetohydraulic explosion, likely a hunter drone. However, I can not fathom who would want to kill a bioship instead of take the opportunity to study and examine it. It's like a crime against science itself!"]]))
   z(_([["The main thing I'm not clear about is how to get Icarus to understand the procedure and getting it prepped up. Since I don't have a brain mapping, I can't manipulate the pain receptors either, however, I do have a rough idea that might work."]]))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.na(fmt.f(_([[Suddenly a blazing alarm breaks Zach's musings.
"ALERT: Hostile ships detected inbound from {sys}."]]),{sys=jumpsys}))
   z(_([["Shit! I guess we have to postpone the surgery. Wait, is Icarus still out there?"
Zach pulls up a hologram of Icarus who can be seen flying outside the station.]]))
   z(_([["Damn, I don't think I can get Icarus in the station in time. Go out and intercept the hostiles, I'll see what I can do at the station!"]]))
   vn.na(_([[Zach rushes out the bar and you follow in pursuit headed towards your ship.]]))
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Defending Sigma-13") )
   misn.setReward( fmt.f(_("The safety of {pnt}"), {pnt=mainpnt} ) )
   misn.setDesc(fmt.f(_("Defend {pnt} from hostiles inbound from {sys}."),{pnt=mainpnt,sys=jumpsys}))

   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 1

   misn.osdCreate( _("Defending Sigma-13"), {
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
   vn.na(_([[You land amidst an Icarus spinning around joyfully around the station.]]))
   z(_([["That was great flying out there! They didn't stand a chance! Icarus is also in one piece, so all is well. Not the best way to prepare for surgery, huh?"]]))
   z(_([["What's really puzzling is that someone or something is really bent on covering up what happened here. Other than Icarus, we haven't really been able to find anything that I could imagine would be worth killing an entire Za'lek expedition for. Furthermore, we are constantly being attacked by Za'lek vessels, which makes even less sense. In-fighting should be left to the Dvaereds!"]]))
   z(_([["I'm going to start preparations for the surgery. Once Icarus is in tiptop shape, we can try to get to the bottom of the mysteries here. Meet me up at the spaceport bar when you are ready."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   faction.modPlayer("Za'lek", zbh.fctmod.zbh06)
   player.pay( reward )
   zbh.log(fmt.f(_([[You defended {pnt} and Icarus from a hostile attack.]]),{pnt=mainpnt}))
   misn.finish(true)
end

-- Set up the enemies
local heartbeat_state = 0
local badguys, icarus
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
   local _fcp, fleet_used = player.fleetCapacity()
   if fleet_used > 120 then
      create_fleet{"Za'lek Demon"}
      create_fleet{"Za'lek Sting", "Za'lek Bomber Drone", "Za'lek Light Drone" }
      create_fleet{"Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone"}
   else
      create_fleet{"Za'lek Sting", "Za'lek Heavy Drone"}
      create_fleet{"Za'lek Heavy Drone", "Za'lek Light Drone"}
      create_fleet{"Za'lek Light Drone", "Za'lek Light Drone"}
   end

   -- Main boss gets hilighted
   local l = badguys[1]
   l:setVisplayer(true)
   l:setHilight(true)

   icarus = zbh.plt_icarus( mainpnt:pos() + vec2.newP(300,rnd.angle()) )
   icarus:faction():dynEnemy( fbadguys )
   icarus:setFriendly(true)
   icarus:control(true)
   icarus:moveto( mainpnt:pos() + vec2.newP( 1000*rnd.rnd(), rnd.angle() ) )
   icarus:setHilight(true)
   icarus:memory().comm_no = _("No response")
   hook.pilot( icarus, "idle", "icarus_idle" )
   hook.pilot( icarus, "attacked", "icarus_attacked" )
   hook.pilot( icarus, "death", "icarus_death" )

   hook.timer( 7, "heartbeat" )
end

function icarus_idle ()
   icarus:moveto( mainpnt:pos() + vec2.newP( 1000*rnd.rnd(), rnd.angle() ) )
end

local atk_timer = 0
function icarus_attacked ()
   local t = naev.ticks()
   if t-atk_timer > 10 then
      pilot.comm( _("Sigma-13"), _("Zach: Icarus is under attack! Protect Icarus!") )
      atk_timer = t
   end
   icarus:control(false)
end

function icarus_death ()
   lmisn.fail(_("Icarus has been destroyed!"))
end

function heartbeat ()
   heartbeat_state = heartbeat_state + 1
   player.autonavReset( 6 )
   if heartbeat_state == 1 then
      pilot.comm( _("Sigma-13"), fmt.f(_("Zach: Hostiles are coming in hot from {sys}!"), {sys=jumpsys}) )

   elseif heartbeat_state == 2 then
      pilot.comm( _("Sigma-13"), _("Zach: Icarus is still out there! Protect Icarus!") )

   else
      local alive = false
      for k,p in ipairs(badguys) do
         if p:exists() then
            alive = true
            break
         end
      end

      if not alive and icarus:exists() then
         -- All gone
         pilot.comm( _("Sigma-13"), fmt.f(_("Zach: It looks like the coast is clear. Come back to {pnt}."),{pnt=mainpnt} ))
         misn.osdActive(2)
         mem.state = 2
         misn.markerMove( mem.mrk, mainpnt )

         icarus:control(true)
         icarus:taskClear()
         icarus:setInvincible(true)
         icarus:follow( player.pilot() )
         return
      end
   end

   hook.timer( 10, "heartbeat" )
end
