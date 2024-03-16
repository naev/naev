--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Onion Society 02 Trigger">
 <unique />
 <priority>3</priority>
 <location>enter</location>
 <chance>6</chance>
 <cond>
   if not player.misnDone("Onion Society 01") then
      return false
   end
   local m = "Onion Society 02"
   if player.misnDone(m) or
      player.misnActive(m) then
      return false
   end
   local _spb, sys = spob.getS("Ulios")
   if sys:jumpDist() &gt; 15 or sys:jumpDist() &lt; 3 then
      return false
   end
   local sf = system.cur():faction()
   if not inlist( {
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Za'lek"),
   }, sf ) then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
  <done_misn name="Onion Society 01" />
 </notes>
</event>
--]]
--[[
   Comm Event for the Baron mission string
--]]
local vn = require "vn"
local fmt = require "format"
local onion = require "common.onion"

local dstspb1, dstsys1 = spob.getS("Ulios")
local dstspb2, dstsys2 = spob.getS("The Frontier Council")

function create ()
   -- If we finished the associated mission, clean up event too
   if player.misnDone("Onion Society 02") then
      return evt.finish(true)
   end

   -- Inclusive claims, so not an issue they overlap with the mission itself
   if not evt.claim( system.cur(), true ) then
      return evt.finish()
   end

   hook.timer( 10 + 20*rnd.rnd(), "trigger" )
   hook.land("finish")
   hook.jumpout("finish")
end

-- Possesses a ship to get close to the player
local possessed
function trigger ()
   local plt = pilot.get( { faction.get("Independent") } )
   if #plt <= 0 then
      -- No valid pilots, so we failed
      return evt.finish()
   end
   local pos = player.pos()
   table.sort( plt, function (a, b)
      return pos:dist2(a:pos())<pos:dist2(b:pos())
   end )
   possessed = plt[1]

   possessed:control()
   possessed:follow( player.pilot() )
   possessed:effectAdd("Onionized")
   hook.pilot( possessed, "death", "trigger" ) -- Retrigger if dies
   hook.pilot( possessed, "hail", "hail" )
   hook.timer( 1, "heartbeat" )
end

-- Triggers when gets close to player
function heartbeat ()
   if not possessed or not possessed:exists() then
      return
   end

   -- Be nice to the player and hail them when close enough
   if possessed:pos():dist( player.pos() ) < 3e3 then
      possessed:hailPlayer()
      return -- No need for more heartbeat
   end

   hook.timer( 1, "heartbeat" )
end

-- Hailed by player
function hail()
   local accepted = false

   vn.clear()
   vn.scene()
   local o = vn.newCharacter( onion.vn_onion() )
   vn.music( onion.loops.circus )
   vn.transition("electric")
   if var.peek("onion02_hailed") then
      vn.na(_([[You communicate with the strange ship, however, instead of the channel opening with the bridge a familiar hologram appears.]]))
      o(fmt.f(_([["Hey, if it isn't {player}! Had a fun time on {spb}? You should have seen the look on everyone's faces!"]]),
         {player=player.name(), spb=spob.get("Gordon's Exchange")}))
      o(_([["Hacking the mission computer was less effective than I thought at getting someone to help in my endeavours, so I've decided to cut the middleman."]]))
      o(fmt.f(_([["It's an easy no frills job. I need a package from {spb1} in the {sys1} system delivered to {spb2} in the {sys2} system. Capiche? Easy job for some easy credits. You in for the ride?"]]),
         {spb1=dstspb1, sys1=dstsys1, spb2=dstspb2, sys2=dstsys2}))
      vn.func( function () var.push("onion02_hailed",true) end )
   else
      vn.na(_([[You hail the hacked ship, and the familiar onion character hologram appears on-screen.]]))
      o(fmt.f(_([["Hey {player}, I still need that package delivered from {spb1} in the {sys1} system to {spb2} in the {sys2} system. Easy job for some easy credits. You ready to mambo?"]]),
         {player=player.name(), spb1=dstspb1, sys1=dstsys1, spb2=dstspb2, sys2=dstsys2}))
   end
   vn.menu{
      {_([[Accept]]), "01_accept"},
      {_([[Decline]]), "01_decline"},
   }

   vn.label("01_accept")
   vn.func( function () accepted = true end )
   o(fmt.f(_([["Bravo, you're now officially my errand person. The payload is waiting at {spb1}. Try not to get into trouble on the way."]]),
      {spb1=dstspb1}))
   vn.music()
   vn.scene()
   vn.transition("electric")
   vn.na(fmt.f(_([[The hologram fades away, and you are left with a confused pilot for a split second before the communication channel closes. Time to head to the {sys1} system.]]),
      {sys1=dstsys1}))
   vn.done()

   vn.label("01_decline")
   vn.na(_([[You promptly decline, it's best not to get involved with hackers right now. However, you have a feeling that something good could come out of this in the end.]]))
   vn.music()
   vn.scene()
   vn.transition("electric")
   vn.na(_([[The hologram fades away, and you are left with a confused pilot for a split second before the communication channel closes.]]))
   vn.run()

   -- Reset the NPC
   possessed:effectRm("Onionized")
   possessed:control(false)

   -- Clean up and start mission
   player.commClose()
   if accepted then
      naev.missionStart("Onion Society 02")
      return evt.finish(false) -- Can't set to true in case the mission gets failed or whatever, so it can be started again
   else
      -- TODO add timer so the mission doesn't appear for 20 periods or so
      return evt.finish(false)
   end
end

function finish ()
   return evt.finish(false)
end
