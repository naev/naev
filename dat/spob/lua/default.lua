local vn = require 'vn'
local fmt = require "format"
local ccomm = require "common.comm"

local land_spb, land_fct, bribed

function load( spb )
   land_spb = spb
   land_fct = spb:faction()
   bribed = false
   -- Don't return anything to make the code handle it
end

function can_land ()
   if bribed then
      return true, _("Make it quick.")
   end
   if land_fct:playerStanding() < 0 then
      return false, ("Landing request denied.")
   end
   return true, _("Permission to land granted.")
end

function comm ()
   print("yeeah")

   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, land_spb, bribed )
   vn.transition()

   vn.label("menu")
   vn.menu( function ()
      local opts = { _("Leave"), "leave" }
      if land_fct:playerStanding() < 0 then
         table.insert( opts, { _("Bribe"), "bribe" } )
      end
      return opts
   end )

   local bribe_cost
   vn.label("bribe")
   vn.func( function ()
      local std = land_fct:playerStanding()
      if std < -30 then
         vn.jump("dangerous")
         return
      end
      bribe_cost = -std * 1e3 * player.pilot():ship():size() + 5e3
   end )
   spb(_([["I'll let you land for the modest price of {credits}."

Pay {credits}?]]),
      {credits=fmt.credits( bribe_cost )} )
   vn.menu{
      { fmt.f(_("Pay {credits}"),{credits=fmt.credits( bribe_cost )}), "bribe_yes" },
      { _("Refuse"), "bribe_no" },
   }

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

   vn.label("dangerous")
   spb(_([["I'm not dealing with dangerous criminals like you!"]]))
   vn.jump("menu")

   vn.label("leave")
   vn.run()
end
