--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 5">
 <unique />
 <location>Bar</location>
 <chance>100</chance>
 <spob>Minerva Station</spob>
 <done>Kex's Freedom 4</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Freeing Kex 5

   Player has to go see Dr. Strangelove but lots of bounty hunters around. Has
   to defeat them to check out the station, which is set to explode. Afterwards
   gets a message from Dr. Strangelove for a long last sermon before going back
   to Kex. Player is constantly harassed by thugs while mission is active.
--]]
local minerva  = require "common.minerva"
local love_audio = require 'love.audio'
local vn       = require 'vn'
local equipopt = require 'equipopt'
local love_shaders = require 'love_shaders'
local reverb_preset = require 'reverb_preset'
local lmusic   = require 'lmusic'
local fmt = require "format"
local lmisn = require "lmisn"

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted try to go find Dr. Strangelove
--  1: Destroy Bounty Hunter
--  2: meet Dr. Strangelove
--  3: return to kex
mem.misn_state = nil
local strangelove_ship, thug_leader, thug_pilots -- Non-persistent state

local landed_lab -- Forward-declared functions

local targetplanet = spob.get("Strangelove Lab")
local targetsys = system.get("Westhaven") --same as targetplanet:system(), but only after the below diff gets applied

local eccdiff = "strangelove"

local money_reward = minerva.rewards.kex5

function create ()
   if not misn.claim( targetsys ) then
      misn.finish( false )
   end
   misn.setReward( _("A step closer to Kex's freedom") )
   misn.setTitle( _("Freeing Kex") )
   misn.setDesc( fmt.f(_("Kex wants you to kill Dr. Strangelove at {pnt} in the {sys} system."), {pnt=targetplanet, sys=targetsys}) )

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   love_audio.setEffect( "reverb_drugged", reverb_preset.drugged() )

   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex, {pitch=0.65, effect="reverb_drugged"} )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition( "hexagon" )

   vn.na(_("You approach Kex, who is sort of slumping at his usual spot. He doesn't seem to be in very good condition."))
   kex(_([[It takes a bit before Kex notices your presence. His feathers are rather disheveled.
"Oh, hey kid."]]))
   kex(_([[It takes a while before he continues.
"Compared to the vastness of all the universe, we are completely insignificant. Our sorrows and glories go unheard and seemingly devoid of all meaning."]]))
   kex(_([["Coming to terms with one's own insignificance does give a clarity of mind and makes clear one's deepest desires. What has to be done then becomes clear."]]))
   kex(_([[Without blinking Kex states "I need you to kill Strangelove."]]))
   vn.menu{
      { _("Accept"), "accept" },
      { _("Decline"), "decline" },
   }
   vn.label("decline")
   kex(_([[He looks even more dejected.
"I see. I'll be around."]]))
   vn.done()

   vn.label("accept")
   kex(_([["I should have realized it sooner. It was obvious from the start that Strangelove would be involved in this travesty of a station. I found his name when reviewing some of the documents you gave me, then it was all clear."]]))
   kex(_([["There aren't too many trails to where he is right now, but I do think I know where to find him."]]))
   kex(_([["Wait, you know where he is? How you know is not important now, all I need you to do is go over there and end his rotten life once and for all."]]))
   kex(_([["The universe will be a much better place with scum like him gone, and I'll finally get my vengeance."]]))
   vn.disappear(kex)
   vn.na(fmt.f(_([[Without saying anything else, Kex waddles off stumbling into the darkness of the station. You feel like it is best to leave him alone right now and decide to go see Strangelove. He should be in the {sys} system.]]), {sys=targetsys}))

   vn.func( function ()
      mem.misn_state = 0
   end )
   vn.run()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end
   misn.accept()

   minerva.log.kex(_("You have agreed to help Kex deal with Dr. Strangelove.") )

   misn.osdCreate( _("Freeing Kex"),
      { fmt.f(_("Go find Dr. Strangelove in the {sys} system"), {sys=targetsys} ),
      _("Return to Kex at Minerva Station") } )
   mem.misn_marker = misn.markerAdd( targetsys )

   hook.enter("enter")
   hook.land("landed")
   hook.load("landed")
end

function landed ()
   -- Can't use spob.get() here for when the diff is removed
   if mem.misn_state==1 and spob.cur() == targetplanet then
      landed_lab()

   elseif mem.misn_state==3 and spob.get("Minerva Station")==spob.cur() then
      misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
   end
end

function approach_kex ()
   love_audio.setEffect( "reverb_drugged", reverb_preset.drugged() )
   local strangelove_death = var.peek("strangelove_death")
   local pitchlow = 0.7
   local pitchhigh = 1.1

   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex, {pitch=pitchlow, effect="reverb_drugged"} )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition( "hexagon" )
   vn.na(_("You approach Kex who is sort of slumping at his usual spot. He doesn't seem much better than last time you met him."))
   kex(_([["Hey kid."
He seems a bit nervious and speaks softer than usual.
"Did you do what I asked?"]]))

   if strangelove_death=="comforted" then
      vn.na(_([[You explain to him how the mission went, handing him the drone you found in the laboratory, and explain how you comforted the dying Dr. Strangelove as he passed away from his mysterious illness in front of your eyes.]]))
      kex(_([[He looks at you after your explanation and furrows his brows a bit.
"I can't believe you comforted him. Do you know all the shit he's done! The shit he's put me through!"]]))
      kex(_([["Damn it, not only am I not feeling any relief, but I can't even get angry at you."
He looks down sadly at the floor for a while before looking at you again.]]))

   else
      if strangelove_death=="shot" then
         vn.na(_([[You explain to him how the mission went and go into graphic detail of how you finished Dr. Strangelove off by shooting at him point-blank as Kex asked.]]))
         vn.na(_([[You also hand him the droid you recovered. Kex looks at it briefly before pocketing it somewhere inside his mat of feathers.]]))
      elseif strangelove_death=="unplug" then
         vn.na(_([[You explain to him how the mission went and go into details of how you finished Dr. Strangelove off by unplugging him as Kex asked.]]))
         vn.na(_([[You also hand him the droid you recovered. Kex looks at it briefly before pocketing it somewhere inside his mat of feathers.]]))
      else
         vn.na(_([[You explain to him how the mission went, handing him the drone you found in the laboratory, and explain how Dr. Strangelove died of his mysterious illness in front of your eyes.]]))
      end
      kex(_([[He looks at you after your explanation and furrows his brows a bit.
"I was expecting some sort of relief. You know, having Strangelove dead is all I ever wanted since I escaped from the laboratory. All I ever dreamed of, and when I finally get it… nothing?"]]))

   end

   kex(_([["He's dead. He's completely toasted! Why aren't we celebrating?! Why aren't we happy!?"]]))
   vn.animation( 1.5, function (progress, _dt)
      lmusic.setPitch( nil, pitchlow + (pitchhigh-pitchlow)*progress )
   end )
   kex(_([["We should be dancing!"]]))
   local function saddance ()
      vn.animation( 2.0, function (progress, _dt, offset)
         kex.offset = offset + (100/vn.display_w)*math.sin( 2 * math.pi * progress )
      end, nil, nil, function () return kex.offset end )
   end
   saddance()
   kex(_([["We should be partying!"]]))
   saddance()
   kex(_([["We should…"]]))
   vn.animation( 0.5, function (progress, _dt)
      lmusic.setPitch( nil, pitchhigh + (pitchlow-pitchhigh)*progress )
   end )
   kex(_([["Shit, who am I kidding."
He looks as glum as ever.
"You can't kill your way out of depression…"]]))
   kex(_([["Hey, kid, I'm really grateful for all you've done, but I don't think I can go on. I just need some time alone to gather my thoughts."]]))
   kex(_([["I've scrounged up some cash, I don't think I'll be needing it. Here take it."]]))

   local sfxparams = { pitch=pitchlow, effect="reverb_drugged" }
   vn.sfxMoney( sfxparams )
   vn.func( function () player.pay( money_reward ) end )
   vn.na(fmt.reward( money_reward ))
   vn.sfxVictory( sfxparams )

   vn.na(_([[Without saying another word, Kex slowly disappears into the shadows. It doesn't seem like he'll recover soon. Maybe there is another way to help him without getting him directly involved?]]))

   vn.run()

   minerva.log.kex(_("You reported back to Kex, who gave up on working for his freedom due to his crippling depression. Maybe there is another way to help him indirectly?"))

   misn.finish( true )
end

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

function enter ()
   if mem.misn_state==2 and system.cur() ~= targetsys then
      lmisn.fail(_("You never met up with Dr. Strangelove."))
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
      if mem.misn_state==0 then
         table.insert( thugs, 1, "Vigilance" )
      end
      local fbh = faction.dynAdd( "Mercenary", "kex_bountyhunter", _("Bounty Hunter"), {ai="baddie_norun"} )
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
   if system.cur() == targetsys then
      -- No spawns nor anything in Westhaven
      pilot.clear()
      pilot.toggleSpawn(false)

      if mem.misn_state == 0 then
         -- TODO better handling, maybe more fighting with drones and a close-up cinematic?
         mem.thug_chance = mem.thug_chance / 0.8
         local pos = targetplanet:pos()
         spawn_thugs( pos, false )
         hook.timer( 5, "thug_heartbeat" )
         player.landAllow( false, "#r".._("You are unable to land while the bounty hunters are still active.").."#0" )

         hook.timer( 3, "thug_check" )

         -- Add some disabled drones for effect
         for i=1,5 do
            local d
            if rnd.rnd() < 0.2 then
               d = "Za'lek Heavy Drone"
            else
               d = "Za'lek Light Drone"
            end
            local p = pilot.add( d, "Za'lek", pos + vec2.newP( 100+700*rnd.rnd(), rnd.angle() ) )
            p:setInvincible(true)
            p:setInvisible(true)
            p:disable()
         end

      -- State 1: Do nothing (just in case the player kills bounty hunters and goes off somewhere)

      elseif mem.misn_state == 2 then
         -- Should be taking off from the Lab

         -- Spawn
         local pos = targetplanet:pos() + vec2.new(2000, 5000)
         local p = pilot.add("Za'lek Sting", "Za'lek", pos, minerva.strangelove.name )
         p:setInvincible(true)
         p:setActiveBoard(true)
         p:control()
         p:brake()
         local aimem = p:memory()
         aimem.comm_no = _("No response.")
         hook.pilot( p, "board", "strangelove_board" )
         strangelove_ship = p
         hook.timer( 5, "strangelove_hail" )
      end

   elseif mem.misn_state~=1 and rnd.rnd() < mem.thug_chance then
      -- Make sure system isn't claimed, but we don't claim it (inclusive test)
      if naev.claimTest( system.cur(), true ) then
         -- Spawn near the center, they home in on player
         spawn_thugs( vec2.newP(0.7*system.cur():radius()*rnd.rnd(), rnd.angle()), false )
         -- Timer
         hook.timer( 5, "thug_heartbeat" )
      end
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
      if mem.misn_state==0 then
         thug_leader:broadcast( _("Looks like we have company!"), true )
      else
         thug_leader:broadcast( msglist[ rnd.rnd(1,#msglist) ], true )
      end

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

function thug_check ()
   local stillalive = {}
   for k,v in ipairs(thug_pilots) do
      if v:exists() then
         table.insert( stillalive, v )
      end
   end
   thug_pilots = stillalive
   if #thug_pilots == 0 then
      hook.timer( 4, "thugs_cleared" )
   else
      hook.timer( 3, "thug_check" )
   end
end

function thugs_cleared ()
   vn.clear()
   vn.scene()
   vn.transition(nil, 3)
   vn.na(_([[All that remains of the bounty hunters is floating debris. It seems like you should be able to safely land and search for Dr. Strangelove at his laboratory now.]]))
   vn.run()

   mem.misn_state = 1
   player.landAllow( true )
end

function landed_lab ()
   local check_living = false
   local check_rr = false
   local check_back = false

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_("Your ship sensors indicate that Strangelove's laboratory is no longer pressurized and without an atmosphere, so you don your space suit before entering."))
   vn.na(_("The first thing you notice is that the laboratory has been ravaged, likely by the Bounty Hunters you encountered outside. The place was already a mess the last time you came, but now you have to jump over obstacles as you progress through the laboratory."))

   vn.label("menu")
   vn.func( function ()
      if check_living and check_rr and check_back then
         vn.jump("check_done")
      end
   end )
   vn.menu( function ()
      local opts = {}
      if not check_living then
         table.insert( opts, {_("Check the Living Quarters"), "living" } )
      end
      if not check_rr then
         table.insert( opts, {_("Check the Recreation Room"), "recreation" } )
      end
      if not check_back then
         table.insert( opts, {_("Check the Back Room"), "back" } )
      end
      return opts
   end )

   vn.label("living")
   vn.na(_("You make your way through a tight, nearly impassible passageway towards the living quarters. Although it is a mess, you don't really see any signs of anyone living here. It rather seems all abandoned."))
   vn.na(_("Eventually, you reach a very small room with a bed capsule. The ground and bed are splattered with blood, but it seems very old and desiccated. Some empty medical syringes and pouches are scattered on the floor."))
   vn.na(_("You turn over everything, but find nothing of interest."))
   vn.func( function () check_living = true end )
   vn.jump("menu")

   vn.label("recreation")
   vn.na(_("You head towards, and eventually reach, the recreation room, which has been completely trashed. Even pieces of the wall have been ripped out, revealing shoddy cable connections and lots of weird mold."))
   vn.na(_("Looking around, you find that the hologram projector you used last time is, unsurprisingly, completely wrecked. It doesn't seem like you'll be able to use it to get in touch with Dr. Strangelove anymore."))
   vn.na(_("Try as you might, you are unable to find anything of interest."))
   vn.func( function () check_rr = true end )
   vn.jump("menu")

   vn.label("back")
   vn.na(_([[You find a tunnel leading towards the back room. It seems to have been blocked off once, but now has been torn wide open. You ignore the "KEEP OUT" sign that is bent up underfoot and make your way through.]]))
   vn.na(_("The tunnel twists and turns as it heads towards the centre of the asteroid. There is no lighting along the way, so you rely on the lamps of your suit."))
   vn.na(_("Eventually you reach a room at the core of the asteroid. It is fairly large compared to the rest of the laboratory, but is jammed with junk and rubbish, lots of which has been recently moved around."))
   vn.na(_("You start to sift through the things to try to find something of interest, but after a long time of looking around and not making significant progress, you decide to take a break and sit on a fallen bookcase."))
   vn.na(_("As you survey the room, you notice a small movement next to your feet. Slowly and carefully you lift up some debris to uncover a small and heavily damaged droid."))
   vn.na(_("You pick up the droid and dust it off. Its primary lens and hover engine seem completely destroyed, but it does seem like it still has power and likely data in it. You try to interface to it, but it seems to be locked with strong encryption. You pocket it as it seems like it might be something of interest to Kex."))
   vn.na(_("You get back to searching around the stuff and end up finding nothing else of interest."))
   vn.func( function ()
      check_back = true
      minerva.log.kex(_("You collected a small robot in Dr. Strangelove's laboratory."))
   end )
   vn.jump("menu")

   vn.label("check_done")
   vn.na(_("You spend your time carefully going through the entire station, but it ends up being mostly in vain: Dr. Strangelove is nowhere to be found. At least you found a droid that might be useful."))
   vn.na(_("You let out a sigh as you head outside. Maybe Dr. Strangelove escaped somewhere else?"))
   vn.run()

   -- Take off
   mem.misn_state = 2
   player.takeoff()

   -- Disable landing, will get disabled on entering new system
   player.landAllow( false, _("You have better things to do right now.") )
end

function strangelove_hail ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.strangelove )
   local dr = vn.newCharacter( minerva.vn_strangelove{ shader=love_shaders.hologram() } )
   vn.transition( "electric", 3 )
   -- TODO small scene
   vn.na(_("As you loiter around the system, you suddenly receive an unexpected incoming transmission."))
   dr(_([[A hologram of Dr. Strangelove appears before you. He is laying down and his body seems to be sapped of all energy. He seems to muster up energy to vaguely look in your direction, although his eyes seem unnaturally clouded.
"Is that you?"]]))
   dr(_([["I've been expecting you."
A coughing fit wracks his body.
"There is not much time, come to my ship."]]))
   vn.na(_("The communication channel closes and you receive coordinates to a ship."))
   vn.done("electric")
   vn.run()

   strangelove_ship:setHilight(true)
   strangelove_ship:setVisplayer(true)
end

function strangelove_board ()
   var.pop("strangelove_death") -- Reset variable if player aborted mission after this for some reason
   local kexcount = 0

   vn.clear()
   vn.scene()
   local dr = minerva.vn_strangelove()
   vn.transition()
   -- TODO small scene
   vn.na(_("You cautiously board the ship, not sure what you are about to encounter. Trusting your system's sensors indicating a proper atmosphere in the ship, you board without your space suit. However, when you enter the ship, a strong, pungent odour makes you regret your decision."))
   vn.na(_("As you move to the command room, you notice small movements from the corner of your eyes. Upon closer inspection you make out all sorts of small moving objects, reminding you of cleaning droids on most ships, however, these move in a fairly clunky fashion, as if they had some of their moving apparatus damaged."))
   vn.na(_("You pay little attention to the small critters, which seem harmless, and proceed through the ship. After a short walk you enter the command room."))
   love_audio.setEffect( "reverb_psychotic", reverb_preset.psychotic() )
   vn.music( minerva.loops.strangelove, {effect="reverb_psychotic"} )
   vn.appear(dr)
   vn.na(_("You enter the room and meet Dr. Strangelove face-to-face for the first time. Dr. Strangelove is laid out in a medical bed connected to a lot of weird machines, with a medical droid attending him. If it weren't for the fact that everything is fairly rundown and dirty, you would think you were in a planetary hospital instead of a space ship at the edge of the universe."))
   dr(_("You take a close look at Dr. Strangelove, and he seems like a husk of what might have once been a man. His cheekbones protrude from his pale face and his laboratory coat seems to be a few sizes too large on his frail body. His vitals monitor also confirms your suspicion that he is more dead than alive."))
   dr(_([["I see you were able to make it."
He coughs violently.
"Sorry, I haven't been feeling too well lately."
He doesn't seem to be focusing much on you and is just talking in your general direction.]]))
   dr(_([["It is all going well. I think we are close to a breakthrough, the next time I should be able to succeed. After all this time, finally I'll be able to see the truth!"
His vital monitor's warning light flashes on.]]))
   dr(_([["You were right. It was and is all connected. I should have listened to you sooner, but I was foolish and naïve."
He lets out a sigh.]]))
   dr(_([["I thought I almost had it with the last specimen. It was all going so smoothly… The zero-point bionimorphic interface connection went flawlessly, and the entropy flux was nearly stable, but the connection was never established."]]))
   dr(_([["It could be the distance, but my calculations indicated that it shouldn't matter. It should be everywhere. Everything is everywhere! Folding onto itself and twisted around in higher dimensions. Measly Euclidean distances shouldn't matter at all."]]))
   dr(_([["…but it didn't work. Almost as if making a mockery of science itself and spitting in my face. Almost as if the universe itself is unwilling to give up its secrets."
He coughs violently.
"It failed. Like always. It failed."]]))
   dr(_([["Williams, why did you have to die? You know so much. You surpassed me in everything I could possibly imagine. You would have been able to make it work!"]]))
   vn.menu{
      {_("Ask about Kex"), "1kex"},
      {_("Say you are sorry"), "1cont"},
      {_("Mention he is delirious"), "1cont"},
      {_("…"), "1cont"},
   }

   vn.label("1kex")
   vn.func( function () kexcount = kexcount+1 end )
   vn.label("1cont")
   dr(_([[He goes on.
"You were always the brightest in the class. I always admired how fast you were able to solve partial differential equations so elegantly."]]))
   dr(_([["Remember that one time we nearly caused a subatomic implosion when the hyperphased quantum auto-cycle hit the self-refractive frequency? Your quick thinking saved us from turning the entire system into a sterile void."]]))
   dr(_([["If only you had been with me this entire time. We could have done so much together."
His voice tears up slightly.]]))
   dr(_([["Why did you have to do it?"]]))

   vn.menu( function ()
      local opts = {
         { _([["Do what?"]]), "2cont" },
         { _("…"), "2cont" },
      }
      if kexcount == 0 then
         table.insert( opts, 1, {_("Ask about Kex"), "2kex"} )
      else
         table.insert( opts, 1, {_("Ask about Kex again"), "2kex"} )
      end
      return opts
   end )

   vn.label("2kex")
   vn.func( function () kexcount = kexcount+1 end )
   vn.label("2cont")
   dr(_([["Why did you have to kill yourself?"
His sightless eyes look vacantly while tears flow down his face.]]))
   dr(_([[You don't know how to answer and silence envelopes the room. Finally, he breaks the calmness.
"I should have been the one to try to defuse the reactor core, not you. I should have been the one vaporized in the resulting explosion, not you."]]))
   dr(_([[The vitals monitor starts flashing red, and the medical droid begins to tweak and make adjustments to intravenous drips and other medical devices.
He coughs a bit and you can see blood flecks splatter onto his clothes.]]))
   dr(_([["If only our places had been switched, you would have been able to solve it and find me again. I was so close and yet it still feels like there is some sort of impenetrable wall between me and the solution. What did I miss, where did I go wrong?"
The vitals monitor is still flashing.]]))

   vn.menu( function ()
      local opts = {
         { _([["Everything."]]), "3conteverything" },
         { _([["It is already connected."]]), "3contalready" },
         { _([["It never connected."]]), "3contnever" },
         { _("…"), "3cont" },
      }
      if kexcount == 0 then
         table.insert( opts, 1, {_("Ask about Kex"), "3kex"} )
      elseif kexcount == 1 then
         table.insert( opts, 1, {_("Ask about Kex again"), "3kex"} )
      else
         table.insert( opts, 1, {_("Insist about Kex"), "kex_talk"} )
      end
      return opts
   end )

   vn.label("3conteverything")
   dr(_([["Everything? You mean I missed the fundamental nature of it? I see. What we are seeing is just the lower dimensional projection of an intrinsically higher dimension space. Then everything has to be rewritten."
He tries feebly to get up before coughs wrack his body and stain it further with specks of blood. He falls back to the bed impotently.]]))
   vn.jump("3cont")

   vn.label("3contalready")
   dr(_([["Already connected? You mean that there was no need to force a connection, we just weren't able to see it? Then there might be still time to redo the last experiment. Maybe if I recalibrate the oscillator I could…"
He tries feebly to get up before coughs wrack his body and stain it further with specks of blood. He falls back to the bed impotently.]]))
   vn.jump("3cont")

   vn.label("3contnever")
   dr(_([["Even in the last experiment? The energy wasn't sufficient? Maybe if I reroute the ship's power system and redo the experiment I might be able to connect. I must redo the experiment…"
He tries feebly to get up before coughs wrack his body and stain it further with specks of blood. He falls back to the bed impotently.]]))
   vn.jump("3cont")

   vn.label("3kex")
   vn.func( function () kexcount = kexcount+1 end )
   vn.label("3contsilence")
   dr(_([["I am so close, I can't stop here in front of you. Recalibrating the polymorphic tissue might stabilize the connection…"
He tries feebly to get up before coughs wrack his body and stain it further with specks of blood. He falls back to the bed impotently.]]))
   vn.label("3cont")

   dr(_([[He seems out of breath.
"Today is not a good day. I just need a bit more rest and I can get started again."]]))
   dr(_([["It was just like you said, the nebula is just a manifestation, not an entity in itself, but establishing the connection is less straight-forward than what you left in your notes. I've tried correcting the harmonic equation, but it doesn't…"
His talking is slowing down and starting to get muddled. You have trouble making out what he's saying.]]))

   vn.menu( function ()
      local opts = {
         { _([["You are dying."]]), "4cont" },
         { _([["It is over."]]), "4cont" },
         { _("…"), "4cont" },
      }
      if kexcount == 0 then
         table.insert( opts, 1, {_("Ask about Kex"), "4kex"} )
      elseif kexcount == 1 then
         table.insert( opts, 1, {_("Ask about Kex again"), "4kex"} )
      else
         table.insert( opts, 1, {_("Insist about Kex"), "kex_talk"} )
      end
      return opts
   end )

   vn.label("4kex")
   vn.func( function () kexcount = kexcount+1 end )
   vn.label("4cont")
   vn.na(_([[His lips begin moving and you can hear some sort of rasping sound coming out, however, you can no longer make out what he is saying. You glance at the monitors and you see his vitals are tanking. It looks like he has run out of time.]]))
   vn.label("nonviolent_death")
   vn.na(_([[He slowly lifts up an arm as if trying to reach out and grasp something. It it extremely thin and pale, almost transparent. His lips move as if trying to say something, but you can't make out a single sound. You see the strength slowly ebb out of him as he collapses one last time and his pulse flatlines.]]))
   vn.disappear( dr, "slideup", nil, "ease-out" ) -- played backwards so should be down
   vn.label("dr_death")
   vn.musicStop() -- Stop music
   vn.na(_([[Silence once again envelopes the room. You look around and decide to try to access the command console to see if there is any information left. It is a bit unsettling with a corpse nearby, but you try to focus on getting the grime off the console and interfacing with it. ]]))
   vn.na(_([[You notice that everything seems to be heavily encrypted, much more so than the standard on even military Za'lek vessels, and try to break into the system. After a few unsuccessful attempts you manage to find what looks like a flaw in the cryptographic armour and try to access it.]]))
   vn.na(_([[Suddenly a bright message starts flashing on all the monitors:
#rHONEYPOT ##329 ACTIVATED
SELF-DESTRUCT SEQUENCE ENGAGED
30 SECONDS REMAINING#0]]))
   vn.na(_([[Cursing to yourself, you realize you don't have enough time to override the console and you make a dash for your ship. As the airlock closes behind you you can hear explosions starting to rip Dr. Strangelove's ship apart.]]))
   vn.done()

   -- Player insists and goes to kex arc
   vn.label("kex_talk")
   dr(_([[He suddenly seems to regain a bit of lucidity.
"Kex? Kex… where have I heard that name before… Wait… You mean Experiment #085? The one taken from the nebula wreck?"]]))
   dr(_([["Someone also came asking about that awhile back, but my memory is all fuzzy. I don't recall much of the encounter. Must have not been interesting anyway. Most non-research talk is boring."]]))
   dr(_([["Anyway, the subject of that experiment is long gone. It was a failure."]]))
   vn.menu{
      { _([["Failure?"]]), "kex_failure" },
      { _([[Ask why he did it.]]), "kex_why" },
      { _([["Kex sends you his regards." (draw weapon)]]), "kex_weapon" },
   }

   vn.label("kex_why")
   dr(_([["Why would I not go on with that perfect experiment? The subjects found in the wreck were already long dead by the modern definition of the world. You don't need any ethics committee approval for experimenting on cadavers, not that I am subject to that oversight anymore."]]))
   dr(_([["Having spent so long in that wreck, whey were the perfect candidates for testing. Had I not performed it, it would have been a crime against science! It was my moral obligation to do it!"]]))
   vn.jump("kex_failure")

   vn.label("kex_failure")
   dr(_([["Yes, it was a failure. It never became a vessel to the void. It was just a husk of what it should have been. So much potential, all wasted."]]))
   dr(_([["Even though the vitals and neural responses seemed nominal, it didn't really seem to display any standard conscious responses. It even failed the LEVEL II test, a sign of a weak mind. An utter failure."
He coughs violently again, spasms overtaking his body. The medical robot seems to inject him with something and he calms down again.]]))
   vn.menu{
      { _([["Kex sends you his regards" (draw weapon)"]]), "kex_weapon" },
      { _([[Unplug his life support.]]), "kex_unplug" },
      { _([[Comfort him.]]), "kex_letbe" },
      { _([[…]]), "4cont" },
   }

   vn.label("kex_unplug")
   vn.na(_([[You unplug his life support and see the little colour left in his face fade away. His lips begin moving and you can hear some sort of rasping sound coming out, however, you can no longer make out what he is saying.]]))
   vn.func( function () var.push("strangelove_death","unplug") end )
   vn.jump("nonviolent_death")

   vn.label("kex_letbe")
   vn.na(_([[Seeing him in his last stages brings out your compassion. Whether Dr. Strangelove is a monster or not is not something that you are meant to judge. You get close to him and hold his hand. It feels very cold to the touch.]]))
   dr(_([["I can feel it… getting close…"
He seems to be looking at something in the distance.]]))
   dr(_([[Suddenly, he opens his eyes wide, but his expression is less of terror and more of curiosity and awe.
"It's more beautiful… than I… thought."]]))
   vn.na(_([[You look into his nearly opaque eyes and, for a second, you think you can see the nebula in all its fury and glory reflected in them. You look around the room but nothing has changed.]]))
   vn.func( function () var.push("strangelove_death","comforted") end )
   vn.jump("nonviolent_death")

   vn.label("kex_weapon")
   vn.na(_([[You draw your weapon and press it against Dr. Strangelove's forehead. He doesn't seem to flinch when the cold barrel makes contact with his skin. In fact, he seems very oblivious to everything around him.]]))
   vn.na(_([[You look at yourself and think of what you are about to do. Even if this is what Kex thinks he wants, is there any point in violently ending the life of someone who doesn't look like they will survive the next period? Dr. Strangelove seems unresponsive to your actions.]]))
   vn.menu{
      { _([[Pull the trigger.]]), "kex_shot" },
      { _([[Unplug his life support.]]), "kex_unplug" },
      { _([[Comfort him.]]), "kex_letbe" },
      { _([[…]]), "4cont" },
   }

   vn.label("kex_shot")
   vn.func( function () var.push("strangelove_death","shot") end )
   vn.disappear( dr, "slideup", nil, "ease-out" ) -- played backwards so should be down
   vn.na(_([[You pull the trigger and Dr. Strangelove's body goes completely limp and lifeless. The vitals monitor sounds an alarm before becoming silent, confirming his death. You did the task and you can only hope this will bring Kex peace of mind, although you somehow doubt it.]]))
   vn.jump("dr_death")

   vn.run()

   -- Remove station
   diff.remove( eccdiff )

   -- Advance mission
   mem.misn_state = 3
   misn.osdActive(2)
   misn.markerMove( mem.misn_marker, spob.get("Minerva Station") )
   player.unboard()
   local strangelove_death = var.peek("strangelove_death")
   if strangelove_death=="shot" then
      minerva.log.kex(_("You shot a dying Dr. Strangelove in cold blood, fulfilling Kex's wish."))
   elseif strangelove_death=="unplug" then
      minerva.log.kex(_("You unplugged the life support system of Dr. Strangelove who quickly passed away."))
   elseif strangelove_death=="comforted" then
      minerva.log.kex(_("You comforted a dying Dr. Strangelove before he passed away."))
   else
      minerva.log.kex(_("You watched Dr. Strangelove die before your eyes."))
   end

   -- And that's all folks
   strangelove_ship:setHealth(-1,-1)
   hook.timer( 10, "strangelove_dead" )
end

function strangelove_dead ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[The explosions clear and the system is once again silent except for your heavy breathing from running back to the ship.]]))
   vn.na(_([[As you survey the system again, you notice that you are no longer able to detect Dr. Strangelove's laboratory. Even pointing your sensors to the position where it should be, you are not able to find anything other than inert asteroids. It is possible that the self-destruct sequence didn't affect only the ship…]]))
   vn.na(_([[As your mind wanders to all you just experienced, you realize that you should get back to Kex to report what happened. Is this what Kex wanted? You feel like not even he will know the answer to that question.]]))
   vn.run()
end
