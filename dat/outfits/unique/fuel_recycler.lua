local fmt = require "format"
local luatk = require "luatk"
--local lg = require 'love.graphics'
--local audio = require 'love.audio'
local helper = require "outfits.lib.helper"

local active = 10 -- active time in seconds
local cooldown = 15 -- cooldown time in seconds

--local sfx = audio.newSource( 'snd/sounds/ping.ogg' )

local recyclables = {
   -- Rare
   ["Kermite"]    = 50,
   ["Therite"]    = 50,
   ["Vixilium"]   = 50,
   -- Uncommon
   ["Gold"]       = 15,
   ["Platinum"]   = 15,
   ["Rhodium"]    = 15,
   ["Yttrium"]    = 15,
   -- Common
   ["Clay"]       = 5,
   ["Iron"]       = 5,
   ["Nickel"]     = 5,
   ["Silicate"]   = 5,
   ["Olivine"]    = 5,
   ["Water"]      = 5,
}

local function recycle_interface ()
   local recycled = 0

   local function cargo_list ()
      local function can_recycle( c )
         return recyclables[ c:nameRaw() ]
      end

      local clist = {}
      for k,v in ipairs(player.fleetCargoList()) do
         local amount = can_recycle( v.c )
         if amount then
            v.a = amount
            table.insert( clist, v )
         end
      end
      table.sort( clist, function ( a, b )
         return a.a < b.a
      end )

      local cnames = {}
      for k,v in ipairs(clist) do
         cnames[k] = fmt.f(_("{cargo} ({amount} fuel/t, {tonnes})"),
            {amount=v.a, cargo=v.c, tonnes=fmt.tonnes_short(v.q)})
      end
      return clist, cnames
   end

   local clist, cnames = cargo_list ()
   if #clist <= 0 then
      return -1
   end

   local w, h = 450, 400
   local lstw = 250
   local wdw = luatk.newWindow( nil, nil, w, h )

   luatk.newText( wdw, 0, 10, w, 20, _("Fuel Recycler"), nil, "center" )

   local txt = luatk.newText( wdw, lstw+40, 65, w-lstw-40-20, h-65-20, "" )
   local function update_text( idx )
      local c = clist[ idx ]

      local pps = player.pilot():stats()
      local d = c.c:name().."\n"
      d = d.."#n".._("Amount: ").."#0"..fmt.tonnes_short(c.q).."\n"
      d = d.."#n".._("Fuel: ").."#0"..fmt.f(_("{amount} fuel/t"),{amount=c.a}).."\n"
      d = d.."#n".._("Price: ").."#0"..fmt.credits(c.c:price()).."\n\n"

      d = d.."#n".._("Fuel: ").."#0"..fmt.f(_("{fuel} / {fuel_max}"),{fuel=pps.fuel, fuel_max=pps.fuel_max}).."\n"
      d = d.."#n".._("Jumps: ").."#0"..tostring(pps.jumps).."\n"
      txt:set( d )
   end

   luatk.newText( wdw, 20, 40, w-40, 20, _("Select cargo to recycle:") )
   local lst = luatk.newList( wdw, 20, 65, lstw, h-65-20, cnames, function ( _csel, idx  )
      update_text( idx )
   end )
   update_text( 1 )

   luatk.newButton( wdw, w-20-140, h-20-30, 140, 30, _("Close"), luatk.close )
   local btn_recycle
   btn_recycle = luatk.newButton( wdw, w-20-140, h-20-30-10-30, 140, 30, _("Recycle"), function ()
      local _c, id = lst:get()
      local c = clist[ id ]

      local pps = player.pilot():stats()
      local max = math.min( math.ceil( (pps.fuel_max - pps.fuel) / c.a ), c.q )

      luatk.msgFader( fmt.f(_("Recycle {cargo}"),{cargo=c.c}),
         fmt.f(_("How many tonnes of {cargo} do you wish to recycle?"),{cargo=c.c}), 1, max, 10, function( val )
         if not val then
            return
         end
         val = math.floor( val + 0.5 )
         luatk.yesno( fmt.f(_("Recycle {cargo}?"), {cargo=c.c}),
            fmt.f(_("Are you sure you want to recycle {tonnes} of {cargo} to obtain {fuel} fuel?"),
               {tonnes=fmt.tonnes(val), cargo=c.c, fuel=c.a*val} ),
            function ()
               local q = player.fleetCargoRm( c.c, val )
               recycled = recycled + q
               player.pilot():setFuel( pps.fuel + c.a * q )
               luatk.msg( fmt.f(_("Bye Bye {cargo}, Hello Fuel"),{cargo=c.c} ),
                  fmt.f(_("You activate the Fuel Recycler and convert {tonnes} of {cargo} to {fuel} fuel."),
                     {tonnes=fmt.tonnes(q), cargo=c.c, fuel=c.a*val}) )

                  lst:destroy()
                  clist, cnames = cargo_list ()
                  if #clist <= 0 then
                     cnames = { _("None") }
                     btn_recycle:disable()
                  end
                  lst = luatk.newList( wdw, 20, 65, lstw, h-65-20, cnames, function ( _csel, idx  )
                     update_text( idx )
                  end )
                  update_text( 1 )
            end )
      end )
   end )
   luatk.run()

   return recycled
end


local function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end

   -- Only player can use
   if not mem.isp then return false end

   local ps = p:stats()
   if ps.fuel >= ps.fuel_max then
      helper.msgnospam("#r".._("You already have maximum fuel!").."#0")
      return false
   end

   local ret = recycle_interface ()
   if ret < 0 then
      helper.msgnospam("#r".._("You do not have any cargo you can recycle.").."#0")
      return false
   elseif ret == 0 then
      return false -- Didn't do anything
   end

   po:state("on")
   po:progress(1)
   mem.timer = active
   mem.active = true

   return true
end

local function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   mem.active = false
   return true
end

function init( p, po )
   turnoff( p, po )
   mem.timer = nil
   po:state("off")
   mem.isp = (p == player.pilot())
end

function update( p, po, dt )
   if not mem.timer then return end

   mem.timer = mem.timer - dt
   if mem.active then
      po:progress( mem.timer / active )
      if mem.timer < 0 then
         turnoff( p, po )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      mem.lastmsg = nil -- clear helper.msgnospam timer
      return turnoff( p, po )
   end
end
