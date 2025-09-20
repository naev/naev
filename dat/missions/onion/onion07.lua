--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 07">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <done>Onion Society 06</done>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion 07
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local trigger = require "trigger"
local fleet = require "fleet"
local lmisn = require "lmisn"
local pilotai = require "pilotai"
--local tut = require "common.tutorial"
--local lg = require "love.graphics"

-- Reference to honeypot (trap)
local title = _("Onion and Honey")

-- Mission states
local STATE_SET_UP_HONEYPOT = 1
local STATE_FINISH_SCANS = 2
local STATE_BEAT_MERCENARIES = 3
mem.state = nil

-- Candidates are somewhat uninhabited spobs along the main trade lanes
local TARGETSYS_CANDIDATES = {
   system.get("Gremlin"),
   system.get("Overture"),
   system.get("Daan"),
   system.get("Santoros"),
   system.get("Fidelis"),
}

local SHIPS_TO_SCAN = 10 -- Ships it says to scan
local SHIPS_TO_SCAN_REAL = 3 -- The true amount after which the player is attacked

function create()
   misn.finish(false) -- Disabled for now

   -- Try to find a closeby acceptable target
   local targets = {}
   for t in ipairs(TARGETSYS_CANDIDATES) do
      if naev.claimTest( t ) and t:jumpDist() < 6 then
         table.insert( targets, t )
      end
   end
   if #targets <= 0 then return misn.finish(false) end
   mem.targetsys = targets[ rnd.rnd(1, #targets) ]

   -- Need to soft claim
   if not misn.claim( mem.targetsys, false ) then misn.finish(false) end

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )
   misn.setNPC( _("l337_b01"), prt.t.tex, _([[Try to get in touch with l337_b01.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(_([[TODO.]]))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01{pos="left"}
   vn.newCharacter( l337 )
   vn.music( onion.loops.hacker ) -- TODO different music
   vn.transition("electric")

   vn.na(fmt.f())

   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()

   misn.osdCreate( title, {
      fmt.f(_("Go to the location in the {sys} system"),
         {sys=mem.targetsys}),
      _("Scan ships in the system"),
   } )
   hook.enter("enter")
end

local ships_scanned = {}
function enter ()
   local scur = system.cur()
   if scur==mem.targetsys and mem.state==nil then

      -- Try to get a good position
      local rep = 0
      local good
      local position
      local function good_position( _pos )
         return true
      end
      repeat
         position = vec2.newP( 2/3*scur.radius(), rnd.angle() )
         good = good_position(position)
         rep = rep+1
      until good or rep > 100

      mem.honeypot = position
      mem.sysmarker = system.markerAdd( position, _("Honeypot") )

      trigger.player_distance( position, 2000, function ()
         vn.clear()
         vn.scene()
         local l337 = vn.newCharacter( onion.vn_l337b01() )
         vn.music( onion.loops.hacker ) -- TODO different music
         vn.transition("electric")

         l337()

         vn.done("electric")
         vn.run()

         mem.state = STATE_SET_UP_HONEYPOT
         ships_scanned = {}
         system.markerRm( mem.sysmarker )

         hook.pilot( player.pilot(), "scan", "scan" )
      end )
   elseif mem.state~=STATE_BEAT_MERCENARIES then
      -- Reset state
      mem.state = nil
   end
end

local function spawn_baddies()
   -- Clear pilots
   pilotai.clear()

   -- Spawn new ones and send them towards the player
   local baddies = fleet.spawn({
   }, "Mercenary", lmisn.nearestJump() )
   for k,p in ipairs(baddies) do
      p:setHostile(true)
   end
   pilotai.guard( baddies, mem.honeypot )
   baddies[1]:setHilight(true)
   baddies[1]:setVisplayer(true)
   baddies[1]:rename(_("Mercenary Boss"))

   -- Finish when beaten
   trigger.pilots_defeated( baddies, function ()
      mem.state = STATE_BEAT_MERCENARIES
      misn.osdCreate( title, {
         _("Land to speak with l337_b01."),
      } )
      hook.land( "land" )
   end  )

   hook.timer( 5, "announce_baddies" )
end

function announce_baddies ()
   -- TODO some discussion

   -- Update
   misn.osdCreate( title, {
      _("Defeat the mercenaries!"),
   } )
end

function scan( tgt )
   if mem.state~=STATE_SET_UP_HONEYPOT then
      return
   end

   if not inlist( ships_scanned, tgt ) then
      table.insert( ships_scanned, tgt )
   end
   misn.osdCreate( title, {
      fmt.f(_("Scan {n} ships in the system ({left} left)"),
         {n=SHIPS_TO_SCAN, left=SHIPS_TO_SCAN-#ships_scanned}),
   } )
   if #ships_scanned > SHIPS_TO_SCAN_REAL then
      spawn_baddies()
      mem.state = STATE_FINISH_SCANS
   end
end

function land ()


   onion.log(_([[TODO]]))

   misn.finish(true)
end
