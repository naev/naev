--[[
   Communication with a pilot implemented fully in Lua :D
--]]
local vn = require 'vn'
local fmt = require "format"
local ccomm = require "common.comm"

local bribe_group, bribeable, bribeable_all, bribe_nearby_cost, bribe_all_cost

local function can_bribe( plt )
   local mem = plt:memory()
   if mem.bribe_no then
      return false
   end
   if not mem.bribe then
      return false
   end
   if mem.bribe==0 then
      return false
   end
   if mem.bribed_once then
      return false
   end
   if plt:flags("bribed") then
      return false
   end
   if not plt:hostile() then
      return false
   end
   return true
end

local function filter_bribeable( group )
   local ng = {}
   for k,v in ipairs(group) do
      if can_bribe( v ) then
         table.insert( ng, v )
      end
   end
   return ng
end

-- See if part of a fleet
local function bribe_fleet( plt )
   -- Current pilot must be bribeable
   if not can_bribe(plt) then return nil end

   -- Get leader and followers, making sure they exist
   local lea = plt:leader()
   local fol = plt:followers()
   if #fol==0 then fol = nil end

   -- Try to figure out the group
   local group = nil
   if lea then
      -- If leader is not bribeable, allow bribing the current pilot anyway,
      -- but not the group
      if not can_bribe(lea) then return nil end
      -- Should include the current pilot
      -- We want the entire fleet to be bribeable if the leader is
      group = lea:followers()
      table.insert( group, lea )
   elseif fol then
      -- Make sure there are actually followers to bribe
      group = filter_bribeable(fol)
      if #group <= 0 then return nil end
      table.insert( group, plt )
   end

   return group
end

-- Get nearby pilots for bribing (includes current pilot)
-- Respects fleets
local function nearby_bribeable( plt, difffactok )
   local pp = player.pilot()
   local ret = {}
   for k,v in ipairs(pp:getVisible()) do
      if (v:faction() == plt:faction() or (difffactok and not v:areEnemies(plt))) and can_bribe(v) then
         local flt = bribe_fleet( v )
         -- No fleet, so assume single pilot
         if not flt then
            flt = {v}
         end
         -- Make sure it's not already there to avoid duplicates
         for i,p in ipairs(flt) do
            local found = false
            for j,c in ipairs(ret) do
               if c==p then
                  found = true
                  break
               end
            end
            if not found then
               table.insert( ret, p )
            end
         end
      end
   end
   return ret
end

local function bribe_cost( plt )
   if type(plt)=="table" then
      local cost = 0
      for k,v in ipairs(plt) do
         cost = cost + bribe_cost(v)
      end
      return cost
   end

   local mem = plt:memory()
   if not mem.bribe then
      warn(fmt.f(_("Pilot '{plt}' accepts bribes but doesn't give a price!"), {plt=plt} ))
      return 1e9 -- just ridiculous for now, players should report it
   end
   return mem.bribe
end

local function bribe_msgFactions( group )
   local factions = {}
   for k,v in ipairs(group) do
      factions[ v:faction():name() ] = true
   end
   local names = {}
   for k,v in pairs(factions) do
      table.insert( names, k )
   end
   return fmt.list(names)
end

local function bribe_msg( plt, group )
   local mem = plt:memory()
   if group then
      local cost = bribe_cost( group )
      local str = mem.bribe_prompt_nearby or mem.bribe_prompt
      local cstr = fmt.credits(cost)
      local chave = fmt.credits(player.credits())
      if not str then
         str = _([["We'll need at least {credits} to not leave you as a hunk of floating debris."]])
      end
      str = fmt.f( str, {credits=cstr} )
      return fmt.f(n_("{msg}\n\nThis action will bribe {n} {fct_list} pilot.\nYou have {credits}. Pay #r{price}#0?",
                      "{msg}\n\nThis action will bribe {n} {fct_list} pilots.\nYou have {credits}. Pay #r{price}#0?", #bribeable),
            {msg=str, n=#bribeable, fct_list=bribe_msgFactions(group), credits=chave, price=cstr} ), cost
   else
      local cost = bribe_cost( plt )
      local str = mem.bribe_prompt
      local cstr = fmt.credits(cost)
      local chave = fmt.credits(player.credits())
      if not str then
         str = _([["I'm gonna need at least {credits} to not leave you as a hunk of floating debris."]])
      end
      str = fmt.f( str, {credits=cstr} )
      return fmt.f(_("{msg}\n\nYou have {credits}. Pay #r{price}#0?"), {msg=str, credits=chave, price=cstr} ), cost
   end
end

function comm( plt )
   local mem = plt:memory()

   if mem.carried then
      plt:comm(_("The fighter does not respond."), true, true)
      return
   end

   vn.reset()
   vn.scene()

   local p = ccomm.newCharacter( vn, plt )

   -- For bribing purposes
   bribe_group   = bribe_fleet( plt )
   bribeable     = nearby_bribeable( plt )
   bribeable_all = nearby_bribeable( plt, true )

   vn.transition()
   local msg
   if mem.comm_greet then
      if type(mem.comm_greet)=="function" then
         -- Override VN stuff since we want to pass pilot as parameter
         msg = mem.comm_greet( plt )
      else
         msg = mem.comm_greet
      end
   end
   if msg ~= nil then
      p( msg )
   else
      vn.na(fmt.f(_("You open a communication channel with {plt}."), {plt=plt}))
   end
   vn.label("menu")
   vn.menu( function ()
      local hostile = player.pilot():areEnemies(plt)
      local opts = {
         {_("Close"), "close"},
      }
      if hostile and not plt:flags("bribed") then
         if mem.bribe_no then
            table.insert( opts, 1, {_("Bribe"), "bribe_no"} )
         elseif (mem.bribe and mem.bribe == 0) or mem.bribed_once then
            table.insert( opts, 1, {_("Bribe"), "bribe_0"} )
         else
            if #bribeable_all > 1 and #bribeable_all ~= #bribeable then
               table.insert( opts, 1, {
                  fmt.f(
                     n_("Bribe {n} nearby {fct_list} pilot",
                        "Bribe {n} nearby {fct_list} pilots", #bribeable_all),
                        {n=#bribeable_all, fct_list=bribe_msgFactions(bribeable_all)}
                  ), "bribe_all",
               } )
            end
            if #bribeable > 1 and not (bribe_group and #bribe_group==#bribeable) then
               table.insert( opts, 1, {
                  fmt.f(
                     n_("Bribe {n} nearby {fct_list} pilot",
                        "Bribe {n} nearby {fct_list} pilots", #bribeable),
                     {n=#bribeable, fct_list=bribe_msgFactions(bribeable)}
                  ), "bribe_nearby",
               } )
            end

            if bribe_group then
               table.insert( opts, 1, {
                  fmt.f(
                     n_("Bribe fleet ({n} pilot)",
                        "Bribe fleet ({n} pilots)", #bribe_group),
                     {n=#bribe_group}
                  ), "bribe",
               } )
            else
               table.insert( opts, 1, {_("Bribe this pilot"), "bribe"} )
            end
         end
      end
      if not hostile and not mem.carried then
         table.insert( opts, 1, {_("Request Fuel"), "refuel"} )
      end
      if mem.comm_custom then
         for k,v in ipairs(mem.comm_custom) do
            local menumsg
            if type(v.menu)=="function" then
               menumsg = v.menu( plt )
            else
               menumsg = v.menu
            end
            if menumsg ~= nil then
               table.insert( opts, 1, {menumsg, "custom_"..tostring(k)} )
            end
         end
      end
      return opts
   end )

   --
   -- Custom stuff
   --
   if mem.comm_custom then
      for k,v in ipairs(mem.comm_custom) do
         vn.label("custom_"..tostring(k))
         v.setup( vn, p, plt )
         vn.jump("menu")
      end
   end

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
      local str = bribe_msg( plt, bribe_group )
      return str
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
      local cost
      if bribe_group then
         cost = bribe_cost( bribe_group )
      else
         cost = bribe_cost( plt )
      end
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( cost - player.credits() )
      return fmt.f(_("You only have {credits}. You need #r{cdif}#0 more to be able to afford the bribe!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   vn.label("bribe_trypay")
   vn.func( function ()
      local cost = bribe_cost( plt )
      if cost > player.credits() then
         vn.jump("bribe_nomoney")
      end
   end )
   p( mem.bribe_paid or _([["Pleasure to do business with you."]]) )
   vn.func( function ()
      local cost = bribe_cost( plt )
      player.pay( -cost, true )
      if bribe_group then
         for k,v in ipairs(bribe_group) do
            v:credits( bribe_cost(v) )
            v:setBribed(true)
            v:setHostile(false)
            local vmem = v:memory()
            vmem.bribed_once = true -- Disable rebribes
         end
      else
         plt:credits( cost )
         plt:setBribed(true)
         plt:setHostile(false)
         mem.bribed_once = true -- Disable rebribes
      end
      ccomm.nameboxUpdate( plt )
   end )
   vn.jump("menu")

   vn.label("bribe_nearby")
   p( function ()
      local str, cost = bribe_msg( plt, bribeable )
      bribe_nearby_cost = cost
      return str
   end )
   vn.menu{
      {_("Pay"), "bribe_nearby_trypay"},
      {_("Refuse"), "bribe_refuse"},
   }

   vn.label("bribe_nearby_nomoney")
   vn.na( function ()
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( bribe_nearby_cost - player.credits() )
      return fmt.f(_("You only have {credits}. You need #r{cdif}#0 more to be able to afford the bribe!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   vn.label("bribe_nearby_trypay")
   vn.func( function ()
      local cost = bribe_nearby_cost
      if cost > player.credits() then
         vn.jump("bribe_nearby_nomoney")
      end
   end )
   p( mem.bribe_paid or _([["Pleasure to do business with you."]]) )
   vn.func( function ()
      local cost = bribe_nearby_cost
      player.pay( -cost, true )
      for k,v in ipairs(bribeable) do
         v:credits( bribe_cost(v) )
         v:setBribed(true)
         v:setHostile(false)
         local vmem = v:memory()
         vmem.bribed_once = true -- Disable rebribes
      end
      ccomm.nameboxUpdate( plt )
   end )
   vn.jump("menu")

   vn.label("bribe_all")
   p( function ()
      local str, cost = bribe_msg( plt, bribeable_all )
      bribe_all_cost = cost
      return str
   end )
   vn.menu{
      {_("Pay"), "bribe_all_trypay"},
      {_("Refuse"), "bribe_refuse"},
   }

   vn.label("bribe_all_nomoney")
   vn.na( function ()
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( bribe_all_cost - player.credits() )
      return fmt.f(_("You only have {credits}. You need #r{cdif}#0 more to be able to afford the bribe!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   vn.label("bribe_all_trypay")
   vn.func( function ()
      local cost = bribe_all_cost
      if cost > player.credits() then
         vn.jump("bribe_all_nomoney")
      end
   end )
   p( mem.bribe_paid or _([["Pleasure to do business with you."]]) )
   vn.func( function ()
      local cost = bribe_all_cost
      player.pay( -cost, true )
      for k,v in ipairs(bribeable_all) do
         v:credits( bribe_cost(v) )
         v:setBribed(true)
         v:setHostile(false)
         local vmem = v:memory()
         vmem.bribed_once = true -- Disable rebribes
      end
      ccomm.nameboxUpdate( plt )
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
   p(mem.refuel_low or _([["Sorry, I don't have enough fuel to spare at the moment."]]))
   vn.jump("menu")

   vn.label("refuel_busy")
   p(mem.refuel_busy or _([["Sorry, I'm busy now."]]))
   vn.jump("menu")

   vn.label("refueling_already")
   p(mem.refuel_already or _([["What part of 'on my way' don't you understand?"]]))
   vn.jump("menu")

   vn.label("refuel_refuse")
   vn.na(_("You decide not to pay."))
   vn.jump("menu")

   vn.label("refuel")
   vn.func( function ()
      if mem.refuel_no then
         vn.jump("refuel_no")
      end
      local pp = player.pilot()
      local pps = pp:stats()
      if pps.fuel >= pps.fuel_max then
         vn.jump("refuel_full")
      end
      -- Want to have at least 100 units of fuel extra
      local ps = plt:stats()
      if ps.fuel <= ps.fuel_consumption+100 then
         vn.jump("refuel_low")
      end
      local val = mem.refuel
      if val==nil or plt:flags("manualcontrol") then
         vn.jump("refuel_busy")
      end
      if plt:flags("refueling") then
         vn.jump("refueling_already")
      end
   end )
   p( function ()
      local str = mem.refuel_msg
      local cost = mem.refuel
      local cstr = fmt.credits(cost)
      local chave = fmt.credits(player.credits())
      if not str then
         str = ([["I should be able to refuel you for {credits} for 100 units of fuel."]])
      end
      str = fmt.f( str, {credits=cstr} )
      if cost <= 0 then
         -- It's free so give as much as the player wants
         vn.jump("refuel_trypay_max")
      end
      return fmt.f(_("{msg}\n\nYou have {credits}. Pay for refueling??"), {msg=str, credits=chave} )
   end )
   vn.menu( function ()
      local pps = player.pilot():stats()
      local plts = plt:stats()
      local cons = pps.fuel_consumption
      local cost = mem.refuel
      local opts = {
         {fmt.f(_("Pay #r{cost}#0 (100 fuel, {jumps:.1f} jumps)"),{cost=fmt.credits(cost),jumps=100/cons}), "refuel_trypay"},
         {_("Refuse"), "refuel_refuse"},
      }
      -- Only allow multiples of 100
      local maxfuel = math.floor( math.min( pps.fuel_max-pps.fuel, plts.fuel-plts.fuel_consumption ) / 100 ) * 100
      if maxfuel > cons and maxfuel > 100 then
         table.insert( opts, 2, {fmt.f(_("Pay #r{cost}#0 ({amount} fuel, {jumps:.1f} jumps)"),
               {cost=fmt.credits(cost*maxfuel/100), amount=maxfuel, jumps=maxfuel/cons}),
               "refuel_trypay_max"})
      end
      if cons > 100 and maxfuel >= cons then
         table.insert( opts, 2, {fmt.f(_("Pay #r{cost}#0 ({amount} fuel, {jumps:.1f} jumps)"),
               {cost=fmt.credits(cost*cons/100), amount=cons, jumps=1}),
               "refuel_trypay_jump"})
      end
      return opts
   end )

   vn.label("refuel_nomoney")
   vn.na( function ()
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( mem.refuel - player.credits() )
      return fmt.f(_("You only have {credits} credits. You need #r{cdif}#0 more to be able to afford the refueling!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   -- Provides 100 fuel
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
   vn.label("refuel_startmsg")
   p(mem.refuel_start or _([["On my way."]]))
   vn.jump("menu")

   -- Provides fuel for one jump
   vn.label("refuel_trypay_jump")
   vn.func( function ()
      local pps = player.pilot():stats()
      local cons = pps.fuel_consumption
      local cost = mem.refuel * cons / 100
      if cost > player.credits() then
         vn.jump("refuel_nomoney")
      end
   end )
   vn.func( function ()
      local pps = player.pilot():stats()
      local cons = pps.fuel_consumption
      local cost = mem.refuel * cons / 100
      player.pay( -cost, true )
      plt:credits( cost )
      plt:refuel( player.pilot(), cons )
   end )
   vn.jump("refuel_startmsg")

   -- PRovides as much fuel as possible
   vn.label("refuel_trypay_max")
   vn.func( function ()
      local pps = player.pilot():stats()
      local plts = plt:stats()
      local maxfuel = math.floor( math.min( pps.fuel_max-pps.fuel, plts.fuel-plts.fuel_consumption ) / 100 ) * 100
      local cost = mem.refuel * maxfuel / 100
      if cost > player.credits() then
         vn.jump("refuel_nomoney")
      end
   end )
   vn.func( function ()
      local pps = player.pilot():stats()
      local plts = plt:stats()
      local maxfuel = math.floor( math.min( pps.fuel_max-pps.fuel, plts.fuel-plts.fuel_consumption ) / 100 ) * 100
      local cost = mem.refuel * maxfuel / 100
      player.pay( -cost, true )
      plt:credits( cost )
      plt:refuel( player.pilot(), maxfuel )
   end )
   vn.jump("refuel_startmsg")

   vn.label("close")
   vn.run()
end
