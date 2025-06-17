--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Treasure Hunt">
 <location>none</location>
 <priority>9</priority>
 <chance>0</chance>
</mission>
--]]
local th = require "common.treasure_hunt"
local luatk = require "luatk"
local lmap = require "luatk.map"
local vn = require "vn"
local fmt = require "format"
local loot = require "common.loot"

local view_maps
local MAP_WIDTH = 400
local MAP_HEIGHT = 400

local function update_desc ()
   local desc = _([[You have the following treasure maps:]])
   for k,v in ipairs(mem.maps) do
      desc = desc.."\n"..fmt.f(_(" * {mapname}"), {mapname=v.name})
   end
   misn.setDesc( desc )
end

function create ()
   misn.accept()

   misn.setTitle(_("Treasure Hunt"))
   misn.setReward(_("Unknown"))

   mem.maps = {}

   hook.load( "load" )
   hook.land( "land" )
   hook.custom( "treasure_hunt_add", "newmap" )
   load()
end

local btn
function load ()
   btn = player.infoButtonRegister( _("Treasure Maps"), view_maps )
   naev.cache().treasure_maps = #mem.maps
end

local function gen_map( data )
   return th.create_map( data, MAP_WIDTH, MAP_HEIGHT )
end

function view_maps ()
   local w, h = 1050, 500
   local BUTTON_W, BUTTON_H = 120, 30

   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newText( wdw, 0, 10, w, 20, _("Treasure Maps"), nil, "center" )
   local function wdw_close() wdw:destroy() end
   wdw:setCancel( wdw_close )
   luatk.newButton( wdw, w-20-BUTTON_W, h-20-BUTTON_H, BUTTON_W, BUTTON_H, _("Close"), wdw_close )

   local wmap = lmap.newMap( wdw, w-20-MAP_WIDTH, 30, MAP_WIDTH, MAP_HEIGHT )

   local img
   local function update( m )
      if img then img:destroy() end
      if m then
         img = luatk.newImage( wdw, w-20-MAP_WIDTH-20-MAP_WIDTH, 30, MAP_WIDTH, MAP_HEIGHT, gen_map(m) )
         wmap:center( m.start:pos() )
      else
         img = nil
      end
   end
   local lst, abandon
   local function gen_list()
      if lst then lst:destroy() end
      local mw, mh = w-60-MAP_WIDTH-20-MAP_WIDTH, h-50
      if #mem.maps <= 0 then
         lst = luatk.newList( wdw, 20, 20, mw, mh, {_("No maps")} )
         update()
         abandon:disable()
      else
         local maps = {}
         for k,v in ipairs(mem.maps) do
            table.insert( maps, v.name )
         end
         lst = luatk.newList( wdw, 20, 20, mw, mh, maps, function( _name, id )
            update( mem.maps[id] )
         end, 1 )
         update( mem.maps[1] )
      end
   end

   abandon = luatk.newButton( wdw, w-20-BUTTON_W-10-BUTTON_W, h-20-BUTTON_H, BUTTON_W, BUTTON_H, _("Abandon Map"), function ()
      local mapname, mapid = lst:get()
      luatk.yesno( _("Abandon Map?"), fmt.f(_("Are you sure you want to abandon the map '{mapname}'?"), {mapname=mapname}), function ()
         local m = mem.maps[mapid]
         if m.trigger then
            naev.trigger( m.trigger, true )
         end
         table.remove( mem.maps, mapid )
         naev.cache().treasure_maps = #mem.maps
         gen_list()
      end )
   end )
   gen_list()

   luatk.run()

   if #mem.maps <= 0 then
      hook.safe("cleanup")
   end
end

function newmap( data )
   table.insert( mem.maps, data )
   naev.cache().treasure_maps = #mem.maps
   update_desc()
end

local function landed( data )
   if data.trigger then
      th.map_completed()
      naev.trigger( data.trigger, true )
      return true
   end

   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[You land on {spob} that seems to match the treasure map you have.]]),
      {spob=spob.cur()}))
   vn.na(_([[Rerouting all ship power to sensors, you perform a scan of the surrounding area and are able to find a small capsule.]]))

   -- Handle reward
   local reward_str
   if not data.reward then
      local reward = loot.tier1()
      reward_str = reward
      vn.func( function ()
         player.outfitAdd(reward)
      end )
   else -- Defaults to Outfits
      reward_str = data.reward
      vn.func( function ()
         player.outfitAdd(data.reward)
      end )
   end

   vn.na(fmt.f(_([[You open up the capsule up to find {reward}.

{obtain}]]),
      {reward=reward_str, obtain=fmt.reward(reward_str)}))

   vn.run()

   -- Mark as completed
   th.map_completed()

   -- Log
   th.log( fmt.f(_([[You followed a treasure map to {spb} in the {sys} system and found {reward}.]]),
      {sys=system.cur(), spb=spob.cur(), reward=reward_str}) )

   return true
end

function land ()
   local scur = spob.cur()
   local torm = {}
   for k,v in ipairs(mem.maps) do
      if v.spb==scur then
         if landed( v ) then
            table.insert( torm, k )
         end
      end
   end
   -- Second pass to remove
   for i=#torm,1,-1 do
      table.remove( mem.maps, torm[i] )
   end
   naev.cache().treasure_maps = #mem.maps
end

function abort ()
   for k,v in ipairs(mem.maps) do
      if v.trigger then
         return naev.trigger( v.trigger, true )
      end
   end
   player.infoButtonUnregister( btn )
end

function cleanup ()
   abort()
   misn.finish(false)
end
