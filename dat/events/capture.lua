--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Ship Capture">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
--[[
   Capturing ships
--]]
local fmt = require 'format'
local vn = require 'vn'
local ccomm = require "common.comm"

local plt
local function setup_pilot( p )
   plt = p
   local pp = player.pilot()
   plt:setLeader( pp )
   plt:changeAI( "capture" )
   plt:rename( mem.name )
   plt:setNoClear()
   plt:setFriendly(true)
   plt:setFaction( faction.player() )
   plt:outfitRm("all")
   plt:outfitRm("intrinsic")
   plt:outfitsEquip( mem.outfits )
   for k,v in ipairs(mem.intrinsics) do
      plt:outfitAddIntrinsic( v )
   end
   plt:weapsetCleanup() -- Hopefully won't do anything now

   -- Set hooks
   hook.pilot( plt, "death", "plt_death" )
   hook.pilot( plt, "hail", "plt_hail" )
end

function create ()
   local nc = naev.cache()
   plt = nc.capture_pilot.pilot
   mem.cost = nc.capture_pilot.cost
   nc.capture_pilot = nil

   -- Original data
   mem.o = {
      faction = plt:faction(),
      name = plt:name(),
   }

   mem.name = fmt.f(_("Captured {shp}"), {shp=plt:ship():name()} )
   mem.ship = plt:ship()
   mem.outfits = plt:outfits()
   mem.system = system.cur()
   mem.intrinsics = plt:outfitsList("intrinsic")
   setup_pilot( plt )
   local a,s = plt:health()
   plt:setHealth( a, s ) -- Clears disabled state

   hook.land( "land" )
   hook.jumpout( "jumpout" )
   hook.enter( "enter" )

   evt.save(true)
end

function plt_death ()
   player.msg(fmt.f(_("Your captured ship {shp} was destroyed!"),
      {shp=plt:ship():name()}))
   evt.finish(false)
end

function plt_hail ()
   local abandon = false
   player.commClose()

   vn.reset()
   vn.scene()
   ccomm.newCharacter( vn, plt )
   vn.transition()

   vn.label("menu")
   vn.na(_([[What do you wish to do with your captured ship?]]))
   vn.menu{
      {_([[Abandon.]]), "abandon"},
      {_([[Close.]]), "close"},
   }

   vn.label("abandon")
   vn.na(_([[Are you sure you wish to abandon this captured ship? #rThis is irreversible.#0]]))
   vn.menu{
      {_([[Cancel.]]), "menu"},
      {_([[Abandon the ship.]]), "abandon_yes"},
   }

   vn.label("abandon_yes")
   vn.label(fmt.f(_([[You abandon the {ship}.]]),
      {ship=plt}))
   vn.func( function () abandon = true end )
   vn.done()

   vn.label("close")
   vn.run()

   -- Ship is abandoned
   if abandon then
      plt:disable()
      plt:setNoBoard(true)
      plt:setLeader()
      plt:setFaction( mem.o.faction )
      plt:rename( mem.o.name )
      plt:setFriendly(false)
      evt.finish(false)
   end
end

function land ()
   local cur = spob.cur()
   if cur:services()['refuel'] then
      local abandon = false

      -- Success!
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(fmt.f(_([[You land on {spb}.]]),
         {spb=spob.cur()}))
      vn.menu{
         {fmt.f(_([[Repair the {shp} for {amount}]]),{shp=mem.ship, amount=fmt.credits(mem.cost)}), "repair"},
         {fmt.f(_([[Abandon the {shp}]]),{shp=mem.ship}), "abandon"},
      }

      vn.label("abandon")
      vn.na(fmt.f(_([[You refuse to pay the reparation costs and your captured {shp} becomes destined to be scraps. Oh, well.]]),
         {shp=mem.ship}))
      vn.func( function () abandon = true end )
      vn.done()

      vn.label("broke")
      vn.na(fmt.f(_([[You sadly do not have enough to pay the reparation costs and your captured {shp} becomes destined to be scraps. Oh, well.]]),
         {shp=mem.ship}))
      vn.func( function () abandon = true end )
      vn.done()

      vn.label("repair")
      vn.func( function ()
         if player.credits() < mem.cost then
            vn.jump("broke")
            return
         end
         player.pay( -mem.cost )
      end )
      vn.sfxMoney()
      vn.na(fmt.f(_([[You pay {amt} to repair the {shp} to be good as new.]]),
         {shp=mem.ship, amt=fmt.credits(mem.cost)}))

      vn.run()

      if abandon then
         evt.finish(false)
         return
      end

      local newname = player.shipAdd( mem.ship, mem.name, fmt.f(_("You captured this ship in the {sys} system."), {sys=mem.system}) )
      local name = player.pilot():name()
      player.shipSwap( newname, true )
      player.pilot():outfitsEquip( mem.outfits )
      player.shipSwap( name, true )
      evt.finish(true)
   end
   mem.spb = cur
end

local outsys
function jumpout ()
   outsys = system.cur()
end

function enter ()
   local p
   if outsys then
      local j = jump.get( system.cur(), outsys )
      p = pilot.add( mem.ship, faction.player(), j )
      outsys = nil
   elseif mem.spb then
      p = pilot.add( mem.ship, faction.player(), mem.spb )
      mem.spb = nil
   else
      error(_("Unable to determine how to generate captured ship!"))
   end
   setup_pilot( p )
end
