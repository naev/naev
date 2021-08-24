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
local vn       = require 'vn'
local equipopt = require 'equipopt'
local love_shaders = require 'love_shaders'
require 'numstring'

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted try to go find Dr. Strangelove
--  1: Destroy Bounty Hunter
--  2: meet Dr. Strangelove
--  3: return to kex
misn_state = nil

targetplanet = "Strangelove Lab"
targetsys = planet.get(targetplanet):system():nameRaw()

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
   vn.clear()
   vn.scene()
   -- TODO sadder music
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition()

   vn.na(_("You approach Kex who is sort of slumping at his usual spot. He doesn't seem to be in very good condition."))
   kex(_([[It takes a bit before Kex notices your presence. His feathers are rather disheveled.
"Oh, hey kid."]]))
   kex(_([[It takes a while before he continues.
"Compared to the vastness of all the universe, we are completely insignificant. Our sorrows and glories go unheard and seemingly devoid of all meaning."]]))
   kex(_([["Coming to terms with one's own significance does give a clarity of mind and make clear one's deepest desires. What has to be done then becomes clear."]]))
   vn.sfxEerie()
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
   vn.na(string.format(_([[Without saying anything else, Kex walks off stumbling into the darkness of the station. You feel like it is best to leave him alone right now and decide to go see Strangelove. He should be in the %s system.]]),targetsys))

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
      { string.format(_("Go find Dr. Strangelove at %s in the %s system"), _(targetplanet), _(targetsys) ),
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
         hook.timer( 5, "strangelove_hail" )

         -- Remove station
         diff.remove( eccdiff )
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
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_("You once again land on Strangelove's Laboratory, which "))
   vn.run()

   -- Take off
   misn_state = 2
   player.takeoff()
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
   vn.done( "electric" )
   vn.run()

   strangelove_ship:setHilight(true)
   strangelove_ship:setVisplayer(true)
end

function strangelove_board ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.strangelove )
   local dr = vn.newCharacter( strangelove.name,
         { color=strangelove.colour, image=minerva.strangelove.image, } )
   -- TODO small scene
   vn.run()

   misn_state = 3
   player.unboard()
end
