--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Priority Bounty">
 <priority>3</priority>
 <chance>0</chance>
 <location>Computer</location>
</mission>
--]]
--[[
   Handles special scripted bounty missions in a general framework.

   Started from dat/events/priority_bounty/manager.lua
--]]
local fmt = require "format"
local lmisn = require "lmisn"
local bounty = require "common.bounty"

function create ()
   local nc = naev.cache()
   local sb = nc._priority_bounty
   mem.sb = sb

   misn.claim( {sb.system}, true )

   local prefix = (sb.payingfaction and require("common.prefix").prefix(sb.payingfaction)) or ""
   local title = prefix..fmt.f(_("Priority Bounty: {title}"), {title=sb.title})
   local desc = sb.desc.."\n"..fmt.f( [[
#nTarget:#0 {pilotname} ({shipclass}-class ship {escorts})
#nWanted:#0 {wanted}
#nLast seen:#0 {system} system]], {
      pilotname = sb.name,
      shipclass = sb.ships[1]:classDisplay(),
      escorts   = sb.escorts or "",
      wanted    = (sb.alive_only and _("Alive")) or _("Dead or Alive"),
      system    = sb.system,
   } )

   misn.setTitle( title )
   misn.setDesc( desc )
   misn.setReward( sb.reward )
   misn.setDistance( lmisn.calculateDistance( system.cur(), spob.cur():pos(), sb.system ) )

   -- Modify internals a bit
   sb.completefunc   = "finish"
   sb.osd_title      = sb.title

   -- Have to convert from a function to a global
   if sb.spawnfunc then
      sb.spawnfunc = "spawnfunc"
   end

   -- Set up bounty
   bounty.init( sb.system, sb.name, sb.ships, sb.reward, sb )
end

-- Basically a hack to convert the function in the data to a global function that the bounty script can call
function setup ()
   local b = require(mem.sb.filename)
   if b.spawnfunc then
      -- luacheck: globals spawnfunc
      spawnfunc = b.spawnfunc
   end
end

function accept ()
   bounty.accept()
   hook.load( "setup" )
   setup()
end

-- luacheck: globals finish
function finish ()
   var.push( mem.sb.var, true )
   return true -- Will end "normally"
end
