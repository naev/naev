--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Propaganda">
 <priority>3</priority>
 <chance>150</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <cond>
   if var.peek("dp_available") ~= true then
      return false
   end
   return require("misn_test").computer()
 </cond>
 <notes>
  <tier>2</tier>
  <campaign>Dvaered Recruitment</campaign>
  <done_evt name="Deliver Flyers">If you accept</done_evt>
 </notes>
</mission>
--]]
--[[
-- Dvaered Propaganda
-- The player has to overfly a planet and escape ships pursuing them.

   Stages :
   0) Way to target planet
   1) Way back
--]]

local fmt    = require "format"
local dv     = require "common.dvaered"
local vntk   = require 'vntk'
local lmisn  = require "lmisn"
local pir    = require "common.pirate"
require "proximity"


-- Define the flyers commodity
local cargo_flyers
local function _flyers()
   if not cargo_flyers then
      cargo_flyers = commodity.new( N_("Propaganda Posters"), N_("Heaps of posters representing big-nose characters hitting each other."), {gfx_space="flyers.webp"} )
   end
   return cargo_flyers
end

-- Remove the misn cargo or accidentally-scooped posters
local function finish( state )
   if mem.misn_state == 0 then
      misn.cargoRm( mem.cid )
   else -- mem.misn_state == 1
      local cflyers = _flyers()
      local q = player.pilot():cargoHas( cflyers )
      player.pilot():cargoRm( cflyers, q )
   end
   misn.finish( state )
end

function create ()
   -- Select a random Dvaered planet at 4 distance. Stations not allowed.
   local candidates = lmisn.getSpobAtDistance( system.cur(), 1, 4, "Dvaered", true, function (p)
      local c = p:class()
      return not(c=="0" or c=="1" or c=="2" or c=="3")
   end )
   if #candidates == 0 then misn.finish(false) end -- No planet: abort
   mem.pnt = candidates[rnd.rnd(1, #candidates)]
   mem.sys = mem.pnt:system()
   if not misn.claim(mem.sys) then misn.finish(false) end -- Claim

   mem.credits = 3e3*(1+rnd.rnd()) -- (per ton)

   -- Mission details
   misn.setTitle(fmt.f(dv.prefix.._("Warlords Propaganda spreading on {pnt} in {sys}"), {pnt=mem.pnt,sys=mem.sys}))
   misn.setReward( fmt.f(_("{credits} per ton"), {credits=fmt.credits( mem.credits )}) )
   misn.setDesc( fmt.f(_("The Warlords Affairs Office requires a pilot to spread as much posters as possible in the atmosphere of {pnt} in {sys}"), {pnt=mem.pnt,sys=mem.sys}))
   mem.misn_marker = misn.markerAdd( mem.pnt )
end

function accept()
   misn.accept()
   mem.misn_state = 0
   misn.osdCreate( _("Warlords Propaganda"), {
      fmt.f(_("Go to {sys} and overfly {pnt} to spread your posters"), {sys=mem.sys, pnt=mem.pnt} ),
      fmt.f(_("Land on any Dvaered planet or station (except {pnt})"), {pnt=mem.pnt}),
   } )
   hook.enter("enter")
   hook.land("land")

   -- Add the Cargo
   local cflyers = _flyers()
   mem.qtt = player.pilot():cargoFree()
   mem.pay = mem.credits * mem.qtt
   mem.cid = misn.cargoAdd( cflyers, mem.qtt )
   misn.setReward( mem.pay )
end

function enter()
   if system.cur() == mem.sys and mem.misn_state == 0 then
      if not faction.exists( "Dvaered_Warlords" ) then
         mem.dynFact  = faction.dynAdd( faction.get("Dvaered"), "Dvaered_Warlords", _("Dvaered") )
         local allies = mem.dynFact:allies()
         for i, a in ipairs(allies) do -- remove their allies for them not to chase the player
            mem.dynFact:dynAlly( a, true )
         end
      end
      -- Add a few hostile-to-be patrol ships
      for i = 1, 3 do
         pilot.add( "Dvaered Vendetta", mem.dynFact )
         pilot.add( "Dvaered Ancestor", mem.dynFact )
      end
      pilot.add( "Dvaered Phalanx", mem.dynFact )
      pilot.add( "Dvaered Phalanx", mem.dynFact )
      pilot.add( "Dvaered Vigilance", mem.dynFact )
      pilot.add( "Dvaered Goddard", mem.dynFact )

      -- Proximity of the planet
      hook.timer(0.5, "proximity", {location = mem.pnt:pos(), radius = mem.pnt:radius()/.75, funcname = "beginSpread"})
   end
end

-- Sets timers for flyers spread
function beginSpread()
   -- We'll drop them in 10 times. Compute how long it will be
   local time_incr = mem.pnt:radius()/player.pilot():vel():mod()/11
   local d = math.floor(mem.qtt/10)
   local r = mem.qtt - 10*d
   -- We'll drop r times d+1 flyers, and 10-r times d flyers. The total is equal to mem.qtt
   for i = 1, r do
      hook.timer( time_incr*i, "spreadFlyers", d+1 )
   end
   for i = (r+1), 10 do
      hook.timer( time_incr*i, "spreadFlyers", d )
   end
   misn.cargoRm( mem.cid ) -- Remove all at once for simplicity
   hook.timer( 5, "spawnHostiles" ) -- Prepare hostiles
   mem.misn_state = 1
   misn.markerRm(mem.misn_marker)
   misn.osdActive(2)
end

-- Spreads some Flyers
function spreadFlyers( qtt )
   local pos = player.pilot():pos()
   local vel = player.pilot():vel()
   local poC = pos - vel/vel:mod() * 100

   local cflyers = _flyers()
   for i = 1, qtt do
      local ppos = poC + vec2.newP( 10*rnd.sigma(), 2*math.pi*rnd.rnd() )
      system.addGatherable( cflyers, 1, ppos, vel * (.4+.2*rnd.sigma()), .3 + .7*rnd.rnd() )
   end
end

-- Activates hostility
function spawnHostiles()
   mem.dynFact:setPlayerStanding( -100 )
   for i = 1, 2 do
      pilot.add( "Dvaered Vendetta", mem.dynFact, mem.pnt )
   end
   local a = pilot.add( "Dvaered Vendetta", mem.dynFact, mem.pnt )
   local msg = { _("That is an insult to our Lord's honour!"),
                 _("Hey! Smart ass! Did you think about the environmental consequences of what you just did?"),
                 _("Everyone! Get that ship!"),
                 _("Come back! I have got candies for you... and MACE ROCKETS!"),
                 _("You spread posters on my planet. My turn to spread rockets on your face!") }
   a:comm( player.pilot(), msg[rnd.rnd(1,#msg)] )
end

function abort()
   finish( false )
end

function land()
   if mem.misn_state == 1 then

      if spob.cur() == mem.pnt then -- Should not have landed here!
         vntk.msg( _("What are you doing here?"), fmt.f(_([[As you step out of your ship, your eye is catched by a flying poster, carried away by the wind. And suddentry, you remember there was written on your mission pad: 'Land on any Dvaered planet or station (except {pnt})'. A group of angry-looking Dvaered soldiers comes at you: maybe you will discover soon why you were told not to land there...
Your mission is a FAILURE!]]),{pnt=mem.pnt}) ) -- That's not realistic the player doesn't get killed, but that's not realistic either anyone would land here anyways...
         finish( false )
      elseif spob.cur():faction() == faction.get("Dvaered") then -- Pay the player
         vntk.msg( _("Reward"), fmt.f(_("For accomplishing a Warlord Propaganda Mission, you are rewarded {pay}."),{pay=fmt.credits( mem.pay )}) )
         player.pay( mem.pay )
         faction.modPlayerSingle("Dvaered", rnd.rnd(1, 2))
         pir.reputationNormalMission(rnd.rnd(2,3))
         finish( true )
      end
   end
end
