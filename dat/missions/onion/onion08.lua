--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 08">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <done>Onion Society 07</done>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion 08

   Player has to take down lonewolf4 in his modified zebra uber-carrier
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local trigger = require "trigger"
--local fleet = require "fleet"
local lmisn = require "lmisn"
local pilotai = require "pilotai"
local equipopt = require "equipopt"
local pulse = require "luaspfx.pulse"
local alert = require "luaspfx.alert"
local blink = require "luaspfx.blink"
local CTS = require "constants"
--local tut = require "common.tutorial"

-- Reference to honeypot (trap)
local title = _("The Lone Wolf")
--local reward = onion.rewards.misn08

local SYSTEM_START = system.get("Oxuram")
local SYSTEM_END = system.get("PSO")
local SPOB_EPILOGUE, _SYSTEM_EPLOGUE = spob.getS("PSO Monitor")

-- Mission states
local STATE_FIGHT1_START = 1
local STATE_FIGHT1_UNCOVERED = 2
local STATE_WOLF_RANAWAY = 2
local STATE_WOLF_DEFEATED = 3
mem.state = nil

function create()
   if not var.peek("testing") then return misn.finish() end -- Not ready yet

   -- Hard claims now
   if not misn.claim( {SYSTEM_START, SYSTEM_END} ) then return misn.finish(false) end

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )
   misn.setNPC( _("l337_b01"), prt.t.tex, _([[Follow up with l337_b01.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[TODO]])))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01()
   vn.newCharacter( l337 )
   vn.transition("electric")

   l337("...")
   vn.func( function() accepted = true end )

   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()

   mem.state = 0
   hook.enter("enter")
   hook.land("land")

   misn.markerAdd( SYSTEM_START )
   misn.osdCreate( title, {
      fmt.f(_("Confront lonewolf4 at the {sys} system!"),
      {sys=SYSTEM_START}),
   })
end

local function get_fct()
   return faction.dynAdd("Mercenary", _("lonewolf4"), _("lonewolf4") )
end

local function spawn_wolf( pos, hologram )
   local p = pilot.add("Zebra Wolfie", get_fct(), pos, _("lonewolf4"), {naked=true} )
   p:intrinsicSet( "shield", 4000 )
   p:intrinsicSet( "armour", 4000 )
   p:intrinsicSet( "fbay_capacity", 50 )
   p:intrinsicSet( "fbay_reload", 100 )
   p:intrinsicSet( "cpu_max", 1000 )
   equipopt.zalek( p, {
      max_same_weap = 2,
      rnd = 0,
      fighterbay = 1.5,
      pointdefence = 1.5,
      outfits_add = {
         "Guardian Overseer System",
         "Guardian Interception System",
      },
   } )
   if hologram then
      p:intrinsicSet( "weapon_damage", -10000 )
      p:intrinsicSet( "fbay_damage", -10000 )
   end
   p:memory()._hologram = hologram
   return p
end

function enter ()
   local scur = system.cur()
   if scur==SYSTEM_START and mem.state==0 then
      pilot.clear()
      pilot.toggleSpawn(false)
      hook.timer( 5, "fight1_start1" )

   elseif scur==SYSTEM_END and mem.state==STATE_WOLF_RANAWAY then
      pilot.clear()
      pilot.toggleSpawn(false)
      spawn_wolf( vec2.new() )

   elseif mem.state ~= 0 and mem.state~=STATE_WOLF_RANAWAY then
      return lmisn.fail("you abandoned the attack on lonewolf4!")

   end
end

function energy_surge_hook( pos )
   local fct = get_fct()
   local drones = {}
   for k,s in ipairs{
      "Za'lek Light Drone",
      "Za'lek Light Drone",
      "Za'lek Bomber Drone",
      "Za'lek Bomber Drone",
      "Za'lek Heavy Drone",
      "Za'lek Heavy Drone",
   } do
      local p = pos + vec2.newP( 300*math.sqrt(rnd.rnd()), rnd.angle() )
      local d = pilot.add( s, fct, p )
      d:effectAdd("Blink")
      d:setHostile(true)
      -- Slow them down so the player can "dodge"
      d:intrinsicSet("speed_mod", -50)
      d:intrinsicSet("accel_mod", -50)
      table.insert( drones, d )
   end
   hook.timer( 5, "energy_surge_end_hook", drones )
end
function energy_surge_end_hook( drones )
   for k,d in ipairs(drones) do
      if d:exists() then
         d:effectAdd("Fade-Out")
      end
   end
end
local function energy_surge( pos )
   player.msg(_("Energy surge detected!"))
   alert( pos, {
      size = 300,
   } )
   hook.timer( 3, "energy_surge_hook", pos )
end

local function _energy_surge_at_player ()
   local pp = player.pilot()
   local pos = pp:pos() + 3 * pp:vel()
   energy_surge( pos + vec2.newP( math.sqrt(rnd.rnd())*50, rnd.angle() ) )
end

--[[

Stage 1 fight:
1. Sends drones periodically while moving around - there are 2 holograms doing the same (but they are hologram drones)
2. When player damages the real one switches into combat mode.
3. WHen shield drops to 0, jumps out in a cloud of drones.

--]]
local real, fake1, fake2, bosses, discovered
function fight1_start1 ()
   player.msg(_([[l337_b01: ]]), true)
   mem.state = STATE_FIGHT1_START

   local rad = system.cur():radius()*0.8
   local ang = player.pos():angle()
   local off = rnd.permutation{ -math.pi*0.5, math.pi*0.5, math.pi }
   local pos = {}
   for k,o in ipairs(off) do
      pos[k] = vec2.newP( rad, ang+o )
   end

   -- Spawn them
   real  = spawn_wolf( pos[1], false )
   fake1 = spawn_wolf( pos[2], true )
   fake2 = spawn_wolf( pos[3], true )

   -- Generate waypoints
   local waypoints = {}
   local N = 16
   local reverse = 1
   if rnd.rnd() < 0.5 then reverse = -1 end
   for i=1,N do
      table.insert( waypoints, vec2.newP( rad, math.pi*2 * (i-1)/N * reverse ) )
   end

   -- Set them up
   local min_stealth = 3000/CTS.STEALTH_MIN_DIST*100
   bosses = { real, fake1, fake2 }
   for k,b in ipairs(bosses) do
      -- They should only be uncoverable at 3000 units
      b:intrinsicSet( "ew_stealth", -99 )
      b:intrinsicSet( "ew_stealth_min", min_stealth )

      hook.pilot( b, "discovered", "fight1_discovered" )
      hook.pilot( b, "attacked", "fight1_attacked" )
      hook.timer( 10, "fight1_launch", b )
   end
   pilotai.patrol( bosses, waypoints )

   hook.timer( 5, "fight1_start2")
end
function fight1_start2 ()
   pulse( player.pos() + vec2.newP( 100, rnd.angle() ), nil, {
      col = {0.1, 0.8, 0.3, 0.5},
   } )
   local pp = player.pilot()
   pp:intrinsicSet("ew_stealth", 300 )
   player.msg(_([[l337_b01: "We've been spotted! Looks like lonewolf4 wants a fight!"]]))
end
function fight1_launch( b )
   if not b:exists() then return end

   local hologram = b:memory()._hologram
   local fct = get_fct()
   for i=1,3 do
      local shp = "Za'lek Light Drone"
      if rnd.rnd() then
         shp = "Za'lek Heavy Drone"
      end
      local p = pilot.add( shp, fct, b:pos()+vec2.newP( 20*rnd.rnd(), rnd.angle() ) )
      if hologram then
         p:intrinsicSet( "ew_detect", 100 )
         p:intrinsicSet( "weapon_damage", -10000 )
         p:memory()._hologram = hologram
      end
   end

   hook.timer( 15+5*rnd.rnd(), "fight1_launch", b )
end
function fight1_discovered( plt )
   plt:setHilight()
   if discovered then return end
   player.msg(_([[l337_b01: "Looks like a heavily modified Zebra. Take it out!"]]), true)
   discovered = true
end
function fight1_attacked( plt )
   -- Only matters at the beginning
   if mem.state ~= STATE_FIGHT1_START then return end

   if plt:memory()._hologram then
      plt:effectAdd("Fade-Out")
      local b = {}
      for k,p in ipairs(bosses) do
         if p~=plt and p:exists() then
            table.insert(b)
         end
      end
      bosses = b
      if #bosses==2 then
         player.msg(_([[l337_b01: "A hologram!? The real ship must be out there still."]]), true)
      else
         player.msg(_([[l337_b01: "Another hologram!?"]]), true)
      end
   else
      mem.state = STATE_FIGHT1_UNCOVERED
      for k,p in ipairs(pilot.get()) do
         if p:memory()._hologram then
            p:effectAdd("Fade-Out")
         end
      end

      real:broadcast(_(""))

      hook.timer( 0.5, "fight1_timer" )
   end
end

function fight1_timer ()
   if not real:exists() or real:shield() < 1 then
      local pos = real:pos()
      blink( real, pos )
      real:rm()
      energy_surge( pos )

      hook.timer( 9, "fight1_end1" )
      trigger{
         { 9, _("l337_b01: What the hell was that?") },
         { 5, fmt.f(_("l337_b01: Wait, they're nearby in the {sys} system!"),
            {sys=SYSTEM_END}) },
         { 0, function ()
            misn.osdCreate( title, {
               fmt.f(_("Chase lonewolf4 to the {sys} system!"),
                  {sys=SYSTEM_END}),
            })
            misn.markerRm()
            misn.markerAdd( SYSTEM_END )
            mem.state = STATE_WOLF_RANAWAY
         end },
         { 5, fmt.f(_("l337_b01: No time to lose, onwards to {sys} system!"),
            {sys=SYSTEM_END}) },
      }
   else
      hook.timer( 0.5, "fight1_timer" )
   end
end

--[[

Stage 2 fight:
0. Shields are down permanently
1. Does an energy surge attack every so often
2. When starts to take damage, charges up for 10 seconds and jumps away

--]]

function land ()
   if mem.state==STATE_WOLF_DEFEATED and spob.cur()==SPOB_EPILOGUE then
      vn.clear()
      vn.scene()
      local l337 = onion.vn_l337b01()
      vn.newCharacter( l337 )
      vn.transition("electric")

      l337()

      vn.done("electric")
      vn.run()

      misn.finish(true)
   end
end
