--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Qex Racing">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Reidd</spob>
</mission>
--]]
--[[

   Qex Racing

--]]
local fmt = require "format"
local spfxtrack = require "luaspfx.racetrack"
local vntk = require "vntk"

local elapsed_time, race_done

local col_next = {0, 1, 1, 0.3}
local col_past = {1, 0, 1, 0.2}

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

local track_list = {
   {
      name = _("Qex Tour"),
      laps = 1,
      track = {
         {
            vec2.new(  5e3,  2e3 ), -- start near Qex IV
            vec2.new( -1e3,  3e3 ),
            vec2.new(  1e3, -1e3 ),
            vec2.new(  2e3,  2e3 ),
         }, {
            vec2.new(  2e3,  2e3 ),
            vec2.new( -1e3,  1e3 ),
            vec2.new(  2e3,    0 ),
            vec2.new( -1e3,  5e3 ), -- passes by Qex V
         }, {
            vec2.new( -1e3,  5e3 ),
            vec2.new( -6e3,    0 ),
            vec2.new(  1e3,  1e3 ),
            vec2.new( -8e3,  2e3 ), -- circle Qex II
         }, {
            vec2.new( -8e3,  2e3 ),
            vec2.new( -1e3, -1e3 ),
            vec2.new( -1e3,  1e3 ),
            vec2.new( -8e3,  4e3 ),
         }, {
            vec2.new( -8e3,  4e3 ),
            vec2.new(  1e3, -1e3 ),
            vec2.new(    0,  2e3 ),
            vec2.new( -1e3,  2e3 ), -- Through asteroid field
         }, {
            vec2.new( -1e3,  2e3 ),
            vec2.new(    0, -4e3 ),
            vec2.new(  1e3, -3e3 ),
            vec2.new(  5e3,  2e3 ),
         }
      },
   },
}

function create ()
   if not var.peek("testing") then return end

   mem.race_spob = spob.cur()
   if not misn.claim( system.cur() ) then
      misn.finish(false)
   end
   misn.setNPC(_("A laid back person"), "neutral/unique/laidback.webp", _("You see a laid back person, who appears to be one of the locals, looking around the bar."))
end

function accept ()
   if not vntk.yesno(_("Race?"),_("Do you wanto to race?")) then
      return
   end

   misn.accept()
   misn.setDesc(_("You're participating in a race!"))
   misn.setReward(_("Endless Riches!"))

   hook.load( "loaded" ) -- don't load into the race

   -- TODO track selector
   mem.track = track_list[1].track
   hook.safe("start_race")
   player.takeoff() -- take off and start the race!
end

function loaded ()
   misn.finish(false)
end

local function display_time( time )
   return fmt.f(_("{1:02.0f}:{2:02.0f}.{3:.0f}"),{
      math.floor( time / 60 ),
      math.floor(math.fmod( time, 60 ) ),
      math.floor(math.fmod( time, 1 )*10),
   })
end

local omsg_timer
function start_race ()
   pilot.clear()
   pilot.toggleSpawn(false)
   -- TODO add spectators / cameras?

   local lp
   local elapsed = 0
   local gates_p = {}
   for k,trk in ipairs(mem.track) do
      for t = 0,1,0.005 do
         local p = cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] )
         local d = (lp and p:dist(lp)) or math.huge
         if d > 500 then
            table.insert( gates_p, p )
            lp = p
            if d < math.huge then
               elapsed = elapsed + d
            end
         end
      end
   end
   --print("Total Length: "..tonumber(elapsed))

   local gates = {}

   local function gate_activate( k )
      for i=0,4 do
         if k-i > 0 then
            local c = {col_past[1], col_past[2], col_past[3], col_past[4]*(1-i/4)}
            gates[k-i]:setCol( c )
         end
      end

      local ngates = #gates
      if k >= ngates then
         -- done here
         race_done = true
         omsg_timer = player.omsgAdd(display_time(elapsed_time), 5, 50)
         -- TODO sound effect?
         hook.timer( 5, "race_complete" )
         return
      end
      -- TODO sound effect?

      local gate_next = gates[ k+1 ]
      gate_next:setReady( true )
      gate_next:setCol( col_next )

      for i=1,6 do
         if k+i < ngates then
            local c = {col_next[1], col_next[2], col_next[3], col_next[4]*(1-i/6)}
            gates[k+i]:setCol( c )
         end
      end
   end
   local function gate_add( k, p, angle )
      local sg = spfxtrack( p, angle, function ()
         gate_activate(k)
      end )
      gates[k] = sg
      system.markerAdd( p )
   end

   for k,g in ipairs(gates_p) do
      local gprev = gates_p[k-1] or gates_p[#gates_p]
      local gnext = gates_p[k+1] or gates_p[1]
      local angle = -(gnext-gprev):angle()
      gate_add( k, g, angle )
   end

   local pp = player.pilot()
   pp:setPos( gates_p[1] )
   pp:setVel( vec2.new() )
   pp:setDir( (gates_p[2]-gates_p[1]):angle() )
   pp:control(true)
   pp:setNoJump(true)
   pp:setNoLand(true)
   camera.setZoom(2)
   gate_activate(0)

   -- TODO music?

   omsg_timer = player.omsgAdd(_("3…"), 0, 50)
   hook.timer( 1, "countdown", _("2…") )
   hook.timer( 2, "countdown", _("1…") )
   hook.timer( 3, "allowmove" )
end

function race_complete ()
   local pp = player.pilot()
   pp:setNoJump(false)
   pp:setNoLand(false)
   player.land( mem.race_spob )
end

function countdown( msg )
   -- TODO sound
   player.omsgChange( omsg_timer, msg, 0 )
end

function update_timer ()
   elapsed_time = elapsed_time+0.1
   misn.osdCreate(_("Qex Racing"), {
      _("Beat the Race!"),
      display_time( elapsed_time ),
   } )

   if not race_done then
      hook.timer( 0.1, "update_timer" )
   end
end

function allowmove ()
   player.omsgChange( omsg_timer, _("GO!"), 5 )
   -- TODO sound

   player.pilot():control(false)

   elapsed_time = 0
   hook.timer( 0.1, "update_timer" )
end

function abort ()
   camera.setZoom()
   local pp = player.pilot()
   pp:setNoJump(false)
   pp:setNoLand(false)
   player.land( mem.race_spob )
   misn.finish()
end
