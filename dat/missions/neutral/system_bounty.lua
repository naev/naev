--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="System Bounty">
 <priority>3</priority>
 <chance>0</chance>
 <location>Computer</location>
</mission>
--]]
local fmt = require "format"
local lmisn = require "lmisn"
local pir = require "common.pirate"

local REWARD = 200

function create ()
   local nc = naev.cache()
   local sb = nc._system_bounty

   mem.sys = sb.sys
   mem.finish = sb.finish
   mem.fct = mem.sys:faction()
   if not mem.fct then return misn.finish(false) end

   local prefix = require("common.prefix").prefix(mem.fct)

   misn.setTitle(prefix..fmt.f(_("System Bounty in {sys}"), {sys=mem.sys}))
   misn.setDesc(fmt.f(_([[The local authoraties of the {sys} system have requested aid from the Astra Vigilis guild members to help deal with increased pirate raids. Payment will be per pirate ship destroyed or permanently incapacitated.

#nReputation Gained:#0 {fct}]]), {
      sys = mem.sys,
      fct = mem.fct,
   }))
   misn.setReward(fmt.f(_("{credits} per point of ship destroyed"), {
      credits = fmt.credits(REWARD),
   }))
   misn.setDistance( lmisn.calculateDistance( system.cur(), spob.cur():pos(), mem.sys ) )
   misn.markerAdd( mem.sys, "computer" )
end

local function update_osd()
   misn.osdCreate( _("System Bounty"), {
      fmt.f(_("Travel to the {sys} system"), {sys=mem.sys}),
      fmt.f(_("Get rid of pirates ({left} left)"), {
         left = mem.finish - time.cur(),
      } ),
   } )
   if system.cur() == mem.sys then
      misn.osdActive(2)
   end
end

function add_metadata ()
   local nc = naev.cache()
   local prm = nc._pirate_raid_mission or {}
   prm[ mem.sys:nameRaw() ] = true
   nc._pirate_raid_mission = prm
end

function accept ()
   misn.accept()

   hook.enter("enter")
   hook.date( time.new(0,1,0), "date" )
   hook.load( "add_metadata" )
   add_metadata()
end

function enter ()
   if system.cur() ~= mem.sys then
      misn.osdActive(1)
      return
   end
   misn.osdActive(2)

   mem.got_bounty = {}
   -- Hooks removed on changing system
   hook.pilot( nil, "board", "try_give_bounty" )
   hook.pilot( nil, "death", "try_give_bounty" )
   hook.pilot( nil, "create", "pilot_create" )
end

function pilot_create( plt )
   if not pir.factionIsPirate( plt:faction() ) then return end

   -- We want them to attack everywhere now
   local pm = plt:memory()
   pm.lanedistance = 0
end

function try_give_bounty( plt, attacker )
   -- Player must get killing blow
   if not attacker or not attacker:withPlayer() then return end
   -- Handles the case the player boarded to permanently disable
   if not plt:hostile() then return end
   -- Only pirates (for now?)
   if not pir.factionIsPirate( plt:faction() ) then return end

   local points = plt:points()
   player.pay( points * REWARD )
   mem.fct:hit( points * 0.1, system.cur() )
   player.msg(fmt.f(_([[Obtained {amount} for eliminating {pilot}.]]), {
      amount = fmt.credits(points * REWARD),
      pilot  = plt,
   }))
end

function abort ()
   local nc = naev.cache()
   local prm = nc._pirate_raid_mission or {}
   prm[ mem.sys:nameRaw() ] = nil
   nc._pirate_raid_mission = prm
   misn.finish(false)
end

function date ()
   if time.cur() > mem.finish then
      player.msg("#r"..fmt.f(_("The system bounty in {sys} has expired."), {
         sys = mem.sys,
      } ).."#0" )
      return abort()
   end
   update_osd()
end
