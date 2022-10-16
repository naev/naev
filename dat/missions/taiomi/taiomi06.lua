--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 6">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 5</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 06

   Player has to destroy 37 ships in Bastion
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"

local reward = taiomi.rewards.taiomi06
local title = _("Human Extermination")
local base, basesys = spob.getS("One-Wing Goddard")
-- The systems below backtrack from Taiomi to Haven
local fightsys = system.get("Bastion")

local NUMBER_SHIPS = 37 -- Number of ships to kill

mem.pilots = {} -- stores damage done to ships, reset on enter
mem.killed = 0 -- number of ships killed

local function osd ()
   local left = NUMBER_SHIPS - mem.killed
   misn.osdCreate( title, {
      fmt.f(_("Destroy {total} ships in {sys} ({left} left)"),{sys=fightsys, total=NUMBER_SHIPS, left=left}),
      fmt.f(_("Return to {base} ({basesys})"),{base=base, basesys=basesys}),
   } )
   if left <= 0 then
      misn.osdActive(2)
   end
end

function create ()
   if not misn.claim( {fightsys}, true) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Destroy {num} ships in the {sys} system.

Only ships to which you or your fleet deal over 50% damage will count towards the number of ships destroyed.]]),
      {num = NUMBER_SHIPS, sys = fightsys} ))
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( base )

   osd()

   hook.enter( "enter" )
   hook.land( "land" )
end

-- Only count as player kill if player did >50% damage
function pilot_death( p, _attacker )
   if p:withPlayer() then
      return
   end

   local id = p:id()
   local pt = mem.pilots[id] or { player=0, nonplayer=0}

   if pt.player >= pt.nonplayer then
      mem.killed = mem.killed + 1
      osd()
      if (mem.killed >= NUMBER_SHIPS) and not mem.marker then
         mem.marker = misn.markerAdd( base )
      end
   end
end

-- Compare damage of player vs non-player
function pilot_attacked( p, attacker, dmg )
   if not attacker or not attacker:exists() then
      return
   end
   if p:withPlayer() then
      return
   end

   local id = p:id()
   local pt = mem.pilots[id] or { player=0, nonplayer=0}
   if attacker:withPlayer() then
      pt.player = pt.player + dmg
   else
      pt.nonplayer = pt.nonplayer + dmg
   end
   mem.pilots[id] = pt
end

function enter ()
   mem.pilots = {}

   -- Only set up hooks when necessary
   if system.cur() == fightsys then
      mem.hook_death = hook.pilot( nil, "death", "pilot_death" )
      mem.hook_attacked = hook.pilot( nil, "attacked", "pilot_attacked" )
   elseif mem.hook_death then
      hook.rm( mem.hook_death )
      hook.rm( mem.hook_attacked )
      mem.hook_death = nil
      mem.hook_attacked = nil
   end
end

function land ()
   if mem.killed < NUMBER_SHIPS then
      return -- Not done yet
   end

   vn.clear()
   vn.scene()
   vn.run()
end
