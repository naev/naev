--[[

   The FLF Catastrophe
   Copyright (C) 2018 Julie Marchant <onpon4@riseup.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

include "fleethelper.lua"
include "misnhelper.lua"
include "dat/missions/flf/flf_common.lua"


title = {}
text = {}

title[1] = _("Catastrophe Looms")
text[1] = _([[As you enter the bar on Sindbad, you immediately know that something is wrong. Everyone is frantic and you sense dread around your comrades. You are about to ask around when Benito approaches.]])

text[2] = _([["%s, it's horrible," she says with a look of dread in her eyes. "They found us. The damn Empire found us! I'm going to be frank, I don't even know if we can survive this." You stammer for a moment. No, it can't be true! It has to be some mistake! How can the FLF be defeated like this? Now the commotion makes perfect sense.
    "It's those damn traitors!" Benito continues. "One of them went off and told the Empire where our base is, and they even told them where our hidden jumps are! This is terrible!"]])

text[3] = _([["Listen, we don't have much longer. A combined Empire and Dvaered fleet is just about to enter from %s. They have Peacemakers, Hawkings, Goddards, Pacifiers, you name it. They're ready to wipe us off the map. %s, I don't know how to say this, but..." Benito digs in her pockets a bit before pulling out a small data chip. "Here, take this," she says as she presses it into your hand. "That's plan B. I hope it doesn't come to that, but... keep it safe, just in case."
    Benito clears her throat. "But I'm sure you can guess what your mission is. You need to join with the others in the fight against the incoming Empire ships. You have to destroy all of the ships. I will be here helping to man the ship; our defenses are weak, but better than nothing."]])

text[4] = _([[Silence seems to engulf the room. You see the mouths of your comrades frantically talking, but you cannot hear them. Benito puts her hand on your shoulder and looks at you in the eye. "Don't die, soldier," she says, only this time, it's not in the usual playful tone you tend to expect of her. She then hurriedly exits toward the station's weapon controls room.]])

text[5] = _([[There's no time to lose. You go to the hangar bay and immediately take off.]])

title[6] = _("Escape and Live On")
text[6] = _([[As the last shot penetrates the hull of Sindbad, a sense of dread comes over you. The FLF, Benito, your comrades... no...
    Just as you think this, the now exploding station hails you. You immediately accept, and you see Benito on your screen once again. You hear sirens and explosions in the background, but you pay them no mind.]])

text[7] = _([["Well, this is it, soldier. My last transmission to you. I can't say I wanted it to go this way, but...
    "Listen. That chip I handed you before? It's a map. It shows the location of a hidden jump from Iris to an unknown system deep in the nebula. Go straight there, right now. Escape the Empire, find what lies beyond, and live on. Perhaps, in your future travels, you'll find a way to destroy the Dvaereds, and the Empire, once and for all." Benito smiles as more and more of the station detonates around her. "Goodbye, %s. Stay vigilant." The transmission then cuts as you are forced to watch Sindbad finally erupt in a fiery explosion.]])


function create ()
   if not evt.claim( system.cur() ) then
      evt.finish( false )
   end

   emp_srcsys = system.get( "Arcanis" )
   emp_shptypes = {
      "Empire Lge Attack", "Empire Shark", "Empire Shark",
      "Empire Shark" }
   emp_minsize = 6

   bar_hook = hook.land( "enter_bar", "bar" )
   abort_hook = hook.enter( "takeoff_abort" )
end


function enter_bar ()
   local flf_missions = {
      "FLF Commodity Run", "Eliminate a Dvaered Patrol",
      "Divert the Dvaered Forces", "Eliminate an Empire Patrol",
      "FLF Pirate Disturbance", "Rogue FLF" }
   if player.jumps() >= 5 and not anyMissionActive( flf_missions ) then
      if bar_hook ~= nil then hook.rm( bar_hook ) end
      if abort_hook ~= nil then hook.rm( abort_hook ) end
      music.stop()
      music.load( "tension" )
      music.play()
      tk.msg( title[1], text[1] )
      tk.msg( title[1], text[2]:format( player.name() ) )
      tk.msg( title[1], text[3]:format( emp_srcsys:name(), player.name() ) )
      tk.msg( title[1], text[4] )
      tk.msg( title[1], text[5] )

      takeoff_hook = hook.enter( "takeoff" )
      player.takeoff()
   end
end


function takeoff_abort ()
   evt.finish( false )
end


function takeoff ()
   if takeoff_hook ~= nil then hook.rm( takeoff_hook ) end

   pilot.toggleSpawn( false )
   pilot.clear()

   local nf, ss, s, shptypes

   ss, s = planet.get( "Sindbad" )

   nf = pilot.add( "Sindbad", "flf_norun", ss:pos() )
   flf_base = nf[1]
   flf_base:rmOutfit( "all" )
   flf_base:rmOutfit( "cores" )
   flf_base:addOutfit( "Dummy Systems" )
   flf_base:addOutfit( "Dummy Plating" )
   flf_base:addOutfit( "Dummy Engine" )
   flf_base:addOutfit( "Base Ripper MK2", 8 )
   flf_base:setVisible()
   flf_base:setHilight()
   hook.pilot( flf_base, "death", "pilot_death_sindbad" )

   -- Spawn FLF ships
   shptypes = {
      "FLF Pacifier", "FLF Lancelot", "FLF Vendetta", "FLF Lancelot",
      "FLF Vendetta", "FLF Lancelot", "FLF Vendetta" }
   flf_ships = addShips( shptypes, "flf_norun", ss:pos(), 5 )
   for i, j in ipairs( flf_ships ) do
      j:setVisible()
      j:memory( "aggressive", true )
   end

   -- Spawn Empire ships
   emp_ships = addShips( emp_shptypes, "empire_norun", emp_srcsys )
   for i, j in ipairs( emp_ships ) do
      j:setHostile()
      j:setVisible()
      hook.pilot( j, "death", "pilot_death_emp" )
      if rnd.rnd() < 0.5 then
         j:control()
         j:attack( flf_base )
      end
   end

   -- Spawn Dvaered ships
   shptypes = {
      "Dvaered Goddard", "Dvaered Vigilance", "Dvaered Vigilance",
      "Dvaered Phalanx", "Dvaered Ancestor", "Dvaered Ancestor",
      "Dvaered Ancestor", "Dvaered Vendetta", "Dvaered Vendetta",
      "Dvaered Vendetta", "Dvaered Vendetta" }
   dv_ships = addShips( shptypes, "dvaered_norun", emp_srcsys )
   for i, j in ipairs( dv_ships ) do
      j:setHostile()
      j:setVisible()
   end

   diff.apply( "flf_dead" )
   player.pilot():setNoJump( true )
end


function pilot_death_emp( pilot, attacker, arg )
   local emp_alive = {}
   for i, j in ipairs( emp_ships ) do
      if j:exists() then
         emp_alive[ #emp_alive + 1 ] = j
      end
   end

   if #emp_alive < emp_minsize or rnd.rnd() < 0.1 then
      emp_ships = emp_alive
      local nf = addShips( emp_shptypes, "empire_norun", emp_srcsys )
      for i, j in ipairs( nf ) do
         j:setHostile()
         j:setVisible()
         hook.pilot( j, "death", "pilot_death_emp" )
         if rnd.rnd() < 0.5 then
            j:control()
            if flf_base:exists() then
               j:attack( flf_base )
            end
         end
         emp_ships[ #emp_ships + 1 ] = j
      end
   end
end


function pilot_death_sindbad( pilot, attacker, arg )
   music.stop()
   music.load( "machina" )
   music.play()
   music.delay( "ambient", 270 )
   music.delay( "combat", 270 )

   player.pilot():setInvincible()
   player.cinematics()
   camera.set( flf_base )

   tk.msg( title[6], text[6] )
   tk.msg( title[6], text[7]:format( player.name() ) )
   player.pilot():setNoJump( false )
   flf_setReputation( 100 )
   faction.get("FLF"):setPlayerStanding( 100 )
   player.addOutfit( "Map: Inner Nebula Secret Jump" )

   if diff.isApplied( "flf_pirate_ally" ) then
      diff.remove( "flf_pirate_ally" )
   end

   for i, j in ipairs( emp_ships ) do
      if j:exists() then
         j:control( false )
         j:changeAI( "empire" )
         j:setVisible( false )
      end
   end
   for i, j in ipairs( dv_ships ) do
      if j:exists() then
         j:changeAI( "dvaered" )
         j:setVisible( false )
      end
   end

   pilot.toggleSpawn( true )
   hook.timer( 8000, "timer_plcontrol" )
end


function timer_plcontrol ()
   camera.set( player.pilot() )
   player.cinematics( false )
   hook.timer( 2000, "timer_end" )
end


function timer_end ()
   player.pilot():setInvincible( false )
   evt.finish( true )
end
