local vn = require 'vn'
local lg = require 'love.graphics'
require 'numstring'
function comm( plt )
   local mem = plt:memory()
   local gfx = lg.newImage( plt:ship():gfxComm() )

   -- TODO display more information like faction logo, standing, etc...

   vn.clear()
   vn.scene()
   local p = vn.newCharacter( plt:name(), { image=gfx } )
   vn.transition()
   vn.na(string.format(_("You open a communication channel with %s."), plt:name()))
   vn.label("menu")
   vn.menu( function ()
      local hostile = plt:hostile()
      local opts = {
         {_("Close"), "close"},
      }
      if hostile and not plt:flags("bribed") then
         if mem.bribe_no then
            table.insert( opts, 1, {"Bribe", "bribe_no"} )
         else
            table.insert( opts, 1, {"Bribe", "bribe"} )
         end
      end
      if not hostile then
         table.insert( opts, 1, {"Request Fuel", "refuel"} )
      end
      return opts
   end )


   --
   -- Bribing
   --
   vn.label("bribe_no")
   p( mem.bribe_no )
   vn.jump("menu")

   vn.label("bribe_0")
   p(_([["Money won't save your hide now!"]]))
   vn.jump("menu")

   vn.label("bribe")
   vn.func( function ()
      if not mem.bribe then
         warn(string.format(_("Pilot '%s' accepts bribes but doesn't give a price!"), plt:name() ))
      end
      if mem.bribe == 0 then
         vn.jump("bribe_0")
      end
   end )
   p( function ()
      local str = mem.bribe_prompt
      local cost = mem.bribe
      local cstr = creditstring(cost)
      local chave = creditstring(player.credits())
      if str then
         return string.format(_("%s\n\nYou have %s.\nPay #r%s#0?"), str, chave, cstr )
      end
      return string.format(_([["I'm gonna need at least %s to not leave you as a hunk of floating debris."

You have %s.
Pay #0%s#0r?"]]), cstr, chave, cstr )
   end )
   vn.menu{
      {_("Pay"), "bribe_trypay"},
      {_("Refuse"), "bribe_refuse"},
   }

   vn.label("bribe_refuse")
   vn.na(_("You decide not to pay."))
   vn.jump("menu")

   vn.label("bribe_nomoney")
   vn.na( function ()
      local cstr = creditstring( player.credits() )
      local cdif = creditstring( mem.bribe - player.credits() )
      return string.format(_("You only have %s credits. You need #r%s#0 more to be able to afford the bribe!"), cstr, cdif )
   end )
   vn.jump("menu")

   vn.label("bribe_trypay")
   vn.func( function ()
      local cost = mem.bribe
      if cost > player.credits() then
         vn.jump("bribe_nomoney")
      end
   end )
   p(_([["Pleasure to do business with you."]]))
   vn.func( function ()
      local cost = mem.bribe
      player.pay( -cost, true )
      plt:credits( cost )
      plt:setBribed(true)
      plt:setHostile(false)
      mem.bribe = 0 -- Disable rebribes
   end )
   vn.jump("menu")

   --
   -- REFUELING
   --
   vn.label("refuel_no")
   p( function () return mem.refuel_no end )
   vn.jump("menu")

   vn.label("refuel_full")
   vn.na(_("Your fuel deposits are already full."))
   vn.jump("menu")

   vn.label("refuel_low")
   p(_([["Sorry, I don't have enough fuel to spare at the moment."]]))
   vn.jump("menu")

   vn.label("refuel_busy")
   p(_([["Sorry, I'm busy now."]]))
   vn.jump("menu")

   vn.label("refuel_refueling")
   vn.na(_("Pilot is already refueling you."))
   vn.jump("menu")

   vn.label("refuel_refuse")
   vn.na(_("You decide not to pay."))
   vn.jump("menu")

   vn.label("refuel")
   vn.func( function ()
      if mem.refuel_no then
         vn.jump("refuel_no")
      end
      local pps = player.pilot():stats()
      if pps.fuel >= pps.fuel_max then
         vn.jump("refuel_full")
      end
      local ps = plt:stats()
      if ps.fuel < 200 then
         vn.jump("refuel_low")
      end
      local msg = mem.refuel_msg
      local val = mem.refuel
      if val==nil or plt:flags("manualcontrol") then
         vn.jump("refuel_busy")
      end
   end )
   p( function ()
      local str = mem.refuel_msg
      local cost = mem.refuel
      local cstr = creditstring(cost)
      local chave = creditstring(player.credits())
      if str then
         return string.format(_("%s\n\nYou have %s.\nPay #r%s#0?"), str, chave, cstr )
      end
      return string.format(_([["I should be able to refuel you for %s."

You have %s.
Pay #r%s#0?]]), cstr, chave, cstr )
   end )
   vn.menu{
      {_("Pay"), "refuel_trypay"},
      {_("Refuse"), "refuel_refuse"},
   }

   vn.label("refuel_nomoney")
   vn.na( function ()
      local cstr = creditstring( player.credits() )
      local cdif = creditstring( mem.refuel - player.credits() )
      return string.format(_("You only have %s credits. You need #r%s#0 more to be able to afford the refueling!"), cstr, cdif )
   end )
   vn.jump("menu")

   vn.label("refuel_trypay")
   vn.func( function ()
      local cost = mem.refuel
      if cost > player.credits() then
         vn.jump("refuel_nomoney")
      end
   end )
   vn.func( function ()
      local cost = mem.refuel
      player.pay( -cost, true )
      plt:credits( cost )
      plt:refuel( player.pilot() )
   end )
   p(_([["On my way."]]))
   vn.jump("menu")

   vn.label("close")
   vn.run()
end

