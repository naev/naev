--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 4">
 <unique />
 <location>Bar</location>
 <chance>100</chance>
 <spob>Minerva Station</spob>
 <done>Kex's Freedom 3</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]
--[[
   Freeing Kex 4

   Player has to go confront a guy in the frontier and is constantly harassed
   by bounty hunters while active. Fairly straight forward with a pretty hard
   fight at the end.
--]]
local minerva  = require "common.minerva"
local portrait = require 'portrait'
local love_audio = require 'love.audio'
local vn       = require 'vn'
local equipopt = require 'equipopt'
local reverb_preset = require 'reverb_preset'
local fmt = require "format"
local lmisn = require "lmisn"

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted try to go to targetplanet
--  1: fight with dude
--  2: return to kex
mem.misn_state = nil
local enemies, pjie, thug_leader, thug_pilots -- Non-persistent state

local targetplanet, targetsys = spob.getS("Jorlan")

-- TODO custom graphic?
mem.jie_portrait = portrait.get()
mem.jie_image = portrait.getFullPath(mem.jie_portrait)

local money_reward = minerva.rewards.kex4

function create ()
   if not misn.claim( targetsys ) then
      misn.finish( false )
   end
   misn.setReward( _("A step closer to Kex's freedom") )
   misn.setTitle( _("Freeing Kex") )
   misn.setDesc( fmt.f(_("You have been tasked with obtaining information from Jie de Luca at {pnt} in the {sys} system."), {pnt=targetplanet, sys=targetsys}) )

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   approach_kex()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end
   misn.accept()

   minerva.log.kex(_("You have agreed to help Kex obtain information from Jie de Luca.") )

   misn.osdCreate( _("Freeing Kex"),
      { fmt.f(_("Go to {pnt} in the {sys} system to find Jie de Luca"), {pnt=targetplanet, sys=targetsys} ),
      _("Return to Kex at Minerva Station") } )
   mem.misn_marker = misn.markerAdd( targetplanet )

   hook.land("generate_npc")
   hook.load("load_game")
   hook.enter("enter")

   generate_npc()
end

function load_game ()
   if mem.misn_state == 1 then
      mem.misn_state = 0
   end
   generate_npc()
end

function generate_npc ()
   if mem.misn_state==1 then
      lmisn.fail(_("You were supposed to take care of Jie!"))
   end

   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )

   elseif mem.misn_state==0 and spob.cur() == targetplanet then
      misn.npcAdd( "approach_jie", _("Jie de Luca"), mem.jie_portrait, _("You see an individual matching the description of Jie de Luca.") )

   end
end

function approach_kex ()
   love_audio.setEffect( "reverb_drugged", reverb_preset.drugged() )
   local pitch = 0.9
   if mem.misn_state==2 then
      pitch = 0.8
   end

   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex, { pitch=pitch, effect="reverb_drugged" } )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition("hexagon")

   -- Mission is over
   if mem.misn_state==2 then
      vn.na(_("You head towards Kex's usual spot."))
      kex(_([[He looks fairly tired.
"What's up, kid?"]]))
      vn.na(_("You explain that you weren't able to get anything out of Jie and that this time it was a failure."))
      kex(_([["I see. That can't be helped. You win some, you lose some."]]))
      kex(_([["While you were out I, think I got a lead on our next target, but it's not something I really wanted to deal with right now."]]))
      kex(_([["Here, take some credits, and meet me up here in a period or so, I have to finish double checking something."
He gives a sort of half-hearted grin and disappears into the shadows.]]))
      vn.disappear(kex)
      vn.sfxMoney()
      vn.func( function () player.pay( money_reward ) end )
      vn.na(fmt.reward( money_reward ))
      vn.sfxVictory()
      vn.na(_("You take your leave and wander back to the main station."))
      vn.run()

      minerva.log.kex(_("You took down Jie de Luca, who seemed to be a close friend and associate of the Minerva CEO."))
      misn.finish( true )
      return

   elseif mem.misn_state==nil then
      vn.na(_("You approach Kex, who is once again taking a break at his favourite spot at Minerva station."))
      kex(_([[He looks at you fairly somberly straight in the eyes.
"You ever wonder what's the meaning of all this?"]]))
      vn.menu{
         { _([["Nice to see you again too."]]), "intro1" },
         { _([["This?"]]), "intro1" },
         { _([["No."]]), "intro1" },
         { _([["What are you talking about?"]]), "intro1" },
      }
      vn.label("intro1")
      kex(_([["Well, I mean…"
He seems at a loss of words.
"You know…"]]))
      vn.menu{
         { p_("you to Kex", [["What?"]]), "intro2" },
         { _("…"), "intro2" },
      }
      vn.label("intro2")
      kex(_([[He lets out a deep sigh.
"It's just sort of pointless. Even if we somehow get rid of the CEO and I get free, nothing really changes, you know? I'm still a damn duck. It's not like I'm getting my life back. It's just sometimes it all seems so hopeless and arbitrary. Like we're just some sort of random noise without any real purpose."]]))
      kex(_([[Before you can speak, he keeps on going.
"Anyway, don't let my silly thoughts bother you. Let us get back to topic."
His eyes don't seem to have changed.]]))
      kex(_([["So I was putting together the pieces and found a close collaborator of the CEO, someone called Jie de Luca who seems to be situated in the frontier. They seem to go fairly far back with the CEO and probably have more information. As this is our best lead for now, I hate to ask this of you, but would you be able to check them out and see if we can get some more information?"]]))
      vn.menu{
         { _("Accept"), "accept" },
         { _("Decline"), "decline" },
      }
      vn.label("decline")
      kex(_([[He looks dejected.
"I see. If you change your mind, I'll be around."]]))
      vn.done()

      vn.label("accept")
      kex(fmt.f(_([["Thanks. Jie de Luca should be located at {pnt} in the {sys} system. I don't really know the relationship they have with the CEO, but unlike Baroness Eve or Major Malik, Jie isn't that important of a figure so you probably shouldn't have much of an issue dealing with them."]]), {pnt=targetplanet, sys=targetsys}))
      kex(_([["There shouldn't be much trouble, but I do think that we may have caused too much of a commotion, and I wouldn't be surprised if the CEO was starting to have suspicions. Make sure to be careful out there."]]))
      vn.func( function ()
         mem.misn_state = 0
      end )
   else
      vn.na(_("You find Kex taking a break at his favourite spot at Minerva station."))
   end

   vn.label("menu_msg")
   kex(_([["What's up kid?"]]))
   vn.menu( function ()
      local opts = {
         { _("Ask about the job"), "job" },
         { _("Ask if he's alright"), "alright" },
         { _("Leave"), "leave" },
      }
      return opts
   end )

   vn.label("job")
   kex(fmt.f(_([["Jie de Luca should be located at {pnt} in the {sys} system. I don't really know the relationship they have with the CEO, but unlike Baroness Eve or Major Malik, Jie isn't that important of a figure so you probably shouldn't have much of an issue dealing with them."]]), {pnt=targetplanet, sys=targetsys}))
   kex(_([["There shouldn't be much trouble, but I do think that we may have caused too much of a commotion, and I wouldn't be surprised if the CEO was starting to have suspicions. Make sure to be careful out there."]]))
   vn.jump("menu_msg")

   vn.label("alright")
   kex(_([["Been better, kid. Just had some sleepless nights, staring at the ceiling, unable to move, haunted by past regrets."
He looks visibly tired.
"You ever do anything you regret, kid?"]]))
   vn.menu{
      {_([["Of course"]]), "alright_ofcourse"},
      {_([["No, never"]]), "alright_nonever"},
   }
   vn.label("alright_ofcourse")
   kex(_([[He nods solemnly.
"We all have our inner demons, refusing to let go no matter how many cycles pass."]]))
   vn.jump("alright_cont1")
   vn.label("alright_nonever")
   kex(_([["I envy you kid. My regrets seem to follow me no matter how much I try to run away."]]))
   vn.label("alright_cont1")
   kex(_([["Lately, my past seems to haunt me. When I am able to sleep, I have been waking up in the middle of the night in cold sweat, well probably would if I had some damn sweat glands, gasping for breath."]]))
   kex(_([["I haven't really paid attention to it much, but it's getting worse and worse as time goes by. I'm sure it will all get better when this is over. I mean, it has to, right?"]]))
   kex(_([[After a short pause he continues. "You know, ever since I was a kid, I always loved adventure: being at the centre of everything and braving adversity. I loved the adrenaline rush and could never get enough of it all."]]))
   kex(_([["When I was getting older, I guess I tried to follow what everyone was doing and sort of settle down, but I was never very good at staying still. Whenever I stayed in the same place too long I got all itchy. It's like my soul was yearning to go out and adventure."]]))
   kex(_([["Even when I had my first child, I just couldn't stay put. I just had to go out. But now…"
His biological eye looks a bit hazy.
"I guess you don't appreciate what you have until you lose it all."]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

function approach_jie ()
   vn.clear()
   vn.scene()
   local jie = vn.newCharacter( _("Jie de Luca"), {image=mem.jie_image} )
   vn.transition()
   vn.na(_("You approach Jie de Luca who seems to be reading some sort of document while nonchalantly sipping a drink."))
   jie(_([[Without looking up they address you.
"So you're the one causing a ruckus here and there. You look a bit different from what I expected."
They look up at you.]]))
   jie(_([["Have you come to threaten me? Perhaps kill me?"
They speak calmly, without so much as a tinge of urgency.]]))
   vn.menu{
      { _("Ask about Minerva Station"), "cont1" },
      { _("Ask about the Minerva CEO"), "cont1" },
   }
   vn.label("cont1")
   jie(_([["I see. I knew the day would come. Can't have a bunch of low-lives making a decent living, now, can we?"
They take a sip from their drink.]]))
   jie(_([["Me and the guy you call the Minerva CEO, we were born here in Jorlan. Not sure if we were orphaned or abandoned, but since a young age this world has offered us nothing but cruelty and suffering. Forced to work in the ore mines to survive."]]))
   jie(_([["This shithole taught us that nothing is given for free. You have to fight with tooth and nail to get what you want! No that's not it, what we deserve!"
They hit the table for emphasis.]]))
   jie(_([["This universe is rotten to the core you know. If you don't want to wind up as a husk of a human, toiling for the benefit of others, you have to take what you damn want. I'm not going to wind up as another corpse deep down underground."]]))
   jie(_([["I don't think a hired gun like you would truly understand the kind of suffering of nearly working to death in the ore mines. You're just here to do your master's bidding, like the loyal dog you are."
They give a small chuckle and lean forward.]]))
   jie(_([["Well honestly, I don't give a shit who your master is. The result is the same, you are trying to take what we worked hard for with our blood, sweat, and tears."]]))
   jie(_([[They clench their teeth.
"I won't let you! Minerva Station is OURS! You have no idea of the hell we went through to get here, and I'll be damned if I let that get taken away from us!"]]))
   vn.disappear(jie, "slideleft")
   vn.na(_([[In the blink of an eye they fling their drink at you. You manage to dodge most of it, but as you look back at them you see the table is empty. Shit, that went well…
You rush to your ship to see if you can catch them in pursuit!]]))
   vn.run()

   -- Advance
   mem.misn_state = 1
   player.takeoff()
   misn.osdCreate( _("Freeing Kex"),
      { _("Deal with Jie de Luca"),
      _("Return to Kex at Minerva Station") } )
end

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

function enter ()
   if mem.misn_state==1 and system.cur() ~= targetsys then
      lmisn.fail(_("You were supposed to take care of Jie!"))
   end

   local function spawn_thugs( pos, dofollow )
      thug_leader = nil -- Clear
      local thugs = {
         choose_one{ "Starbridge", "Admonisher", "Phalanx" },
         choose_one{ "Lancelot", "Vendetta", "Shark", "Hyena" },
         choose_one{ "Lancelot", "Vendetta", "Shark", "Hyena" },
      }
      local pp = player.pilot()
      if pp:ship():size() > 4 then
         table.insert( thugs, 1, choose_one{ "Pacifier", "Vigilance" } )
         table.insert( thugs, choose_one{ "Ancestor", "Lancelot" } )
         table.insert( thugs, choose_one{ "Ancestor", "Lancelot" } )
      end
      local fbh = faction.dynAdd( "Mercenary", "kex_bountyhunter", _("Bounty Hunter"), {ai="mercenary"} )
      thug_pilots = {}
      for k,v in ipairs(thugs) do
         local ppos = pos + vec2.new( rnd.rnd()*200, rnd.rnd()*360 )
         local p = pilot.add( v, fbh, ppos, nil, {naked=true} )
         equipopt.pirate( p )
         if not thug_leader then
            thug_leader = p
         else
            p:setLeader( thug_leader )
         end
         table.insert( thug_pilots, p )
      end

      -- Try to make sure they meet up the player
      thug_leader:control()
      if dofollow then
         thug_leader:follow( pp )
      else
         thug_leader:brake()
      end
      mem.thug_following = dofollow
   end

   mem.thug_chance = mem.thug_chance or 0.2
   if mem.misn_state==0 and system.cur() == targetsys and player.pos():dist( targetplanet:pos() ) > 1000 then
      mem.thug_chance = mem.thug_chance / 0.8
      -- Spawn thugs around planet
      spawn_thugs( targetplanet:pos(), false )
      hook.timer( 5, "thug_heartbeat" )

   elseif mem.misn_state~=1 and rnd.rnd() < mem.thug_chance then
      -- Make sure system isn't claimed, but we don't claim it (inclusive test)
      if naev.claimTest( system.cur(), true ) then
         -- Spawn near the center, they home in on player
         spawn_thugs( vec2.newP(0.7*system.cur():radius()*rnd.rnd(), rnd.angle()), false )
         -- Timer
         hook.timer( 5, "thug_heartbeat" )
      end

   elseif mem.misn_state==1 then
      -- Main stuff
      pilot.clear()
      pilot.toggleSpawn(false)

      local pos = targetplanet:pos() + vec2.new( 3000, rnd.rnd()*360 )
      pjie = pilot.add("Kestrel", "Independent", pos, _("Jie de Luca"), {naked=true, ai="baddie_norun"})
      equipopt.generic( pjie, nil, "elite" )
      pjie:setHostile(true)
      pjie:setHilight(true)

      hook.pilot( pjie, "death", "jie_death" )
      hook.pilot( pjie, "board", "jie_board" )

      -- Henchmen
      local henchmen = {
         "Shark",
         "Shark",
         "Hyena",
      }
      local pp = player.pilot()
      if pp:ship():size() > 4 then
         table.insert( henchmen, "Pacifier" )
         table.insert( henchmen, "Ancestor" )
         table.insert( henchmen, "Ancestor" )
      end
      enemies = { pjie }
      for k,v in ipairs(henchmen) do
         local ppos = pos + vec2.new( rnd.rnd()*200, rnd.rnd()*360 )
         local p = pilot.add( v, "Independent", ppos, nil, {naked=true, ai="baddie_norun"} )
         equipopt.pirate( p )
         p:setLeader( pjie )
         p:setHostile(true)
         table.insert( enemies, p )
      end

      hook.timer( 3, "jie_takeoff" )
   end
end

function thug_heartbeat ()
   if not thug_leader or not thug_leader:exists() then return end
   local det, fuz = thug_leader:inrange( player.pilot() )
   if det and fuz then
      -- Start the attack, should be close enough to aggro naturally
      thug_leader:control(false)
      for k,p in ipairs(thug_pilots) do
         p:setHostile(true)
      end

      local msglist = {
         _("Looks like we found our target!"),
         _("That's the one!"),
         _("Time to collect our bounty!"),
         _("Target locked. Engaging."),
      }
      -- Broadcast after hostile
      thug_leader:broadcast( msglist[ rnd.rnd(1,#msglist) ], true )

      -- Decrease chance
      mem.thug_chance = mem.thug_chance * 0.8

      -- Reset autonav just in case
      player.autonavReset( 5 )
      return
   end

   -- Only chase if not hidden
   local pp = player.pilot()
   if pp:flags("stealth") then
      if mem.thug_following then
         thug_leader:taskClear()
         thug_leader:brake()
         mem.thug_following = false
      end
   else
      if not mem.thug_following then
         thug_leader:taskClear()
         thug_leader:follow( pp )
         mem.thug_following = true
      end
   end

   -- Keep on beating
   hook.timer( 1, "thug_heartbeat" )
end

function jie_death ()
   misn.markerMove( mem.misn_marker, spob.get("Minerva Station") )
   misn.osdActive(2)
   mem.misn_state = 2

   hook.timer( 5, "jie_epilogue" )
end

function jie_board ()
   vn.clear()
   vn.scene()
   local cjie = vn.Character.new( _("Jie de Luca"), {image=mem.jie_image} )
   vn.transition()
   vn.na(_("You board the ship with your weapons drawn and make your way to the command centre. You don't encounter any resistance on the way there."))
   vn.na(_("Eventually you reach the command centre and cautiously enter. The room is empty except for a chair in the centre with its back facing towards you."))
   vn.appear(cjie)
   vn.na(_("Slowly the chair turns to reveal Jie de Luca, holding their head from which blood is gushing out. In their other hand they clutch a weapon, but it doesn't really look like they are in any condition to use it."))
   cjie(_([["Feeling proud of yourself dog? Wagging your tail thinking about getting a prize from your master?"
They cough and you can see some blood drip from the side of their mouth.]]))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   cjie(_([["This is why we can't have nice things…"
As their head slouches down, you hear an alarm blaring, and a voice announcing "Self-destruct imminent!".]]))
   vn.na(_("You leave Jie behind as you rush to get back to your ship to get away from the explosion."))
   vn.run()

   pjie:setHealth(-1,-1)
   player.unboard()
end

function jie_takeoff ()
   pjie:broadcast(_("You aren't leaving this alive, dog!"))
end

function jie_epilogue ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_("As the debris of Jie's ship disperses you realize you never got around to questioning them or getting any information at all…"))
   vn.na(_("Might be best to head back to Kex for now and see what can be done."))
   vn.run()

   -- Spawns come back
   pilot.toggleSpawn(true)
end
