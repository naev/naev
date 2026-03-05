
--[[
<?xml version='1.0' encoding='utf8'?>
<event name="P-1587 Dragon Intro">
 <location>land</location>
 <chance>100</chance>
 <spob>P-1587</spob>
 <unique />
</event>
--]]
local vn = require "vn"
local fmt = require "format"
local luatk = require "luatk"
local ferals = require "common.ferals"

local SPB, SYS = spob.getS("P-1587")
local GOODS    = commodity.get("Luxury Goods")
local MVAR_NAME= "visited_p1587_dragon"
local CHANCE   = 0.001 -- Very low chance

local WEAPON_TYPE_BROAD = {"Bolt Weapon", "Beam Weapon", "Launcher"}
local function has_weapon ()
   for k,w in ipairs(player.pilot():outfitsList("weapon")) do
      local tb = w:typeBroad()
      if inlist( WEAPON_TYPE_BROAD, tb ) then
         return true
      end
   end
   return false
end

function create ()
   -- We have to claim with the next event, inclusive should be fine
   if not evt.claim( SYS, true ) then
      return evt.finish(false)
   end

   local firsttime = (var.peek(MVAR_NAME) == nil)
   local taken = 0
   local started = false

   vn.clear()
   vn.scene()
   vn.transition()
   if firsttime then
      vn.na(fmt.f(_([[As you perform a premilinary scan of {spb}, it seems as there's a large abnormality on the planet's surface. Upon further inspection, it's a large pile of {goods}.]]),
         {spb=SPB, goods=GOODS}))
      vn.na(fmt.f(_([[A secondary scan confirms that your eyes aren't deceiving you, and there seems to be a lot of {goods} just abandoned on the surface. The amount is utterly ridiculous, and it seems it won't fit all in your ship, but you can try to take as much as you want.

What do you do?]]),
         {goods=GOODS}))
   else
      vn.na(fmt.f(_([[You return to the pile of {goods}, as impressive as ever.

What do you want to do?]]),
         {goods=GOODS}))
   end

   vn.menu( function ()
      local opts = {}
      if player.fleetCargoFree() > 0 then
         table.insert( opts, { fmt.f(_("Take {goods}"), {goods=GOODS}), "01_take" } )
         table.insert( opts, { _("Leave it alone."), "01_leave"} )
      else
         table.insert( opts, { _("Leave it alone. (No free space)"), "01_leave"} )
      end
      if has_weapon() then
         table.insert( opts, { _("Shoot at the pile."), "01_shoot" } )
      end
      return opts
   end )

   vn.label("01_leave")
   vn.na(_([[Some things are best not trifled with. You decide to leave the pile alone and take your leave.]]))
   vn.done()

   vn.label("01_take")
   local try_take = 0
   luatk.vn( function ()
      try_take = 0
      local max = player.fleetCargoFree()
      luatk.msgFader( _("Take how much?"),
            fmt.f(_("Take how many tonnes of {goods}?"),
               {goods=GOODS}), 0, max, max, function( val )
         try_take = val
      end )
   end )
   vn.func( function ()
      if try_take <= 0 then
         return vn.jump("01_leave")
      end
      local awoke = false
      taken = 0
      for i = 1,try_take do
         if rnd.rnd() < CHANCE then
            awoke = true
         end
         taken = taken+1
      end
      taken = player.fleetCargoAdd( GOODS, taken )
      if awoke then
         vn.jump("01_awoke")
      end
   end )
   vn.na( function () return fmt.f(_([[You load up on {amount} of {goods}. Seems like there's still plenty left.]]),
      {goods=GOODS, amount=taken}) end )
   vn.done()

   vn.label("01_awoke")
   vn.na(fmt.f(_([[You are loading the {goods} when you hear a large rumble. Wait, the mountain of {goods} is moving!]]),
      {goods=GOODS}))
   vn.sfx( ferals.sfx.spacewhale2 )
   vn.na(_([[You make a mad dash to your ships and scramble to your cockpit as the floor seems to collapse under you.]]))
   vn.jump("01_awoke_takeoff")

   vn.label("01_shoot")
   vn.na(fmt.f(_([[The moment your ship's weapon systems lock on the pile, the ground starts to shake. {spb} shouldn't have siesmic activity.]]),
      {spb=SPB}))
   vn.jump("01_awoke_takeoff")

   vn.label("01_awoke_takeoff")
   vn.na(fmt.f(_([[As you try to process the situation, there's an explosion of {goods}, which are now falling like rain. Seems like it's your call to get out of here.]]),
      {goods=GOODS}))
   vn.na(fmt.f(_([[You slam down on your thrusters and quickly leave {spb} behind, only for your ship's sensors to indicate you are being followed. What the hell is that?]]),
      {spb=SPB}))
   vn.func( function ()
      naev.eventStart("P-1587 Dragon Awake")
      player.takeoff()
      started = true
   end )
   vn.sfx( ferals.sfx.spacewhale1 )

   vn.run()

   var.push(MVAR_NAME, true)

   -- Will be marked as finished if awoken
   evt.finish(started)
end
