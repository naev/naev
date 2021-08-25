--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 5">
 <flags>
  <unique />
 </flags>
 <avail>
  <location>Bar</location>
  <chance>100</chance>
  <planet>Minerva Station</planet>
  <done>Kex's Freedom 4</done>
 </avail>
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
local minerva  = require "campaigns.minerva"
local love_audio = require 'love.audio'
local vn       = require 'vn'
local equipopt = require 'equipopt'
local love_shaders = require 'love_shaders'
local reverb_preset = require 'reverb_preset'
require 'numstring'

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted try to go find Dr. Strangelove
--  1: Destroy Bounty Hunter
--  2: meet Dr. Strangelove
--  3: return to kex
misn_state = nil

targetplanet = "Strangelove Lab"
targetsys = "Westhaven" --planet.get(targetplanet):system():nameRaw()

misn_reward = _("A step closer to Kex's freedom")
misn_title = _("Freeing Kex")
misn_desc = string.format(_("Kex wants you to kill Dr. Strangelove at %s in the %s system."), _(targetplanet), _(targetsys))

eccdiff = "strangelove"

money_reward = 700e3

function create ()
   if not var.peek("testing") then misn.finish(false) end
   if not misn.claim( system.get(targetsys) ) then
      misn.finish( false )
   end
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
   misn.setDesc( misn_desc )

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   love_audio.setEffect( "reverb_drugged", reverb_preset.drugged() )

   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex, {pitch=0.65, effect="reverb_drugged"} )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition()

   vn.na(_("You approach Kex who is sort of slumping at his usual spot. He doesn't seem to be in very good condition."))
   kex(_([[It takes a bit before Kex notices your presence. His feathers are rather disheveled.
"Oh, hey kid."]]))
   kex(_([[It takes a while before he continues.
"Compared to the vastness of all the universe, we are completely insignificant. Our sorrows and glories go unheard and seemingly devoid of all meaning."]]))
   kex(_([["Coming to terms with one's own significance does give a clarity of mind and make clear one's deepest desires. What has to be done then becomes clear."]]))
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
   kex(_([["I should have realized it sooner. It was obvious from the start that Strangelove would be involved in this travesty of a station."]]))
   kex(_([["There's not too many trails, but I do think I know where to find him. Wait, you know where he is? How you know is not important now, all I need you to do is go over there and end his rotten life once and for all."]]))
   kex(_([["The universe will be a much better place with scum like him gone, and I'll finally get my vengeance."]]))
   vn.disappear(kex)
   vn.na(string.format(_([[Without saying anything else, Kex walks off stumbling into the darkness of the station. You feel like it is best to leave him alone right now and decide to go see Strangelove. He should be in the %s system.]]),_(targetsys)))

   vn.func( function ()
      misn_state = 0
   end )
   vn.run()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      return
   end
   misn.accept()

   minerva.log.kex(_("You have agreed to help Kex deal with Dr. Strangelove.") )

   misn.osdCreate( misn_title,
      { string.format(_("Go find Dr. Strangelove in the %s system"), _(targetsys) ),
      _("Return to Kex at Minerva Station") } )
   misn_marker = misn.markerAdd( system.get(targetsys) )

   hook.enter("enter")
end

function generate_npc ()
   -- Can't use planet.get() here for when the diff is removed
   if misn_state==1 and planet.cur():nameRaw() == targetplanet then
      landed_lab()
   end
end

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

function enter ()
   if misn_stage==2 and system.cur() ~= targetsys then
      player.msg(_("MISSION FAILED! You never met up with Dr. Strangelove."))
      misn.finish(false)
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
      if misn_state==0 then
         table.insert( thugs, 1, "Vigilance" )
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
      thug_following = dofollow
   end

   thug_chance = thug_chance or 0.2
   if system.cur() == system.get(targetsys) then
      -- No spawns nor anything in Westhaven
      pilot.clear()
      pilot.toggleSpawn(false)

      if misn_state == 0 then
         -- TODO better handling, maybe more fighting with drones and a close-up cinematic?
         thug_chance = thug_chance / 0.8
         local pos = planet.get(targetplanet):pos()
         spawn_thugs( pos, false )
         hook.timer( 5, "thug_heartbeat" )
         player.allowLand( false, _("#rYou are unable to land while the bounty hunters are still active.#0") )

         for k,p in ipairs(thug_pilots) do
            pilot.hook( p, "death", "thug_dead" )
         end

         -- Add some disabled drones for effect
         for i=1,5 do
            local d
            if rnd.rnd() < 0.2 then
               d = "Za'lek Heavy Drone"
            else
               d = "Za'lek Light Drone"
            end
            local p = pilot.add( d, "Za'lek", pos + vec2.newP( 100+700*rnd.rnd(), 360*rnd.rnd() ) )
            p:setInvincible(true)
            p:setInvisible(true)
            p:disable(true)
         end

      elseif misn_state == 2 then
         -- Should be taking off from the Lab

         -- Spawn
         local pos = planet.get(targetplanet):pos() + vec2.new(4000, 6000)
         local p = pilot.add("Za'lek Sting", "Za'lek", pos, minerva.strangelove.name )
         p:setInvincible(true)
         p:setActiveBoard(true)
         p:control()
         p:brake()
         local mem = p:memory()
         mem.comm_no = _("No response.")
         hook.pilot( p, "board", "strangelove_board" )
         strangelove_ship = p
         hook.timer( 10, "strangelove_hail" )
      end

   elseif misn_state~=1 and rnd.rnd() < thug_chance then
      -- Spawn near the center, they home in on player
      spawn_thugs( vec2.newP(0.7*system.cur():radius()*rnd.rnd(),360*rnd.rnd()), false )
      -- Timer
      hook.timer( 5, "thug_heartbeat" )

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
      if misn_state==0 then
         thug_leader:broadcast( _("Looks like we have company!"), true )
      else
         thug_leader:broadcast( msglist[ rnd.rnd(1,#msglist) ], true )
      end

      -- Decrease chance
      thug_chance = thug_chance * 0.8

      -- Reset autonav just in case
      player.autonavReset( 5 )
      return
   end

   -- Only chase if not hidden
   local pp = player.pilot()
   if pp:flags("stealth") then
      if thug_following then
         thug_leader:taskClear()
         thug_leader:brake()
         thug_following = false
      end
   else
      if not thug_following then
         thug_leader:taskClear()
         thug_leader:follow( pp )
         thug_following = true
      end
   end

   -- Keep on beating
   hook.timer( 1, "thug_heartbeat" )
end

function thug_dead ()
   local stillalive = {}
   for k,v in ipairs(thug_pilots) do
      if v:exists() then
         table.insert( stillalive, v )
      end
   end
   thug_pilots = stillalive
   if #thug_pilots == 0 then
      hook.timer( 4, "thugs_cleared" )
   end
end

function thugs_cleared ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.run()

   misn_state = 1
   player.allowLand( true )
end

function landed_lab ()
   local check_living = false
   local check_rr = false
   local check_back = false

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_("Your ship sensors indicate that Strangelove's Laboratory is no longer pressurized and without an atmosphere, so you don your space suit before entering."))
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
   vn.na(_("You make your way through a nearly impassible very tight passageway towards the living quarters. Although it is a mess, you don't really see any signs of anyone living here, it rather seems all abandoned."))
   vn.na(_("Eventually you reach a really small room with a bed capsule. The ground and bed are splattered in blood, but it seems very old and dessicated. Some empty medicinal syringes and pouches are scattered on the floor."))
   vn.na(_("You turn over everything, but find nothing of interest."))
   vn.func( function () check_living = true end )
   vn.jump("menu")

   vn.label("recreation")
   vn.na(_("You head towards and eventually reach the recreation room, which has been completely trashed. Even pieces of the wall have been ripped out, revealing shoddy cable connections and lots of weird mold."))
   vn.na(_("Looking around, you find that the hologram projector you used last time is, unsurprisingly,  completely wrecked. It doesn't seem like you'll be able to use it to get in touch with Dr. Strangelove anymore."))
   vn.na(_("Try as you might, you are unable to find anything of interest."))
   vn.func( function () check_rr = true end )
   vn.jump("menu")

   vn.label("back")
   vn.na(_([[You find a tunnel leading towards the back room. It seems to have been once blocked off, but now has been torn wide open. You ignore the "KEEP OUT" sign that is bent up underfoot and make your way through.]]))
   vn.na(_("The tunnel twists and turns as it heads towards the center of the asteroid. There is no lighting the entire way, so you rely on the lamps of your suit."))
   vn.na(_("Eventually you reach a room at the core of the asteroid. It is fairly large compared to the rest of the laboratory, but is jammed with junk and rubbish, lots of which has been recently moved around."))
   vn.na(_("You start to sift through the things to try to find something of interest, but after a long time of looking around and not making significant progress given the large amounts of stuff, you decide to take a break and set on a fallen bookcase."))
   vn.na(_("As you survey the room, you notice a small movement next to your feet. The motion is small that you can only notice it by being still nearby. Slowly and carefully you lift up some debris to uncover a small and heavily damaged droid."))
   vn.na(_("You pick up the droid and dust it off. The primary lens and hover engine seem completely damaged, but it does seem like it still has power and likely data in it. You try to interface to it, but it seems to be locked with strong encryption. You pocket it as it seems like it might be something of interest to Kex."))
   vn.na(_("You get back to searching around the stuff and end up finding nothing else of interest."))
   vn.func( function () check_back = true end )
   vn.jump("menu")

   vn.menu("check_done")
   vn.na(_("You spend your time to go carefully through the entire station, but it ends up being mainly in vain: Dr. Strangelove is no where to be found. At least you found a droid that might be useful."))
   vn.na(_("You let out a sigh as you head outside. Maybe Dr. Strangelove escaped somewhere else?"))
   vn.run()

   -- Take off
   misn_state = 2
   player.takeoff()

   -- Disable landing, will get disabled on entering new system
   player.allowLand( false, _("You have better things to do right now.") )
end

function strangelove_hail ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.strangelove )
   local dr = vn.newCharacter( strangelove.name,
         { color=strangelove.colour, image=minerva.strangelove.image,
           shader=love_shaders.hologram() } )
   vn.transition( "electric" )
   -- TODO small scene
   vn.na(_("As you loiter around a system, you suddenly receive an unexpected incoming transmission."))
   dr(_([[A hologram of Dr. Strangelove appears before you. He is laying down and his body seems to be sapped of all energy. He seems to muster up energy to roughly look in your direction, although his eyes seem unnaturally clouded.
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
   local kexcount = 0

   vn.clear()
   vn.scene()
   local dr = vn.Character.new( strangelove.name,
         { color=strangelove.colour, image=minerva.strangelove.image, } )
   -- TODO small scene
   vn.na(_("You carefully board the ship, not sure what you are about to encounter. Trusting your systems sensors indicating a proper atmosphere in the ship, you board without your space suit. However, when you first enter the ship, a strong pungent odour makes you partially regret your decision."))
   vn.na(_("As you move to the command room, you notice small movements from the corner of your eyes. Upon closer inspection you make out all sorts of small moving objects, reminding you of cleaning droids on most ships, however, these move in a fairly clunky fashion, as if they had some of their moving apparatus damaged."))
   vn.na(_("You pay as little attention to the small critters, which seem harmless and proceed through the ship. After a short walk you enter the command room."))
   love_audio.setEffect( "reverb_psychotic", reverb_preset.psychotic() )
   vn.music( minerva.loops.strangelove, {effect="reverb_psychotic"} )
   vn.appear(dr)
   vn.na(_("You enter the room and meet Dr. Strangelove for the first time face-to-face. Your first impression is that more than a space ship, it looks like a hospital. Dr. Strangelove is laid out in a medical bed connected to a lot of weird machines, with a medical droid attending him. If it weren't for the fact that everything is fairly rundown and dirty, you would think you were in a planetary hospital instead of a space ship at the edge of the universe."))
   dr(_("You take a close look at Dr. Strangelove, and he seems like a husk of what might of been formally a man. His cheekbones protrude from his pale face, and his laboratory coat seems to be a few sizes too big on his frail body. His vitals monitor also confirms your suspicion that he is more dead than alive."))
   dr(_([["I see you were able to make it."
He coughs violently.
"Sorry, I haven't been feeling too well lately."
He doesn't seem to be focusing much on you and is just talking in your general direction.]]))
   dr(_([["It is all going well. I think we are close to a breakthrough, the next time I should be able to succeed. After all this time, finally I'll be able to see the truth!"
His vital monitor's warning light flashes on.]]))
   dr(_([["You were right. It was and is all connected. I should have listened to you sooner, but I was foolish and naïve."
He lets out a sigh.]]))
   dr(_([["I thought I almost had it with the last specimen. It was all going so smoothly… The zero-point bionimorphic interface connection went flawlessly, and the entropy flux stable was nearly stable, but the connection was never established."]]))
   dr(_([["It could be the distance, but my calculations indicated that it shouldn't matter. It should be everywhere. Everything is everywhere! Folding onto of itself and twisted around in higher dimensions. Measly Euclidean distances shouldn't matter at all."]]))
   dr(_([["…but it didn't work. Almost as if making a mockery of science itself and spitting in my face. Almost as if the universe itself is unwilling to give up its secrets."
He coughs violently.
"But it failed. Like always. It failed."]]))
   dr(_([["Williams, why did you have to die? You know so much, you surpassed me in everything I could possibly imagine. You would have been able to make it work!"]]))
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
"You were always the brightest in the class. I admired how fast you were able to solve partial differential equations so elegantly."]]))
   dr(_([["Remember that one time we nearly caused a subatomic implosion when the hyperphased quantum autocycle hit the self-refractive frequency? Your quick thinking save us from turning the entire system into a sterile void."]]))
   dr(_([["If only you had been with me this entire time. We could have done so much together."
His voice tears up slightly.]]))
   dr(_([["Why did you have to do it?"]]))

   vn.menu( function ()
      local opts = {
         { _("Do what?"), "2cont" },
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

   vn.run()

   -- Remove station
   diff.remove( eccdiff )

   -- Advance mission
   misn_state = 3
   misn.osdActive(2)
   player.unboard()
end
