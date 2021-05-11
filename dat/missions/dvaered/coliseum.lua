--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Totoran Tournament">
 <avail>
  <priority>3</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Totoran</planet>
 </avail>
</mission>
--]]

local vn = require 'vn'
require 'numstring'

logidstr = "log_morrigan"
logname  = _("Totoran Tournament")
logtype  = _("Totoran Tournament")

npc_portrait   = "dvaered_thug1.png"
npc_image      = "dvaered_thug1.png"
npc_name       = _("Tournament Organizer")
npc_description= _("The Totoran Tournament organizer.")
npc_colour     = {1, 0.7, 0.3}

misn_title  = _("Totoran Tournament")
misn_desc   = _("Annihilate all enemies in Coliseum.")
misn_reward = _("Great riches!")

coliseum = system.get("Coliseum")

function create ()
   if not var.peek("testing") then
      misn.finish(false) -- Disabled for now
   end
   -- We'll have different NPCs for each tournament type
   npc_wave = misn.npcAdd( "approach_wave", npc_name, npc_portrait, npc_description )
end
function cleanup_npc ()
   misn.npcRm( npc_wave )
end


-- Land is unified for all types of combat
function land ()
   -- TODO show performance and give reward based on it
   --[[
   vn.clear()
   vn.scene()
   vn.transition()
   vn.sfxMoney()
   vn.func( function () player.pay( rewardcredits ) end )
   vn.na(string.format(_("You received #g%s#0."), creditstring( rewardcredits )))
   vn.run()
   --]]

   misn.finish(true)
end


function pairsByKeys( t, f )
   local a = {}
   for n in pairs(t) do table.insert(a, n) end
      table.sort(a, f)
      local i = 0      -- iterator variable
      local iter = function ()   -- iterator function
      i = i + 1
      if a[i] == nil then return nil
      else return a[i], t[a[i]]
      end
   end
   return iter
end


function approach_wave ()
   vn.clear()
   vn.scene()
   local dv = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   -- TODO first time message

   vn.label("menu")
   dv("Yo")
   vn.menu{
      { "Enter in the Light Category", "light" },
      --{ "Enter in the Medium Category", "medium" },
      --{ "Enter in the Heavy Category", "heavy" },
      { "More Information", "info" },
      { "Maybe later", "leave" },
   }

   vn.label("light")
   vn.func( function () wave_category = "light" end )
   vn.done()

   -- TODO info
   vn.label("info")
   dv("here be info")
   vn.jump("menu")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.done()

   vn.run()

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
   shiplog.createLog( logidstr, logname, logtype )

   -- Create the OSD
   osd = misn.osdCreate( _("Totoran Tournament"),
         { _("Defeat all the other adversaries!") } )

   hook.safe("enter_the_ring")
   player.allowSave( false ) -- Don't want to save the mission
   player.takeoff() -- take off and enter the ring!

   -- Wave meta-information
   coliseum_enter = "enter_wave"
   wave_round = 1
end

--[[
   Common Teleporting Functions
--]]
-- Enters Coliseum
function enter_the_ring ()
   -- Teleport the player to the Coliseum and hide the rest of the universe
   local sys = coliseum
   hook.enter( coliseum_enter )
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)

   -- Set up player stuff
   player.pilot():setPos( vec2.new( 0, 0 ) )
   player.allowSave(true)
   player.teleport(coliseum)

   -- Player lost info
   local pp = player.pilot()
   --hook.pilot( pp, "disable", "player_lost" )
   hook.pilot( pp, "exploded", "player_lost" )
end
-- Goes back to Totoran (landed)
function leave_the_ring ()
   local sys = coliseum
   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   hook.land("land")
   player.land( planet.get("Totoran") )
   local pp = player.pilot()
   pp:setHide( true ) -- clear hidden flag
   pp:setInvincible( false )
   player.cinematics( false )
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
   if misn_state==3 then return end

   local idx = nil
   for k,v in ipairs(enemies) do
      if v==p then
         idx=k
         break
      end
   end
   table.remove( enemies, idx )
   if wave_started and #enemies == 0 then
      wave_started = false
      hook.timer( 3000, "enemy_out_delay" )
   end
end
function enemy_out_delay ()
   all_enemies_dead()
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
function player_lost ()
   local pp = player.pilot()
   pp:setHealth( 1, 0 ) -- Heal up to avoid game over if necessary
   pp:setHide( true )
   pp:setInvincible( true )
   player.cinematics( true )

   -- omsg does not display so we need a custom solution
   --player.omsgAdd( _("YOU LOST!"), 5 )
   --shiplog.appendLog( logidstr, string.format(_("You defeated a %s in one-on-one combat."), enemy_ship) )
   exploded_hook = hook.timer( 5000, "leave_the_ring")
end


--[[
   Wave stuff
--]]
function enter_wave ()
   if system.cur() ~= system.get("Coliseum") then
      return
   end

   -- Get rid of pilots
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Metafactions
   enemy_faction = faction.dynAdd( "Mercenary", "Combatant", _("Combatant") )

   -- Start round
   wave_round_setup()
end
function wave_round_setup ()
   local pp = player.pilot()
   enemies = {}
   local function addenemy( shipname, pos )
      local p = pilot.add( shipname, enemy_faction, pos, nil, "baddie_norun" )
      p:setInvincible(true)
      p:control(true)
      p:setHostile(true)
      p:brake()
      p:face( pp )
      local mem = p:memory()
      mem.comm_no = _("No response.") -- Don't allow talking
      hook.pilot( p, "disable", "p_disabled" )
      hook.pilot( p, "death", "p_death" )
      table.insert( enemies, p )
   end
   local function addenemies( ships )
      local pos = vec2.new( -500, 500 )
      for k,v in ipairs(ships) do
         local shipname = v
         addenemy( shipname, pos )
      end
   end

   local round_enemies
   if wave_category == "light" then
      round_enemies = {
         { "Hyena" }, -- 1
         { "Hyena" },
         { "Shark" },
         { "Lancelot" },
         { "Vendetta" }, -- 5
         { "Hyena", "Hyena" },
         { "Shark", "Hyena" },
         { "Lancelot", "Hyena" },
         { "Hyena", "Hyena", "Hyena" },
         { "Admonisher" }, -- 10
      }
   end
   addenemies( round_enemies[wave_round] )

   -- Count down
   player.omsgAdd( string.format( _("WAVE %d"), wave_round ), 8 )
   countdown_start()

   all_enemies_dead = wave_end
end
function wave_end ()
   if wave_round < 10 then
      wave_round = wave_round + 1
      wave_round_setup()
      return
   end

   -- TODO play sound and cooler text
   player.omsgAdd( _("YOU ARE VICTORIOUS!"), 5 )
   --shiplog.appendLog( logidstr, string.format(_("You defeated a %s in one-on-one combat."), enemy_ship) )
   hook.timer( 5000, "leave_the_ring")
end


