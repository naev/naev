--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Alacrity">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Alacrity</system>
</event>
--]]
local textoverlay = require "textoverlay"
local chakra = require "luaspfx.chakra_explosion"
local srs = require "common.sirius"

local prevship
local markers
local reward = outfit.get("Feather Drive")
local obelisk = spob.get("Kal Maro Obelisk")

local track = {
   {
      vec2.new( 578, 354 ),
      vec2.new( -2, -86 ),
      vec2.new( -61, -102 ),
      vec2.new( 800, 319 ),
   },
   {
      vec2.new( 800, 319 ),
      vec2.new( 59, 98 ),
      vec2.new( 43, -121 ),
      vec2.new( 951, 496 ),
   },
   {
      vec2.new( 951, 496 ),
      vec2.new( -43, 121 ),
      vec2.new( 73, 23 ),
      vec2.new( 552, 696 ),
   },
   {
      vec2.new( 552, 696 ),
      vec2.new( -186, -90 ),
      vec2.new( 186, 38 ),
      vec2.new( 323, 448 ),
   },
   {
      vec2.new( 323, 448 ),
      vec2.new( -186, -38 ),
      vec2.new( -82, 19 ),
      vec2.new( 332, 246 ),
   },
}

local function lerp(a, b, t)
	return a + (b - a) * t
end

local function cubicBezier( t, p0, p1, p2, p3 )
	local l1 = lerp(p0, p1, t)
	local l2 = lerp(p1, p2, t)
	local l3 = lerp(p2, p3, t)
	local l4 = lerp(l1, l2, t)
	local l5 = lerp(l2, l3, t)
   return lerp(l4, l5, t)
end

local hook_done
function create ()
   srs.obeliskEnter( obelisk )

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Lesser", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   pp:weapsetSetInrange(nil,false)
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new(0,-200) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      accel_mod      = -30,
      speed_mod      = -30,
      turn_mod       = -30,
   }, true ) -- overwrite all
   pp:control(true)

   -- First puzzle
   local r = 0
   local origin = track[1][1]
   for k,trk in ipairs(track) do
      for t = 0,1,0.05 do
         local p = origin - cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] )
         r = math.max( r, p:polar() )
      end
   end
   local scale = (system.cur():radius()-500) / r
   local lp
   markers = {}
   for k,trk in ipairs(track) do
      for t = 0,1,0.005 do
         local p = (origin - cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] )) * scale
         local d = (lp and p:dist(lp)) or math.huge
         if d > 500 then
            local m = pilot.add("Psychic Orb", "Independent", p, nil, {ai="dummy"} )
            m:setInvincible(true)
            m:effectAdd("Psychic Orb On")
            table.insert( markers, m )
            lp = p
         end
      end
   end

   textoverlay.init( "#y".._("Test of Alacrity").."#0",
      "#y".._("Collect All the Orbs").."#0",
      { length=6} )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )

   hook.timer( 6, "start" )
end

local time_left = 60
local omsg_id
function start ()
   local pp = player.pilot()
   pp:control(false)

   omsg_id = player.omsgAdd( "", 0 )

   hook.timer( 0.1, "puzzle01" )
end

function puzzle01 ()
   time_left = time_left - 0.1
   if time_left < 0 then
      player.omsgRm( omsg_id )
      textoverlay.init( "#r".._("Test Failed").."#0", nil, {length=6})
      hook.timer( 6, "cleanup" )
      return
   end
   player.omsgChange( omsg_id, string.format("%.1f",time_left), 0 )

   local n = 0
   local ppos = player.pos()
   for k,m in ipairs(markers) do
      if m:pos():dist( ppos ) < 100 then
         n = k
         break
      end
   end
   -- None matched, keep waiting
   if n <= 0 then
      hook.timer( 0.1, "puzzle01" )
      return
   end

   -- Remove and keep waiting if not done
   local mm = markers[n]
   chakra( mm:pos(), vec2.new(), 100 )
   mm:rm()
   table.remove( markers, n )
   if #markers > 0 then
      hook.timer( 0.1, "puzzle01" )
      return
   end

   -- All done, so give ability
   player.omsgRm( omsg_id )
   srs.sfxGong()
   if player.outfitNum( reward ) > 0 then
      textoverlay.init( "#y".._("Test Completed").."#0", nil, {length=6})
   else
      player.outfitAdd( reward )
      textoverlay.init( "#y"..reward:name().."#0",
         "#y".._("New Flow Ability Unlocked").."#0",
         {length=6})
   end
   hook.timer( 6, "cleanup" )
end

local function cleanup_player ()
   -- Restore previous ship
   player.shipSwap( prevship, true, true )
end

function done ()
   cleanup_player()
   srs.obeliskExit()

   evt.finish()
end

function cleanup ()
   hook.rm( hook_done )
   srs.obeliskCleanup( cleanup_player, evt.finish )
end
