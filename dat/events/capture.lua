--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Ship Capture">
 <location>none</location>
 <chance>0</chance>
 <tags>
  <tag>fleetcap_10</tag>
 </tags>
</event>
--]]
--[[
   Capturing ships
--]]
local fmt = require 'format'
local vn = require 'vn'
local ccomm = require "common.comm"
local escapepod = require "outfits.lib.escape_pod"
local tut = require "common.tutorial"

local plt
local function setup_pilot( p )
   plt = p
   local pp = player.pilot()
   plt:setLeader( pp )
   plt:changeAI( "capture" )
   plt:rename( mem.name )
   plt:setDisable()
   plt:setNoClear()
   plt:setFriendly(true)
   plt:setInvincPlayer(true)
   plt:setFaction( faction.player() )
   plt:setNoBoard(true)
   plt:outfitRm("purge")
   plt:outfitRm("intrinsic")
   plt:outfitsEquip( mem.outfits )
   for k,v in ipairs(mem.intrinsics) do
      plt:outfitAddIntrinsic( v )
   end
   plt:weapsetCleanup() -- Hopefully won't do anything now
   plt:intrinsicSet( "nebu_absorb", 10e3 )

   -- Assume hooked up to the player for now
   -- Full mass is a bit too harsh due to quadratic penalty scaling, so reduce somewhat
   drag_change()

   -- Set hooks
   hook.pilot( plt, "death", "plt_death" )
   hook.pilot( plt, "hail", "plt_hail" )
end

function drag( dt )
   if not plt or not plt:exists() then return end

   local pp = player.pilot()
   local tdist = pp:radius() * 1.2 + 20
   local relpos = plt:pos() - pp:pos()
   local dist = relpos:dist()
   if dist < tdist then return end

   local mod = (tdist - dist) * pp:mass() / plt:mass()
   local pos = plt:pos()
   local vel = plt:vel()
   local acc = relpos:normalize() * mod
   vel = vel + acc * dt
   pos = pos + vel * dt
   plt:setPos( pos )
   plt:setVel( vel )

   -- TODO some sort of dragging visual, energy tether?
end

local oplt
function create ()
   local nc = naev.cache()
   oplt = nc.capture_pilot.pilot
   -- We clone the pilot to get a new ID and invalidate all references to the old one
   plt = oplt:clone()
   mem.cost       = nc.capture_pilot.cost
   mem.costnaked  = nc.capture_pilot.costnaked
   mem.outfitsnaked = nc.capture_pilot.outfitsnaked
   nc.capture_pilot = nil

   -- Free the followers!
   for k,f in ipairs(oplt:followers()) do
      if f:flags("carried") then
         -- Just disable and make them fade out. Not sure if anything else can be done here.
         f:setDisable()
         f:effectAdd("Fade-Out")
      else
         f:setLeader()
      end
   end

   -- Original data
   mem.o = {
      faction = oplt:faction(),
      name = oplt:name(),
   }

   -- Handle case of escape pod (launch and remove)
   local ep = outfit.get("Escape Pod")
   for k,o in ipairs(oplt:outfitsList("intrinsic")) do
      if o==ep then
         escapepod.launch(oplt)
         oplt:outfitRmIntrinsic(o)
      end
   end

   mem.name = fmt.f(_("Captured {shp}"), {shp=oplt:ship():name()} )
   mem.ship = oplt:ship()
   mem.outfits = oplt:outfits()
   for k,v in pairs(mem.outfits) do
      -- Ignore outfits that can't be stolen
      if v and v:tags().nosteal then
         mem.outfits[k] = false
      end
   end
   mem.system = system.cur()
   mem.intrinsics = {}
   for k,v in ipairs( oplt:outfitsList("intrinsic" ) ) do
      if not v:tags().nosteal then
         table.insert( mem.intrinsics, v )
      end
   end
   setup_pilot( plt )
   local pp = player.pilot()
   if pp:target()==oplt then
      -- Move the target over for the player only
      pp:setTarget(plt)
   end

   hook.land( "land" )
   hook.jumpout( "jumpout" )
   hook.enter( "enter" )
   hook.update( "drag" )
   hook.custom( "drag_change", "drag_change" )

   evt.save(true)

   -- Trigger capture hook
   naev.trigger( "capture", plt )
   hook.safe( "plt_remove" )
end

function plt_remove ()
   -- We drop the old pilot, thus invalidating references
   if oplt then
      oplt:rm()
      oplt = nil
   end
end

function plt_death ()
   player.msg(fmt.f(_("Your captured ship {shp} was destroyed!"),
      {shp=plt:ship():name()}))
   plt = nil
   player.pilot():intrinsicSet( "mass", 0 )
   naev.trigger( "drag_change" )
   evt.finish(false)
end

-- We handle mass updates here, because this is being hacked through instead of
-- being something more robust
function drag_change ()
   if plt:exists() then
      local m = plt:mass()
      player.pilot():intrinsicSet( "mass", m * 0.5 )
   end
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
      {"#r".._([[Abandon the ship.]]).."#0", "abandon_yes"},
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
      plt:setDisable()
      plt:setNoBoard(true)
      plt:setLeader()
      plt:setFaction( mem.o.faction )
      plt:rename( mem.o.name )
      plt:setFriendly(false)
      plt:setInvincPlayer(false)
      plt = nil
      player.pilot():intrinsicSet( "mass", 0 )
      naev.trigger( "drag_change" )
      evt.finish(false)
   end
end

function land ()
   local cur = spob.cur()
   if cur:services()['refuel'] and not cur:tags().noshipcapture then
      local repair = false
      local naked = false

      -- Success!
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(fmt.f(_([[You land on {spb} with your {shp}.]]),
         {spb=spob.cur(), shp=mem.ship}))
      vn.menu{
         {fmt.f(_([[Repair the {shp} for {amount}]]),{shp=mem.ship, amount=fmt.credits(mem.cost)}), "repair"},
         {fmt.f(_([[Repair the {shp} for {amount} (without outfits)]]),{shp=mem.ship, amount=fmt.credits(mem.costnaked)}), "repair_naked"},
         {fmt.f(_([[Keep the {shp} without repairing (without outfits)]]),{shp=mem.ship}), "norepair"},
      }

      vn.label("norepair")
      vn.na(fmt.f(_([[You decide to keep your captured {shp} without doing any repairs, at least the ship isn't lost. All the non-critical outfits are lost, but you may be able to fix it up so it is usable in the future.]]), {
         shp = mem.ship,
      }))
      vn.done()

      vn.label("broke")
      vn.na(fmt.f(_([[You sadly do not have enough to pay the reparation costs and your captured {shp}, but at least you can keep the ship in its shoddy state.]]), {
         shp = mem.ship,
      }))
      vn.done()

      vn.label("repair_naked")
      vn.func( function ()
         if player.credits() < mem.costnaked then
            vn.jump("broke")
            return
         end
         naked = true
         repair = true
         player.pay( -mem.costnaked )
      end )
      vn.sfxMoney()
      vn.na(fmt.f(_([[You pay {amt} to repair the {shp} to be good as new, minus the outfits.]]), {
         shp = mem.ship,
         amt = fmt.credits(mem.costnaked),
      }))
      vn.done()

      vn.label("repair")
      vn.func( function ()
         if player.credits() < mem.cost then
            vn.jump("broke")
            return
         end
         player.pay( -mem.cost )
         repair = true
      end )
      vn.sfxMoney()
      vn.na(fmt.f(_([[You pay {amt} to repair the {shp} to be good as new.]]), {
         shp = mem.ship,
         amt = fmt.credits(mem.cost),
      }))
      vn.done()

      vn.run()

      local newname = player.shipAdd( mem.ship, mem.name, fmt.f(_("You captured this ship in the {sys} system."), {sys=mem.system}) )
      -- Mark ship as captured, before swapping
      player.shipvarPush( "captured", true, newname )
      local name = player.pilot():name()
      player.shipSwap( newname, true )
      local pp = player.pilot()
      if repair then
         if naked then
            pp:outfitsEquip( mem.outfitsnaked )
         else
            pp:outfitsEquip( mem.outfits )
         end
         for k,v in ipairs(mem.intrinsics) do
            pp:outfitAddIntrinsic( v )
         end
      else
         pp:outfitRm( "purge" )
         pp:outfitAddIntrinsic( outfit.get("Heavily Damaged") )
         -- Store cost to repair for the future
         player.shipvarPush( "repair_cost", mem.costnaked )
      end
      player.shipSwap( name, true )

      -- First capture gives fleet capacity bonus
      if not player.evtDone("Ship Capture") then
         vn.clear()
         vn.scene()
         local sai = vn.newCharacter( tut.vn_shipai() )
         vn.transition( tut.shipai.transition )

         vn.na(_([[Your ship AI materializes in front of you.]]))
         sai(fmt.f(_([["Hello, {player}. I have been following your capturing of the {shp}. Following your technique, I have been able to adjust the fleet control hyper-parameters of your ship, potentially allowing you to have more ships deployed at once."]]),
            {player=player.name(), shp=mem.ship}))

         vn.done( tut.shipai.transition )
         vn.run()
      end

      plt = nil
      pp:intrinsicSet( "mass", 0 )
      evt.finish(true)
   end

   mem.spb = cur
end

local outsys
function jumpout ()
   outsys = system.cur()
end

function enter ()
   local spawn = nil
   if outsys then
      spawn = jump.get( system.cur(), outsys )
   elseif mem.spb then
      spawn = mem.spb
   end
   if not spawn then
      spawn = player.pos() + vec2.newP( 100+rnd.rnd(0,20), rnd.angle() )
   end
   outsys = nil
   mem.spb = nil
   local p = pilot.add( mem.ship, faction.player(), spawn, mem.name, {naked=true, ai="capture"} )
   setup_pilot( p )
end
