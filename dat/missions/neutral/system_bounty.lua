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

local REWARD = 200

function create ()
   local nc = naev.cache()
   local sb = nc._system_bounty

   mem.sys = sb.sys
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

function accept ()
   misn.accept()
   misn.osdCreate( _("System Bounty"), {
      fmt.f(_("Travel to the {sys} system"), {sys=mem.sys}),
      _("Get rid of pirates"),
   } )

   hook.enter("enter")
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
end

function try_give_bounty( plt, attacker )
   -- Handles the case the player boarded to permanently disable
   if not plt:hostile() then return end
   -- Player must get killing blow
   if not attacker:withPlayer() then return end

   local points = plt:points()
   player.pay( points * REWARD )
   mem.fct:hit( points * 0.1, system.cur() )
   player.msg(fmt.f(_([[Obtained {amount} for eliminating {pilot}.]]), {
      amount = points * REWARD,
      pilot  = plt,
   }))
end
