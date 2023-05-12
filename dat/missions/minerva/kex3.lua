--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 3">
 <unique />
 <location>Bar</location>
 <chance>100</chance>
 <spob>Minerva Station</spob>
 <done>Kex's Freedom 2</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]
--[[
   Freeing Kex 3

   Go to planet A, get ambushed twice, and then duel in the Crimson Gauntlet.
--]]
local minerva  = require "common.minerva"
local gauntlet = require "common.gauntlet"
local pp_shaders = require 'pp_shaders'
local vn       = require 'vn'
local equipopt = require 'equipopt'
local luaspfx  = require 'luaspfx'
local fmt      = require "format"

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted go to targetplanet and try to find dvaered dude
--  1: get ambushed
--  2: get ambushed again
--  3: go to totoran
--  4: duel finished
--  5: return to Kex
mem.misn_state = nil
local enemies, enemies_weak, enemy_faction, noise_shader, pmalik, thug_leader, thug_pilots -- Non-persistent state

local gauntlet_start -- Forward-declared functions

local targetplanet, targetsys = spob.getS("Trincea")
local lastplanet, lastsys = spob.getS("Totoran")
local gauntletsys = system.get("Crimson Gauntlet")

local malik_portrait = "major_malik.webp"
local malik_image = "major_malik.webp"

local money_reward = minerva.rewards.kex3

function create ()
   if not misn.claim( {targetsys, lastsys} ) then
      misn.finish( false )
   end
   misn.setReward( _("A step closer to Kex's freedom") )
   misn.setTitle( _("Freeing Kex") )
   misn.setDesc( fmt.f(_("You have been assigned with obtaining information from Major Malik at {pnt} in the {sys} system."), {pnt=targetplanet, sys=targetsys}))

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   approach_kex()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   misn.accept()

   minerva.log.kex(_("You have agreed to help Kex obtain information from Major Malik.") )

   misn.osdCreate( _("Freeing Kex"),
      { fmt.f(_("Go to {pnt} in the {sys} system to find Major Malik"), {pnt=targetplanet, sys=targetsys} ),
      _("Return to Kex at Minerva Station") } )
   mem.misn_marker = misn.markerAdd( targetplanet )

   hook.land("generate_npc")
   hook.load("generate_npc")
   hook.enter("enter")

   generate_npc()
end


function generate_npc ()
   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )

   elseif mem.misn_state==0 and spob.cur() == targetplanet then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_("You land and promptly proceed to try to find Major Malik and get the job over with. After glancing over the map of the installations, you are able to quickly locate his office and head down there."))
      vn.sfxBingo()
      vn.na(fmt.f(_("You arrive and inquire about Major Malik, but are told he is apparently enjoying some leisure time at {pnt} in the {sys} system. Looks like you have no choice but to try to look for him there."), {pnt=lastplanet, sys=lastsys}))
      vn.run()

      -- Advance the state
      mem.misn_state = 1
      misn.markerMove( mem.misn_marker, lastplanet )
      misn.osdCreate( _("Freeing Kex"),
         { fmt.f(_("Look for Major Malik at {pnt} in the {sys} system"), {pnt=lastplanet, sys=lastsys} ),
         _("Return to Kex at Minerva Station") } )

   elseif (mem.misn_state==2 or mem.misn_state==3) and spob.cur() == lastplanet then
      mem.misn_state = 3
      misn.npcAdd( "approach_malik", _("Major Malik"), malik_portrait, _("You see Major Malik who is fairly similar to the image shown to you by Kex.") )

   elseif mem.misn_state==4 and spob.cur() == lastplanet then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[You disconnect from the virtual reality of the Crimson Gauntlet and look around the private room you were in. There is a distinct odour of sweat permeating the room. Furthermore, it looks like someone left in a hurry, but at least it seems like Major Malik is still at his terminal.]]))
      vn.na(_([[You approach Major Malik and notice immediately that something is off. You remove the headgear from him and immediately see that his eyes are glazed over and there is some white froth coming out of his mouth. Seems like some sort of mental shock killed him.]]))
      vn.na(_([[From the corner of your eye you see that the terminal is still logged in. Since there is nothing you can do anymore for Major Malik, you rotate the screen to you and try to see if you can find any of the information that Kex was looking for.]]))
      vn.sfxBingo()
      vn.na(_([[The directories are nicely organized and it doesn't take long before you find what seems to be the correct files. You copy them over to your holodrive. You hope the files satisfy Kex this time.]]))
      vn.na(_([[You survey the room one last time before you leave. You notice that the chassis covering some of the terminal has been pried open, exposing some internal connections. There is no doubt that someone jacked into your Crimson Gauntlet session. However, given the dead body in the room, you decide to take your leave before it attracts attention.]]))
      vn.na(_([[You better head back to Minerva Station to relay the information to Kex.]]))
      vn.run()

      mem.misn_state = 5 -- We're done here, go back to kex:)
      misn.osdActive(2)
      misn.markerMove( mem.misn_marker, spob.get("Minerva Station") )
   end
end

function approach_kex ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition("hexagon")

   -- Mission is over
   if mem.misn_state==5 then

      vn.na(_("Still exhausted from the entire ordeal, and with Major Malik's dead eyes still ingrained in your memory, you approach Kex."))
      kex(_([["You look like a mess, kid. What happened?"]]))
      vn.na(_("You explain what happened in the Crimson Gauntlet server and how you barely escaped with your life, and hand him the holodrive with the data you were able to collect."))
      kex(_([["Damn. When I was still a human, we didn't have that fancy Virtual Reality shit. It's wild. Don't think I want to try it after hearing about your experience though. I'm glad you made it out of there alive. Don't know what I would do without you now."]]))
      kex(_([["Let me take a look at the data."
He once again plugs the drive into a port somewhere under his wing and his eyes go blank.
"Seems like that Major Malik was into some major shit."
He seems amused by his pun.]]))
      kex(_([[He snaps out of his stupor and looks at you again.
"Seems like you got much more than I expected. It's going to take me a while to go over this. Meet me back here in a period or so after I process all this data."]]))
      kex(_([["Ah yes, before I get absorbed by the data processing, here, let me reward you for your troubles."]]))
      vn.sfxMoney()
      vn.func( function () player.pay( money_reward ) end )
      vn.na(fmt.reward( money_reward ))
      vn.sfxVictory()
      vn.na(_("As you take your leave you hear Kex beginning to hum an ancient-sounding tune."))
      vn.run()

      minerva.log.kex(_("You defeated Major Malik in a lopsided duel in the Crimson Gauntlet, and acquired information related to money laundering at Minerva Station."))
      misn.finish( true )
      return

   elseif mem.misn_state==nil then
      vn.na(_("You find Kex taking a break at his favourite spot at Minerva station. His eyes light up when he sees you and he runs over to you."))
      kex(_([["Good news kid! It looks like the information we obtained from Baroness Eve had some dirt on the CEO. Looks like they're involved in some sort of money laundering, but this alone won't get us anywhere, we need more intel."]]))
      kex(_([["We have a lead on another co-conspirator, this time in Dvaered space. Some individual, named Major Malik, seems to also be in the money laundering. Would you be up for paying him a visit and seeing what you can get out of him?"]]))
      vn.menu( {
         { _("Accept"), "accept" },
         { _("Decline"), "decline" },
      } )
      vn.label("decline")
      kex(_([[He looks dejected.
"I see. If you change your mind, I'll be around."]]))
      vn.done()

      vn.label("accept")
      kex(fmt.f(_([["This time I'm hoping it's a cinch. Major Malik should be at {pnt} in the {sys} system. He should be fairly old, so it should be enough to just outright confront him and get him to talk. I'll give you a note to help convince him you mean business. I'll also send you his picture so you can easily recognize him when you see him."]]), {pnt=targetplanet, sys=targetsys}))
      kex(_([["The note? It's just your run-of-the-mill blackmail. We don't really care about Major Malik himself, what we want is dirt on the CEO. Hopefully he'll be sensible and give us what we want. However, I trust you will do what it takes in case he doesn't."
He winks his cyborg eye at you.]]))
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
         { _("Ask about his past"), "past" },
         { _("Leave"), "leave" },
      }
      return opts
   end )

   vn.label("job")
   if not mem.misn_state or mem.misn_state < 1 then
      kex(_([["The job is pretty straightforward. We need Major Malik to talk about his dealings with the Minerva CEO. If you hand him the note I gave you it should be enough to convince him."]]))
      kex(fmt.f(_([["You should be able to find him at {pnt} in the {sys} system. I don't think he should give much trouble."]]), {pnt=targetplanet, sys=targetsys}))
   else
      kex(fmt.f(_([["Oh, so Major Malik wasn't at {targetplanet}? That is really weird. Let's hope you can find him at {pnt} in the {sys} system."]]), {targetplanet=targetplanet, pnt=lastplanet, sys=lastsys}))
   end
   vn.jump("menu_msg")

   vn.label("past")
   kex(_([["You are quite the curious kid. At the time I didn't think my life was all that great and exciting, but lately I yearn to take to the skies like the good old times. Did I ever tell you about the story of when I spent a kilo-period drifting in the Nebula?"
"No? Great!"]]))
   kex(_([["It was not uncommon for things to break down and get weird fast when going deep into the Nebula. The radiation is really damaging to ship armour, so you have to maximize energy to shields. It makes it hard to bring firepower, but it's not like it'll do you any good in the deep Nebula. Nothing is alive there, or at least, not in the sense we sort of think of as being alive."]]))
   kex(_([["So me and Mireia had picked up a tip about some sort of weird wreck deep past Arandon. Some poor fools had gone to search for it and hadn't come back for a few hecto-periods and one of their husbands was beginning to lose it, so they came to us. Now, we weren't interested in trying to save them -- once you enter the Nebula, you forfeit all chance of rescue. Search parties tend to only count up the bodies."]]))
   kex(_([["However, from the looks of it, it was a very interesting find. Sometimes you find really weird things in the Nebula."
He leans closer to you and whispers.
"…things that shouldn't exist. Or better said, things that can't possibly exist."]]))
   kex(_([["So we loaded up our trusty Mule with all the shield generators and survival gear we could fit, and headed out towards the last known location of the wreck. At first it was fairly standard, our shields and radiators running at maximum power. You get used to the hum and the chaos of the Nebula, but you can never relax. It sort of gets into your bones and makes you lose your sleep."]]))
   kex(_([["We continued like that for what seemed ages. Sensors get a bit fuzzy in there, but they'd been scanning for about a deca-period when we saw it. It was part of something very big, almost like some sort of metallic skeleton. We approached it cautiously because the radiation was off the charts. The more you looked at it, the more it felt like it was burning into the back of your retina."]]))
   kex(_([["After our cursory analysis, we went to grab a sample. However, when we tried to extend the robotic arm to it, it never seemed to get closer. It was at some sort of constant distance from us no matter what we tried. We only had a small laser turret, but we even tried shooting at it and nothing. Zip-O."]]))
   kex(_([["It could have been the exhaustion or something else, I don't know, but we decided to try to ram it. So we accelerated at maximum velocity, and that's when it happened. Suddenly there was some horrible screeching sound and everything shook. Next thing we know, it felt like we were getting pulled through the eye of a needle, like hyperspace sickness but much worse. And we passed out."]]))
   kex(_([["When we woke up, there was nothing around us. Only Nebula. One of the shield generators had gone offline. We weren't going to last long. We did some emergency repairs and hauled our asses out of there. When we got out, our systems said that we had only been gone for about 22 periods, but when we hailed a ship, we found out there was a 1087 period difference between our system clock and theirs."]]))
   kex(_([["The weird thing is we'd hardly used up our supplies, so there's no way that could be true. I still scratch my head thinking about this today. I did tend to lose track of time and lose hunger in the deep Nebula, but nothing like that. We also had nothing from our encounter with the wreck or whatever that was."]]))
   kex(_([["After that I headed back home and totally got chewed out by my wife. I had to spend a few weeks sleeping on the couch and Maikki didn't speak to me for ages."
He sighs wistfully.
"That's the part I miss the most of my previous life. Having a place to go after adventuring."
He looks a bit glum.]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function enter ()
   local function spawn_thugs( pos, dofollow )
      thug_leader = nil -- Clear
      local thugs = {
         "Dvaered Vigilance",
         "Dvaered Vendetta",
         "Dvaered Vendetta",
      }
      local pp = player.pilot()
      if pp:ship():size() > 4 then
         table.insert( thugs, "Dvaered Vendetta" )
         table.insert( thugs, "Dvaered Ancestor" )
         table.insert( thugs, "Dvaered Ancestor" )
      end
      local fdv = faction.dynAdd( "Dvaered", "dv_thug", _("Dvaered Thug"), {clear_enemies=true, clear_allies=true} )
      thug_pilots = {}
      for k,v in ipairs(thugs) do
         local ppos = pos + vec2.new( rnd.rnd()*200, rnd.rnd()*360 )
         local p = pilot.add( v, fdv, ppos )
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
   end

   if mem.misn_state==1 and system.cur() == targetsys then
      -- Spawn thugs after the player. Player should likely be going to Dvaer
      spawn_thugs( vec2.new( 15000, 15000 ), true )
      -- Move to next state
      mem.misn_state = 2
      -- Timer
      hook.timer( 5, "thug_heartbeat" )
   elseif mem.misn_state==2 and system.cur() == lastsys then
      -- Spawn thugs from Totoran
      spawn_thugs(lastplanet:pos(), false )
      -- Timer
      hook.timer( 5, "thug_heartbeat" )
   end
end

function thug_heartbeat ()
   local det, fuz = thug_leader:inrange( player.pilot() )
   if det and fuz then
      -- Start the attack, should be close enough to aggro naturally
      thug_leader:control(false)
      for k,p in ipairs(thug_pilots) do
         p:setHostile(true)
      end
      -- Broadcast after hostile
      thug_leader:broadcast( _("That's the the one!"), true )

      -- Reset autonav just in case
      player.autonavReset( 5 )
      return
   end
   -- Keep on beating
   hook.timer( 5, "thug_heartbeat" )
end

function approach_malik ()
   local dogauntlet
   vn.clear()
   vn.scene()
   local malik = vn.newCharacter( _("Major Malik"), {image=malik_image} )
   vn.transition()
   vn.na(_("You approach Major Malik who seems to be enjoying a drink."))
   malik(p_("Malik", [["What do you want?"]]))
   vn.na(_("You hand him the note you got from Kex. He reads it and furrows his brows a bit."))
   malik(_([["I see. You think at my age I worry about what happens to me? I've had a good career with no regrets. Your threats don't sway me."]]))
   malik(_([["However, I'm in a good mood today. How about we play a game? I have a private Crimson Gauntlet server here, let us duel in classic Dvaered fashion. If you win, I'll give you the information you want."]]))
   malik(_([["To make it more exciting, we can both duel in Vendettas. I'll provide you with one. What do you say?"]]))
   vn.menu{
      {_("Accept"),  "accept"},
      {_("Decline"), "decline"},
   }
   vn.label("accept")
   vn.func( function () dogauntlet = true end )
   malik(_([["OK. It's settled then. Follow me to my private room and we'll get started."]]))
   vn.na(_("Major Malik leads you through the installation to a private Crimson Gauntlet server room. You enter and see that there are two virtual reality terminals."))
   malik(_([["Please sit in that terminal over there, I'll be at the one here. You know how this works right? Just connect yourself, and I'll set up the server details."]]))
   vn.na(_([[You sit down and do as you are told. Quickly it changes to the Crimson Gauntlet boot screen. You idly scan the server detail information as it boots up. Suddenly something catches your eye:
#rHUMAN LIFE SAFETY OVERRIDE.
DUEL TO THE DEATH MODE ENABLED.#0]]))
   vn.sfxEerie()
   vn.na(_([[You try to remove the gear, but it's too late. The virtual reality of the Crimson Gauntlet envelopes you…]]))
   vn.done()

   vn.label("decline")
   malik(p_("Malik", [["OK. I'll be here if you change your mind."]]))
   vn.run()

   if dogauntlet then
      gauntlet_start()
   end
end

function gauntlet_start ()
   hook.safe("enter_the_ring")
   player.takeoff()
end

function abort ()
   if noise_shader then
      shader.rmPPShader( noise_shader )
   end
   if system.cur() == gauntletsys then
      leave_the_ring()
   end
end

--[[
   Common Teleporting Functions
--]]
-- Enters Crimson Gauntlet
function enter_the_ring ()
   -- If the player reloads when accepted the mission, the hook will be saved
   -- and error out because it can't teleport
   if player.isLanded() then return end

   -- Enter the ring!
   gauntlet.enter_the_ring()

   -- Set up Player
   local player_vendetta = player.shipAdd( "Vendetta", _("Ketchup"), _("It's virtual reality!"), true )
   mem.player_prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_vendetta, true )
   equipopt.generic( player.pilot(), {type_range={Afterburner={min=1}}}, "elite" )
   hook.pilot( player.pilot(), "death", "player_death" )

   -- Set up Major Malik
   enemy_faction = faction.dynAdd( "Dvaered", "Combatant", _("Dvaered"), {ai="dvaered_norun"} )
   local pos = vec2.new( -1500, 1500 )
   pmalik = pilot.add( "Dvaered Vendetta", enemy_faction, pos, _("Major Malik") )
   pmalik:setInvincible(true)
   pmalik:setHostile(true)
   pmalik:control(true)
   pmalik:brake()
   pmalik:face( player.pilot() )
   hook.pilot( pmalik, "death", "malik_death" )
   enemies = {pmalik}

   -- Taunt stuff
   hook.timer( 1, "countdown_start" )
   hook.timer( 10, "malik_taunt" )
end
-- Goes back to Totoran (landed)
function leave_the_ring ()
   gauntlet.leave_the_ring ()

   -- Give the player back their old ship
   player.shipSwap( mem.player_prevship, true, true )
   player.land( spob.get("Totoran") )
end

--[[
   Countdown stuff
--]]
function countdown_start ()
   mem.omsg_id = player.omsgAdd( _("5…"), 1.1 )
   hook.timer( 1, "countdown", _("4…") )
   hook.timer( 2, "countdown", _("3…") )
   hook.timer( 3, "countdown", _("2…") )
   hook.timer( 4, "countdown", _("1…") )
   hook.timer( 5, "countdown_done" )
end
function countdown( str )
   -- TODO play countdown sound
   player.omsgChange( mem.omsg_id, str, 1.1 )
end
function countdown_done ()
   -- TODO play sound and cooler text
   player.omsgChange( mem.omsg_id, _("FIGHT!"), 3 )

   for k,p in ipairs(enemies) do
      p:setInvincible(false)
      p:control(false)
   end
end

function malik_taunt ()
   if pmalik and pmalik:exists() then
      local taunts = {
         _("Back in my day we used to fly uphill both ways to go to work!"),
         _("I'll be damned if I let a young whippersnapper get the best of me!"),
         _("Nothing like some dogfighting to feel alive!"),
         _("Wait until this old dog shows you some new tricks!"),
         _("Youth is overrated!"),
         _("Feel the taste of my cannons!"),
      }
      mem.tauntperm = mem.tauntperm or rnd.permutation(taunts)
      mem.tauntid = mem.tauntid or rnd.rnd(1,#taunts)
      mem.tauntid = (mem.tauntid % #taunts)+1
      pmalik:comm( player.pilot(), mem.tauntperm[mem.tauntid], true )

      hook.timer( 7, "malik_taunt" )
   end
end

function malik_death ()
   hook.timer( 5, "malik_speech" )
end

local function malik_respawn ()
   enemies = {}
   enemies_weak = {}
   local pos = player.pos()
   luaspfx.alert( pos, {size=200} )
   hook.timer( 2, "malik_respawn_real", pos )
end
function malik_respawn_real( pos )
   pmalik = pilot.add("Dvaered Goddard", enemy_faction, pos, _("Major Malik"), {naked=true})
   equipopt.dvaered( pmalik, {
      type_range = {
         ["Beam Turret"] = { max = 0 },
         ["Beam Cannon"] = { max = 0 },
      }
   } )
   pmalik:intrinsicSet( "armour_mod", -50 )
   pmalik:setHostile(true)
   pmalik:setNoDeath(true)
   hook.pilot( pmalik, "disable", "malik_disabled" )
   hook.pilot( pmalik, "board", "malik_boarded" )
end
function malik_disabled ()
   shader.rmPPShader( noise_shader )
   pmalik:disable()
   player.omsgAdd( _("Aaaaaargh!"), 3 )

   -- Kill all enemies
   for k,v in ipairs(enemies) do
      if v:exists() then
         v:setHealth( -100, -100 )
      end
   end

   -- Maikki and friends go away
   pilot.clearSelect("Wild Ones")
end
function player_death ()
   if noise_shader then
      shader.rmPPShader( noise_shader )
   end
end

function malik_spawn_more ()
   local pos = player.pos() + vec2.new( 200*rnd.rnd(), 360*rnd.rnd() )
   luaspfx.alert( pos, {size=100} )
   hook.timer( 2, "malik_spawn_more_real", pos )
end
function malik_spawn_more_real( pos )
   local p = pilot.add("Dvaered Vendetta", enemy_faction, pos )
   p:setHostile(true)
   table.insert( enemies, p )
   table.insert( enemies_weak, p )
end
local function malik_spawn_more2 ()
   malik_spawn_more()
   hook.timer( 0.5, "malik_spawn_more" )
end

local function noise_start ()
   noise_shader = pp_shaders.corruption( 1.0 )
   shader.addPPShader( noise_shader, "gui" )
end

local function noise_worsen ()
   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 1.5 )
   shader.addPPShader( noise_shader, "gui" )
end

local function maikki_arrives ()
   local pos = player.pos()
   local mc = minerva.maikkiP.colour
   local col = {mc[1], mc[2], mc[3], 0.3}
   luaspfx.alert( pos, {size=200, col=col} )
   hook.timer( 2, "maikki_arrives_real", pos )
   -- Add more extras
   for i=1,4 do
      hook.timer( 0.2, "maikki_arrives_extra" )
   end
end
function maikki_arrives_real( pos )
   local p = pilot.add( "Pirate Kestrel", "Wild Ones", pos, _("Pink Demon"), {naked=true} )
   equipopt.pirate( p, {
      type_range = {
         ["Launcher"] = { max = 0 },
         ["Turret Launcher"] = { max = 0 },
      }
   } )
   p:setFriendly(true)
   p:control()
   p:attack( pmalik )
   --pmaikki = p

   -- Make really really strong
   p:intrinsicSet( "armour", 1e6 )
   p:intrinsicSet( "shield", 1e6 )
   p:intrinsicSet( "energy", 1e6 )
   p:intrinsicSet( "fwd_damage", 500 )
   p:intrinsicSet( "tur_damage", 500 )
   p:intrinsicSet( "fwd_dam_as_dis", 50 )
   p:intrinsicSet( "tur_dam_as_dis", 50 )

   -- Fancy message
   local mc = minerva.maikkiP.colour
   local col = colour.new( mc[1], mc[2], mc[3], 1.0 )
   player.omsgAdd( _("Ho ho ho and a bottle of rum!"), 5, nil, col )

   -- Disable some of the vendettas
   for k,pk in ipairs(enemies_weak) do
      if pk:exists() and rnd.rnd() < 0.5 then
         pk:disable()
      end
   end
end
function maikki_arrives_extra ()
   local mc = minerva.maikkiP.colour
   local col = {mc[1], mc[2], mc[3], 0.3}
   local pos = player.pos() + vec2.new( 200*rnd.rnd(), 360*rnd.rnd() )
   luaspfx.alert( pos, {size=100, col=col} )
   hook.timer( 2, "maikki_arrives_extra_real", pos )
end
function maikki_arrives_extra_real( pos )
   local p = pilot.add( "Pirate Shark", "Wild Ones", pos )
   p:setFriendly(true)
   p:intrinsicSet( "armour", 1e5 )
   p:intrinsicSet( "shield", 1e5 )
   p:intrinsicSet( "energy", 1e5 )
   p:intrinsicSet( "fwd_damage", 400 )
   p:intrinsicSet( "tur_damage", 400 )
end
local function malik_music ()
   music.play('battlesomething2.ogg')
end

function malik_speech ()
   mem.malik_speech_state = mem.malik_speech_state or 0
   mem.malik_speech_state = mem.malik_speech_state + 1
   local speeches = {
      { delay=5, txt=_("You come to my office to harass me…"), func=malik_music },
      { delay=5, txt=_("You interrupt my leisure with blackmail…") },
      { delay=5, txt=_("You really think you would be getting out of here alive?…"), func=malik_respawn },
      { delay=5, txt=_("You are in my realm now, kid!"), func=malik_spawn_more },
      { delay=5, txt=_("And the only way out is in a body bag!"), func=malik_spawn_more },
      { delay=5, txt=_("My power is limitless here!"), func=malik_spawn_more },
      { delay=5, txt=_("You are a fool to have walked into my trap so willingly."), func=malik_spawn_more2 },
      { delay=2, txt=_("Not so tough anymore! Ha ha ha!"), func=malik_spawn_more2 },
      { delay=3, func=noise_start },
      { delay=5, txt=_("What is going on? It's not responding!"), func=noise_worsen },
      { delay=5, func=maikki_arrives },
      { delay=5, txt=_("What the hell?!") },
   }

   local s = speeches[ mem.malik_speech_state ]
   if not s then return end
   if s.txt then
      player.omsgAdd( s.txt, s.delay-0.1 )
   end
   if s.func then
      s.func()
   end
   if s.delay then
      hook.timer( s.delay, "malik_speech" )
   end
end

function malik_boarded ()
   player.unboard()
   mem.misn_state = 4
   hook.safe( "leave_the_ring" )
end
