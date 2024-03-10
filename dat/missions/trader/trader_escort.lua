--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Trader Escort">
 <priority>5</priority>
 <cond>
   require("misn_test").mercenary()
 </cond>
 <chance>560</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Traders Society</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--Escort a convoy of traders to a destination--
local pir = require "common.pirate"
local fleet = require "fleet"
local lmisn = require "lmisn"
local car = require "common.cargo"
local fmt = require "format"
local vntk = require "vntk"
local escort = require "escort"

local misn_title = {}
misn_title[1] = _("Escort a tiny convoy to {pnt} in {sys}")
misn_title[2] = _("Escort a small convoy to {pnt} in {sys}")
misn_title[3] = _("Escort a medium convoy to {pnt} in {sys}")
misn_title[4] = _("Escort a large convoy to {pnt} in {sys}")
misn_title[5] = _("Escort a huge convoy to {pnt} in {sys}")

local piracyrisk = {}
piracyrisk[1] = _("#nPiracy Risk:#0 None")
piracyrisk[2] = _("#nPiracy Risk:#0 Low")
piracyrisk[3] = _("#nPiracy Risk:#0 Medium")
piracyrisk[4] = _("#nPiracy Risk:#0 High")

function create()
   -- Try to cache the route to make it so that the same route doesn't appear over and over
   local c = naev.cache()
   local t = time.get()
   if not c.misn_escorts then
      c.misn_escorts = {}
   end
   if c.misn_escorts._t ~= t then
      c.misn_escorts = { _t=t } -- Regenerate
   end

   -- This mission does not make any system claims
   mem.destspob, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute( rnd.rnd(1,2), {remove_spob=c.misn_escorts} )

   if mem.destspob == nil then
      misn.finish(false)
   elseif mem.numjumps == 0 then
      misn.finish(false) -- have to escort them at least one jump!
   elseif mem.avgrisk * mem.numjumps <= 25 then
      misn.finish(false) -- needs to be a little bit of piracy possible along route
   -- Have to be able to do an inclusive claim
   elseif not misn.claim( lmisn.getRoute( system.cur(), mem.destsys ), true ) then
      misn.finish(false)
   end
   c.misn_escorts[ mem.destspob:nameRaw() ] = true -- Mark system

   if mem.avgrisk == 0 then
      piracyrisk = piracyrisk[1]
   elseif mem.avgrisk <= 150 then
      piracyrisk = piracyrisk[2]
   elseif mem.avgrisk <= 300 then
      piracyrisk = piracyrisk[3]
   else
      piracyrisk = piracyrisk[4]
   end

   mem.convoysize = rnd.rnd(1,5)

   -- Choose mission reward.
   -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
   if mem.convoysize == 1 then
      mem.jumpreward = 6*commodity.price(mem.cargo)
      mem.distreward = math.log(500*commodity.price(mem.cargo))/100
   elseif mem.convoysize == 2 then
      mem.jumpreward = 7*commodity.price(mem.cargo)
      mem.distreward = math.log(700*commodity.price(mem.cargo))/100
   elseif mem.convoysize == 3 then
      mem.jumpreward = 8*commodity.price(mem.cargo)
      mem.distreward = math.log(800*commodity.price(mem.cargo))/100
   elseif mem.convoysize == 4 then
      mem.jumpreward = 9*commodity.price(mem.cargo)
      mem.distreward = math.log(900*commodity.price(mem.cargo))/100
   elseif mem.convoysize == 5 then
      mem.jumpreward = 10*commodity.price(mem.cargo)
      mem.distreward = math.log(1000*commodity.price(mem.cargo))/100
   end
   mem.reward = 2.0 * (mem.avgrisk * mem.numjumps * mem.jumpreward + mem.traveldist * mem.distreward) * (1. + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f( misn_title[mem.convoysize], {pnt=mem.destspob, sys=mem.destsys} ) )
   car.setDesc( fmt.f(_("A convoy of traders needs protection while they go to {pnt} ({sys} system). You must stick with the convoy at all times, waiting to jump or land until the entire convoy has done so. You may only escort one group of traders at a time."), {pnt=mem.destspob, sys=mem.destsys} ), mem.cargo, nil, mem.destspob, nil, piracyrisk )
   misn.markerAdd(mem.destspob, "computer")
   misn.setReward(mem.reward)
end

function accept()
   if player.misnActive("Trader Escort")  then
      vntk.msg(_([[Hands Full]]),_([[You may only escort one group of traders at a time!]]))
      return
   end

   if player.jumps() < mem.numjumps then
      if not vntk.yesno( _("Not enough fuel"), fmt.f( _([[The destination is {1} away, but you only have enough fuel for {2}. You cannot stop to refuel. Accept the mission anyway?]]), {fmt.jumps(mem.numjumps), fmt.jumps(player.jumps())} ) ) then
         return
      end
   end

   misn.accept()
   misn.osdCreate(_("Convoy Escort"), {
      fmt.f(_("Escort a convoy of traders to {pnt} ({sys} system)"), {pnt=mem.destspob, sys=mem.destsys}),
   })
   misn.markerAdd( mem.destspob )

   -- Choose convoy
   local convoy_ships
   if mem.convoysize == 1 then
      convoy_ships = trepeat( {"Llama"}, 3 )
   elseif mem.convoysize == 2 then
      convoy_ships = trepeat( {"Koala"}, 4 )
   else
      convoy_ships = trepeat( {"Rhino", "Mule"}, mem.convoysize-1 )
   end
   mem.num_ships = #convoy_ships

   -- Begin the escort
   escort.init( convoy_ships, {
      func_pilot_create = "trader_create",
      func_pilot_attacked = "trader_attacked",
   })
   escort.setDest( mem.destspob, "success" )

   hook.enter( "spawn_ambush" )
end

-- luacheck: globals success
function success ()
   local alive = escort.num_alive()
   local alive_frac = alive / mem.num_ships

   local reward_orig = mem.reward
   mem.reward = mem.reward * alive_frac

   if alive_frac >= 1 then
      vntk.msg( _("Success!"), fmt.f(_("You successfully escorted the trading convoy to the destination. There wasn't a single casualty, and you are rewarded the full amount of #g{credits}#0."), {credits=fmt.credits(mem.reward)}) )
      faction.get("Traders Society"):modPlayer(rnd.rnd(2,3))
   elseif alive_frac >= 0.6 then
      vntk.msg( _("Success with Casualties"), fmt.f(_("You've arrived with the trading convoy more or less intact. Your pay is docked slightly due to the loss of part of the convoy. You receive #g{credits}#0 of the original promised reward of {reward}."), {credits=fmt.credits(mem.reward), reward=fmt.credits(reward_orig)}) )
      faction.get("Traders Society"):modPlayer(1)
   else
      vntk.msg( _("Success with Heavy Casualties"), fmt.f(_("You arrive with what's left of the convoy. It's not much, but it's better than nothing. You are paid a steeply discounted amount of #g{credits}#0 from the {reward} originally promised."), {credits=fmt.credits(mem.reward), reward=fmt.credits(reward_orig)}) )
   end
   player.pay( mem.reward )
   pir.reputationNormalMission(rnd.rnd(2,3))
   misn.finish( true )
end

-- luacheck: globals trader_create
function trader_create( p )
   for j, c in ipairs( p:cargoList() ) do
      p:cargoRm( c.name, c.q )
   end
   p:cargoAdd( mem.cargo, p:cargoFree() )
   p:rename(_("Convoy ")..p:ship():name())
end

local last_spammed = 0
local unsafe = false
-- Handle the convoy getting attacked.
-- luacheck: globals trader_attacked
function trader_attacked( p, _attacker )
   unsafe = true
   p:control( false )
   p:setNoJump( true )
   p:setNoLand( true )

   local t = naev.ticks()
   if (t-last_spammed) > 10 then
      p:comm( _("Convoy ships under attack! Requesting immediate assistance!") )
      last_spammed = t
   end
end

function trader_safe()
   hook.timer( 3.0, "trader_safe" )
   if unsafe then
      unsafe = false
      escort.reset_ai()
   end
end

function spawn_ambush ()
   -- Make it interesting
   local ambush_src = mem.destspob
   if system.cur() ~= mem.destsys then
      ambush_src = lmisn.getNextSystem( system.cur(), mem.destsys )
   end

   local ambush
   local ambushes = {
      {"Pirate Ancestor", "Pirate Vendetta", "Pirate Hyena", "Pirate Hyena"},
      {"Pirate Ancestor", "Pirate Vendetta", "Pirate Vendetta", "Pirate Vendetta", "Pirate Hyena", "Pirate Hyena"},
      {"Pirate Admonisher", "Pirate Rhino", "Pirate Rhino", "Pirate Shark", "Pirate Shark"},
      {"Pirate Admonisher", "Pirate Phalanx", "Pirate Phalanx", "Pirate Shark", "Pirate Shark", "Pirate Hyena", "Pirate Hyena"},
      {"Pirate Kestrel", "Pirate Admonisher", "Pirate Rhino", "Pirate Shark", "Pirate Shark", "Pirate Hyena", "Pirate Hyena", "Pirate Hyena"},
   }
   if mem.convoysize == 1 then
      ambush = fleet.add( 1, ambushes[1], "Marauder", ambush_src, nil, {ai="baddie_norun"} )
   elseif mem.convoysize == 2 then
      ambush = fleet.add( 1, ambushes[rnd.rnd(1,2)], "Marauder", ambush_src, nil, {ai="baddie_norun"} )
   elseif mem.convoysize == 3 then
      ambush = fleet.add( 1, ambushes[rnd.rnd(2,3)], "Marauder", ambush_src, nil, {ai="baddie_norun"} )
   elseif mem.convoysize == 4 then
      ambush = fleet.add( 1, ambushes[rnd.rnd(2,4)], "Marauder", ambush_src, nil, {ai="baddie_norun"} )
   else
      ambush = fleet.add( 1, ambushes[rnd.rnd(3,5)], "Marauder", ambush_src, nil, {ai="baddie_norun"} )
   end
   for _,p in ipairs(ambush) do
      p:setHostile(true)
   end

   hook.timer( 1.0, "trader_safe" )
end
