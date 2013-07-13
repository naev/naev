--[[

   A dangerous cargo mission for Pirate players, where you are attacked. On
   occasion.

   Author: bobbens                 (Empire Cargo 00)
      minor edits by Infiltrator   (Empire Cargo 00)
      major edits by Lukc

]]--

include "numstring.lua"
include "jumpdist.lua"
include "dat/missions/pirate/common.lua"

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   bar_desc = "You see a pirate lord raving about something. A significant crowd has gathered around."
   misn_title = "Clans trade"
   misn_reward = "%s credits"
   misn_desc = "Deliver some boxes to the pirate clan of %s, in the %s system."
   title = {}
   title[1] = "Spaceport Bar"
   title[2] = title[1]
   title[3] = "Mission Accomplished"
   text = {}
   text[1]  = [[It seems like this planet's clan is looking for a pilot to transport a package to another pirate world. Obviously, quite a few mercenaries or even fellow pirates would try to stop anyone transporting that package, and there is probably no need to say the only ways to the other pirate worlds are through hostile territory.
   
Will you accept the mission?]]
   text[2] = [[You roll up your sleeve and head off to your ship.]]
   text[3]   = "Your mission was a complete success! The clan you just gave the packages have already paid you."
end


function create ()
   -- Note: this mission does not make any system claims.
   local landed = planet.cur():name()

   -- target destination
   local planets = {}
   for _, p in pairs({"Vorca", "New Haven","Sanchez"}) do
      if p ~= landed then
         planets[#planets+1] = { planet.get(p) }
         planets[#planets][2] = planets[#planets][1]:system()
      end
   end

   local index = rnd.rnd(1, #planets)
   dest = planets[index][1]
   sys = planets[index][2]

   misn.setNPC( "Pirate Lord", pir_getLordRandomPortrait() )
   misn.setDesc( bar_desc )
end


function accept ()
   misn.markerAdd( sys, "low" )

   -- Intro text
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission details
   reward = rnd.rnd(10,20) * 100000 -- Hey, this mission is probably hell, after all.
   misn.setTitle(misn_title)
   misn.setReward( string.format(misn_reward, numstring(reward)) )
   misn.setDesc( string.format(misn_desc,dest:name(),sys:name()))

   -- Flavour text and mini-briefing
   tk.msg( title[2], string.format( text[2], dest:name() ))
   misn.osdCreate(title[2], {misn_desc:format(dest:name(),sys:name())})

   -- Set up the goal
   packages = misn.cargoAdd("Packages", 5)
   hook.land("land")
   hook.enter("enter")
end


function land()
   local landed = planet.cur()
   if landed == dest then
      if misn.cargoRm(packages) then
         tk.msg( title[3], text[3] )

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
      r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if enter_vect:dist() < 1000 or r < 2 then
         a = rnd.rnd() * 2 * math.pi
         d = rnd.rnd( 400, 1000 )
         enter_vect:add( math.cos(a) * d, math.sin(a) * d )
         invoke_enemies()
      -- Enter after player
      else
         t = hook.timer(rnd.int( 2000, 5000 ) , "enemies")
      end
   end
end

function abort()
   misn.finish(false)
end


-- Mostly taken from es01.
function invoke_enemies()
   -- Choose mercenaries
   merc = {}
   -- Note: New random numbers are *WANTED*.
   if rnd.rnd() < 0.1 then table.insert( merc, "Pirate Kestrel" ) end
   if rnd.rnd() < 0.2 then table.insert( merc, "Pirate Rhino" ) end
   if rnd.rnd() < 0.3 then table.insert( merc, "Pirate Rhino" ) end
   if rnd.rnd() < 0.4 then table.insert( merc, "Pirate Phalanx" ) end
   if rnd.rnd() < 0.5 then table.insert( merc, "Pirate Phalanx" ) end
   if rnd.rnd() < 0.6 then table.insert( merc, "Pirate Phalanx" ) end
   if rnd.rnd() < 0.7 then table.insert( merc, "Pirate Vendetta" ) end
   if rnd.rnd() < 0.8 then table.insert( merc, "Pirate Vendetta" ) end
   if rnd.rnd() < 0.9 then table.insert( merc, "Pirate Vendetta" ) end

   -- Add mercenaries
   for k,v in ipairs(merc) do
      -- Move position a bit
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 50, 75 )
      enter_vect:add( math.cos(a) * d, math.sin(a) * d )
      -- Add pilots
      p = pilot.add( v, "mercenary", enter_vect )
      -- Set hostile
      for k,v in ipairs(p) do
         v:setHostile()
      end
   end
end

