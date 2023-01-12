--[[
<?xml version='1.0' encoding='utf8'?>
<event name="FLF Catastrophe">
 <location>enter</location>
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

--]]
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"
local fmt = require "format"
local lmisn = require "lmisn"
local cinema = require "cinema"

local dv_ships, emp_ships, flf_ships, flf_base -- State of battle phase (jumping out to save not allowed).

-- Event constants
local emp_srcsys = system.get( "Arcanis" )
local emp_shptypes = {
   "Empire Peacemaker", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot",
   "Empire Lancelot", "Empire Lancelot", "Empire Lancelot",
   "Empire Admonisher", "Empire Admonisher", "Empire Admonisher", "Empire Pacifier", "Empire Hawking",
   "Empire Shark", "Empire Shark", "Empire Shark" }
local emp_minsize = 6

function create ()
   if not evt.claim( system.cur() ) then
      evt.finish( false )
   end

   mem.found_thurion = false

   mem.bar_hook = hook.land( "enter_bar", "bar" )
   mem.abort_hook = hook.enter( "takeoff_abort" )
end


function enter_bar ()
   local flf_missions = {
      "FLF Commodity Run", "Eliminate a Dvaered Patrol",
      "Divert the Dvaered Forces", "Eliminate an Empire Patrol",
      "FLF Pirate Disturbance", "Rogue FLF" }
   if not lmisn.anyMissionActive( flf_missions ) then
      hook.rm( mem.bar_hook )
      hook.rm( mem.abort_hook )
      music.play( "tension.ogg" )
      tk.msg( _("Catastrophe Looms"), _([[As you enter the bar on Sindbad, you immediately know that something is wrong. Everyone is frantic and you sense dread around your comrades. You are about to ask around when Benito approaches.]]) )
      tk.msg( _("Catastrophe Looms"), fmt.f( _([["{player}, it's horrible," she says with a look of dread in her eyes. "They found us. The damn Empire found us! I'm going to be frank, I don't even know if we can survive this." You stammer for a moment. No, it can't be true! It has to be some mistake! How can the FLF be defeated like this? Now the commotion makes perfect sense.
    "It's those damn traitors!" Benito continues. "One of them went off and told the Empire where our base is, and they even told them where our hidden jumps are! This is terrible!"]]), {player=player.name()} ) )
      tk.msg( _("Catastrophe Looms"), fmt.f( _([["Listen, we don't have much longer. A combined Empire and Dvaered fleet is just about to enter from {sys}. They have Peacemakers, Hawkings, Goddards, Pacifiers, you name it. They're ready to wipe us off the map. {player}, I don't know how to say this, but..." Benito digs in her pockets a bit before pulling out a small data chip. "Here, take this," she says as she presses it into your hand. "That's plan B. I hope it doesn't come to that, but... keep it safe, just in case."
    Benito clears her throat. "But I'm sure you can guess what your mission is. You need to join with the others in the fight against the incoming Empire ships. You have to destroy all of the ships. I will be here helping to man the station; our defences are weak, but better than nothing."]]), {sys=emp_srcsys, player=player.name()} ) )
      tk.msg( _("Catastrophe Looms"), _([[Silence seems to engulf the room. You see the mouths of your comrades frantically talking, but you cannot hear them. Benito puts her hand on your shoulder and looks at you in the eye. "Don't die, soldier," she says, only this time, it's not in the usual playful tone you tend to expect of her. She then hurriedly exits toward the station's weapon controls room.]]) )
      tk.msg( _("Catastrophe Looms"), _([[There's no time to lose. You go to the hangar bay and immediately take off.]]) )

      mem.takeoff_hook = hook.enter( "takeoff" )
      player.takeoff()
   end
end


function takeoff_abort ()
   evt.finish( false )
end


function takeoff ()
   hook.rm( mem.takeoff_hook )

   pilot.toggleSpawn( false )
   pilot.clear()

   local ss = spob.get( "Sindbad" )

   flf_base = pilot.add( "Sindbad", "FLF", ss:pos(), nil, {ai="flf_norun", naked=true} )
   flf_base:outfitAdd( "Dummy Systems" )
   flf_base:outfitAdd( "Dummy Plating" )
   flf_base:outfitAdd( "Dummy Engine" )
   flf_base:outfitAdd( "Base Ripper MK2", 8 )
   flf_base:setVisible()
   flf_base:setHilight()
   hook.pilot( flf_base, "attacked", "pilot_attacked_sindbad" )
   hook.pilot( flf_base, "death", "pilot_death_sindbad" )

   -- FLF are not a natural enemy of the Empire, we have to enforce that
   local factflf = faction.dynAdd( "FLF", "flf_laststand", _("FLF") )
   factflf:dynEnemy( "Empire" )

   -- Spawn FLF ships
   local norm = (jump.get( system.cur(), emp_srcsys ):pos()-ss:pos()):normalize()
   local shptypes = {"Pacifier", "Lancelot", "Vendetta", "Lancelot", "Vendetta", "Lancelot", "Vendetta"}
   flf_ships = {}
   for i=1,5 do
      for k,s in ipairs(shptypes) do
         local pos = ss:pos() + vec2.newP(2000*rnd.rnd(), rnd.angle()) + norm*1000
         local p = pilot.add( s, factflf, pos, nil, {ai="guard"} )
         p:setVisible()
         table.insert( flf_ships, p )
      end
   end

   -- Spawn Empire ships
   emp_ships = fleet.add( 1, emp_shptypes, "Empire", emp_srcsys, nil, {ai="empire_norun"} )
   for i, j in ipairs( emp_ships ) do
      j:setHostile()
      j:setVisible()
      hook.pilot( j, "death", "pilot_death_emp" )
      if rnd.rnd() < 0.5 then
         j:control()
         j:attack( flf_base )
      end
      local aimem = j:memory()
      aimem.enemyclose = nil
   end

   -- Spawn Dvaered ships
   shptypes = {
      "Dvaered Goddard", "Dvaered Vigilance", "Dvaered Vigilance",
      "Dvaered Phalanx", "Dvaered Ancestor", "Dvaered Ancestor",
      "Dvaered Ancestor", "Dvaered Vendetta", "Dvaered Vendetta",
      "Dvaered Vendetta", "Dvaered Vendetta" }
   dv_ships = fleet.add( 1, shptypes, "Dvaered", emp_srcsys, nil, {ai="dvaered_norun"} )
   for i, j in ipairs( dv_ships ) do
      j:setHostile()
      j:setVisible()
      local aimem = j:memory()
      aimem.enemyclose = nil
   end

   diff.remove("FLF_base") -- Get rid of Sindbad
   diff.apply( "flf_dead" )
   player.pilot():setNoJump( true )
end


function pilot_death_emp( _pilot, _attacker, _arg )
   local emp_alive = {}
   for i, j in ipairs( emp_ships ) do
      if j:exists() then
         emp_alive[ #emp_alive + 1 ] = j
      end
   end

   if #emp_alive < emp_minsize or rnd.rnd() < 0.1 then
      emp_ships = emp_alive
      local nf = fleet.add( 1, emp_shptypes, "Empire", emp_srcsys, nil, {ai="empire_norun"} )
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
         local aimem = j:memory()
         aimem.enemyclose = nil
      end
   end
end


function pilot_attacked_sindbad( _pilot, attacker, _arg )
   if attacker and attacker:withPlayer()
         and faction.get("FLF"):playerStanding() > -100 then
      -- Punish the player with a faction hit every time they attack
      faction.get("FLF"):modPlayer(-10)
   end
end


function pilot_death_sindbad( pilot, attacker, _arg )
   player.pilot():setNoJump( false )
   pilot.toggleSpawn( true )

   if diff.isApplied( "flf_pirate_ally" ) then
      diff.remove( "flf_pirate_ally" )
   end

   for i, j in ipairs( flf_ships ) do
      if j:exists() then
         j:control( false )
         j:changeAI( "flf_norun" )
         j:setVisible( false )
      end
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

   if (attacker and attacker:withPlayer())
         or faction.get("FLF"):playerStanding() < 0 then
      -- Player decided to help destroy Sindbad for some reason. Set FLF
      -- reputation to "enemy", add a log entry, and finish the event
      -- without giving the usual rewards.
      faction.get("FLF"):setPlayerStanding( -100 )
      flf.addLog( _([[You turned on the FLF and helped the Empire and the Dvaereds destroy Sindbad for some reason.]]) )
      evt.finish( true )
   end

   music.play( "machina.ogg" )
   var.push( "music_wait", true )

   cinema.on()
   camera.set( flf_base, true )

   tk.msg( _("Escape and Live On"), _([[As the last shot penetrates the hull of Sindbad, a sense of dread comes over you. The FLF, Benito, your comrades... no...
    Just as you think this, the now exploding station hails you. You immediately accept, and you see Benito on your screen once again. You hear sirens and explosions in the background, but you pay them no mind.]]) )
   tk.msg( _("Escape and Live On"), fmt.f( _([["Well, this is it, soldier. My last transmission to you. I can't say I wanted it to go this way, but...
    "Listen. That chip I handed you before? It's a map. It shows the location of a hidden jump from Iris to an unknown system deep in the nebula. Go straight there, right now. Escape the Empire, find what lies beyond, and live on. Perhaps, in your future travels, you'll find a way to destroy the Dvaereds, and the Empire, once and for all." Benito smiles as more and more of the station detonates around her. "Goodbye, {player}. Stay vigilant." The transmission then cuts as you are forced to watch Sindbad finally erupt in a fiery explosion.]]), {player=player.name()} ) )
   flf.setReputation( 100 )
   faction.get("FLF"):setPlayerStanding( 100 )
   flf.addLog( fmt.f( _([[The Empire discovered Sindbad. Try as you might, you and your comrades could not stop the combined onslaught of the Empire and the Dvaereds, and Sindbad erupted in a fiery explosion, killing Benito and all of your other comrades who were within Sindbad. Before the station exploded, Benito gave you a map leading into the unknown reaches of the inner nebula and told you to use the map to find what lies within in the hopes that one day, you can help the FLF rise again and defeat the Dvaereds once and for all. Her last words were short, but memorable: "Goodbye, {player}. Stay vigilant."]]), {player=player.name()} ) )
   player.outfitAdd( "Map: Inner Nebula Secret Jump" )
   hook.jumpin( "jumpin" )
   hook.land( "land" )
   hook.timer( 8.0, "timer_plcontrol" )
end


function timer_plcontrol ()
   cinema.off()
   camera.set( player.pilot(), true )
   hook.timer( 2.0, "timer_end" )
end


function timer_end ()
   player.pilot():setInvincible( false )
end


function jumpin ()
   if not mem.found_thurion and system.cur() == system.get("Oriantis") then
      music.stop()
      music.play( "intro.ogg" )
      var.push( "music_wait", true )
      hook.timer( 5.0, "timer_thurion" )
   elseif mem.found_thurion and system.cur() == system.get("Metsys") then
      diff.apply( "Thurion_found" )
   end
end


function timer_thurion ()
   mem.found_thurion = true
   tk.msg( _("Strange Happenings"), _([[Your instruments go haywire and your ship goes dark. You swear under your breath as you try to get the system back online. This isn't good. How could your core system fail, and at such a critical moment? Is this going to be the end for you, just like Benito and your other comrades moments ago?
    Just as you start to panic, the power comes back on and you breathe a sigh of relief. An unfamiliar face appears on your viewscreen. She glares silently for what seems like an eternity. Finally, she speaks up.]]) )
   tk.msg( _("Strange Happenings"), _([["So the FLF is dead, I see. Well, not so much dead as a shadow of its former self. I can see that you did manage to more or less achieve your goal; Dvaered forces have largely withdrawn from Frontier space. I suppose that was the double-edged sword you wielded when you got pirates involved. The Empire was never going to allow you to live once you did that, but it did give you the edge you needed to weaken the Dvaered forces temporarily.
    "Not that it matters anyway, of course. The ultimate result will be the same either way. But that's neither here nor there. FLF pilot, I can see that you have acquired some insider knowledge. That said, you still know nothing in the grand scheme of things. So I'll tell you what: proceed to Metsys if you dare. I will explain everything then. Be warned: the nebula here is highly unstable, so your shields had better be up to par. If you don't have the guts for it, turn back now while you still can. But if you do, you turn back for good."]]) )
   tk.msg( _("Strange Happenings"), _([[Before you can even respond, the mysterious figure disappears. You detect no presences nearby, but you notice that your ship has been refueled somehow! Odd. In any case, it looks like you have two choices: do you brave the dangers of the nebula and proceed to Metsys, or do you turn back now? The figure said that you can only make this decision once, so you'd better make sure it's the right one.]]) )
   player.refuel()
end


function land ()
   if spob.cur():faction() == faction.get("Thurion") then
      tk.msg( _("Welcome To the Nebula"), _([[As you land on the mysterious station, armed guards immediately surround your ship and order you out into the hangar. You comply, and they take you to a room that appears to be an interrogation room, where you wait for a few nerve-wracking hectoseconds. Finally, a holoscreen flickers on, showing the figure you had seen earlier. She looks in your direction.
    "Well met," she says. "I see you have made the right choice and survived the nebula. Very good."]]) )
      tk.msg( _("Welcome To the Nebula"), _([["I'm sure you're wondering who we are. We are the Thurion, a civilization left over from one of the Empire's former secret projects. Have you met the Za'lek? The Collective? Those too were the Empire's "great projects", as were the now dead Proteron. But we were kept a lot more in the dark.
    "You see, when the Empire gave up on Project Thurion many cycles ago, they tried to kill us so that word of our existence would never get out. They thought us an embarrassment to the Empire. Little did they know, however, that we had discovered a method of uploading the human mind to a computer, and so when they came after us, we uploaded ourselves and escaped into what are now our core systems to rebuild."]]) )
      tk.msg( _("Welcome To the Nebula"), _([["Now, about half of us are uploaded, myself included. This face you see and this voice you hear are but projections, a reflection of myself that I have freely chosen. Being uploaded is wonderful, I must say; there is no suffering, we are immortal, and we can have whatever human experiences we desire and much more. And when we decide we have lived to our fullest, we can delete ourselves and disappear into the emptiness just like any other person does when they die. It's not uncommon for people to choose to self-terminate after they have been uploaded for about 200 cycles or so. I, however, have stuck around from the very beginning. I very much enjoy seeing the biological Thurion grow, learn, and join us in uploaded consciousness when their human bodies grow old and weary.
    "Sadly, I don't think you can be uploaded. The uploading process tends to fail with people who have even the slightest brain damage, and you outsiders usually drink far too many brain-damaging substances. That said, the time to check if you are eligible for uploading is many cycles away anyhow, so we'll see when we get there, should you have an interest in being uploaded."]]) )
      tk.msg( _("Welcome To the Nebula"), fmt.f( _([[Fascinated, you finally speak up, prompting a smile from the uploaded Thurion and a fairly long, friendly discussion about her experiences as an uploaded Thurion and your experience as a pilot on the outside. You find out that her name is Alicia. Somewhere in the conversation, a pair of human Thurion guards enters the room. They ask Alicia how it went, and she says that you've been properly introduced to Thurion culture and can be trusted to roam free. The two guards then smile and shake your hand. "Welcome to the nebula, {player}," one of them says. "You now have permission to roam Thurion space freely and conduct your business. Of course, I trust you won't reveal our secret location to anyone. That would be just as bad for you as it would be for us."
    You affirm that you will keep the Thurion's secret safe. "Yes, welcome," Alicia says. "And do check out our bars and mission computers from time to time. We very well might have some missions for you in the future. In the meantime, buy yourself some nebula-resistant coating, and make yourself comfortable. You are our honored guest and, I hope, the first of many outsiders to learn the wonders of our way of life."
    The guards then promptly but politely escort you back to your ship, which has been refueled while you were gone. This should be an interesting experience....]]), {player=player.name()} ) )
      faction.get("Thurion"):setKnown( true )
      flf.addLog( _([[Having braved the nebula, you were introduced to the Thurion by Alicia, one of many uploaded Thurion. The Thurion are the remnants of a secret project initiated by the Empire, Project Thurion. The Za'lek, the Collective, and the now-dead Proteron were also "great projects" of the Empire, but Project Thurion was seen as an embarrassment, prompting the Empire to attempt to kill the Thurion so word of their existence wouldn't get out. However, the Thurion learned a way to upload the human mind to a computer, allowing them to escape and rebuild.
    Now, the Thurion have formed a secret civilization. About half of the Thurion population is uploaded and Alicia was quick to extol the virtues of being uploaded, but also noted that you probably can't be uploaded due to a likelihood of excessive brain damage.
    In any case, you have earned the Thurion's trust and have been granted permission to roam Thurion space freely. You have promised to keep the Thurion's secret safe. Alicia has said that the Thurion may have missions for you in the future and has also recommended that you buy one of the Thurion's nebula-resistant ships.]]) )
   elseif diff.isApplied( "Thurion_found" ) then
      diff.remove( "Thurion_found" )
   end
   var.pop( "music_wait" )
   evt.finish( true )
end
