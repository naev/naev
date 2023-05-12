--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Qex Racing">
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Melendez Dome</spob>
</mission>
--]]
--[[

   Qex Racing

--]]
local fmt = require "format"
local spfxtrack = require "luaspfx.racetrack"
local vn = require "vn"
local luatk = require "luatk"
local bezier = require "luatk.bezier"
local portrait = require 'portrait'
local luasfx = require "luaspfx.sfx"
local lmisn = require "lmisn"

local DEFAULT_REWARD = 100e3

local elapsed_time, race_done

local col_next = {0, 1, 1, 0.3}
local col_past = {1, 0, 1, 0.2}

local sfx = audio.new("snd/sounds/race_start.ogg")

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

local track_list = require "missions.neutral.race.tracks_qex"

local npc_portrait   = "minerva_terminal.png"
local npc_description= _("A terminal to let you participate in the different available races.")
local laid_back_portrait = portrait.getFullPath("neutral/unique/laidback.webp")

local function track_besttime( track )
   return "race_bt_"..track.name
end

function create ()
   mem.race_spob = spob.cur()
   if not misn.claim( system.cur() ) then
      misn.finish(false)
   end

   -- Load track properties
   for k,track in ipairs(track_list) do
      -- Length of the track
      local length = 0
      local lp = track.track[1][1]
      local scale = track.scale or 1
      for i,trk in ipairs(track.track) do
         for t = 0,1,0.01 do -- only 100 samples
            local p = cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] ) * scale
            length = length + p:dist(lp)
            lp = p
         end
      end

      track.length = length
      track.goaltime = length / 600 -- TODO something better
      track.reward = track.reward or DEFAULT_REWARD
      track.besttime = var.peek( track_besttime(track) ) or math.huge
   end

   misn.npcAdd( "approach_terminal", _("Racing Terminal"), npc_portrait, npc_description )
end

local function display_time( time )
   return fmt.f(_("{1:02.0f}:{2:02.0f}.{3:.0f}"),{
      math.floor( time / 60 ),
      math.floor(math.fmod( time, 60 ) ),
      math.floor(math.fmod( time, 1 )*10),
   })
end

function approach_terminal ()
   local accept = false
   local track

   if not var.peek( "racing_intro" ) then
      vn.clear()
      vn.scene()
      local lbp = vn.newCharacter(_("Laid Back Person"), {image=laid_back_portrait})
      vn.transition()

      vn.na(_([[One of the locals appears as you get close to the racing terminal.]]))

      lbp(_([["Hiya there! You seem like a new face. You want an explanation on how racing works?"]]))
      vn.menu{
         {_([[Get an explanation]]),"cont01_yes"},
         {_([[Skip explanation]]),"cont01_no"},
      }

      vn.label("cont01_yes")
      lbp(_([["Racing is quite simple, you have to go through all the gates in order until you reach the final goal. There are no limitations on what ship and outfits you can use, but you're best sticking to Yacht or Interceptor-class ships if you want to make good times."]]))
      lbp(_([["Each track has a goal time to beat. If you beat the time, you will get a nice credit reward. Furthermore, if you beat all the tracks here, a Melendez Corporation representative will give you a nice trophy to commemorate!"]]))
      lbp(_([["You can try as many times as you want with no penalty. Your best time will also be saved so you can try to beat it again."]]))
      lbp(_([["It's really fun, just be careful not to get addicted. He he.]]))

      vn.label("cont01_no")
      lbp(_([["OK, look forward to seeing your racing skills!"]]))
      vn.run()
      var.push("racing_intro", true)
   end

   local w, h = 460, 400
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newButton( wdw, -20-80-20, -20, 80, 30, _("Race!"), function ()
      local worthy, reason = player.pilot():spaceworthy()
      if not worthy then
         luatk.msg(_("Not Spaceworthy!"), _("Your ship is not spaceworthy and can not participate in the race right now for the following reasons:\n\n")..reason)
      else
         accept = true
         luatk.close()
      end
   end )
   luatk.newButton( wdw, -20, -20, 80, 30, _("Close"), luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Choose Race Track"), nil, "center" )

   local txt_race = luatk.newText( wdw, 240, 40, w-260, 200 )

   local bzr_race = bezier.newBezier( wdw, 20+200+20, 130, 200, 200 )

   local track_names = {}
   for k,v in ipairs(track_list) do
      table.insert( track_names, v.name )
   end
   local lst_race = luatk.newList( wdw, 20, 40, 200, h-60, track_names, function ( _name, idx )
      track = track_list[idx]
      local txt = ""

      bzr_race:set( track.track )

      --txt = txt.."#n".._("Name: ").."#0"     ..track.name.."\n"
      txt = txt.."#n".._("Length: ").."#0"   ..fmt.number(track.length).."\n"
      txt = txt.."#n".._("Goal Time: ").."#0"..display_time(track.goaltime).."\n"
      txt = txt.."#n".._("Reward: ").."#0"   ..fmt.credits(track.reward).."\n"
      if track.besttime == math.huge then
         txt = txt.."#n".._("Best Time: ").."#0".._("N/A")
      else
         local col = ""
         if track.besttime <= track.goaltime then
            col = "#g"
         end
         txt = txt.."#n".._("Best Time: ").."#0"..col..display_time(track.besttime)
      end

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
         lmisn.sfxVictory()
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
   luasfx( true, nil, sfx )
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
   local beat_time = (elapsed_time <= mem.track.goaltime)
   local best_improved = false
   -- Update best time if applicable
   if elapsed_time < mem.track.besttime or mem.track.besttime <= 0 then
      mem.track.besttime = elapsed_time
      var.push( track_besttime(mem.track), elapsed_time )
      best_improved = true
   end
   local reward = mem.track.reward
   local reward_outfit = outfit.get("Racing Trophy")

   vn.clear()
   vn.scene()
   vn.transition()
   if beat_time then
      if best_improved then
         vn.na(fmt.f(_("You finished the race in {elapsed} and beat the goal time of {goal}! This is your new best time! Congratulations!"), {
            elapsed="#g"..display_time( elapsed_time ).."#0",
            goal=display_time( mem.track.goaltime ),
         }))
      else
         vn.na(fmt.f(_("You finished the race in {elapsed} and beat the goal time of {goal}! Congratulations!"), {
            elapsed="#g"..display_time( elapsed_time ).."#0",
            goal=display_time( mem.track.goaltime ),
         }))
      end
      local did_all = true
      for k,v in ipairs(track_list) do
         if v.besttime > v.goaltime then
            did_all = false
            break
         end
      end
      if player.numOutfit( reward_outfit ) <= 0 and did_all then
         vn.na(fmt.f(_([[An individual in a suit and tie suddenly takes you up onto a stage. A large name tag on their jacket says 'Melendez Corporation'. "Congratulations on your win," they say, shaking your hand, "That was a great race! On behalf of Melendez Corporation, and for beating the goal times of all the courses here at {spobname}, I would like to present to you your trophy!".
They hand you one of those fake oversized cheques for the audience, and then a credit chip with the actual prize money on it. At least the trophy looks cool.]]),
            {spobname=spob.cur()}))
         vn.na(fmt.reward(reward_outfit).."\n"..fmt.reward(reward))
         vn.func( function ()
            player.outfitAdd( reward_outfit )
         end )
      else
         vn.na(fmt.reward(reward))
      end
      vn.func( function ()
         player.pay(reward)
      end )
   elseif best_improved then
      vn.na(fmt.f(_("You finished the race in {elapsed}, but were {short} of the goal time. This is your new best time! Keep trying!"), {
         elapsed="#g"..display_time( elapsed_time ).."#0",
         short="#r"..display_time( mem.track.goaltime - elapsed_time ).."#0",
      }))
   else
      vn.na(fmt.f(_("You finished the race in {elapsed}, but were {short} of the goal time. Keep trying!"), {
         elapsed="#g"..display_time( elapsed_time ).."#0",
         short="#r"..display_time( mem.track.goaltime - elapsed_time ).."#0",
      }))
   end

   vn.run()

   misn.finish( beat_time )
end

function countdown( msg )
   luasfx( true, nil, sfx )
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
   player.omsgChange( omsg_timer, _("GO!"), 3 )
   luasfx( true, nil, sfx, {pitch=2} )

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
