--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <location>Bar</location>
  <chance>100</chance>
  <planet>Minerva Station</planet>
  <done>Kex's Freedom 2</done>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Freeing Kex 3

   Go
--]]
local minerva  = require "campaigns.minerva"
local portrait = require 'portrait'
local pp_shaders = require 'pp_shaders'
local vn       = require 'vn'
local equipopt = require 'equipopt'
local luaspfx  = require 'luaspfx'
require 'numstring'

logidstr = minerva.log.kex.idstr

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted go to targetplanet
--  1: try to find dvaered dude
--  2: get ambushed
--  3: go to totoran
--  4: duel finished
--  5: return to Kex
misn_state = nil

targetplanet = "Trincea"
targetsys = planet.get(targetplanet):system():nameRaw()

lastplanet = "Totoran"
lastsys = planet.get(lastplanet):system():nameRaw()

gauntletsys = system.get("Crimson Gauntlet")

misn_reward = _("A step closer to Kex's freedom")
misn_title = _("Freeing Kex")
misn_desc = string.format(_("You have been assigned with obtaining information from Major Malik at %s in the %s system."), _(targetplanet), _(targetsys))

malik_portrait = "major_malik.webp"
malik_image = "major_malik.webp"
malik_portrait = minerva.kex.portrait
malik_image = minerva.kex.image

money_reward = 400e3

function create ()
   if not var.peek("testing") then
      misn.finish(false)
   end

   if not misn.claim( system.get(targetsys) ) then
      misn.finish( false )
   end
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
   misn.setDesc( misn_desc )

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   approach_kex()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      return
   end

   misn.accept()

   shiplog.append( logidstr, _("You have agreed to help Kex obtain information from Major Malik.") )

   misn.osdCreate( misn_title,
      { string.format(_("Go to %s in the %s system to find Major Malik"), _(targetplanet), _(targetsys) ),
      _("Return to Kex at Minerva Station") } )
   misn_marker = misn.markerAdd( system.get(targetsys) )

   hook.land("generate_npc")
   hook.load("generate_npc")
   hook.enter("enter")

   generate_npc()
end


function generate_npc ()
   if planet.cur() == planet.get("Minerva Station") then
      npc_kex = misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )

   elseif misn_state==0 and planet.cur() == planet.get(targetplanet) then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_("You land and promptly proceed to try to find Major Malik and get the job over with. After glossing over the map of the installations, you are able to quickly locate their office and head down there."))
      vn.sfxBingo()
      vn.na(string.format(_("You arrive and inquire about Major Malik, but are told he is apparently enjoying some leisure time at %s in the %s system. Looks like you have no choice but to try to look for them there."), _(lastplanet), _(lastsys)))
      vn.run()

      -- Advance the state
      misn_state = 2
      misn.markerMove( misn_marker, system.get(lastsys) )
      misn.osdCreate( misn_title,
         { string.format(_("Look for Major Malik at %s in the %s system"), _(lastplanet), _(lastsys) ),
         _("Return to Kex at Minerva Station") } )

   elseif misn_state==3 and planet.cur() == planet.get(lastplanet) then
      npc_malik = misn.npcAdd( "approach_malik", _("Major Malik"), malik_portrait, _("You see Major Malik who is fairly similar to the image shown to you by Kex.") )

   elseif misn_state==4 and planet.cur() == planet.get(lastplanet) then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na("TODO")
      vn.run()

      misn_state = 5 -- We're done here, go back to kex:)
   end
end

function approach_kex ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition()

   -- Mission is over
   if misn_state==5 then

      vn.na(_("You return tired from your escapade once again to Kex, who is at his favourite spot at Minerva station."))
      vn.sfxMoney()
      vn.func( function () player.pay( money_reward ) end )
      vn.na(string.format(_("You received #g%s#0."), creditstring( money_reward )))
      vn.sfxVictory()
      vn.run()

      shiplog.append( logidstr, _("TODO"))
      misn.finish( true )
      return

   elseif misn_state==nil then
      vn.na(_("You find Kex taking a break at his favourite spot at Minerva station. His eyes light up when he sees you and he runs over to you."))
      kex(_([["Good news kid! It looks like the information obtained from Baroness Eve got some dirt on the CEO. It seems like they're involved in some sort of money laundering, but this alone won't get us anywhere, we need more intel."]]))
      kex(_([["We have another lead on another co-conspirator, this time in Dvaered space. Some individual named Major Malik seems to also be in the money laundering. Would you be up for paying them a visit and seeing what you can get out of them?"]]))
      vn.menu( {
         { _("Accept"), "accept" },
         { _("Decline"), "decline" },
      } )
      vn.label("decline")
      kex(_([[He looks dejected.
   "I see. If you change your mind, I'll be around."]]))
      vn.done()

      vn.label("accept")
      kex(string.format(_([["This time I'm hoping it's a cinch. Major Malik should be at %s in the %s system. They should be fairly old, so it should be to just outright confront him and get him to talk. I'll give you a note that if you show him should be easy enough to convince him. I'll send you a picture of him so you can easily recognize him when you see him."]]), _(targetplanet), _(targetsys)))
      kex(_([["The note? It's just your run-of-the-mill blackmail. We don't really care about Major Malik himself, what we want is mud on the CEO. Hopefully they'll be sensible and give us what we want. However, I trust you will do what it takes in they case they don't."
He winks his cyborg eye at you.]]))
      vn.func( function ()
         misn_state = 0
      end )
   else
      vn.na(_("You find Kex taking a break at his favourite spot at Minerva station."))
   end

   vn.label("menu_msg")
   kex(_([["What's up kid?"]]))
   vn.menu( function ()
      local opts = {
         { _("Ask about the job"), "job" },
         --{ _("Ask about his family"), "family" },
         -- TODO more fancy text
         { _("Leave"), "leave" },
      }
      return opts
   end )

   vn.label("job")
   if not misn_state or misn_state < 2 then
      kex(_([["The job is pretty straight forward. We need Major Malik to talk about his dealings with the Minerva CEO. If you hand him the letter I gave you it should be enough to convince him."]]))
      kex(string.format(_([["You should be able to find him at %s in the %s system. I don't think he should give much trouble."]]), _(targetplanet), _(targetsys)))
   else
      kex(string.format(_([["Oh, so Major Malik wasn't at %s? That is really weird. Let's hope you can find him at %s in the %s system."]]), _(targetplanet), _(lastplanet), _(lastsys)))
   end
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function enter ()
   if misn_state==2 and system.cur() == system.get(targetsys) then
      -- Spawn thugs after the player. Player should likely be going to Dvaer
      local pos = vec2.new( 15000, 15000 )
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
      thug_leader:follow( pp )

      -- Move to next state
      misn_state = 3

      -- Timer
      hook.timer( 5e3, "thug_heartbeat" )
   end
end

function thug_heartbeat ()
   local det, fuz = thug_leader:inrange( player.pilot() )
   if det and fuz then
      -- Start the attack, should be close enough to aggro naturally
      thug_leader:control(false)
      thug_leader:broadcast( _("That's the the one!"), true )
      for k,p in ipairs(thug_pilots) do
         p:setHostile(true)
      end

      -- Reset autonav just in case
      player.autonavReset( 5 )
      return
   end
   -- Keep on beating
   hook.timer( 5e3, "thug_heartbeat" )
end

function approach_malik ()
   local dogauntlet
   vn.clear()
   vn.scene()
   local malik = vn.newCharacter( _("Major Malik"), {image=malik_image} )
   vn.transition()
   vn.na(_("You approach Major Malik who seems to be enjoying a drink."))
   malik(_([["What do you want?"]]))
   vn.na(_("You hand him the note you got from Kex. He reads it and furrows his brows a bit."))
   malik(_([["I see. You think at my age I worry about what happens to me? I've had a good ..."]]))
   -- TODO finish
   vn.func( function () dogauntlet = true end )
   vn.run()

   if dogauntlet then
      gauntlet_start()
   end
end

gauntletsys = system.get("Crimson Gauntlet")

function gauntlet_start ()
   hook.safe("enter_the_ring")
   player.takeoff()
end

function abort ()
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

   -- Teleport the player to the Crimson Gauntlet and hide the rest of the universe
   local sys = gauntletsys
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)

   -- Set up player stuff
   player.pilot():setPos( vec2.new( 0, 0 ) )
   -- Disable escorts if they exist
   var.push("hired_escorts_disabled",true)
   player.teleport(sys)
   var.pop("hired_escorts_disabled")

   -- Clean up
   pilot.clear() -- Want no escorts and the likes

   -- Set up Player
   local player_vendetta = player.addShip( "Vendetta", _("Ketchup"), true )
   player_prevship = player.pilot():name() -- Ship to go back to
   player.swapShip( player_vendetta, true )
   equipopt.generic( player.pilot(), {type_range={Afterburner={min=1}}}, "elite" )

   -- Set up Major Malik
   enemy_faction = faction.dynAdd( "Dvaered", "Combatant", _("Dvaered"), {ai="dvaered_norun"} )
   pos = vec2.new( -1500, 1500 )
   pmalik = pilot.add( "Dvaered Vendetta", enemy_faction, pos, _("Major Malik") )
   pmalik:setInvincible(true)
   pmalik:setHostile(true)
   pmalik:control(true)
   pmalik:brake()
   pmalik:face( player.pilot() )
   hook.pilot( pmalik, "death", "malik_death" )
   enemies = {pmalik}

   -- Taunt stuff
   hook.timer( 3e3, "countdown_start" )
   hook.timer( 10e3, "malik_taunt" )
end
-- Goes back to Totoran (landed)
function leave_the_ring ()
   -- Stop spfx stuff
   luaspfx.exit()

   -- Clear pilots so escorts get docked
   pilot.clear()
   -- Give the player back their old ship
   player.swapShip( player_prevship, true, true )
   -- Fix the map up
   local sys = gauntletsys
   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   -- Undo player invincibility stuff and land
   local pp = player.pilot()
   player.land( planet.get("Totoran") )
end

--[[
   Countdown stuff
--]]
function countdown_start ()
   omsg_id = player.omsgAdd( _("5…"), 1.1 )
   hook.timer( 1*1000, "countdown", _("4…") )
   hook.timer( 2*1000, "countdown", _("3…") )
   hook.timer( 3*1000, "countdown", _("2…") )
   hook.timer( 4*1000, "countdown", _("1…") )
   hook.timer( 5*1000, "countdown_done" )
end
function countdown( str )
   -- TODO play countdown sound
   player.omsgChange( omsg_id, str, 1.1 )
end
function countdown_done ()
   -- TODO play sound and cooler text
   player.omsgChange( omsg_id, _("FIGHT!"), 3 )

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
      tauntperm = tauntperm or rnd.permutation(taunts)
      tauntid = tauntid or rnd.rnd(1,#taunts)
      tauntid = (tauntid % #taunts)+1
      pmalik:comm( player.pilot(), tauntperm[tauntid], true )

      hook.timer( 7e3, "malik_taunt" )
   end
end

function malik_death ()
   hook.timer( 5e3, "malik_speech" )
end

function malik_respawn ()
   luaspfx.init()

   enemies = {}
   local pos = player.pos()
   luaspfx.addfg( luaspfx.effects.alert, {size=200}, 3.2, pos )
   hook.timer( 3e3, "malik_respawn_real", pos )
end
function malik_respawn_real( pos )
   pmalik = pilot.add("Dvaered Goddard", enemy_faction, pos, _("Major Malik"))
   pmalik:setHostile(true)
   pmalik:setNoDeath(true)
   hook.pilot( pmalik, "disable", "malik_disabled" )
   hook.pilot( pmalik, "board", "malik_boarded" )
end
function malik_disabled ()
   shader.rmPPShader( noise_shader )
   pmalik:disable(true)
   player.omsgAdd( _("Aaaaaargh!"), 3 )

   -- Kill all enemies
   for k,v in ipairs(enemies) do
      if v:exists() then
         v:setHealth( -100, -100 )
      end
   end

   -- Maikki and friends go away
   for k,v in ipairs(pilot.get({faction.get("Pirate")})) do
      v:rm()
   end
end

function malik_spawn_more ()
   local pos = player.pos() + vec2.new( 200*rnd.rnd(), 360*rnd.rnd() )
   luaspfx.addfg( luaspfx.effects.alert, {size=100}, 2.2, pos )
   hook.timer( 2e3, "malik_spawn_more_real", pos )
end
function malik_spawn_more_real( pos )
   local p = pilot.add("Dvaered Vendetta", enemy_faction, pos )
   p:setHostile(true)
   table.insert( enemies, p )
end
function malik_spawn_more2 ()
   malik_spawn_more()
   malik_spawn_more()
end

function noise_start ()
   noise_shader = pp_shaders.corruption( 0.8 )
   shader.addPPShader( noise_shader, "gui" )
end

function noise_worsen ()
   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 1.5 )
   shader.addPPShader( noise_shader, "gui" )
end

function maikki_arrives ()
   local pos = player.pos()
   local mc = minerva.maikkiP.colour
   local col = {mc[1], mc[2], mc[3], 0.3}
   luaspfx.addfg( luaspfx.effects.alert, {size=200, col=col}, 1.2, pos )
   hook.timer( 1e3, "maikki_arrives_real", pos )
   -- Add more extras
   for i=1,4 do
      local pos = player.pos() + vec2.new( 200*rnd.rnd(), 360*rnd.rnd() )
      luaspfx.addfg( luaspfx.effects.alert, {size=100, col=col}, 1.2, pos )
      hook.timer( 1e3, "maikki_arrives_extra", pos )
   end
end
function maikki_arrives_real( pos )
   local p = pilot.add( "Pirate Kestrel", "Pirate", pos, _("Pink Demon") )
   p:setFriendly(true)
   p:control()
   p:attack( pmalik )
   pmaikki = p

   -- Make really really strong
   p:intrinsicSet( "armour", 1e6 )
   p:intrinsicSet( "shield", 1e6 )
   p:intrinsicSet( "energy", 1e6 )
   p:intrinsicSet( "fwd_damage", 400 )
   p:intrinsicSet( "tur_damage", 400 )
   p:intrinsicSet( "fwd_dam_as_dis", 50 )
   p:intrinsicSet( "tur_dam_as_dis", 50 )

   local mc = minerva.maikkiP.colour
   local col = colour.new( mc[1], mc[2], mc[3], 1.0 )
   player.omsgAdd( _("Ho ho ho and a bottle of rum!"), 5, nil, col )

   -- Be nice to player and make the AI rethink priorities
   for k,v in ipairs(pilot.get({enemy_faction})) do
      v:taskClear()
   end
end
function maikki_arrives_extra( pos )
   local p = pilot.add( "Pirate Shark", "Pirate", pos )
   p:setFriendly(true)
   p:intrinsicSet( "armour", 1e5 )
   p:intrinsicSet( "shield", 1e5 )
   p:intrinsicSet( "energy", 1e5 )
   p:intrinsicSet( "fwd_damage", 400 )
   p:intrinsicSet( "tur_damage", 400 )
end

function malik_speech ()
   malik_speech_state = malik_speech_state or 0
   malik_speech_state = malik_speech_state + 1
   local speeches = {
      { delay=5e3, txt=_("You come to my office to harass me…") },
      { delay=5e3, txt=_("You interrupt my leisure with blackmail…") },
      { delay=5e3, txt=_("You really think you would be getting out of here alive?…"), func=malik_respawn },
      { delay=5e3, txt=_("You are in my realm now kid!"), func=malik_spawn_more },
      { delay=5e3, txt=_("And the only way out is in a body bag!"), func=malik_spawn_more },
      { delay=5e3, txt=_("My power is limitless here!"), func=malik_spawn_more },
      { delay=5e3, txt=_("You are a fool to have walked into my trap so willingly."), func=malik_spawn_more2 },
      { delay=2e3, txt=_("Not so tough anymore! Ha ha ha!"), func=malik_spawn_more2 },
      { delay=3e3, func=noise_start },
      { delay=5e3, txt=_("What is going on? It's not responding!"), func=noise_worsen },
      { delay=5e3, func=maikki_arrives },
      { delay=5e3, txt=_("What the hell?!") },
   }

   local s = speeches[ malik_speech_state ]
   if not s then
      return
   end
   if s.txt then
      player.omsgAdd( s.txt, s.delay / 1000 )
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
   misn_state = 4
   hook.safe( "leave_the_ring" )
end
