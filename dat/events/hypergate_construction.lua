--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Hypergate Construction">
 <location>enter</location>
 <chance>100</chance>
 <chapter>0</chapter>
</event>
--]]
--[[
   Hypergate construction animations and possibility to sell rare ores.
--]]
local fmt = require "format"
local vn = require "vn"
local der = require "common.derelict"
local ccomm = require "common.comm"


local hypergates_list = {
   "Hypergate Dvaer", -- Dvaered
   "Hypergate Feye", -- Soromid
   "Hypergate Gamma Polaris", -- Empire
   "Hypergate Kiwi", -- Sirius
   "Hypergate Ruadan", -- Za'lek
   --"Hypergate NGC-14549", -- Pirate
   --"Hypergate Polaris", -- Ruined
}
local systems_list = {}
for k,h in ipairs(hypergates_list) do
   hypergates_list[k], systems_list[k] = spob.getS(h)
end

local boss_ship_list = {
   ["Za'lek"]  = "Za'lek Mammon",
   ["Dvaered"] = "Dvaered Arsenal",
   ["Soromid"] = "Soromid Copia",
   ["Sirius"]  = "Sirius Providence",
   ["Empire"]  = "Empire Rainmaker",
}

local boss_name_list = {
   ["Za'lek"]  = _("Supervisor"),
   ["Dvaered"] = _("Boss"),
   ["Soromid"] = _("Chief"),
   ["Sirius"]  = _("Shepherd"),
   ["Empire"]  = _("Administrator"),
}

local boss_message_list = {
   ["Za'lek"]  = _("Mineral acquiration skills required. Inquire within."),
   ["Dvaered"] = _("Interested in blowing asteroids up? Looking for miners!"),
   ["Soromid"] = _("Seeking mineral gatherers."),
   ["Sirius"]  = _("Requiring miners for Sirichana."),
   ["Empire"]  = _("Mine minerals for the Empire!"),
}

local mineral_list = { "Therite", "Kermite", "Vixilium" } -- Only rares
local markup = 1.2 -- Multiplier for amount being paid
local standing = 0.1 -- Multiplier for standing increase

local id, hypergate, boss, talked_check, traded_amount
local traded_total = "hypconst_traded_total"

function create ()
   local csys = system.cur()
   -- Make sure system isn't claimed, but we don't claim it
   if not naev.claimTest( csys, true ) then evt.finish() end

   -- Only care if we're in a system with hypergates
   local sysid
   for k,h in ipairs(systems_list) do
      if h==csys then
         sysid = k
         break
      end
   end
   if not sysid then evt.finish() end

   hypergate = spob.get( hypergates_list[sysid] )

   -- We assume dominant faction is the one we want here
   local sysfct = hypergate:faction()
   id = sysfct:nameRaw()
   local shiptype = boss_ship_list[ id ] or "Zebra"
   local shipname = boss_name_list[ id ] or _("Supervisor")
   local pos = hypergate:pos() + vec2.newP( 200+300*rnd.rnd(), rnd.angle() )

   -- Some useful variable names
   talked_check = "hypconst_"..sysfct:nameRaw().."_talked"
   traded_amount = "hypconst_"..sysfct:nameRaw().."_traded"

   -- Add the head guy
   boss = pilot.add( shiptype, sysfct, pos, shipname, {ai="guard"} )
   local pmem = boss:memory()
   pmem.aggressive = false
   if not boss:hostile() then
      boss:setHilight(true)
   end

   hook.pilot( boss, "hail", "boss_hail" )
   if var.peek( talked_check ) then
      boss:setActiveBoard(true)
      hook.pilot( boss, "board", "boss_board" )
   else
      hook.timer( 5, "boss_first" )
   end

   -- Event finishes when leaving system
   hook.land( "endevent" )
   hook.jumpout( "endevent" )
end

function boss_first ()
   if not boss or not boss:exists() then return end

   local dist = player.pos():dist( boss:pos() )
   if dist > 5000 then
      hook.timer( 5, "boss_first" )
      return
   end

   local msg = boss_message_list[ id ] or _("TODO")

   boss:broadcast( msg )
   if not boss:hostile() then
      boss:hailPlayer()
   end
end

function boss_hail ()
   vn.reset()
   vn.scene()
   local b = ccomm.newCharacter( vn, boss )
   vn.transition()

   if boss:hostile() then
      b(_([["We don't deal with the likes of you!"]]))
   else
      if not var.peek( talked_check ) then
         local mineral_name_list = {}
         for i,m in ipairs(mineral_list) do
            mineral_name_list[i] = _(mineral_list[i])
         end
         b(fmt.f(_([["We are looking for miners to obtain valuable minerals such as {minerals}. Given the difficulty of acquiring them, we are willing to pay {markup}% of the market price. If you are interested, please bring the minerals and board to do the transaction."]]),{minerals=fmt.list(mineral_name_list),markup=markup*100}))
         vn.func( function ()
            boss:setActiveBoard(true)
            hook.pilot( boss, "board", "boss_board" )
            var.push( talked_check, true )
         end )
      else
         b(_([["Do you have minerals available? If so, please board for the transaction."]]))
      end
   end

   vn.run()

   player.commClose()
end

function boss_board ()
   local pp = player.pilot()

   if boss:hostile() then
      local _a, _s, _str, disabled = boss:health()
      if disabled then
         return -- Should allow the player to ransack the ship
      end
      player.unboard()
      player.msg(_("You are not allowed to board the ship!"))
      return
   end

   local minerals = {}
   for i,m in ipairs(mineral_list) do
      minerals[i] = commodity.get(m)
   end

   -- Boarding sound
   der.sfx.board:play()

   vn.reset()
   vn.scene()
   local b = ccomm.newCharacter( vn, boss )
   vn.transition()

   vn.na( function ()
      local s = fmt.f(_([[You board the {ship}, and find that the cargo bay has been set up to efficiently process minerals. There is a holosign with the needed resources and their prices:]]),{ship=boss:name()})
      for i,m in ipairs(minerals) do
         s = s .. "\n   " .. fmt.f(_("{mineral}: {price}"),{mineral=m, price=fmt.credits(m:price()*markup)})
      end
      return s
   end )

   vn.label("menu")
   vn.na(_("What do you want to do?"))
   vn.menu( function ()
      local opts = {
         {_("Leave."), "leave"}
      }
      for i,m in ipairs(minerals) do
         local a = pp:cargoHas(m)
         if a > 0 then
            table.insert( opts, 1, {
               fmt.f(_("Trade {mineral} for {value} (You have {amount})"),
                  {mineral=m, amount=fmt.tonnes(a), value=fmt.credits(markup*m:price())}), m:nameRaw() } )
         end
      end
      table.insert( opts, 1, { _("Ask about the construction."), "ask" } )
      return opts
   end )

   vn.label("ask")
   b(_([["It's some sort of high power quantum energy translator or something like that. I'm not too up-to-date on the details, but I do recall there was something similar to this back at Sol before the incident…"]]))
   vn.jump("menu")

   -- Mineral options
   for i,m in ipairs(minerals) do
      vn.label( m:nameRaw() )
      b(function ()
         local a = pp:cargoHas(m)
         return fmt.f(_([[You deliver the {amount} of {mineral}.
{reward}]]),{amount=a, mineral=m, reward=fmt.reward(a * markup * m:price())})
      end )
      vn.func( function ()
         local a = pp:cargoHas(m)
         pp:cargoRm( m, a )
         player.pay( a * markup * m:price() )
         -- Add some faction too
         hypergate:faction():modPlayer( a * standing )
         -- Store how much was traded
         local q = var.peek( traded_amount ) or 0
         var.push( traded_amount, q+a )
         q = var.peek( traded_total ) or 0
         var.push( traded_total, q+a )
      end )
      vn.jump("menu")
   end

   vn.label("leave")
   vn.run()
   player.unboard()

   -- Boarding sound
   der.sfx.unboard:play()
end

function endevent ()
   evt.finish()
end
