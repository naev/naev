--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Ship Repairs">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[

   Event to let the player repair ships that they captured.

--]]
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"

local NAME = _("Ship Repairer")
local OUTFIT = outfit.get("Heavily Damaged")

function create ()
   hook.land("land")
   hook.safe("land")
end

local function is_damaged( name )
   for i,o in ipairs(player.shipOutfits( name )) do
      if o==OUTFIT then
         return true
      end
   end
   return false
end

local function repairable_ships ()
   local repairables = {}
   if is_damaged() then
      local pp = player.pilot()
      table.insert( repairables, {
         name = pp:name(),
         ship = pp:ship(),
         deployed = true,
      } )
   end
   for k,s in ipairs(player.ships()) do
      if is_damaged( s.name ) then
         table.insert( repairables, s )
      end
   end
   return repairables
end

local img, prt, npcid
function land ()
   local scur = spob.cur()
   if not scur or not scur:services().shipyard then return end

   if #repairable_ships() <= 0 then return end

   img, prt = vni.generic()
   npcid = evt.npcAdd( "approach", NAME, prt, _("A ship handyperson who may be able to repair your #oHeavily Damaged#0 ships that you captured.") )
end

function approach ()
   vn.clear()
   vn.scene()
   local npc = vn.newCharacter( NAME, { image=img } )
   vn.transition()
   npc(_([["Hey, it looks like you have some #oHeavily Damaged#0 ships. I can fix them up for you."]]))

   vn.label("menu")
   npc( function ()
      return fmt.f(_([["Which ship do you wish to repair?"

You have {amount}.]]), {
      amount = fmt.credits(player.credits()),
   } )
   end )
   local tmp_ship, noshipsleft
   vn.menu( function ()
      local opts = { { _("Nevermind."), "cancel" } }
      local repairs = repairable_ships()
      for k,s in ipairs( repairs ) do
         local cost = player.shipvarPeek( "repair_cost", s.name )
         local c = ""
         if cost > player.credits() then
            c = "#r"
         end
         table.insert( opts, 1, { fmt.f(_([[{name} ({ship}, {cost})]]), {
            name = s.name,
            ship = s.ship,
            cost = c..fmt.credits(cost).."#0",
         } ), k  } )
      end
      return opts
   end, function ( selection )
      if type(selection)=="string" then
         return vn.jump(selection)
      end
      local repairs = repairable_ships()
      local r = repairs[selection]
      local cost = player.shipvarPeek( "repair_cost", r.name )
      if player.credits() < cost then
         tmp_ship = r
         tmp_ship.cost = cost
         return vn.jump("broke")
      end

      player.pay( -cost )
      local curship = player.pilot():name()
      if curship==r.name then
         player.pilot():outfitRmIntrinsic( OUTFIT )
      else
         player.shipSwap( r.name, true )
         player.pilot():outfitRmIntrinsic( OUTFIT )
         player.shipSwap( curship, true )
      end
   end )
   vn.na(_([[You pay the heft sum, and watch via remote holo-vid how they do thorough repairs on your ships as you sip drinks idly in the bar. Eventually they come back covered in sweat after a job well down, and down a pitcher. Work hard, play hard you guess.]]))
   vn.func( function ()
      if #repairable_ships() > 0 then
         return vn.jump("menu")
      end
      noshipsleft = true
   end )
   npc(_([["Looks like you have to ships lefts to repair."

They get up and leave the spaceport bar whistling to themselves.]]))
   vn.done()

   vn.label("broke")
   vn.na( function ()
      return fmt.f(_([[You do not have enough credits to repair the {name}. You need {cost}, but only have {owned}.]]), {
         name = tmp_ship.name,
         cost = fmt.credits(tmp_ship.cost),
         owned = fmt.credits(player.credits()),
      } )
   end )
   vn.jump("menu")

   vn.label("cancel")
   vn.run()

   -- Remove
   if noshipsleft then
      evt.npcRm( npcid )
   end
end
