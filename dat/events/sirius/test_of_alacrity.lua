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
   pp:setPos( vec2.new(0,-500) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      thrust_mod     = -30,
      speed_mod      = -30,
      turn_mod       = -30,
   }, true ) -- overwrite all

   -- First puzzle
   -- TODO much better generation scheme
   markers = {}
   local n = 5
   for i=1,n do
      local dir = math.pi*0.5 + (i-1)/n*math.pi*2.0
      local pos = vec2.newP( 200, dir )
      local m = pilot.add("Psychic Orb", "Independent", pos, nil, {ai="dummy"} )
      m:setInvincible(true)
      m:setInvisible(true)
      m:effectAdd("Psychic Orb On")
      table.insert( markers, m )
   end

   textoverlay.init( "#y".._("Test of Alacrity").."#0",
      "#y".._("Collect All the Orbs").."#0" )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )

   hook.timer( 0.1, "puzzle01" )
end

function puzzle01 ()
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
   srs.obeliskCleanup( cleanup_player )
end
