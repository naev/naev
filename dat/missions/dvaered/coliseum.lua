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

coliseum = system.get("Coliseum")

function create ()
   misn.finish(false) -- Disabled for now
   -- We'll have different NPCs for each tournament type
   npc_1v1 = misn.npcAdd( "approach_1v1", npc_name, npc_portrait, npc_description )
end
function cleanup_npc ()
   misn.npcRm( npc_1v1 )
end


-- Land is unified for all types of combat
function land ()
   if misn_state==3 then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.sfxMoney()
      vn.func( function () player.pay( rewardcredits ) end )
      vn.na(string.format(_("You received #g%s#0."), creditstring( rewardcredits )))
      vn.run()

      misn.finish(true)
   elseif misn_state>0 then
      player.msg(_("#rMISSION FAILED! You were not supposed to land after the fight started!!"))
      misn.finish(false)
   end
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


function approach_1v1 ()
   vn.clear()
   vn.scene()
   local dv = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   -- TODO first time message

   dv("Yo")
   vn.menu{
      { "Enter the Tournament", "enter" },
      { "Ask for more information", "info" },
      { "Maybe later", "leave" },
   }

   vn.label("enter")
   dv("What type of tournament?")
   vn.menu{
      { "One on one", "onevone" },
      --{ "Free for all", "ffa" },
      { "Never mind", "leave" },
   }

   opponents_list = {
      Hyena = 10e3,
      Shark = 20e3,
      Vendetta = 30e3,
      Admonisher = 50e3,
      Pacifier= 100e3,
      Kestrel = 200e3,
      Hawking = 300e3,
   }
   vn.label("onevone")
   dv("What enemy?")
   rewardcredits = 0
   vn.menu( function ()
         local opts = {}
         local sortfunc = function(a,b) return opponents_list[a]<opponents_list[b] end
         for k,v in pairsByKeys(opponents_list, sortfunc) do
            local str = string.format(_("%s (#g%s#0)"), k, creditstring(v))
            table.insert( opts, { str, k } )
         end
         table.insert( opts, {_("Never mind"), "leave"} )
         return opts
      end,
      function( opt )
         if opt=="leave" then
            vn.jump(opt)
            return
         end
         rewardcredits = opponents_list[opt]
         enemy_ship = opt
         misn_state = 0
      end
   )
   vn.done()

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.done()

   vn.run()

   -- If not accepted, misn_state will still be nil
   if enemy_ship==nil then
      return
   end

   -- Accept mission
   misn.accept()

   -- Set details
   misn.setDesc( misn_desc )
   misn.setReward( creditstring(rewardcredits) )
   misn.setTitle( misn_title )

   -- Add to log
   shiplog.createLog( logidstr, logname, logtype )

   -- Create the OSD
   osd = misn.osdCreate( _("Totoran Tournament"),
         { _("Defeat all the other adversaries!") } )

   hook.safe("enter_the_ring")
   player.allowSave( false ) -- Don't want to save the mission
   player.takeoff() -- take off and enter the ring!
end
function enter_the_ring ()
   -- Teleport the player to the Coliseum and hide the rest of the universe
   local sys = coliseum
   hook.enter("enter")
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)
   player.pilot():setPos( vec2.new( 0, 0 ) )
   player.allowSave(true)
   player.teleport(coliseum)
end
function leave_the_ring ()
   local sys = coliseum
   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   hook.land("land")
   player.land( planet.get("Totoran") )
end


function enter ()
   if system.cur() ~= system.get("Coliseum") then
      return
   end

   -- Get rid of pilots
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Set marker
   start_pos = vec2.new( 0, 0 )
   system.mrkAdd( _("Starting Position"), start_pos )
   hook.timer( 500, "heartbeat" )

   -- Metafactions
   local fact = faction.dynAdd( "Mercenary", "Combatant", _("Combatant") )
   enemies = {}
   local function addenemy( shipname, name )
      local pos = vec2.new( -500, 500 )
      local p = pilot.add( shipname, fact, pos, name )
      p:setInvincible(true)
      p:control(true)
      p:brake()
      hook.pilot( p, "disable", "p_disabled" )
      hook.pilot( p, "death", "p_death" )
      table.insert( enemies, p )
   end

   addenemy( enemy_ship, _("Combatant") )
end
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
   if #enemies == 0 then
      misn_state = 3
      -- TODO play sound and cooler text
      player.omsgAdd( _("YOU ARE VICTORIOUS!"), 5 )
      shiplog.appendLog( logidstr, string.format(_("You defeated a %s in one-on-one combat."), enemy_ship) )
      hook.timer( 5000, "leave_the_ring")
   end
end
function p_disabled( p )
   p:disable()
   enemy_out( p )
end
function p_death( p )
   enemy_out( p )
end

function heartbeat ()
   local dist = player.pilot():pos():dist( start_pos )
   if dist > 3000 then
      hook.timer( 500, "heartbeat" )
      return
   end

   misn_state = 1

   player.autonavAbort(_("It is time to fight!"))
   hook.timer( 0*1000, "countdown", _("5…") )
   hook.timer( 1*1000, "countdown", _("4…") )
   hook.timer( 2*1000, "countdown", _("3…") )
   hook.timer( 3*1000, "countdown", _("2…") )
   hook.timer( 4*1000, "countdown", _("1…") )
   hook.timer( 5*1000, "countdown_done" )
end
function countdown( str )
   -- TODO play countdown sound
   player.omsgAdd( str, 1 )
end
function countdown_done ()
   -- TODO play sound and cooler text
   player.omsgAdd( _("FIGHT!"), 3 )
   misn_state = 2

   for k,p in ipairs(enemies) do
      p:setInvincible(false)
      p:setHostile(true)
      p:control(false)
   end
end


