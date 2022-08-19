--[[
<?xml version='1.0' encoding='utf8'?>
<event name="NPC">
 <location>land</location>
 <chance>100</chance>
</event>
--]]

--[[
Event for creating random characters in the spaceport bar.
The random NPCs will tell the player things about the Naev universe in general,
about their faction, or about the game itself.
--]]
local vn = require 'vn'
local lf = require "love.filesystem"

-- luacheck: globals land npc_talk (NPC functions passed by name)

local npcs, npc_list, npc_spawners

function create()
   -- Try to load all the modular npc files
   npc_list = {}
   for k,v in ipairs(lf.enumerate("events/npc")) do
      table.insert( npc_list, require( "events.npc."..string.gsub(v,".lua","") ) )
   end

   hook.land( "land" )
   land()
end

function land ()
   -- Logic to decide what to spawn, if anything.
   local cur = spob.cur()

   -- Do not spawn any NPCs on restricted spobs or that don't want NPC
   local t = cur:tags()
   if t.nonpc then
      return
   end

   local total_w = 0
   npc_spawners = {}
   for k,v in ipairs(npc_list) do
      local s = v()
      if s then
         s.w = s.w or 1 -- default weight
         total_w = total_w + s.w
         table.insert( npc_spawners, s )
      end
   end
   table.sort( npc_spawners, function( a, b )
      return a.w > b.w
   end )

   local num_npc = rnd.rnd(1, 5)
   local w = 0
   npcs = {}
   for i=0, num_npc do
      local r = rnd.rnd() * total_w
      local npc
      for k,v in ipairs(npc_spawners) do
         w = w+v.w
         if r < w then
            npc = v.create()
            break
         end
      end

      if npc then
         local id = evt.npcAdd( "npc_talk", npc.name, npc.portrait, npc.desc, 10 )
         npcs[id] = npc
      else
         warn(_("NPC spawner failed to spawn NPC!"))
      end
   end
end

function npc_talk( id )
   local npcdata = npcs[id]

   vn.clear()
   vn.scene()
   local npc = vn.newCharacter( npcdata.name, { image=npcdata.image } )
   vn.transition()
   if npcdata.func then
      vn.func( function ()
         npcdata.func( npcdata )
      end )
   end
   npc( npcdata.msg )
   vn.run()
end
