local vn = require 'vn'
local lg = require 'love.graphics'
require 'numstring'

-- Get nearby pilots for bribing
local function nearby_bribeable( plt )
   local pp = player.pilot()
   local bribeable = {}
   for k,v in ipairs(pp:getVisible()) do
      if v~=plt and (v:faction() == plt:faction() and v:hostile()) then
         local vmem = v:memory()
         if not vmem.bribe_no and (vmem.bribe and vmem.bribe~=0) then
            table.insert( bribeable, v )
         end
      end
   end
   return bribeable
end

local function bribe_cost( plt )
   local mem = plt:memory()
   if not mem.bribe then
      warn(string.format(_("Pilot '%s' accepts bribes but doesn't give a price!"), plt:name() ))
      return 1e6 -- just ridiculous for now, players should report it
   end
   return mem.bribe
end

function comm( plt )
   local mem = plt:memory()
   local gfx = lg.newImage( plt:ship():gfxComm() )

   -- TODO display more information like faction logo, standing, etc...
   local nw, nh = naev.gfx.dim()
   vn.menu_x = math.min( -1, 500 - nw/2 )
   --nw - nw/2 - 100

   vn.clear()
   vn.scene()
   local p = vn.newCharacter( plt:name(), { image=gfx } )
   vn.transition()
   if mem.comm_greet then
      p( mem.comm_greet )
   else
      vn.na(string.format(_("You open a communication channel with %s."), plt:name()))
   end
   vn.label("menu")
   vn.menu( function ()
      local hostile = plt:hostile()
      local opts = {
         {_("Close"), "close"},
      }
      if hostile and not plt:flags("bribed") then
         if mem.bribe_no then
            table.insert( opts, 1, {"Bribe", "bribe_no"} )
         elseif mem.bribe and mem.bribe == 0 then
            table.insert( opts, 1, {"Bribe", "bribe_0"} )
         else
            bribeable = nearby_bribeable( plt ) -- global
            if #bribeable > 0 then
               table.insert( opts, 1, {string.format(_("Bribe %d nearby %s pilots"), #bribeable+1, plt:faction():name()), "bribe_nearby"} )
            end
            table.insert( opts, 1, {_("Bribe"), "bribe"} )
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
   p( function ()
      local cost = bribe_cost( plt )
      local str = mem.bribe_prompt
      local cstr = creditstring(cost)
      local chave = creditstring(player.credits())
      if not str then
         str = string.format(_([["I'm gonna need at least %s to not leave you as a hunk of floating debris."]]), cstr)
      end
      return string.format(_("%s\n\nYou have %s. Pay #r%s#0?"), str, chave, cstr )
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
      local cost = bribe_cost( plt )
      if cost > player.credits() then
         vn.jump("bribe_nomoney")
      end
   end )
   p( function ()
      if mem.bribe_paid then
         return mem.bribe_paid
      end
      return _([["Pleasure to do business with you."]])
   end )
   vn.func( function ()
      local cost = bribe_cost( plt )
      player.pay( -cost, true )
      plt:credits( cost )
      plt:setBribed(true)
      plt:setHostile(false)
      mem.bribe = 0 -- Disable rebribes
   end )
   vn.jump("menu")

   vn.label("bribe_nearby")
   p( function ()
      local cost = bribe_cost( plt )
      for k,v in ipairs(bribeable) do
         cost = cost + bribe_cost( v )
      end
      bribe_nearby_cost = cost -- save as global again
      local str = mem.bribe_prompt_nearby
      local cstr = creditstring(cost)
      local chave = creditstring(player.credits())
      if not str then
         str = _([["We'll need at least %s to not leave you as a hunk of floating debris."]])
      end
      str = string.format( str, cstr )
      return string.format(_("%s\n\nThis action will bribe %d %s pilots.\nYou have %s. Pay #r%s#0?"), str, #bribeable+1, plt:faction():name(), chave, cstr )
   end )
   vn.menu{
      {_("Pay"), "bribe_nearby_trypay"},
      {_("Refuse"), "bribe_refuse"},
   }

   vn.label("bribe_nearby_nomoney")
   vn.na( function ()
      local cstr = creditstring( player.credits() )
      local cdif = creditstring( bribe_nearby_cost - player.credits() )
      return string.format(_("You only have %s credits. You need #r%s#0 more to be able to afford the bribe!"), cstr, cdif )
   end )
   vn.jump("menu")

   vn.label("bribe_nearby_trypay")
   vn.func( function ()
      local cost = bribe_nearby_cost
      if cost > player.credits() then
         vn.jump("bribe_nearby_nomoney")
      end
   end )
   p( function ()
      if mem.bribe_paid then
         return mem.bribe_paid
      end
      return _([["Pleasure to do business with you."]])
   end )
   vn.func( function ()
      local cost = bribe_nearby_cost
      player.pay( -cost, true )
      table.insert(bribeable, plt)
      for k,v in ipairs(bribeable) do
         v:credits( bribe_cost(v) )
         v:setBribed(true)
         v:setHostile(false)
         local vmem = v:memory()
         vmem.bribe = 0 -- Disable rebribes
      end
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
      if not str then
         str = string.format(_([["I should be able to refuel you for %s."]]), cstr)
      end
      return string.format(_("%s\n\nYou have %s. Pay #r%s#0?"), str, chave, cstr )
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

