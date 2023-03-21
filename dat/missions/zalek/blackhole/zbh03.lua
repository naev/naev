--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 3">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 2</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 03

   Ward off enemy attack from evil PI
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"
local lmisn = require "lmisn"


local reward = zbh.rewards.zbh03

local mainpnt, mainsys = spob.getS("Research Post Sigma-13")
local jumpsys = system.get("NGC-23")

function create ()
   if not misn.claim( mainsys ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You find Zach at the bar, he seems to be staring dreamily at a photograph. When he notices your presence he quickly puts it away and sobers up.]]))
   z(_([["Damn, you nearly gave me a heart attack. I think I must be spending too much time alone on this station. By the way, how do you like it? I wrote a custom cleaning routine for all the droids and it seems to be turning out much better than expected. Almost has a homey touch to it."
He runs his hand on the counter top.]]))
   vn.menu{
      {_([["It's looking great!"]]), "01great"},
      {_([["Could use more work."]]), "01bad"},
      {_([["Looks awful."]]), "01bad"},
      {_([["…"]]), "01none"},
   }

   vn.label("01great")
   z(_([["I know! Who would have thought that a standard research post model could have so much character? Some of the decorations could use some work, but that's just a matter of time."]]))
   vn.jump("01cont")

   vn.label("01bad")
   z(_([["I don't think it's that bad given the circumstances. If I had followed Za'lek standard procedure, we would have had to completely rebuild the station. That's not something that can be done without a proper fleet of constructor drones."]]))
   vn.jump("01cont")

   vn.label("01none")
   z(_([["So great that you're at a loss of words? My skills have that effect on people sometimes."
He chuckles slightly.]]))
   vn.jump("01cont")

   vn.label("01cont")
   z(fmt.f(_([[His expression becomes more serious.
"I have been looking at recovering the station logs. Lots of the data has been lost, but I'm hoping I'll be able to recover most of it, although it's going to take quite a long time. In the meanwhile, it seems like the logs were referring to some anomaly in the system. This could be related to the unidentified object you saw when bringing supplies back. Due to a lack of better options at the moment, could you do a patrol of {sys}? This might give us a hint on what is going on here."]]),{sys=mainsys}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(fmt.f(_([["Great, just doing a quick circle of {sys} should suffice for now. Make sure to keep your eyes out for anything strange. If you encounter the unidentified object, try to approach it, but be cautious. We don't know what to expect, and seeing all the damage to the station, it might be best to assume the worst."]]),{sys=mainsys}))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Black Hole Scouting") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Patrol the {sys} system and report your observations to Zach at {pnt}."), {pnt=mainpnt, sys=mainsys}) )

   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 1

   misn.osdCreate( _("Black Hole Scouting"), {
      fmt.f(_("Scout around ({sys} system)"), {sys=mainsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=mainpnt, sys=mainsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

local heartbeat_state = 0
function land ()
   if mem.state~=2 or spob.cur() ~= mainpnt then
      if heartbeat_state > 0 then
         lmisn.fail(_("You were supposed to eliminate the hostiles!"))
      end
      if mem.hook_heartbeat then
         hook.rm( mem.hook_heartbeat )
         mem.hook_heartbeat = nil
      end
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[Zach is waiting for you at the docks as your ship lands. He immediately comes over to you.]]))
   z(_([["Are you OK? What the hell were those guys? They were clearly Za'lek vessels, but who could be in charge of them? The research here was approved by Central Station, it makes no sense… They also knew I was here, this just makes it all the more confusing. What could they be after? More questions and more questions, but we aren't getting any answers."
He rubs his temples.]]))
   z(_([["I'm going to have to rethink everything from the beginning, and try to make some sense out of this. Meet me up in the spaceport, I should hopefully have some idea on our next steps."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   faction.modPlayer("Za'lek", zbh.fctmod.zbh03)
   player.pay( reward )
   zbh.log(fmt.f(_("During a patrol of the {sys} system, you were attacked by some unidentified Za'lek forces. You were able to destroy them, but have not yet found out what is going on at {pnt}."),{sys=mainsys,pnt=mainpnt}))
   misn.finish(true)
end

-- Set up the enemies
function enter ()
   if system.cur() ~= mainsys then
      if mem.state==1 and heartbeat_state > 0 then
         lmisn.fail(_("You were supposed to eliminate the hostiles!"))
      end
      return
   end

   mem.hook_heartbeat = hook.timer( 90, "heartbeat" )
end

local badguys
function heartbeat ()
   heartbeat_state = heartbeat_state + 1
   player.autonavReset( 6 )
   if heartbeat_state == 1 then
      pilot.comm( _("Sigma-13"), fmt.f(_("Zach: I've detected some incoming ships from {sys}!"), {sys=jumpsys}) )

      local fbadguys = faction.dynAdd( "Za'lek", "zbh_baddies", _("Za'lek"), {ai="baddiepos"} )
      local ships = {"Za'lek Sting", "Za'lek Light Drone", "Za'lek Light Drone"}
      if player.pilot():ship():size() >= 5 then
         table.insert( ships, 1, "Za'lek Demon" )
      end
      local jp = jump.get( system.cur(), jumpsys )
      badguys = fleet.add( 1, ships, fbadguys, jp )
      for k,p in ipairs(badguys) do
         local m = p:memory()
         m.guardpos = mainpnt:pos()
      end
      local l = badguys[1]
      l:setVisplayer(true)
      l:setHilight(true)

   elseif heartbeat_state == 2 then
      pilot.comm( _("Sigma-13"), fmt.f(_("Zach: It looks like they are coming to {pnt}!"), {pnt=mainpnt}) )

   elseif heartbeat_state == 3 then
      pilot.broadcast( _("Sigma-13"), fmt.f(_("This is {pnt}. Please state your purpose."), {pnt=mainpnt}) )

   elseif heartbeat_state == 4 then
      badguys[1]:broadcast( _("You should have left when you had a chance Zach!"), true )

   elseif heartbeat_state == 5 then
      pilot.comm( _("Sigma-13"), _("Zach: It looks like they're hostile, engage the enemy!") )
      for k,p in ipairs(badguys) do
         p:setHostile(true)
      end

      misn.osdCreate( _("Black Hole Scouting"), {
         fmt.f(_("Eliminate hostiles in {sys}"), {sys=mainsys}),
         fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=mainpnt, sys=mainsys}),
      } )
   else
      local alive = false
      for k,p in ipairs(badguys) do
         if p:exists() then
            alive = true
            break
         end
      end

      if not alive then
         -- All gone
         pilot.comm( _("Sigma-13"), fmt.f(_("Zach: It looks like the coast is clear. Come back to {pnt}."),{pnt=mainpnt} ))
         misn.osdActive(2)
         mem.state = 2
         misn.markerMove( mem.mrk, mainpnt )
         return
      end
   end

   hook.timer( 6, "heartbeat" )
end
