--[[

   The FLF Catastrophe

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

require "fleethelper.lua"
require "misnhelper.lua"
require "dat/missions/flf/flf_common.lua"


title = {}
text = {}

title[1] = _("Catastrophe Looms")
text[1] = _([[As you enter the bar on Sindbad, you immediately know that something is wrong. Everyone is frantic and you sense dread around your comrades. You are about to ask around when Benito approaches.]])

text[2] = _([["%s, it's horrible," she says with a look of dread in her eyes. "They found us. The damn Empire found us! I'm going to be frank, I don't even know if we can survive this." You stammer for a moment. No, it can't be true! It has to be some mistake! How can the FLF be defeated like this? Now the commotion makes perfect sense.
    "It's those damn traitors!" Benito continues. "One of them went off and told the Empire where our base is, and they even told them where our hidden jumps are! This is terrible!"]])

text[3] = _([["Listen, we don't have much longer. A combined Empire and Dvaered fleet is just about to enter from %s. They have Peacemakers, Hawkings, Goddards, Pacifiers, you name it. They're ready to wipe us off the map. %s, I don't know how to say this, but..." Benito digs in her pockets a bit before pulling out a small data chip. "Here, take this," she says as she presses it into your hand. "That's plan B. I hope it doesn't come to that, but... keep it safe, just in case."
    Benito clears her throat. "But I'm sure you can guess what your mission is. You need to join with the others in the fight against the incoming Empire ships. You have to destroy all of the ships. I will be here helping to man the station; our defenses are weak, but better than nothing."]])

text[4] = _([[Silence seems to engulf the room. You see the mouths of your comrades frantically talking, but you cannot hear them. Benito puts her hand on your shoulder and looks at you in the eye. "Don't die, soldier," she says, only this time, it's not in the usual playful tone you tend to expect of her. She then hurriedly exits toward the station's weapon controls room.]])

text[5] = _([[There's no time to lose. You go to the hangar bay and immediately take off.]])

title[6] = _("Escape and Live On")
text[6] = _([[As the last shot penetrates the hull of Sindbad, a sense of dread comes over you. The FLF, Benito, your comrades... no...
    Just as you think this, the now exploding station hails you. You immediately accept, and you see Benito on your screen once again. You hear sirens and explosions in the background, but you pay them no mind.]])

text[7] = _([["Well, this is it, soldier. My last transmission to you. I can't say I wanted it to go this way, but...
    "Listen. That chip I handed you before? It's a map. It shows the location of a hidden jump from Iris to an unknown system deep in the nebula. Go straight there, right now. Escape the Empire, find what lies beyond, and live on. Perhaps, in your future travels, you'll find a way to destroy the Dvaereds, and the Empire, once and for all." Benito smiles as more and more of the station detonates around her. "Goodbye, %s. Stay vigilant." The transmission then cuts as you are forced to watch Sindbad finally erupt in a fiery explosion.]])

title[8] = _("Strange Happenings")
text[8] = _([[Your instruments go haywire and your ship goes dark. You swear under your breath as you try to get the system back online. This isn't good. How could your core system fail, and at such a critical moment? Is this going to be the end for you, just like Benito and your other comrades moments ago?
    Just as you start to panic, the power comes back on and you breathe a sigh of relief. An unfamiliar face appears on your viewscreen. She glares silently for what seems like an eternity. Finally, she speaks up.]])

text[9] = _([["So the FLF is dead, I see. Well, not so much dead as a shadow of its former self. I can see that you did manage to more or less achieve your goal; Dvaered forces have largely withdrawn from Frontier space. I suppose that was the double-edged sword you wielded when you got pirates involved. The Empire was never going to allow you to live once you did that, but it did give you the edge you needed to weaken the Dvaered forces temporarily.
    "Not that it matters anyway, of course. The ultimate result will be the same either way. But that's neither here nor there. FLF pilot, I can see that you have acquired some insider knowledge. That said, you still know nothing in the grand scheme of things. So I'll tell you what: proceed to Metsys if you dare. I will explain everything then. Be warned: the nebula here is highly unstable, so your shields had better be up to par. If you don't have the guts for it, turn back now while you still can. But if you do, you turn back for good."]])

text[10] = _([[Before you can even respond, the mysterious figure disappears. You detect no presences nearby, but you notice that your ship has been refueled somehow! Odd. In any case, it looks like you have two choices: do you brave the dangers of the nebula and proceed to Metsys, or do you turn back now? The figure said that you can only make this decision once, so you'd better make sure it's the right one.]])

title[11] = _("Welcome To the Nebula")
text[11] = _([[As you land on the mysterious station, armed guards immediately surround your ship and order you out into the hangar. You comply, and they take you to a room that appears to be an interrogation room, where you wait for a few nerve-wracking hectoseconds. Finally, a holoscreen flickers on, showing the figure you had seen earlier. She looks in your direction.
    "Well met," she says. "I see you have made the right choice and survived the nebula. Very good."]])

text[12] = _([["I'm sure you're wondering who we are. We are the Thurion, a superior people and one of the Empire's former secret projects. Have you met the Za'lek? The Collective? Those too were the Empire's "great projects", as were the now dead Proteron. But we were kept a lot more in the dark.
    "You see, when the Empire gave up on Project Thurion many cycles ago, they tried to kill us so that word of our existence would never get out. They thought us an embarrassment to the Empire. Little did they know, however, that we had discovered a method of uploading the human mind to a computer, and so when they came after us, we uploaded ourselves and escaped into what are now our core systems to rebuild."]])

text[13] = _([["Now, most of us are uploaded, myself included. This face you see and this voice you hear are but projections, a reflection of myself that I have freely chosen. Being uploaded is wonderful, I must say; there is no suffering, we are immortal, and we can have whatever human experiences we desire and much more. And when we decide we have lived to our fullest, we can delete ourselves and disappear into the emptiness just like any other person does when they die. It's not uncommon for people to choose to self-terminate after they have been uploaded for about 200 cycles or so. I, however, have stuck around from the very beginning. I very much enjoy seeing the biological Thurion grow, learn, and join us in uploaded consciousness when their human bodies grow old and weary.
    "Sadly, I don't think you can be uploaded. The uploading process tends to fail with people who have even the slightest brain damage, and you outsiders usually drink far too many brain-damaging substances. That said, the time to check if you are eligible for uploading is many cycles away anyhow, so we'll see when we get there, should you have an interest in being uploaded."]])

text[14] = _([[Fascinated, you finally speak up, prompting a smile from the uploaded Thurion and a fairly long, friendly discussion about her experiences as an uploaded Thurion and your experience as a pilot on the outside. You find out that her name is Alicia. Somewhere in the conversation, a pair of human Thurion guards enters the room. They ask Alicia how it went, and she says that you've been properly introduced to Thurion culture and can be trusted to roam free. The two guards then smile and shake your hand. "Welcome to the nebula, %s," one of them says. "You now have permission to roam Thurion space freely and conduct your business. Of course, I trust you won't reveal our secret location to anyone. That would be just as bad for you as it would be for us."
    You affirm that you will keep the Thurion's secret safe. "Yes, welcome," Alicia says. "And do check out our bars and mission computers from time to time. We very well might have some missions for you in the future. In the meantime, buy yourself one of our nebula-resistant ships, and make yourself comfortable. You are our honored guest and, I hope, the first of many outsiders to learn the wonders of our way of life."
    With that, the guards escort you back to your ship, which has been refueled while you were gone. This should be an interesting experience....]])


function create ()
   if not evt.claim( system.cur() ) then
      evt.finish( false )
   end

   emp_srcsys = system.get( "Arcanis" )
   emp_shptypes = {
      "Empire Lge Attack", "Empire Shark", "Empire Shark",
      "Empire Shark" }
   emp_minsize = 6
   found_thurion = false

   bar_hook = hook.land( "enter_bar", "bar" )
   abort_hook = hook.enter( "takeoff_abort" )
end


function enter_bar ()
   local flf_missions = {
      "FLF Commodity Run", "Eliminate a Dvaered Patrol",
      "Divert the Dvaered Forces", "Eliminate an Empire Patrol",
      "FLF Pirate Disturbance", "Rogue FLF" }
   if not anyMissionActive( flf_missions ) then
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
   hook.jumpin( "jumpin" )
   hook.land( "land" )

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
end


function jumpin ()
   if not found_thurion and system.cur() == system.get("Oriantis") then
      music.stop()
      music.load( "intro" )
      music.play()
      music.delay( "ambient", 154 )
      music.delay( "combat", 154 )
      hook.timer( 5000, "timer_thurion" )
   elseif found_thurion and system.cur() == system.get("Metsys") then
      diff.apply( "Thurion_found" )
   end
end


function timer_thurion ()
   found_thurion = true
   tk.msg( title[8], text[8] )
   tk.msg( title[8], text[9] )
   tk.msg( title[8], text[10] )
   player.refuel()
end


function land ()
   if planet.cur():faction() == faction.get("Thurion") then
      tk.msg( title[11], text[11] )
      tk.msg( title[11], text[12] )
      tk.msg( title[11], text[13] )
      tk.msg( title[11], text[14]:format( player.name() ) )
      faction.get("Thurion"):setKnown( true )
   elseif diff.isApplied( "Thurion_found" ) then
      diff.remove( "Thurion_found" )
   end
   evt.finish( true )
end
