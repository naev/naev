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
local luatk = require "luatk"
local bezier = require "luatk.bezier"

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
   }, {
      name = _("Easy"),
      scale = 15,
      center = true,
      track = {
         {
            vec2.new( 939, 394 ),
            vec2.new( 6, -128 ),
            vec2.new( 70, 2 ),
            vec2.new( 874, 134 ),
         },
         {
            vec2.new( 874, 134 ),
            vec2.new( -70, -2 ),
            vec2.new( -34, -33 ),
            vec2.new( 809, 208 ),
         },
         {
            vec2.new( 809, 208 ),
            vec2.new( 34, 39 ),
            vec2.new( 118, 57 ),
            vec2.new( 725, 270 ),
         },
         {
            vec2.new( 725, 270 ),
            vec2.new( -118, -57 ),
            vec2.new( 39, -42 ),
            vec2.new( 470, 250 ),
         },
         {
            vec2.new( 470, 250 ),
            vec2.new( -39, 42 ),
            vec2.new( -91, 17 ),
            vec2.new( 549, 338 ),
         },
         {
            vec2.new( 549, 338 ),
            vec2.new( 147, -31 ),
            vec2.new( -144, 53 ),
            vec2.new( 735, 395 ),
         },
         {
            vec2.new( 735, 395 ),
            vec2.new( 82, -37 ),
            vec2.new( -15, 226 ),
            vec2.new( 939, 394 ),
         },
      },
   }, {
      name = _("Race 2"),
      scale = 15,
      center = true,
      track = {
         {
            vec2.new( 942, 381 ),
            vec2.new( 5, -104 ),
            vec2.new( 69, -1 ),
            vec2.new( 896, 165 ),
         },
         {
            vec2.new( 896, 165 ),
            vec2.new( -69, 1 ),
            vec2.new( 61, 55 ),
            vec2.new( 733, 177 ),
         },
         {
            vec2.new( 733, 177 ),
            vec2.new( -61, -55 ),
            vec2.new( 86, 80 ),
            vec2.new( 551, 176 ),
         },
         {
            vec2.new( 551, 176 ),
            vec2.new( -86, -80 ),
            vec2.new( -169, -106 ),
            vec2.new( 544, 322 ),
         },
         {
            vec2.new( 544, 322 ),
            vec2.new( 73, 42 ),
            vec2.new( -154, 98 ),
            vec2.new( 723, 379 ),
         },
         {
            vec2.new( 723, 379 ),
            vec2.new( 154, -99 ),
            vec2.new( 288, -3 ),
            vec2.new( 687, 465 ),
         },
         {
            vec2.new( 687, 465 ),
            vec2.new( -181, -1 ),
            vec2.new( 36, -183 ),
            vec2.new( 402, 398 ),
         },
         {
            vec2.new( 402, 398 ),
            vec2.new( -15, 66 ),
            vec2.new( -81, 55 ),
            vec2.new( 515, 506 ),
         },
         {
            vec2.new( 515, 506 ),
            vec2.new( 81, -56 ),
            vec2.new( -105, 94 ),
            vec2.new( 702, 532 ),
         },
         {
            vec2.new( 702, 532 ),
            vec2.new( 105, -94 ),
            vec2.new( -83, 111 ),
            vec2.new( 896, 503 ),
         },
         {
            vec2.new( 896, 503 ),
            vec2.new( 21, -30 ),
            vec2.new( 0, 63 ),
            vec2.new( 942, 381 ),
         },
      },
   }, {
      name = _("Race 3"),
      scale = 15,
      center = true,
      track = {
         {
            vec2.new( 942, 466 ),
            vec2.new( 3, -288 ),
            vec2.new( 132, -1 ),
            vec2.new( 830, 176 ),
         },
         {
            vec2.new( 830, 176 ),
            vec2.new( -132, 1 ),
            vec2.new( -2, -113 ),
            vec2.new( 711, 291 ),
         },
         {
            vec2.new( 711, 291 ),
            vec2.new( 2, 113 ),
            vec2.new( -56, 1 ),
            vec2.new( 772, 409 ),
         },
         {
            vec2.new( 772, 409 ),
            vec2.new( 56, -1 ),
            vec2.new( -7, -172 ),
            vec2.new( 830, 577 ),
         },
         {
            vec2.new( 830, 577 ),
            vec2.new( -5, 178 ),
            vec2.new( 144, -11 ),
            vec2.new( 651, 698 ),
         },
         {
            vec2.new( 651, 698 ),
            vec2.new( -123, 1 ),
            vec2.new( 1, 116 ),
            vec2.new( 530, 585 ),
         },
         {
            vec2.new( 530, 585 ),
            vec2.new( -1, -116 ),
            vec2.new( -114, -3 ),
            vec2.new( 651, 466 ),
         },
         {
            vec2.new( 651, 466 ),
            vec2.new( 59, -1 ),
            vec2.new( 2, -63 ),
            vec2.new( 709, 528 ),
         },
         {
            vec2.new( 709, 528 ),
            vec2.new( 2, 51 ),
            vec2.new( -170, 0 ),
            vec2.new( 884, 579 ),
         },
         {
            vec2.new( 884, 579 ),
            vec2.new( 62, -1 ),
            vec2.new( 4, 108 ),
            vec2.new( 942, 471 ),
         },
      },
   }, {
      name = _("Race 4"),
      scale = 15,
      center = true,
      track = {
         {
            vec2.new( 1062, 350 ),
            vec2.new( 3, -112 ),
            vec2.new( 117, -4 ),
            vec2.new( 945, 236 ),
         },
         {
            vec2.new( 945, 236 ),
            vec2.new( -117, 4 ),
            vec2.new( 2, 60 ),
            vec2.new( 825, 171 ),
         },
         {
            vec2.new( 825, 171 ),
            vec2.new( 4, -54 ),
            vec2.new( 120, -4 ),
            vec2.new( 706, 118 ),
         },
         {
            vec2.new( 706, 118 ),
            vec2.new( -240, -1 ),
            vec2.new( 3, -108 ),
            vec2.new( 471, 231 ),
         },
         {
            vec2.new( 471, 231 ),
            vec2.new( -3, 108 ),
            vec2.new( -113, -1 ),
            vec2.new( 587, 350 ),
         },
         {
            vec2.new( 587, 350 ),
            vec2.new( 113, 1 ),
            vec2.new( 2, -113 ),
            vec2.new( 709, 466 ),
         },
         {
            vec2.new( 709, 466 ),
            vec2.new( 1, 175 ),
            vec2.new( 120, 3 ),
            vec2.new( 590, 639 ),
         },
         {
            vec2.new( 590, 639 ),
            vec2.new( -120, -3 ),
            vec2.new( 2, 121 ),
            vec2.new( 474, 521 ),
         },
         {
            vec2.new( 474, 521 ),
            vec2.new( 0, -50 ),
            vec2.new( -116, -2 ),
            vec2.new( 589, 470 ),
         },
         {
            vec2.new( 589, 470 ),
            vec2.new( 116, 2 ),
            vec2.new( -118, -2 ),
            vec2.new( 946, 468 ),
         },
         {
            vec2.new( 946, 468 ),
            vec2.new( 118, 2 ),
            vec2.new( -3, 112 ),
            vec2.new( 1062, 350 ),
         },
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

local function display_time( time )
   return fmt.f(_("{1:02.0f}:{2:02.0f}.{3:.0f}"),{
      math.floor( time / 60 ),
      math.floor(math.fmod( time, 60 ) ),
      math.floor(math.fmod( time, 1 )*10),
   })
end

function accept ()
   local accept = false
   local track

   local w, h = 460, 400
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newButton( wdw, -20-80-20, -20, 80, 30, _("Race!"), function ()
      accept = true
      luatk.close()
   end )
   luatk.newButton( wdw, -20, -20, 80, 30, _("Close"), luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Choose Race Track"), nil, "center" )

   local txt_race = luatk.newText( wdw, 240, 40, w-260, 200 )

   local bzr_race = bezier.newBezier( wdw, 20+200+20, 120, 200, 200 )

   local track_names = {}
   for k,v in ipairs(track_list) do
      table.insert( track_names, v.name )
   end
   local lst_race = luatk.newList( wdw, 20, 40, 200, h-60, track_names, function ( _name, idx )
      track = track_list[idx]
      local txt = ""

      -- Length of the track
      local length = 0
      local lp = track.track[1][1]
      local scale = track.scale or 1
      for k,trk in ipairs(track.track) do
         for t = 0,1,0.01 do -- only 100 samples
            local p = cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] ) * scale
            length = length + p:dist(lp)
            lp = p
         end
      end

      bzr_race:set( track.track )

      txt = txt.."#n".._("Name: ").."#0"..track.name.."\n"
      txt = txt.."#n".._("Length: ").."#0"..fmt.number(length).."\n"
      txt = txt.."#n".._("Best Time: ").."#0"..display_time(0)

      txt_race:set(txt)
   end )
   lst_race:set( 1 )

   luatk.run()

   if not accept then return end

   misn.accept()
   misn.setDesc(_("You're participating in a race!"))
   misn.setReward(_("Endless Riches!"))

   hook.load( "loaded" ) -- don't load into the race

   mem.spob = spob.cur()
   mem.track = track
   hook.safe("start_race")
   player.takeoff() -- take off and start the race!
end

function loaded ()
   misn.finish(false)
end

local omsg_timer
function start_race ()
   pilot.clear()
   pilot.toggleSpawn(false)
   -- TODO add spectators / cameras?

   local scale = mem.track.scale or 1
   local translate = mem.track.translate or vec2.new()
   if mem.track.center then
      local minx, maxx, miny, maxy = math.huge, -math.huge, math.huge, -math.huge
      for k,trk in ipairs(mem.track.track) do
         for t = 0,1,0.05 do
            local p = cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] ) * scale
            local x,y = p:get()
            minx = math.min( minx, x )
            maxx = math.max( maxx, x )
            miny = math.min( miny, y )
            maxy = math.max( maxy, y )
         end
      end
      local pos = mem.spob:pos()
      translate = pos - vec2.new( minx, miny ) - vec2.new( (maxx-minx)*0.5, (maxy-miny)*0.5 )
   end

   local lp
   local elapsed = 0
   local gates_p = {}
   for k,trk in ipairs(mem.track.track) do
      for t = 0,1,0.005 do
         local p = cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] ) * scale + translate
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
      -- Note that this function is called from luaspfx stuff. We can't use
      -- hooks or the likes here, only set local variables to communicate with
      -- the mission. Worst case we can trigger custom hooks if necessary
      for i=0,4 do
         if k-i > 0 then
            local c = {col_past[1], col_past[2], col_past[3], col_past[4]*(1-i/4)}
            gates[k-i]:setCol( c )
         end
      end

      local ngates = #gates
      if k >= ngates then
         race_done = true -- This should trigger the timer hook
         omsg_timer = player.omsgAdd(display_time(elapsed_time), 5, 50)
         -- TODO sound effect?
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

   local angle = (gates_p[2]-gates_p[1]):angle()
   local pp = player.pilot()
   pp:setPos( gates_p[1] )
   pp:setVel( vec2.new() )
   pp:setDir( angle )
   pp:control(true)
   pp:setNoJump(true)
   pp:setNoLand(true)
   camera.setZoom(2)
   gate_activate(1)

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
   hook.land("race_landed")
   player.land( mem.race_spob )
end

function race_landed ()
   vntk.msg(_("Race Finished!"),fmt.f(_("You finished the race in {elapsed}. Congratulations!"), {
      elapsed="#g"..display_time( elapsed_time ).."#0",
   }))
   misn.finish(false) -- false until we actually set up proper rewards and stuff
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
   else
      hook.timer( 5, "race_complete" )
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
