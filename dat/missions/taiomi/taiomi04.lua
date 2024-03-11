--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 4">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 3</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 04

   Player has to do some resource collection / mining
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local equipopt = require "equipopt"
local pilotai = require "pilotai"
local luatk = require "luatk"

local reward = taiomi.rewards.taiomi04
local title = _("Therite for Scavenger")
local base, basesys = spob.getS("One-Wing Goddard")
local scenesys = system.get("Bastion")
-- Asks for Therite which easiest obtained from Haven nearby
-- Closer systems only have Vixilium
local minesys = system.get("Haven")
local resource = commodity.get("Therite")
local resource_bonus = resource:price()*1.5
local amount = 30

--[[
   0: mission started
   1: interaction with ships on the way there
   2: obtained materials
--]]
mem.state = 0

function create ()
   if not misn.claim(scenesys) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()
   mem.brought = 0

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to fly to obtain {resource} to help do an experiment. {resource} can be found in the nearby {sys} system."),
      {resource=resource, sys=minesys}))
   misn.setReward(reward)
   mem.marker = misn.markerAdd( minesys )

   update_osd()

   hook.enter( "enter" )
   hook.land( "land" )

   -- Conditions to update OSD
   hook.comm_buy( "update_osd" )
   hook.comm_sell( "update_osd" )
   hook.comm_jettison( "update_osd" )
   hook.custom( "mine_drill", "update_osd" )
   hook.takeoff( "update_osd" )
   hook.gather( "update_osd" )
end

function update_osd ()
   local have = mem.brought+player.fleetCargoOwned(resource);
   misn.osdCreate( title, {
      fmt.f(_("Obtain {amount} of {resource} (have {have})"),
         {resource=resource,
            amount=fmt.tonnes_short(amount-mem.brought),
            have=fmt.tonnes_short(player.fleetCargoOwned(resource))}),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
   } )
   -- Tell to go back
   if have >= amount then
      misn.osdActive(2)
      misn.markerMove( mem.marker, basesys )
   else
      misn.markerMove( mem.marker, minesys )
   end
end

function enter ()
   if mem.timer then
      hook.rm( mem.timer )
      mem.timer = nil
   end
   if mem.state > 1 then
      return
   end
   local scur = system.cur()

   if mem.state==0 and scur == scenesys then
      -- Small cutscene with the curious drones
      mem.timer = hook.timer( 3+rnd.rnd()*3, "scene00" )
   end

   if scur ~= minesys then
      return
   end

   -- Create small convoy that tries to leave
   local pos = vec2.new( 6000, 6000 ) -- Center of asteroid field
   local fct = faction.dynAdd( nil, "shady_miners", _("Shady Miners"), {ai="independent"} )
   local l = pilot.add( "Rhino", fct, pos, _("Shady Miner"), {naked=true} )
   equipopt.pirate( l )
   l:setHilight( l )
   l:intrinsicSet( "speed_mod", -30 ) -- Slower than normal and should be heavily loaded
   l:cargoAdd( resource, l:cargoFree() )
   for i=1,4 do
      local s = "Shark"
      if rnd.rnd() < 0.5 then
         s = "Vendetta"
      end
      local p = pilot.add( s, fct, pos, nil, {naked=true} )
      equipopt.pirate( p )
      p:setLeader( l )
   end

   -- Just try to get out of the system
   pilotai.hyperspace( l, jump.get( minesys, system.get("Gamel") ) )
end

local pilot_ya, pilot_yb
function scene00 ()
   local jmp = jump.get( scenesys, basesys )
   pilot_ya = pilot.add( "Drone", "Independent", jmp, taiomi.younga.name )
   pilot_yb = pilot.add( "Drone", "Independent", jmp, taiomi.youngb.name )
   for k,p in ipairs{pilot_ya, pilot_yb} do
      p:setFriendly(true)
      p:setInvincible(true)
      hook.pilot( p, "hail", "hail_youngling" )
   end
   hook.timer( 4, "scene01" )
end

function hail_youngling( p )
   p:comm( _("(You hear some sort of giggling over the comm. Is it laughing?)") )
   player.commClose()
end

function scene01 ()
   player.msg(fmt.f(_("It looks like {namea} and {nameb} followed you!"),
      {namea=taiomi.younga.name, nameb=taiomi.youngb.name}), true)
   player.autonavReset( 5 )
   hook.timer( 4, "scene02" )
end

function scene02 ()
   vn.clear()
   vn.scene()
   local ya = taiomi.vn_younga{ pos="farleft",  flip=true }
   local yb = taiomi.vn_youngb{ pos="farright", flip=false }
   vn.transition()
   vn.appear( {ya,yb}, "slidedown" )
   vn.na(_([[You open a transmission channel with the two curious drones following you.]]))
   ya(_([["Hello!"]]))
   yb(_([["You found us!"]]))
   vn.menu{
      {fmt.f(_([["You should stay in {basesys}."]]),{basesys=basesys}), "cont01"},
      {_([["It's dangerous out here!"]]), "cont01"},
      {_([["What are you doing out here?"]]), "cont01"},
      {_([[…]]), "cont01"},
   }
   vn.label("cont01")
   yb(fmt.f(_([["It's boring in {basesys}! They never let us out!"]]),
      {basesys=basesys}))
   ya(_([["We can help too! We always sneak out when nobody is watching!"]]))
   yb(fmt.f(_([["Sssh, you're not supposed to say that {namea}!"]]),
      {namea=taiomi.younga.name}))
   ya(_([["Oops, don't tell anyone!"]]))
   vn.menu{
      {_([["You should go back."]]), "cont02"},
      {_([["We can play later, OK?"]]), "cont02"},
      {_([["Fine, but make sure to stay hidden!"]]), "cont02_ok"},
      {_([[…]]), "cont02"},
   }

   vn.label("cont02")
   ya(_([["Fine, we will go back, but you have to promise to play with us later!"]]))
   yb(_([["Yes! Swear by your auxiliary capacitor that you'll play with us later!"]]))
   ya(_([["Humans don't have auxiliary capacitors do they?"]]))
   yb(_([["Sure they do, that's the bumpy thing they have under their eyes no?"]]))
   vn.na(fmt.f(_([[Chattering amongst theselves, the two young drones head back to {basesys}.]]),
      {basesys=basesys}))
   vn.func( function ()
      local jmp = jump.get( scenesys, basesys )
      pilotai.hyperspace( pilot_ya, jmp )
      pilotai.hyperspace( pilot_yb, jmp )
   end )
   vn.jump("done")

   vn.label("cont02_ok")
   yb(_([["Really!?!"]]))
   ya(_([["You're awesome!"]]))
   yb(_([["We'll stay in the shadows, like, what were they called. Ninjas!"]]))
   ya(_([["Ninjas! Ninjas!"]]))
   vn.na(_([[Chattering amongst themselves, they begin to hide themselves along the stars.]]))
   vn.func( function ()
      for k,p in ipairs{pilot_ya, pilot_yb} do
         -- Shouldn't be targetted by other enemies since they are invincible
         p:control()
         p:stealth()
      end
   end )
   vn.jump("done")

   vn.label("done")
   vn.disappear( {ya,yb}, "slidedown" )
   vn.done()
   vn.run()
   -- Move to the next stuff
   mem.state = 1
end

function land ()
   local c = spob.cur()
   if c ~= base then
      return
   end

   local have = player.fleetCargoOwned( resource )
   if have <= 0 then
      return
   elseif have > 0 and have+mem.brought < amount then
      -- Just allow dropping off a bit
      luatk.yesno(
         fmt.f(_("Drop off {resource}?"),{resource=resource}),
         fmt.f(_("Do you wish to drop off {have} of {resource}? You still need to get {necessary} for Scavenger."),
            {resource=resource, have=fmt.tonnes(have), necessary=fmt.tonnes(amount-mem.brought-have)}),
         function ()
            local q = player.fleetCargoRm( resource, have )
            mem.brought = mem.brought + q
            luatk.msg(_("Resources dropped off"), fmt.f(_("You store the {amount} of {resource} at the {base}."),{amount=fmt.tonnes(q), resource=resource, base=base}))
         end )
      luatk.run()
      return
   end

   -- Player should have the full amount now, we remove all of it
   local excess = have - (amount-mem.brought)
   player.fleetCargoRm( resource, have )
   reward = reward + excess * resource_bonus -- Give a nice bonus

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(fmt.f(_([[You bring the {resource} aboard the Goddard and find Scavenger waiting for you.]]),
      {resource=resource}))
   if excess > 0 then
      s(_([["I see you managed to bring more than all the needed resources. This will be plenty for us to start working on our project. I will be outside getting things set up. We may still need something else so make sure to check in after you get some rest."
Scavenger backs out of the Goddard and returns to space.]]))
   else
      s(_([["I see you managed to bring all the needed resources. This will be enough for us to start working on our project. I will be outside getting things set up. We may still need something else so make sure to check in after you get some rest."
Scavenger backs out of the Goddard and returns to space.]]))
   end
   vn.sfxVictory()
   vn.func( function ()
      player.pay( reward )
   end )
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   taiomi.log.main(_("You collected important resources for the inhabitants of Taiomi."))
   misn.finish(true)
end
