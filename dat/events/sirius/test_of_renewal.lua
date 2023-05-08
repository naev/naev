--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Renewal">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Renewal</system>
</event>
--]]
local textoverlay = require "textoverlay"
--local chakra = require "luaspfx.chakra_explosion"
local srs = require "common.sirius"
local fmt = require "format"

local prevship
local reward = outfit.get("Astral Projection")
local obelisk = spob.get("Kal Niut Obelisk")

local hook_done, start_marker
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
      thrust_mod     = -30,
      speed_mod      = -30,
      turn_mod       = -30,
   }, true ) -- overwrite all
   local _n1, seeking_chakra = pp:outfitAdd( "Seeking Chakra" )
   local _n2, feather_drive = pp:outfitAdd( "Feather Drive" )
   pp:weapsetAdd( 1, seeking_chakra )
   pp:weapsetAdd( 2, feather_drive )
   pp:outfitAddIntrinsic( "Astral Flow Amplifier" )

   -- First puzzle
   local m = pilot.add("Psychic Orb", "Independent", vec2.new(), nil, {ai="dummy"} )
   m:setNoDeath(true)
   m:setNoDisable(true)
   m:setHostile(true)
   m:setInvisible(true)
   start_marker = m

   textoverlay.init( "#y".._("Test of Renewal").."#0",
      "#y".._("Collect All the Orbs").."\n"..
      fmt.f(_("{key1}: use seeking chakra\n{key2}: use feather drive"),
         {key1=naev.keyGet("weapset1"),key2=naev.keyGet("weapset2")}).."#0",
      { length=8 } )

   -- Player lost hooks
   hook.pilot( pp, "disable", "player_lost_disable" )
   hook.pilot( pp, "exploded", "player_lost" )

   -- TODO proper puzzle
   hook.custom( "puzzle01" )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )
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

function puzzle01( p )
   if p==start_marker then
      start_marker:rm()
      start_marker = nil
   end

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
