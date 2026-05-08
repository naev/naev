--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pirate System Raid">
 <location>none</location>
 <chance>0</chance>
 <unique />
</event>
--]]

--[[
   Manages pirate raids in the background.
--]]
local fmt = require "format"

local RAID_LENGTH       = time.new( 0, 60, 0 )
local UNIDIFF           = [[
<unidiff name="{name}">
 <system name="{sysname}">
  <spob_virtual_add>Pirate Raid</spob_virtual_add>
 </system>
</unidiff>
]]

local function diff_name ()
   return mem.sys:nameRaw().." Pirate Raid"
end

-- Create helper mission if applicable
function land ()
   local scur = spob.cur()
   local fct = scur:faction()
   if not fct or not fct:tags().generic then return false end

   -- TODO check if already accepted
   hook.safe( "mission_start" )
end

function mission_start ()
   local nc = naev.cache()
   nc._system_bounty = {
      sys      = mem.sys,
      finish   = mem.finish,
   }
   naev.missionStart("System Bounty")
   nc._system_bounty = nil
end

-- Need to regenerate the diff on start
function initialize ()
   local sys = mem.sys:nameRaw()
   diff.newDynamic( fmt.f( UNIDIFF, {
      name     = diff_name(),
      sysname  = sys,
   } ))

   local nc = naev.cache()
   local t = nc._pirate_raid_active or {}
   t[ sys ] = true
   nc._pirate_raid_active = t
end

function create ()
   evt.save(true)

   local nc = naev.cache()
   local pr = nc._pirate_raid
   mem.sys  = pr.sys
   mem.start= time.cur()
   mem.finish= mem.start + RAID_LENGTH

   -- Inclusive claim, should be tested already
   evt.claim( {mem.sys}, true )
   initialize()

   news.add{ {
      faction  = "Generic",
      head     = fmt.f(_("Pirate Raids in {sys}"), {sys=mem.sys}),
      body     = fmt.f(_("Reports say there seems to be a significant increase of pirate activity in the {sys} system. Local authorities request civilians to avoid the system until the increased hostilities cease, which are expected to continue until {date}."), {
         sys   = mem.sys,
         date  = mem.finish,
      }),
      date_to_rm = mem.finish,
   } }

   hook.load("initialize")
   hook.land("land")
   hook.date( time.new( 0, 1, 0 ), "date" )

   nc._pirate_raid = nil
end

function date ()
   if time.cur() < mem.finish then return end

   if system.cur() == mem.sys then
      player.msg(fmt.f(_("Pirates have stopped raiding {sys}."), {
         sys = mem.sys,
      } ) )
   end

   diff.remove( diff_name() )
   naev.cache()._pirate_raid_active[ mem.sys:nameRaw() ] = nil
   evt.finish(false) -- Don't want it to register as finished
end
