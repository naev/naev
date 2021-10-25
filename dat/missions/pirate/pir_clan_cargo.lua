--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Clan Shipping">
 <avail>
  <priority>2</priority>
  <cond>faction.playerStanding("Pirate") &gt;= 20 and var.peek("ps_misn") ~= nil and var.peek("ps_misn") &gt;= 2</cond>
  <chance>10</chance>
  <location>Bar</location>
  <faction>Wild Ones</faction>
  <faction>Black Lotus</faction>
  <faction>Raven Clan</faction>
  <faction>Dreamer Clan</faction>
  <faction>Pirate</faction>
 </avail>
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
local fmt = require "format"
local portrait = require "portrait"


function create ()
   -- Note: this mission does not make any system claims.
   local landed = planet.cur():nameRaw()

   -- target destination
   local planets = {}
   for _, p in pairs({"Vorca", "New Haven", "Sanchez"}) do
      if p ~= landed then
         planets[#planets+1] = { planet.getS(p) }
      end
   end

   local index = rnd.rnd(1, #planets)
   dest = planets[index][1]
   sys = planets[index][2]

   misn.setNPC( _("Pirate Lord"), portrait.getMil("Pirate"), _("You see a pirate lord raving about something. A significant crowd has gathered around.") )
end


function accept ()
   misn.markerAdd( sys, "low" )

   -- Intro text
   if not tk.yesno( _("Spaceport Bar"), _([[It seems like this planet's clan is looking for a pilot to transport a package to another pirate world. Obviously, quite a few mercenaries or even fellow pirates would try to stop anyone transporting that package, and there is probably no need to say the only ways to the other pirate worlds are through hostile territory.

Will you accept the mission?]]) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission details
   reward = rnd.rnd(10,20) * 100e3 -- Hey, this mission is probably hell, after all.
   misn.setTitle(_("Clans trade"))
   misn.setReward( fmt.credits(reward) )
   misn.setDesc( string.format(_("Deliver some boxes to the pirate clan of %s, in the %s system."), dest:name(), sys:name()))

   -- Flavour text and mini-briefing
   tk.msg( _("Spaceport Bar"), string.format( _([[You roll up your sleeve and head off to your ship.]]), dest:name() ))
   misn.osdCreate(_("Spaceport Bar"), {_("Deliver some boxes to the pirate clan of %s, in the %s system."):format(dest:name(), sys:name())})

   -- Set up the goal
   local c = misn.cargoNew(_("Pirate Packages"), _("A bunch of pirate packages. You don't want to know what's inside."))
   packages = misn.cargoAdd(c, 5)
   hook.land("land")
   hook.enter("enter")
end


function land()
   local landed = planet.cur()
   if landed == dest then
      if misn.cargoRm(packages) then
         tk.msg( _("Mission Accomplished"), _("Your mission was a complete success! The clan you just gave the packages have already paid you.") )

         player.pay(reward)
         faction.modPlayerSingle("Pirate",5);

         n = var.peek("ps_clancargo_misn") or 0
         var.push("ps_clancargo_misn", n + 1)
         -- The first time this mission is done, the player’s max standing is
         -- increased by 5.
         if n == 0 then
             var.push("_fcap_pirate", var.peek("_fcap_pirate") + 5)
         end

         misn.finish(true)
      end
   end
end

function enter ()
   sys = system.cur()

   if misn_stage == 1 then

      -- Mercenaries appear after a couple of jumps
      jumped = jumped + 1
      if jumped <= 5 then
         return
      end

      -- The player might have to get through hostile territory. No need to add
      -- mercenaries everywhere.
      if rnd.rnd() < 0.5 then
         return
      end

      -- Get player position
      enter_vect = player.pos()

      -- Calculate where the enemies will be
      local r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if enter_vect:dist() < 1000 or r < 2 then
         local a = rnd.rnd() * 2 * math.pi
         local d = rnd.rnd( 400, 1000 )
         enter_vect:add( math.cos(a) * d, math.sin(a) * d )
         invoke_enemies()
      -- Enter after player
      else
         t = hook.timer(rnd.uniform( 2.0, 5.0 ) , "enemies")
      end
   end
end

function abort()
   misn.finish(false)
end


-- Mostly taken from es01.
function invoke_enemies()
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
   if rnd.rnd() < 0.9 then table.insert( merc, "Hyena" ) end

   -- Add mercenaries
   for k,v in ipairs(merc) do
      -- Move position a bit
      local a = rnd.rnd() * 2 * math.pi
      local d = rnd.rnd( 50, 75 )
      enter_vect:add( math.cos(a) * d, math.sin(a) * d )
      -- Add pilots
      local p = pilot.add( v, "Pirate", enter_vect, nil, {ai="mercenary"} )
      -- Set hostile
      for k2,v2 in ipairs(p) do
         v:setHostile()
      end
   end
end
