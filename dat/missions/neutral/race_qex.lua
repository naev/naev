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

local goal_times = require 'missions.neutral.race.times_qex'
local medal_col = {Bronze= '#o', Silver= '#w', Gold= '#y'}
local metal_display_name = {Bronze=p_('trophy', 'Bronze'), Silver=p_('trophy', 'Silver'), Gold=p_('trophy', 'Gold')}

local elapsed_time, race_done

local col_next = {0, 1, 1, 0.3}
local col_past = {1, 0, 1, 0.2}

local sfx = audiodata.new("snd/sounds/race_start")

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

local npc_portrait   = "minerva_terminal"
local npc_description= _("A terminal to let you participate in the different available races.")
local laid_back_portrait = portrait.getFullPath("neutral/unique/laidback")

local function track_besttime( track )
   return "race_bt_"..track.name
end

local map_all_ships = require 'scripts.map_all_ships'

local function upgrade_trophies(to)
   --outfitAddSlot outfitRmSlot
   map_all_ships(function (pp)
      if pp:outfitHasSlot('accessory') then
         for _i, v in ipairs({'Gold', 'Silver', 'Bronze'}) do
            if pp:outfitSlot('accessory') == outfit.get('Racing Trophy (' .. v .. ')') then
               pp:outfitRmSlot('accessory')
               pp:outfitAddSlot(to, 'accessory')
            end
         end
      end
   end)
   if to == outfit.get('Racing Trophy (Gold)') then
      if player.outfitNum(outfit.get('Racing Trophy (Silver)'), true) >= 1 then
         player.outfitRm('Racing Trophy (Silver)')
      end
      if player.outfitNum(outfit.get('Racing Trophy (Bronze)'), true) >= 1 then
         player.outfitRm('Racing Trophy (Bronze)')
      end
   elseif to == outfit.get('Racing Trophy (Silver)') then
      if player.outfitNum(outfit.get('Racing Trophy (Bronze)'), true) >= 1 then
         player.outfitRm('Racing Trophy (Bronze)')
      end
   end
end

local function add_advice_npc()
   misn.npcAdd( "qex_race_advice", _("Laid Back Person"), laid_back_portrait,
      _("This person will be happy to answer any questions about racing."), 6)
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
      local scale = track.scale or 1
      local lp = track.track[1][1]*scale
      for i,trk in ipairs(track.track) do
         for t = 0,1,0.05 do
            local p = cubicBezier( t, trk[1], trk[1]+trk[2], trk[4]+trk[3], trk[4] ) * scale
            length = length + p:dist(lp)
            lp = p
         end
      end

      track.length = length
      track.goaltime = goal_times[track.name]
      track.reward = track.reward or DEFAULT_REWARD
      track.besttime = var.peek( track_besttime(track) ) or math.huge
   end

   misn.npcAdd( "approach_terminal", _("Racing Terminal"), npc_portrait, npc_description )
   if var.peek( "racing_intro" ) then
      add_advice_npc()
   end
end

function qex_race_advice()
   vn.clear()
   vn.scene()
   local lbp = vn.newCharacter(_("Laid Back Person"), {image=laid_back_portrait})
   vn.transition()

   lbp(_([["Hiya there! How is the racing going ?"]]))

   vn.label("start")
   lbp(_([["Do you need help?"]]))
   vn.menu{
      {_([[Difficulty Levels]]), "cont_difficulty"},
      {_([[Useful Tips]]), "cont_strategy"},
      {_([[The Bronze, Silver, and Gold Series]]), "cont_series"},
      {_([[Leave]]), "cont_quit"},
   }

   vn.label("cont_difficulty")
   lbp(_([["The tracks are in order of increasing difficult, with the Peninsula being the easiest track, while the Qex Tour is considered the hardest. Increased difficulty also means increased payouts, so it's worth tackling the harder races."]]))
   lbp(_([["Furthermore, each track has three goal times: Bronze, Silver and Gold. The payout is also increased based on what goal time you manage to beat, and you can always try to beat your best time. However, note that the goal times get much harder on the harder tracks, so Silver on Peninsula is harder that Bronze on the Qex Tour and the likes."]]))
   lbp(_([["I would recommend you to try to beat the Bronze goal times on all the tracks before moving on to Silver, and finally Gold. It's also a great way to get some more practice racing."]]))
   vn.jump("start")

   vn.label("cont_strategy")
   lbp(_([["The first step of racing is to choose a ship that can be designed for max speed. Interceptors and other smaller ships tend to be easier to modify, so I would recommend to start there. Make sure to grab the best engines you can get your hand on and stack as many speed modifications as possible."]]))
   lbp(_([["However, racing isn't all about passive bonuses! You are not only allowed, but expected to use afterburners while racing. It's important not only to try to get as fast of a ship as possible, but also one that can sustain the afterburner the longest during the race. So make sure to have enough energy regeneration!"]]))
   vn.jump("start")

   vn.label("cont_series")
   lbp(_([["Beating the Bronze goal times in the Bronze series is the easiest and a good way to get practice racing. With any decent ship build for speed it should be possible to clear them."]]))
   lbp(_([["The Silver series is more challenging. It will require something better than the basic Unicorp afterburner, and finding the right ship and outfit to do this is a challenge in itself. It is only accessible for seasoned captains who can afford it."]]))
   lbp(_([["Finally, the Gold series is the hardest challenge with very tight goal times. Only very few pilots manage to complete it and get the prestigious Gold Trophy. At this point, this is all about prestige, as the Gold Trophy in itself is not worth the effort, if you ask me."]]))
   vn.jump("start")

   vn.label("cont_quit")
   vn.run()
end

local function display_time( time )
   local abs_time = math.abs(time)
   return fmt.f(_("{1:02.0f}:{2:02.0f}.{3:.0f}"),{
      (time > 0 and 1 or -1)*math.floor( abs_time / 60 ),
      math.floor(math.fmod( abs_time, 60 ) ),
      math.floor(math.fmod( abs_time, 1 )*10),
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
      lbp(_([["Racing is quite simple, you have to go through all the gates in order until you reach the final goal. There are no limitations on what ship and outfits you can use, but you're best sticking to light or medium-light ships and using an afterburner if you want to make good times."]]))
      lbp(_([["Each track has three goal times to beat: Bronze, Silver and Gold corresponding to three increasing levels of difficulty. The reward you get on each track will depend on which goal time you beat and also on the difficulty of the track itself. In addition, you will get an early finish bonus depending on the improvement over the reference time."]]))
      lbp(_([["If you beat all the tracks here for a given difficulty level, a Melendez Corporation representative will give you a nice trophy to commemorate!"]]))
      lbp(_([["You can try as many times as you want with no penalty. Your best time will also be saved so you can try to beat it again. It's really fun, just be careful not to get addicted. He he."]]))

      vn.label("cont01_no")
      lbp(_([["OK, look forward to seeing your racing skills! I will be around if you have any questions."]]))
      vn.run()
      var.push("racing_intro", true)
      add_advice_npc()
   end

   local w, h = 480, 420
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
   luatk.newText( wdw, 0, 10, w, 20, _("Choose Race Track"), nil, "centre" )

   local txt_race = luatk.newText( wdw, 240, 40, w-260, 220 )

   local bzr_race = bezier.newBezier( wdw, 20+200+20, 160, w-260, 190 )

   local track_names = {}
   for k,v in ipairs(track_list) do
      table.insert( track_names, _(v.name) )
   end
   local lst_race = luatk.newList( wdw, 20, 40, 200, h-60, track_names, function ( _name, idx )
      track = track_list[idx]
      local txt = ""

      bzr_race:set( track.track )

      txt = txt .. '#n' .. fmt.f(_('Length: {length} km'), {length=fmt.number(track.length)})..'#0\n'
      local yet
      for _n, i in ipairs({'Bronze', 'Silver', 'Gold'}) do
         if not yet and track.besttime >= track.goaltime[i] then
            yet = true
            txt = txt .. '#b' .. _('Best Time: ')
            if track.besttime == math.huge then
               txt = txt .. '#n' .. _('N/A') .. '#0\n'
            else
               txt = txt .. display_time(track.besttime) .. '#0\n'
            end
         end
         txt = txt .. '#n' .. fmt.f(_('Goal Time: {time} ({metal_name})'), {time=medal_col[i] .. display_time(track.goaltime[i]) .. '#n', metal_name=metal_display_name[i]}) .. '#0\n'
      end
      if not yet then
         txt = txt .. '#b' .. _('Best Time: ') .. display_time(track.besttime) .. '#0\n'
      end
      txt = txt .. '#n' .. _('Reward: ')
         .. medal_col['Bronze'] .. '0.5#n/' .. medal_col['Silver'] .. '1#n/'
         .. medal_col['Gold'] .. '2#0'
         .. ' #nx ' .. fmt.credits(track.reward) .. '#0\n'

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
   -- Clean up the escorts
   for k,p in ipairs(player.pilot():followers()) do
      print(tostring(k)..'|'..tostring(p))
      if p:flags("carried") then
         p:rm()
      else
         p:setHide( true ) -- Don't remove or it'll mess cargo
      end
   end

   -- TODO add spectators / cameras?

   local scale = mem.track.scale or 1
   local translate = mem.track.translate or vec2.new()
   if mem.track.centre then
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
   camera.setZoom()
   local pp = player.pilot()
   pp:setNoJump(false)
   pp:setNoLand(false)
   hook.land("race_landed")
   player.land( mem.race_spob )
end

function race_landed ()
   local beat_time
   local nxt
   local reward
   local bonus

   if elapsed_time <= 0.0 then return end

   for i, g in ipairs({'Gold', 'Silver', 'Bronze'}) do
      if elapsed_time <= mem.track.goaltime[g] then
         local ratio = mem.track.goaltime[g] / elapsed_time - 1.0
         reward = mem.track.reward  * 2^(2-i)
         bonus = math.floor( ratio * reward / 1000) * 1000
         beat_time = g
         break
      end
      nxt = g
   end

   local imp_str
   -- Update best time if applicable
   if elapsed_time < mem.track.besttime or mem.track.besttime <= 0 then
      mem.track.besttime = elapsed_time
      var.push( track_besttime(mem.track), elapsed_time )
      imp_str = _(' This is your new best time!')
   else
      imp_str = ''
   end

   vn.clear()
   vn.scene()
   vn.transition()
   if beat_time then
      if nxt then
         vn.na(fmt.f(_('You finished the race in {elapsed} and beat the goal time of {goal} ({metal}) but were {short} over the goal time of {ngoal} ({nmetal}).{imp_str} Congratulations!'), {
            elapsed= '#b' .. display_time( elapsed_time ) .. '#0',
            goal= medal_col[beat_time] .. display_time( mem.track.goaltime[beat_time] ) .. '#0',
            metal= metal_display_name[beat_time],
            short= '#r' .. display_time( elapsed_time - mem.track.goaltime[nxt] ) .. '#0',
            ngoal= medal_col[nxt] .. display_time( mem.track.goaltime[nxt] ) .. '#0',
            nmetal= metal_display_name[nxt],
            imp_str= imp_str,
         }))
      else
         vn.na(fmt.f(_("You finished the race in {elapsed} and beat the goal time of {goal} ({metal}).{imp_str} Congratulations!"), {
            elapsed= '#b' .. display_time( elapsed_time ) .. '#0',
            goal= medal_col[beat_time] .. display_time( mem.track.goaltime[beat_time] ) .. '#0',
            metal= beat_time,
            imp_str= imp_str,
         }))
      end
      local completed
      for _n, g in ipairs({'Gold', 'Silver', 'Bronze'}) do
         local did_all = true
         for k, v in ipairs(track_list) do
            if v.besttime > v.goaltime[g] then
               did_all = false
               break
            end
         end
         if did_all then
            completed = g
            break
         end
      end
      local already_have = {}
      for _n, g in ipairs({'Gold', 'Silver', 'Bronze'}) do
         if player.outfitNum(outfit.get('Racing Trophy (' .. g .. ')')) >= 1 then
            already_have[g] = true
         end
      end
      local reward_outfit
      if completed then
         reward_outfit = outfit.get('Racing Trophy (' .. completed .. ')')
         upgrade_trophies(reward_outfit)
      end
      if completed and not already_have[completed] then
         vn.na(fmt.f(_([[An individual in a suit and tie suddenly takes you up onto a stage. A large name tag on their jacket says 'Melendez Corporation'. "Congratulations on your win," they say, shaking your hand, "That was a great race! On behalf of Melendez Corporation, and for beating the goal times of all the courses here at {spobname}, I would like to present to you your {metal} trophy!".
They hand you one of those fake oversized cheques for the audience, and then a credit chip with the actual prize money on it. At least the trophy looks cool.]]),
            {spobname= spob.cur(), metal= metal_display_name[completed]}))
         vn.na(fmt.reward(reward_outfit)..'\n'..fmt.reward(reward)..'\n'..fmt.reward(bonus).._(' (early finish bonus)'))
         if completed == 'Silver' or (not already_have['Silver'] and completed == 'Gold') and not diff.isApplied("melendez_dome_xy37") then
            vn.func( function ()
               diff.apply("melendez_dome_xy37")
            end )
            vn.na(_([[After the formalities finish, a Nexus Engineer comes up to you, nervous and stuttering, "I-I-i love the way you fly! Me and some o-other engineers were thinking you mi-might want to try our prototype. I've g-given you access at the shipyard to check it out.". They quickly scuttle away before you can ask anything else. You wonder what the prototype is.]]))
         end
         vn.func( function ()
            player.outfitAdd( reward_outfit )
         end )
      else
         vn.na(fmt.reward(reward)..'\n'..fmt.reward(bonus).._(' (early finish bonus)'))
      end
      vn.func( function ()
         player.pay(reward)
         player.pay(bonus)
      end)
   else
      vn.na(fmt.f(_("You finished the race in {elapsed}, but were {short} over the goal time of {goal}.{imp_str} Keep trying!"), {
         elapsed= '#b' .. display_time( elapsed_time ) .. '#0',
         short= '#r' .. display_time( elapsed_time - mem.track.goaltime['Bronze'] ) .. '#0',
         goal= medal_col['Bronze'] .. display_time( mem.track.goaltime['Bronze'] ) .. '#0',
         imp_str= imp_str,
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
   if not race_done then
      elapsed_time = elapsed_time+0.1
      misn.osdCreate(_("Qex Racing"), {
         _("Beat the Race!"),
         display_time( elapsed_time ),
      } )
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
