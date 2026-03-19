--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Escort Pers">
 <chance>0</chance>
 <location>None</location>
</mission>
--]]
-- Escort a pers that hailed the player
local pir = require "common.pirate"
local fleet = require "fleet"
local lmisn = require "lmisn"
local fmt = require "format"
local vntk = require "vntk"
local escort = require "escort"

function create()
   local nc = naev.cache()
   local e = nc.__pers_escort
   mem.destspob   = e.dest
   mem.destsys    = e.dest:system()
   mem.reward     = e.reward
   mem.convoysize = e.difficulty
   mem.cargo      = e.cargo
   mem.name       = e.p:name()

   misn.accept()
   misn.setReward( mem.reward )

   misn.osdCreate(_("Trader Escort"), {
      fmt.f(_("Escort a trader to {pnt} ({sys} system)"), {
         pnt=mem.destspob,
         sys=mem.destsys
      }),
   })
   misn.markerAdd( mem.destspob )

   -- Begin the escort
   escort.init( {e.p}, {
      func_pilot_create    = "trader_create",
      func_pilot_attacked  = "trader_attacked",
   })
   escort.setDest( mem.destspob, "success" )

   hook.enter( "spawn_ambush" )
end

-- luacheck: globals success
function success ()
   local reputation = rnd.rnd( mem.convoysize*4, mem.convoysize*6 )

   lmisn.sfxMoney()
   vntk.msg( _("Escort Completed!"), fmt.f(_("You are profously thanked by the Captain of the {name}, who promptly gives you a credstick loaded with {reward}."), {
      name   = mem.name,
      reward = fmt.credits( mem.reward ),
   } ) )
   faction.get("Traders Society"):hit(reputation*2)
   player.pay( mem.reward )
   pir.reputationNormalMission(reputation)
   misn.finish( true )
end

-- luacheck: globals trader_create
function trader_create( p )
   p:cargoRm("all")
   p:cargoAdd( mem.cargo, p:cargoFree() )
   p:rename( mem.name )
end

local last_spammed = 0
-- Handle the convoy getting attacked.
-- luacheck: globals trader_attacked
function trader_attacked( p, attacker )
   -- Attackers have to be marked as hostile
   attacker:setHostile()

   -- Only spam so often
   local t = naev.ticks()
   if (t-last_spammed) > 10 then
      p:comm( _("Trader under attack! Requesting immediate assistance!") )
      last_spammed = t
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
end
