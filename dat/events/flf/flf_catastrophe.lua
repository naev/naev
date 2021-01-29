--[[
<?xml version='1.0' encoding='utf8'?>
<event name="FLF Catastrophe">
 <trigger>enter</trigger>
 <chance>70</chance>
 <cond>system.cur() == system.get("Sigur") and faction.get("FLF"):playerStanding() &gt;= 98 and player.misnDone("The FLF Split")</cond>
 <notes>
  <done_misn name="The FLF Split"/>
  <campaign>Save the Frontier</campaign>
 </notes>
</event>
--]]
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

require "fleethelper"
require "misnhelper"
require "missions/flf/flf_common"


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

text[12] = _([["I'm sure you're wondering who we are. We are the Thurion, a civilization left over from one of the Empire's former secret projects. Have you met the Za'lek? The Collective? Those too were the Empire's "great projects", as were the now dead Proteron. But we were kept a lot more in the dark.
    "You see, when the Empire gave up on Project Thurion many cycles ago, they tried to kill us so that word of our existence would never get out. They thought us an embarrassment to the Empire. Little did they know, however, that we had discovered a method of uploading the human mind to a computer, and so when they came after us, we uploaded ourselves and escaped into what are now our core systems to rebuild."]])

text[13] = _([["Now, about half of us are uploaded, myself included. This face you see and this voice you hear are but projections, a reflection of myself that I have freely chosen. Being uploaded is wonderful, I must say; there is no suffering, we are immortal, and we can have whatever human experiences we desire and much more. And when we decide we have lived to our fullest, we can delete ourselves and disappear into the emptiness just like any other person does when they die. It's not uncommon for people to choose to self-terminate after they have been uploaded for about 200 cycles or so. I, however, have stuck around from the very beginning. I very much enjoy seeing the biological Thurion grow, learn, and join us in uploaded consciousness when their human bodies grow old and weary.
    "Sadly, I don't think you can be uploaded. The uploading process tends to fail with people who have even the slightest brain damage, and you outsiders usually drink far too many brain-damaging substances. That said, the time to check if you are eligible for uploading is many cycles away anyhow, so we'll see when we get there, should you have an interest in being uploaded."]])

text[14] = _([[Fascinated, you finally speak up, prompting a smile from the uploaded Thurion and a fairly long, friendly discussion about her experiences as an uploaded Thurion and your experience as a pilot on the outside. You find out that her name is Alicia. Somewhere in the conversation, a pair of human Thurion guards enters the room. They ask Alicia how it went, and she says that you've been properly introduced to Thurion culture and can be trusted to roam free. The two guards then smile and shake your hand. "Welcome to the nebula, %s," one of them says. "You now have permission to roam Thurion space freely and conduct your business. Of course, I trust you won't reveal our secret location to anyone. That would be just as bad for you as it would be for us."
    You affirm that you will keep the Thurion's secret safe. "Yes, welcome," Alicia says. "And do check out our bars and mission computers from time to time. We very well might have some missions for you in the future. In the meantime, buy yourself one of our nebula-resistant ships, and make yourself comfortable. You are our honored guest and, I hope, the first of many outsiders to learn the wonders of our way of life."
    The guards then promptly but politely escort you back to your ship, which has been refueled while you were gone. This should be an interesting experience....]])

log_text_flf = _([[The Empire discovered Sindbad. Try as you might, you and your comrades could not stop the combined onslaught of the Empire and the Dvaereds, and Sindbad erupted in a fiery explosion, killing Benito and all of your other comrades who were within Sindbad. Before the station exploded, Benito gave you a map leading into the unknown reaches of the inner nebula and told you to use the map to find what lies within in the hopes that one day, you can help the FLF rise again and defeat the Dvaereds once and for all. Her last words were short, but memorable: "Goodbye, %s. Stay vigilant."]])

log_text_betrayal = _([[You turned on the FLF and helped the Empire and the Dvaereds destroy Sindbad for some reason.]])

log_text_thurion = _([[Having braved the nebula, you were introduced to the Thurion by Alicia, one of many uploaded Thurion. The Thurion are the remnants of a secret project initiated by the Empire, Project Thurion. The Za'lek, the Collective, and the now-dead Proteron were also "great projects" of the Empire, but Project Thurion was seen as an embarrassment, prompting the Empire to attempt to kill the Thurion so word of their existence wouldn't get out. However, the Thurion learned a way to upload the human mind to a computer, allowing them to escape and rebuild.
    Now, the Thurion have formed a secret civilization. About half of the Thurion population is uploaded and Alicia was quick to extol the virtues of being uploaded, but also noted that you probably can't be uploaded due to a likelihood of excessive brain damage.
    In any case, you have earned the Thurion's trust and have been granted permission to roam Thurion space freely. You have promised to keep the Thurion's secret safe. Alicia has said that the Thurion may have missions for you in the future and has also recommended that you buy one of the Thurion's nebula-resistant ships.]])


function create ()
   if not evt.claim( system.cur() ) then
      evt.finish( false )
   end

   emp_srcsys = system.get( "Arcanis" )
   emp_shptypes = {
      "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot",
      "Empire Admonisher", "Empire Admonisher", "Empire Admonisher", "Empire Pacifier", "Empire Hawking",
      "Empire Peacemaker", "Empire Shark", "Empire Shark", "Empire Shark" }
   emp_minsize = 6
   found_thurion = false
   player_attacks = 0

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
      var.push( "music_off", true )
      tk.msg( title[1], text[1] )
      tk.msg( title[1], text[2]:format( player.name() ) )
      tk.msg( title[1], text[3]:format( emp_srcsys:name(), player.name() ) )
      tk.msg( title[1], text[4] )
      tk.msg( title[1], text[5] )
      var.pop( "music_off" )

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

   local ss, s

   ss, s = planet.get( "Sindbad" )

   flf_base = pilot.add( "Sindbad", "FLF", ss:pos(), nil, "flf_norun" )
   flf_base:rmOutfit( "all" )
   flf_base:rmOutfit( "cores" )
   flf_base:addOutfit( "Dummy Systems" )
   flf_base:addOutfit( "Dummy Plating" )
   flf_base:addOutfit( "Dummy Engine" )
   flf_base:addOutfit( "Base Ripper MK2", 8 )
   flf_base:setVisible()
   flf_base:setHilight()
   hook.pilot( flf_base, "attacked", "pilot_attacked_sindbad" )
   hook.pilot( flf_base, "death", "pilot_death_sindbad" )

   -- Spawn FLF ships
   shptypes = {"Pacifier", "Lancelot", "Vendetta", "Lancelot", "Vendetta", "Lancelot", "Vendetta"}
   shpnames = {_("FLF Pacifier"), _("FLF Lancelot"), _("FLF Vendetta"), _("FLF Lancelot"), _("FLF Vendetta"), _("FLF Lancelot"), _("FLF Vendetta")}
   flf_ships = addShips( 5, shptypes, "FLF", ss:pos(), shpnames, "flf_norun" )
   for i, j in ipairs( flf_ships ) do
      j:setVisible()
      j:memory( "aggressive", true )
   end

   -- Spawn Empire ships
   emp_ships = addShips( 1, emp_shptypes, "Empire", emp_srcsys, nil, "empire_norun" )
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
   dv_ships = addShips( 1, shptypes, "Dvaered", emp_srcsys, nil, "dvaered_norun" )
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
      local nf = addShips( 1, emp_shptypes, "Empire", emp_srcsys, nil, "empire_norun" )
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


function pilot_attacked_sindbad( pilot, attacker, arg )
   if attacker == player.pilot()
         and faction.get("FLF"):playerStanding() > -100 then
      -- Punish the player with a faction hit every time they attack
      faction.get("FLF"):modPlayer(-10)
   end
end


function pilot_death_sindbad( pilot, attacker, arg )
   player.pilot():setNoJump( false )
   pilot.toggleSpawn( true )

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

   if attacker == player.pilot()
         or faction.get("FLF"):playerStanding() < 0 then
      -- Player decided to help destroy Sindbad for some reason. Set FLF
      -- reputation to "enemy", add a log entry, and finish the event
      -- without giving the usual rewards.
      faction.get("FLF"):setPlayerStanding( -100 )
      flf_addLog( log_text_betrayal )
      evt.finish( true )
   end

   music.stop()
   music.load( "machina" )
   music.play()
   var.push( "music_wait", true )

   player.pilot():setInvincible()
   player.cinematics()
   camera.set( flf_base )

   tk.msg( title[6], text[6] )
   tk.msg( title[6], text[7]:format( player.name() ) )
   flf_setReputation( 100 )
   faction.get("FLF"):setPlayerStanding( 100 )
   flf_addLog( log_text_flf )
   player.addOutfit( "Map: Inner Nebula Secret Jump" )
   hook.jumpin( "jumpin" )
   hook.land( "land" )
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
      var.push( "music_wait", true )
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
      flf_addLog( log_text_thurion )
   elseif diff.isApplied( "Thurion_found" ) then
      diff.remove( "Thurion_found" )
   end
   var.pop( "music_wait" )
   evt.finish( true )
end
