--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Clan Shipping">
 <priority>2</priority>
 <cond>faction.playerStanding("Pirate") &gt;= 20 and var.peek("ps_misn") ~= nil and var.peek("ps_misn") &gt;= 2</cond>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Wild Ones</faction>
 <faction>Black Lotus</faction>
 <faction>Raven Clan</faction>
 <faction>Dreamer Clan</faction>
 <faction>Pirate</faction>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[

   A dangerous cargo mission for Pirate players, where you are attacked. On
   occasion.

   Author: bobbens                 (Empire Cargo 00)
      minor edits by Infiltrator   (Empire Cargo 00)
      major edits by Lukc

]]--
local fleet = require "fleet"
local fmt = require "format"
local pir = require "common.pirate"
local portrait = require "portrait"
local vn = require "vn"
local lmisn = require "lmisn"
local vntk = require "vntk"

local npc_name = _("Pirate Lord")
local npc_portrait = portrait.getMil("Pirate")
local npc_image = portrait.getFullPath( npc_portrait )

function create ()
   -- Note: this mission does not make any system claims.
   local landed = spob.cur():nameRaw()

   -- target destination
   local planets = {}
   for k, p in pairs({"Vorca", "New Haven", "Sanchez"}) do
      if p ~= landed then
         planets[#planets+1] = { spob.getS(p) }
      end
   end

   local index = rnd.rnd(1, #planets)
   mem.dest = planets[index][1]
   mem.sys = planets[index][2]

   misn.setNPC( npc_name, npc_portrait, _("You see a pirate lord raving about something. A significant crowd has gathered around.") )
end


function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local p = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   p(_([[It seems like this planet's clan is looking for a pilot to transport a package to another pirate world. Obviously, quite a few mercenaries or even fellow pirates would try to stop anyone transporting that package, and there is probably no need to say the only ways to the other pirate worlds are through hostile territory.

Will you accept the mission?]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   vn.na(_([[You roll up your sleeves and head off to your ship.]]))
   vn.run()

   if not accepted then return end

   misn.markerAdd( mem.dest, "low" )

   -- Accept the mission
   misn.accept()

   -- Mission details
   mem.reward = rnd.rnd(10,20) * 100e3 -- Hey, this mission is probably hell, after all.
   misn.setTitle(_("Clans trade"))
   misn.setReward(mem.reward)
   misn.setDesc( fmt.f(_("Deliver some boxes to the pirate clan of {pnt}, in the {sys} system."), {pnt=mem.dest, sys=mem.sys}) )

   -- Flavour text and mini-briefing
   misn.osdCreate(_("Spaceport Bar"), {
      fmt.f(_("Deliver some boxes to the pirate clan of {pnt}, in the {sys} system."), {pnt=mem.dest, sys=mem.sys}),
   })

   -- Set up the goal
   local c = commodity.new(N_("Pirate Packages"), N_("A bunch of pirate packages. You don't want to know what's inside."))
   mem.packages = misn.cargoAdd(c, 5)
   mem.misn_stage = 1
   mem.jumped = 0
   hook.land("land")
   hook.enter("enter")
end


function land()
   local landed = spob.cur()
   if landed ~= mem.dest then
      return
   end

   player.pay(mem.reward)
   lmisn.sfxVictory()
   vntk.msg( _("Mission Accomplished"), _("Your mission was a complete success! The clan you just gave the packages have already paid you.").."\n\n"..fmt.reward(mem.reward) )

   faction.modPlayerSingle("Pirate",5)

   local n = var.peek("ps_clancargo_misn") or 0
   var.push("ps_clancargo_misn", n + 1)
   -- The first time this mission is done, the player’s max standing is
   -- increased by 5.
   if n == 0 then
      pir.modReputation( 5 )
   end

   misn.finish(true)
end

function enter ()
   mem.sys = system.cur()

   if mem.misn_stage == 1 then
      -- Mercenaries appear after a couple of jumps
      mem.jumped = mem.jumped + 1
      if mem.jumped <= 5 then
         return
      end

      -- The player might have to get through hostile territory. No need to add
      -- mercenaries everywhere.
      if rnd.rnd() < 0.5 then
         return
      end

      -- Get player position
      local enter_vect = player.pos()

      -- Calculate where the enemies will be
      local r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if enter_vect:dist() < 1000 or r < 2 then
         enter_vect:add( vec2.newP( rnd.rnd(400, 1000), rnd.angle() ) )
         invoke_enemies( enter_vect )
      -- Enter after player
      else
         hook.timer(rnd.uniform( 2.0, 5.0 ), "invoke_enemies", enter_vect)
      end
   end
end

-- Mostly taken from es01.
function invoke_enemies( enter_vect )
   -- Choose mercenaries
   local merc = {}
   -- Note: New random numbers are *WANTED*.
   if rnd.rnd() < 0.1 then table.insert( merc, "Pirate Kestrel" ) end
   if rnd.rnd() < 0.2 then table.insert( merc, "Pirate Starbridge" ) end
   if rnd.rnd() < 0.3 then table.insert( merc, "Pirate Rhino" ) end
   if rnd.rnd() < 0.4 then table.insert( merc, "Pirate Admonisher" ) end
   if rnd.rnd() < 0.5 then table.insert( merc, "Pirate Phalanx" ) end
   if rnd.rnd() < 0.6 then table.insert( merc, "Pirate Ancestor" ) end
   if rnd.rnd() < 0.7 then table.insert( merc, "Pirate Vendetta" ) end
   if rnd.rnd() < 0.8 then table.insert( merc, "Pirate Shark" ) end
   if rnd.rnd() < 0.9 then table.insert( merc, "Pirate Hyena" ) end

   -- Add mercenaries
   local flt = fleet.add( 1, merc, "Pirate", enter_vect, nil, {ai="mercenary"} )
   for k,p in ipairs(flt) do
      p:setHostile()
   end
end
