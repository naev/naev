local vn = require 'vn'
local fmt = require "format"
local ccomm = require "common.comm"

local luaspob = {}

local params, land_spb, land_fct, bribed, bribe_cost_function
local msg_bribed, msg_denied, msg_granted, msg_notyet, msg_cantbribe, msg_trybribe, msg_dangerous
local std_dangerous, std_land, std_bribe

function luaspob.init( spb, init_params )
   land_spb = spb
   params = init_params or {}
   std_land = params.std_land or 0 -- Needed for can_land
end

function luaspob.load ()
   -- Basic stuff
   land_fct = land_spb:faction()
   bribed = false

   bribe_cost_function = params.bribe_cost or function ()
      local std = land_fct:playerStanding()
      return (std_land-std) * 1e3 * player.pilot():ship():size() + 5e3
   end

   std_land = params.std_land or 0
   std_bribe = params.std_bribe or -30
   std_dangerous = params.std_dangerous or -30

   msg_bribed = params.msg_bribed or {
      _([["Make it quick."]]),
      _([["Don't let anyone see you."]]),
      _([["Be quiet about this."]]),
   }
   msg_denied = params.msg_denied or {
      _([["Landing request denied."]]),
      _([["Landing not authorized."]]),
      _([["Landing denied."]]),
   }
   msg_notyet = params.msg_notyet or msg_denied
   msg_granted = params.msg_granted or {
      _([["Permission to land granted."]]),
      _([["You are clear to land."]]),
      _([["Proceed to land."]]),
   }
   msg_cantbribe = params.msg_cantbribe or {
      _([["We do not accept bribes."]]),
   }
   msg_trybribe = params.msg_trybribe or {
      _([["I'll let you land for the modest price of {credits}."

Pay {credits}?]]),
      _([["Some {credits} would make me reconsider letting you land."

Pay {credits}?]]),
   }
   msg_dangerous = params.msg_dangerous or {
      _([["I'm not dealing with dangerous criminals like you!"]]),
   }

   -- Randomly choose
   local function choose( tbl )
      local msg = tbl[ rnd.rnd(1,#tbl) ]
      if type(tbl)=='function' then
         msg = msg()
      end
      return msg
   end
   msg_bribed     = choose( msg_bribed )
   msg_denied     = choose( msg_denied )
   msg_notyet     = choose( msg_notyet )
   msg_granted    = choose( msg_granted )
   msg_cantbribe  = choose( msg_cantbribe )
   msg_trybribe   = choose( msg_trybribe )
   msg_dangerous  = choose( msg_dangerous )
end

function luaspob.unload ()
   bribed = false
end

function luaspob.can_land ()
   local s = land_spb:services()
   if not s.land then
      return false,msg_denied
   end
   if bribed or land_spb:getLandOverride() then
      return true, msg_granted
   end
   local fct = land_spb:faction()
   if not fct then
      return false, msg_denied
   end
   local std = fct:playerStanding()
   if std < 0 then
      return false, msg_denied
   end
   if std < std_land then
      return false, msg_notyet
   end
   return true, msg_granted
end

function luaspob.comm ()
   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, land_spb, bribed )
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with the authorities at {spb}."),
      {spb=land_spb}))

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         { _("Close"), "leave" }
      }
      local std = land_fct:playerStanding()
      if std < std_land then
         table.insert( opts, 1, { _("Bribe"), "bribe" } )
      end
      return opts
   end )

   local bribe_cost
   vn.label("bribe")
   vn.func( function ()
      local std = land_fct:playerStanding()
      if std < std_dangerous then
         vn.jump("dangerous")
         return
      end
      if std < std_bribe then
         vn.jump("nobribe")
         return
      end
      bribe_cost = bribe_cost_function( land_spb )
   end )
   spb( function ()
      return fmt.f( msg_trybribe, {credits=fmt.credits( bribe_cost )} )
   end )
   vn.menu( function ()
      return {
         { fmt.f(_("Pay {credits}"),{credits=fmt.credits( bribe_cost )}), "bribe_yes" },
         { _("Refuse"), "bribe_no" },
      }
   end )

   vn.label("bribe_yes")
   vn.func( function ()
      if bribe_cost > player.credits() then
         vn.jump("player_broke")
         return
      end
      player.pay( -bribe_cost )
      bribed = true
      ccomm.nameboxUpdateSpob( land_spb, bribed )
   end )
   spb( msg_bribed )

   vn.label("player_broke")
   vn.na( function ()
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( bribe_cost - player.credits() )
      return fmt.f(_("You only have {credits} credits. You need #r{cdif}#0 more to be able to afford the bribe!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   vn.label("bribe_no")
   vn.na(_("You refuse to pay the bribe."))
   vn.jump("menu")

   vn.label("nobribe")
   spb( msg_cantbribe )
   vn.jump("menu")

   vn.label("dangerous")
   spb( msg_dangerous )
   vn.jump("menu")

   vn.label("leave")
   vn.run()
end

return luaspob
