--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 8">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 7</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 08

   Have to bring back supplies while avoiding drones on the way back (or fighting through them)
]]--
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local lmisn = require  "lmisn"
local pilotai = require "pilotai"


local reward = zbh.rewards.zbh08
local cargo_name = _("Sensor Upgrades")
local cargo_amount = 32 -- Amount of cargo to take

local retpnt, retsys = spob.getS("Research Post Sigma-13")
local atksys = system.get( "NGC-23" )

function create ()
   if not misn.claim( {retsys, atksys} ) then
      misn.finish()
   end

   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 3, 8, "Za'lek", false, function( p )
      return p:tags().research
   end )
   if not mem.destpnt then
      misn.finish()
      return
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You see Zach a bit more relaxed after the successful surgery.]]))
   z(fmt.f(_([["So glad that Icarus seems to be out in the clear. Now we can start concentrating on the bigger picture again. We've been disrupted so many times, but as I've mentioned before, the notes indicate that there is something weird nearby. However, even though I've restored the station functionality partially, the sensor system is still a shadow of its former glory. I've been able to secure some decent supplies, but I would need you to go to {pnt} in the {sys} system and pick up {amount} of {cargo}. Would you be willing to do that for me for {creds}?"]]),
      {pnt=mem.destpnt, sys=mem.destsys, cargo=cargo_name, amount=fmt.tonnes(cargo_amount), creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   z(fmt.f(_([["Great! So the {cargo} should be already ready at {pnt}, all you have to do is head there and bring it back and I should be able to upgrade Sigma-13's sensors to scan nearby systems and up to the Anubis black hole. Just make sure you keep out an eye for trouble. Losing the cargo would be a pretty major setback. Best of luck!"]]),
      {cargo=cargo_name, pnt=mem.destpnt}))

   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   local title = _("Sigma-13 Sensors")
   misn.setTitle( title )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Pick up the necessary supplies at {pnt} ({sys} system) and bring them back to Zach at {retpnt} ({retsys} system)."),
      {pnt=mem.destpnt, sys=mem.destsys, retpnt=retpnt, retsys=retsys} ))

   mem.mrk = misn.markerAdd( mem.destpnt )
   mem.state = 1

   misn.osdCreate( title, {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==1 and spob.cur() == mem.destpnt then
      local fs = player.pilot():cargoFree()
      if fs < cargo_amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
            {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
         return
      end

      vntk.msg(_("Cargo Loaded"), fmt.f(_("Your ship touches ground and the loading drones identify your cargo and swiftly load the {cargo} onto your ship."),{cargo=cargo_name}))

      local c = commodity.new( N_("Sensor Upgrades"), N_("An assortment of different components that seem to be useful in upgrading deep-space sensorial systems.") )
      misn.cargoAdd( c, cargo_amount )
      misn.osdActive(2)
      mem.state = 2
      misn.markerMove( mem.mrk, retpnt )

   elseif mem.state==2 and spob.cur() == retpnt then

      vn.clear()
      vn.scene()
      local z = vn.newCharacter( zbh.vn_zach() )
      vn.transition( zbh.zach.transition )
      vn.na(fmt.f(_([[You land and find Zach has come to greet you at the space port. You explain your encounter in the {sys} system.]]),
         {sys=atksys}))
      z(_([["Damn, all effort just to blockade the station? I guess they must be preparing for another attack seeing the amount of forces deployed. They're probably taking it carefully after their previous attack failed. This is pretty worrisome. There's no way we're going to be able to defend against an onslaught like that. We're going to have to move fast. I'll start upgrading the sensors right away. Meet me up at the bar later and we can go to the next step."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      faction.modPlayer("Za'lek", zbh.fctmod.zbh08)
      player.pay( reward )
      zbh.log(fmt.f(_("You helped Zach get cargo important to upgrading the sensors of {pnt}, despite heavy enemy patrols on the way."),
         {pnt=retpnt}))
      misn.finish(true)
   end
end

function enter ()
   if mem.state==1 and system.cur() == atksys then
      local j1 = jump.get( atksys, retsys )
      local j2 = jump.get( atksys, "NGC-1001" )
      local p = pilot.add( "Za'lek Scout Drone", zbh.evilpi(), j1:pos()+(j2:pos()-j1:pos()):mul(0.8), nil, {ai="baddie"} )
      p:intrinsicSet( "ew_hide", -50 ) -- Easier to spot
      p:control(true)
      p:stealth()
      hook.pilot( p, "discovered", "scout_discovered" )

   elseif mem.state==2 and system.cur() == atksys then
      pilot.clear()
      pilot.toggleSpawn(false)

      local j1 = jump.get( atksys, retsys )
      local j2 = jump.get( atksys, "NGC-1001" )

      local route0 = {
         j1:pos(),
         j2:pos(),
      }
      local route1 = { -- Around j1
         vec2.new( -5e3, 13e3 ),
         vec2.new( 1e3, 9e3 ),
         vec2.new( 8e3, 13e3 ),
      }
      local route2 = {
         vec2.new( 13e3, 7e3 ),
         vec2.new( 0, -7e3 ),
      }
      local route3 = { -- Around j2
         vec2.new( 4e3, -9e3 ),
         vec2.new( 8e3, 2e3 ),
         vec2.new( 16e3, -5e3 ),
      }
      local route4 = {
         vec2.new( 11e3, 12e3 ),
         vec2.new( 0, 0 ),
         vec2.new( 18e3, -4e3 ),
      }

      local fevil = zbh.evilpi()

      -- Fuzzes a position a bit
      local function fuzz_pos( pos, max_offset )
         max_offset = max_offset or 100
         return vec2.newP(max_offset*rnd.rnd(), rnd.angle()) + pos
      end
      local function spawn_drone( shipname, pos )
         local p = pilot.add( shipname, fevil, fuzz_pos(pos) )
         -- We are nice and make the drones easier to see for this mission
         p:intrinsicSet( "ew_hide", -50 )
         return p
      end

      local function add_patrol_group( route, ships, start )
         start = start or rnd.rnd(1,#route)
         local pos = route[ start ]
         local l
         for k, s in ipairs( ships ) do
            local p = spawn_drone( s, pos )
            p:setHostile(true)
            if k==1 then
               l = p
               pilotai.patrol( p, route )
            else
               p:setLeader( l )
            end
         end
      end

      add_patrol_group( route0, { "Za'lek Mephisto", "Za'lek Demon", "Za'lek Demon" }, 1 )
      add_patrol_group( route1, { "Za'lek Sting", "Za'lek Heavy Drone", "Za'lek Heavy Drone" } )
      add_patrol_group( route2, { "Za'lek Heavy Drone", "Za'lek Heavy Drone", "Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone" } )
      add_patrol_group( route3, { "Za'lek Sting", "Za'lek Heavy Drone", "Za'lek Heavy Drone" } )
      add_patrol_group( route4, { "Za'lek Heavy Drone", "Za'lek Heavy Drone", "Za'lek Light Drone", "Za'lek Light Drone", "Za'lek Light Drone" } )

      player.autonavAbort("#r".._("Hostiles detected!").."#0")

   elseif system.cur() == retsys then
      local feral = zbh.plt_icarus( retpnt:pos() + vec2.newP(300,rnd.angle()) )
      feral:setFriendly(true)
      feral:setInvincible(true)
      feral:control(true)
      feral:follow( player.pilot() )
      hook.pilot( feral, "hail", "feral_hail" )

   end
end

function scout_discovered( scout )
   player.autonavReset(3)
   scout:taskClear()
   scout:hyperspace( system.get("NGC-1001") )
end

local sfx_spacewhale = {
   zbh.sfx.spacewhale1,
   zbh.sfx.spacewhale2
}
function feral_hail ()
   local sfx = sfx_spacewhale[ rnd.rnd(1,#sfx_spacewhale) ]
   sfx:play()
   player.commClose()
end
