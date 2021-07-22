--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Crimson Gauntlet">
 <avail>
  <priority>3</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Totoran</planet>
 </avail>
</mission>
--]]

local vn = require 'vn'
local gauntlet = require 'campaigns.gauntlet'
local gauntlet_gui = require 'missions.dvaered.gauntlet.gui'
require 'missions.dvaered.gauntlet.tables'
require 'numstring'
local equipopt = require 'equipopt'

logidstr = "log_gauntlet"
logname  = _("Totoran Tournament")
logtype  = _("Totoran Tournament")

-- TODO replace portraits/images
npc_portrait   = "minerva_terminal.png"
npc_image      = "minerva_terminal.png"
npc_name       = _("Crimson Gauntlet Terminal")
npc_description= _("A terminal to access the Crimson Gauntlet Virtual Reality environment. This directly allows you to enter the different challenges and tournaments available.")

misn_title  = _("Crimson Gauntlet Challenge")
misn_desc   = _("Annihilate all enemies in the Crimson Gauntlet.")
misn_reward = _("Great riches!")

gauntletsys = system.get("Crimson Gauntlet")

sfx_clear = audio.new( 'snd/sounds/jingles/victory.ogg' )

function create ()
   npc_gauntlet = misn.npcAdd( "approach_gauntlet", npc_name, npc_portrait, npc_description )
end

-- Land is unified for all types of combat
function land ()
   local rewardcredits = total_score
   -- TODO only give emblems from bosses or special clears?
   local rewardemblems = total_score/100

   vn.clear()
   vn.scene()
   vn.transition()
   vn.sfxMoney()
   vn.func( function ()
      player.pay( rewardcredits, "noescorts" )
      gauntlet.emblems_pay( rewardemblems )
   end )
   vn.na(string.format(_([[You received #g%s#0!
You received %s!]]),
         creditstring( rewardcredits ),
         gauntlet.emblems_str( rewardemblems ) ))
   vn.run()

   misn.finish(true)
end

function approach_gauntlet ()
   gtype, gopt, gmods = gauntlet_gui.run()
   if gtype == nil then
      return
   end

   wave_category = string.lower( gopt )

   -- See if we start the event
   if wave_category==nil then
      return
   end

   -- Accept mission
   misn.accept()

   -- Set details
   misn.setDesc( misn_desc )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )

   -- Add to log
   shiplog.create( logidstr, logname, logtype )

   -- Create the OSD
   osd = misn.osdCreate( _("Crimson Gauntlet"),
         { _("Defeat all the other adversaries!") } )

   hook.load( "loaded" )
   hook.safe("enter_the_ring")
   player.takeoff() -- take off and enter the ring!

   -- Wave meta-information
   gauntlet_enter = "enter_wave"
   wave_round = 1
end
function loaded ()
   misn.finish(false)
end
function abort ()
   leave_the_ring()
end

--[[
   Common Teleporting Functions
--]]
-- Enters Crimson Gauntlet
function enter_the_ring ()
   -- Teleport the player to the Crimson Gauntlet and hide the rest of the universe
   local sys = gauntletsys
   hook.enter( gauntlet_enter )
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)

   -- Set up player stuff
   player.pilot():setPos( vec2.new( 0, 0 ) )
   -- Disable escorts if they exist
   var.push("hired_escorts_disabled",true)
   player.teleport(sys)
   var.pop("hired_escorts_disabled")

   -- Player lost info
   local pp = player.pilot()
   pp_hook_disable = hook.pilot( pp, "disable", "player_lost_disable" )
   pp_hook_dead = hook.pilot( pp, "exploded", "player_lost" )
end
-- Goes back to Totoran (landed)
function leave_the_ring ()
   -- Clear pilots so escorts get docked
   pilot.clear()
   -- Fix the map up
   local sys = gauntletsys
   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   -- Undo player invincibility stuff and land
   hook.land("land")
   local pp = player.pilot()
   pp:setHide( true ) -- clear hidden flag
   pp:setInvincible( false )
   pp:setInvisible( false )
   player.cinematics( false )
   player.land( planet.get("Totoran") )
end

--[[
   Countdown stuff
--]]
function countdown_start ()
   omsg_id = player.omsgAdd( _("5…"), 1.1 )
   hook.timer( 1*1000, "countdown", _("4…") )
   hook.timer( 2*1000, "countdown", _("3…") )
   hook.timer( 3*1000, "countdown", _("2…") )
   hook.timer( 4*1000, "countdown", _("1…") )
   hook.timer( 5*1000, "countdown_done" )
end
function countdown( str )
   -- TODO play countdown sound
   player.omsgChange( omsg_id, str, 1.1 )
end
function countdown_done ()
   -- TODO play sound and cooler text
   player.omsgChange( omsg_id, _("FIGHT!"), 3 )
   wave_started = true
   wave_started_time = naev.ticks()

   for k,p in ipairs(enemies) do
      p:setInvincible(false)
      p:control(false)
      -- We let them do their thing
   end
end


--[[
   Common functions
--]]
function enemy_out( p )
   local idx = nil
   for k,v in ipairs(enemies) do
      if v==p then
         idx=k
         break
      end
   end
   if idx then
      table.remove( enemies, idx )
   end
   if wave_started and #enemies == 0 then
      wave_started = false
      all_enemies_dead()
   end
end
function p_disabled( p )
   p:disable() -- don't let them come back
   p:setInvisible( true ) -- can't target
   p:setInvincible( true ) -- just stays there
   enemy_out( p )
end
function p_death( p )
   enemy_out( p )
end
function player_lost_disable ()
   local pp = player.pilot()
   player.cinematics( true )
   pp:setInvincible( true )
   pp:setInvisible( true )
   -- omsg does not display when dead so we will need a custom solution
   --player.omsgAdd( _("YOU LOST!"), 4.5 )
   if not leave_hook then
      leave_hook = hook.timer( 5000, "leave_the_ring" )
   end
end
function player_lost ()
   hook.rm( pp_hook_disable )
   local pp = player.pilot()
   pp:setHealth( 100, 100 ) -- Heal up to avoid game over if necessary
   pp:setHide( true )
   pp:setInvincible( true )
   player.cinematics( true )

   -- omsg does not display when dead so we will need a custom solution
   --player.omsgAdd( _("YOU LOST!"), 5 )
   --shiplog.append( logidstr, string.format(_("You defeated a %s in one-on-one combat."), enemy_ship) )
   if not leave_hook then
      leave_hook = hook.timer( 3000, "leave_the_ring")
   end
end


--[[
   Wave stuff
--]]
function enter_wave ()
   if system.cur() ~= system.get("Crimson Gauntlet") then
      return
   end

   -- Get rid of pilots
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Metafactions
   enemy_faction = faction.dynAdd( "Mercenary", "Combatant", _("Combatant") )

   -- Start round
   total_score = 0
   wave_round_setup()
end
function wave_round_setup ()
   pilot.clear() -- clear remaining pilots if necessary

   -- Heal up and be nice to the player
   local pp = player.pilot()
   if not gmods.nohealing then
      pp:setHealth( 100, 100, 0 )
      pp:setEnergy( 100 )
      pp:setTemp( 0 )
   end
   pp:fillAmmo() -- Have to fill ammo or deployed fighters get "lost"
   -- TODO reset outfit cooldown stuff
   pp:setPos( vec2.new( 0, 0 ) ) -- teleport to middle
   pp:setVel( vec2.new( 0, 0 ) )

   local function addenemy( shipname, pos )
      local p = pilot.add( shipname, enemy_faction, pos, nil, {ai="baddie_norun", naked=true} )
      equipopt.generic( p, nil, "elite" )
      p:setInvincible(true)
      p:control(true)
      p:setHostile(true)
      p:brake()
      p:face( pp )
      if gmods.doubledmgtaken then
         p:intrinsicSet("fwd_damage",   100)
         p:intrinsicSet("tur_damage",   100)
         p:intrinsicSet("launch_damage",100)
         p:intrinsicSet("fbay_damage",  100)
      end
      local mem = p:memory()
      mem.comm_no = _("No response.") -- Don't allow talking
      hook.pilot( p, "disable", "p_disabled" )
      hook.pilot( p, "death", "p_death" )
      return p
   end
   local function addenemies( ships )
      local e = {}
      local posbase = vec2.new( -1500, 1500 )
      local boss = nil
      local layout = ships.layout or "cluster"
      local pos = posbase
      for k,v in ipairs(ships) do
         -- Determine position
         if layout=="circle" then
            local d,a = posbase:polar()
            a = a +(k-1) * 360 / #ships
            pos = vec2.newP( d, a )
         elseif layout=="pincer" then
            local offset = vec2.newP( 300+200*rnd.rnd(), rnd.rnd()*359 )
            if math.fmod(k,2)==1 then
               pos = posbase + offset
            else
               local x, y = posbase:get()
               pos = vec2.new(-x,-y) + offset
            end
         elseif layout=="cluster" then
            pos = posbase + vec2.newP( 300+200*rnd.rnd(), rnd.rnd()*359 )
         else
            warn(string.format("unknown layout '%s'",layout))
         end

         -- Add ship
         local shipname = v
         local p = addenemy( shipname, pos )
         if boss then
            p:setLeader( boss )
         else
            boss = p
         end
         table.insert( e, p )
      end
      return e
   end

   local round_enemies = wave_round_enemies[wave_category]
   local enemies_list = round_enemies[wave_round]
   if gmods.doubleenemy then
      local doublelist = {}
      for k,v in ipairs(enemies_list) do
         table.insert( doublelist, v )
         table.insert( doublelist, v )
      end
      enemies_list = doublelist
   end
   enemies = addenemies( enemies_list )
   wave_enemies = enemies_list

   -- Count down
   player.omsgAdd( string.format( _("#pWAVE %d#0"), wave_round ), 8 )
   countdown_start()

   all_enemies_dead = wave_end
end
function wave_compute_score ()
   local pp = player.pilot()
   local score = 0
   local bonus = 100
   local str = {}

   local elapsed = (naev.ticks()-wave_started_time) / 1000
   table.insert( str, string.format(_("%.1f seconds"), elapsed) )

   wave_killed = wave_killed or {}
   for k,n in ipairs(wave_enemies) do
      local s = wave_score_table[n]
      table.insert( str, string.format("#o%s %d", _(n), s ) )
      score = score + s
      -- Store all the stuff the pilot killed
      local k = wave_killed[n] or 0
      wave_killed[n] = k+1
   end

   local function newbonus( s, b )
      local h
      if b > 0 then
         h = "#g"
      else
         h = "#r"
      end
      table.insert( str, h .. string.format(s,b) )
      bonus = bonus + b
   end
   local c = pp:ship():class()
   if wave_category == "skirmisher" then
      if c=="Corvette" then
         newbonus( "Corvette %d%%", -20 )
      elseif c=="Destroyer" then
         newbonus( "Destroyer %d%%", -40 )
      elseif c=="Cruiser" then
         newbonus( "Cruiser %d%%", -80 )
      elseif c=="Carrier" then
         newbonus( "Carrier %d%%", -90 )
      elseif c=="Battleship" then
         newbonus( "Battleship %d%%", -90 )
      end
      if elapsed < 15 then
         newbonus( "Fast Clear (<15s) %d%%", 25 )
      elseif elapsed > 90 then
         newbonus( "Slow Clear (>90s) %d%%", -25 )
      end
   elseif wave_category == "warrior" then
      if c=="Fighter" then
         newbonus( "Fighter %d%%", 100 )
      elseif c=="Bomber" then
         newbonus( "Bomber %d%%", 100 )
      elseif c=="Corvette" then
         newbonus( "Corvette %d%%", 25 )
      elseif c=="Cruiser" then
         newbonus( "Cruiser %d%%", -20 )
      elseif c=="Carrier" then
         newbonus( "Carrier %d%%", -30 )
      elseif c=="Battleship" then
         newbonus( "Battleship %d%%", -30 )
      end
      if elapsed < 25 then
         newbonus( "Fast Clear (<25s) %d%%", 25 )
      elseif elapsed > 120 then
         newbonus( "Slow Clear (>120s) %d%%", -25 )
      end
   elseif wave_category == "warlord" then
      if c=="Fighter" then -- I'd love to see someone take down a kestrel in a fighter
         newbonus( "Fighter %d%%", 500 )
      elseif c=="Bomber" then
         newbonus( "Bomber %d%%", 400 )
      elseif c=="Corvette" then
         newbonus( "Corvette %d%%", 100 )
      elseif c=="Destroyer" then
         newbonus( "Destroyer %d%%", 50 )
      elseif c=="Cruiser" then
         newbonus( "Cruiser %d%%", 25 )
      end
      if elapsed < 40 then
         newbonus( "Fast Clear (<40s) %d%%", 25 )
      elseif elapsed > 180 then
         newbonus( "Slow Clear (>180s) %d%%", -25 )
      end
   end

   -- Implement global modifier bonuses here
   if gmods.doubledmgtaken then
      newbonus( "Double Damage Enemies %d%%", 50 )
   end
   if gmods.nohealing then
      newbonus( "No Healing Between Waves %d%%", 25 )
   end

   score = math.max( 0, score * bonus / 100 )

   total_score = total_score + score
   table.insert( str, string.format("TOTAL %d (#g+%d#0)", total_score, score ) )
   return str, score
end
function wave_end_msg( d )
   player.omsgAdd( d[1], d[2] )
   -- TODO add sound
end
function wave_end ()
   if wave_round < #wave_round_enemies[wave_category] then
      -- TODO Cooler animation or something
      local score_str, score = wave_compute_score()
      local n = #score_str
      local s = 1.2 -- time to display each message
      local f = (n+2)*s
      player.omsgAdd( string.format( _("#pWAVE %d CLEAR#0"), wave_round ), f )
      sfx_clear:play()
      for k,v in pairs(score_str) do
         local start = k*s
         hook.timer( 1000*start, "wave_end_msg", {v, f-start} )
      end
      wave_round = wave_round + 1
      hook.timer( (f+1)*1000, "wave_round_setup" )
      return
   end

   -- TODO play sound and cooler text
   player.omsgAdd( _("YOU ARE VICTORIOUS!"), 5 )
   sfx_clear:play()
   --shiplog.append( logidstr, string.format(_("You defeated a %s in one-on-one combat."), enemy_ship) )
   hook.timer( 5000, "leave_the_ring")
end


