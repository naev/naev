
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mineral Request">
 <unique/>
 <priority>4</priority>
 <chance>350</chance>
 <location>Computer</location>
 <cond>
   require("misn_test").cargo()
 </cond>
</mission>
--]]
local trader = require "common.trader"
local lmisn = require "lmisn"
local fmt = require "format"
local vn = require "vn"

local flavour = {
   _("A local company has requested at least {amount} of {mineral} at {spob} ({sys} system)."),
   _("The local government of {sys} is in need of at least {amount} of {mineral} at {spob} ({sys} system)."),
   _("A new entrepreneur is in need of some {amount} of {mineral} for a new prototype at {spob} ({sys} system)."),
   _("An individual has made a request of {amount} of {mineral} at {spob} ({sys} system)."),
   _("An anonymous request has been made for {amount} of {mineral} to be delivered at {spob} ({sys} system)."),
   _("A local warehouse has requested a delivery of at least {amount} of {mineral} at {spob} ({sys} system)."),
}

local title = trader.prefix.._("Mining Request of {mineral} to {dest}")

local function good_commodity( c )
   local t = c:tags()
   return t.mining and (t.rare or t.uncommon)
end

local function sys_has_mineral( sys )
   for i,af in ipairs(sys:asteroidFields()) do
      if inlist( af.commodities, mem.mineral ) then
         return true
      end
   end
   return false
end

local function get_rarity( comm )
   local t = comm:tags()
   if t.special then
      return _("special")
   elseif t.rare then
      return "#r".._("rare").."#0"
   elseif t.uncommon then
      return "#o"..("uncommon").."#0"
   else
      return _("common")
   end
end

function create ()
   -- First let us find a mining system
   local candidates = lmisn.getSysAtDistance( nil, 3, 6, function ( sys )
      local af = sys:asteroidFields()
      if #af <= 0 then
         return false
      end

      local good_comm = false
      for i,a in ipairs(af) do
         for j,c in ipairs(a.commodities) do
            if good_commodity( c ) then
               good_comm = true
               break
            end
         end
      end

      return good_comm
   end )
   if #candidates <= 0 then
      misn.finish(false)
   end
   local mining_sys = candidates[ rnd.rnd(1,#candidates) ]

   -- Next determine what we want delivered
   local comm = {}
   for i,a in ipairs(mining_sys:asteroidFields()) do
      for j,c in ipairs(a.commodities) do
         if good_commodity( c ) then
            comm[ c:nameRaw() ] = c
         end
      end
   end
   local ncomm = {}
   for i,c in pairs(comm) do
      table.insert( ncomm, c )
   end
   comm = ncomm
   mem.mineral = comm[ rnd.rnd(1,#comm) ]
   local t = mem.mineral:tags()
   if t.rare then
      mem.minimum = rnd.rnd( 5, 10 )
      mem.tier = 2
   elseif t.uncommon then
      mem.minimum = rnd.rnd( 10, 20 )
      mem.tier = 2
   else
      mem.minimum = rnd.rnd( 20, 30 )
      mem.tier = 2
   end

   -- Now to figure out where to send it
   mem.dest, mem.sys = lmisn.getRandomSpobAtDistance( mining_sys, 1, 3, nil, nil, function ( spb )
      -- Should be a system _without_ the mineral
      return not sys_has_mineral( spb:system() )
   end )
   if not mem.dest then
      misn.finish(false)
   end

   -- Compute the most efficient mining route
   local mindist = math.huge
   for k,sys in ipairs(lmisn.getSysAtDistance( mem.sys, 0, 3, sys_has_mineral )) do
      local d = mem.sys:jumpDist( sys ) + sys:jumpDist()
      if d < mindist and sys_has_mineral( sys, mem.mineral ) then
         mindist = d
      end
   end
   -- No route found, so abort
   if mindist >= math.huge then
      misn.finish(false)
   end

   -- Figure out the reward
   mem.reward_base = 10e3*(1 + 1.5^mem.tier + mindist*0.3) * (1 + 0.05*rnd.twosigma())
   mem.reward_tonne = mem.mineral:priceAt( mem.dest ) * (1.2+0.1*rnd.rnd())

   misn.setTitle( fmt.f( title, {dest=mem.dest, mineral=mem.mineral} ) )
   misn.setDesc( fmt.f(_([[{flavour}

#nNecessary Mineral:#0 {mineral} ({rarity})
#nNecessary Amount:#0 at least {amount}]]), {
      flavour=fmt.f( flavour[ rnd.rnd(1,#flavour) ], {
         spob=mem.dest,
         sys=mem.sys,
         amount=fmt.tonnes(mem.minimum),
         mineral=mem.mineral,
      } ),
      spob=mem.dest,
      sys=mem.sys,
      amount=fmt.tonnes(mem.minimum),
      rarity=get_rarity(mem.mineral),
      mineral=mem.mineral,
   }) )
   misn.setReward( fmt.f(_("{base} plus {extra} per tonne delivered"), {
      base = fmt.credits(mem.reward_base),
      extra = fmt.credits(mem.reward_tonne),
   }) )
   misn.markerAdd( mem.dest, "computer" )
end

function update_osd ()
   misn.osdCreate( _("Mining Request"), {
      fmt.f(_("Mine at least {amount} of {mineral} (you have {have})."), {
            amount=fmt.tonnes(mem.minimum),
            mineral=mem.mineral,
            have=fmt.tonnes( player.fleetCargoOwned( mem.mineral) ),
         }),
      fmt.f(_("Deliver to {spob} ({sys} system)."),
         {spob=mem.dest, sys=mem.sys}),
   } )
end

function accept ()
   misn.accept()
   hook.land( "land" )
   hook.custom( "mine_drill", "update_osd" )
   hook.gather( "update_osd" )
   update_osd()
end

function land ()
   if spob.cur() ~= mem.dest then
      return
   end

   local owned = player.fleetCargoOwned( mem.mineral )
   if owned < mem.minimum then
      return
   end

   local reward = mem.reward_base + owned * mem.reward_tonne
   local done = false

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_([[Deliver {amount} of {mineral} and collect {reward}?]]), {
      amount = fmt.tonnes(owned),
      mineral = mem.mineral,
      reward = fmt.credits(reward),
   }))
   vn.menu{
      {_("Deliver the goods."), "ok"},
      {_("Maybe later"), "cancel"},
   }
   vn.label("ok")
   vn.na(fmt.f(_([[You deliver the {amount} of {mineral} and collect {reward}.]]), {
      amount = fmt.tonnes(owned),
      mineral = mem.mineral,
      reward = fmt.credits(reward),
   }))
   vn.func( function ()
      player.fleetCargoRm( mem.mineral, owned )
      player.pay( reward )
      faction.hit("Traders Society", mem.tier+rnd.rnd(1,2) )
      done = true
   end )
   vn.sfxMoney()
   vn.na( fmt.reward(reward) )
   vn.done()

   vn.label("cancel")
   vn.run()

   if done then
      misn.finish(true)
   end
end
