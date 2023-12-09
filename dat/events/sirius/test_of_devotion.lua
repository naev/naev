--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Devotion">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Devotion</system>
</event>
--]]
local textoverlay = require "textoverlay"
local chakra = require "luaspfx.chakra_explosion"
local srs = require "common.sirius"
local fmt = require "format"

local prevship
local reward = outfit.get("Avatar of Sirichana")
local obelisk = spob.get("Kal Sitra Obelisk")
local survivetime = 180 -- in seconds

local hook_done
function create ()
   srs.obeliskEnter( obelisk )

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Greater", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new(0,-200) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      accel_mod      = -30,
      speed_mod      = -30,
      turn_mod       = -30,
   }, true ) -- overwrite all
   local _n1, cleansing_flames = pp:outfitAdd( "Cleansing Flames" )
   local _n2, astral_projection = pp:outfitAdd( "Astral Projection" )
   pp:outfitAddIntrinsic( "Astral Flow Amplifier" )
   srs.weapsets{ cleansing_flames, astral_projection }

   textoverlay.init( "#y".._("Test of Devotion").."#0",
      "#y"..fmt.f(_("Survive for {amt} seconds"),{amt=survivetime}).."\n"..
      fmt.f(_("{key1}: use cleansing flames\n{key2}: use astral projection"),
         {key1=naev.keyGet("weapset1"),key2=naev.keyGet("weapset2")}).."#0",
      { length=8 } )

   -- Player lost hooks
   hook.pilot( pp, "disable", "player_lost_disable" )
   hook.pilot( pp, "exploded", "player_lost" )

   -- Spawn at constant interval
   hook.timer( 8, "spawn" ) -- Be nice at first
   hook.timer( 180, "eventover" ) -- Player one

   -- Anything will finish the event
   hook_done = hook.enter( "done" )
end

function puzzle01 ()
   hook.timer( 0.5, "puzzle01" )
end

local end_hook
function player_lost_disable ()
   local pp = player.pilot()
   pp:setInvincible( true )
   pp:setInvisible( true )
   if not end_hook then
      textoverlay.init( "#r".._("Test Failed").."#0", nil, {length=6})
      end_hook = hook.timer( 6, "cleanup" )
   end
end
function player_lost ()
   local pp = player.pilot()
   pp:setHealth( 100, 100 ) -- Heal up to avoid game over if necessary
   pp:setHide( true )
   pp:setInvincible( true )
   if not end_hook then
      textoverlay.init( "#r".._("Test Failed").."#0", nil, {length=6})
      end_hook = hook.timer( 6, "cleanup" )
   end
end

local baddies = {}
local noadd = false
function spawn ()
   if noadd then return end

   local newbaddies = {}
   for k,p in ipairs(baddies) do
      if p:exists() then
         newbaddies[ #newbaddies+1 ] = p
      end
   end
   baddies = newbaddies

   -- Too many, so just wait a bit
   if #baddies > 40 then
      hook.timer( 1, "spawn" )
      return
   end

   -- Spawn an enemy
   local pos = player.pos() + vec2.newP( 800+400*rnd.rnd(), rnd.angle() )
   local e = pilot.add( "Astral Projection Lesser", _("Independent"), pos, nil, {ai="baddie_norun"})
   e:effectAdd("Astral Projection")
   e:setHostile(true)
   e:setVisible(true)
   e:setNoDisable(true)
   e:intrinsicSet( { -- Ship is too fast otherwise
      accel_mod      = -30,
      speed_mod      = -30,
      turn_mod       = -30,
      fwd_damage     = -20, -- Don't instagib player
      fwd_firerate   = -50, -- Less spam
      armour_mod     = -90,
      shield_mod     = -90,
   }, true ) -- overwrite all
   local emem = e:memory()
   emem.comm_no = _("No response.")

   -- Explosion creation effect
   chakra( pos, vec2.new(), 20 ) -- TODO sound?

   -- Keep track of them
   table.insert( baddies, e )

   -- Figure out when to respawn
   local nexttime = 3
   if #baddies < 20 then
      nexttime = nexttime * 0.5
   end
   hook.timer( nexttime, "spawn" ) -- Going to rerun in a bit
end

function eventover ()
   -- Get rid of enemies
   for k,v in ipairs(baddies) do
      if v:exists() then
         v:setHealth( -1, -1 )
      end
   end
   noadd = true

   -- All done, so give ability
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
