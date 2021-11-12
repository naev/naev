--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Moving Up">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <done>Garbage Person</done>
  <chance>30</chance>
  <location>Bar</location>
  <faction>Soromid</faction>
  <cond>var.peek("comingout_time") == nil or time.get() &gt;= time.fromnumber(var.peek("comingout_time")) + time.create(0, 20, 0)</cond>
 </avail>
 <notes>
  <campaign>Coming Out</campaign>
 </notes>
</mission>
--]]
--[[

   Moving Up

--]]
local fmt = require "format"
local pilotname = require "pilotname"
local srm = require "common.soromid"
local pir = require "common.pirate"
local equipopt = require 'equipopt'
local lmisn = require "lmisn"

local chelsea -- Non-persistent state
local fail, spawn -- Forward-declared functions

function create ()
   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         return pir.systemPresence( s ) > 0
      end )

   if #systems == 0 then
      -- No pirates nearby
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   pirname = pilotname.pirate()
   credits = 300e3
   started = false

   misn.setNPC( _("Chelsea"), "soromid/unique/chelsea.webp", _("Oh, it's Chelsea! You feel an urge to say hello.") )
end


function accept ()
   local txt
   if started then
      txt = fmt.f( _([["Hey, {player}! Any chance you could reconsider? I could use your help."]]), {player=player.name()} )
   else
      txt = fmt.f(_([[You greet Chelsea as usual and have a friendly chat with them. You learn that they had a close call recently with another thug, but they managed to shake the thug off with their new ship.
    "It's a lot better than before. The work is tough though. I've been picking off small pirates with bounties on their heads, doing relatively safe system patrols, that sort of thing. I'm supposed to be getting a better ship soon, but it's going to be difficult." You ask them why that is. "Well, I came across someone who's offering me a bargain on a new ship! Well, not new exactly. It's used, but in much better condition than that rust bucket I got before. Supposedly this guy used to be a Dvaered warlord and is offering me his old Vigilance if I just take care of this one pirate known as {plt}. Trouble is they're piloting a ship that's stronger than my own...."
    Chelsea pauses in contemplation for a moment. "Say, do you think you could help me out on this one? I just need you to help me kill the pirate in {sys}. I'll give you {credits} for the trouble. How about it?"]]), {plt=pirname, sys=missys, credits=fmt.credits(credits)} )
   end
   started = true

   if tk.yesno( _("A Great Opportunity"), txt ) then
      tk.msg( _("A Great Opportunity"), fmt.f( _([["Fantastic! Thank you for the help! I'll meet you in {sys} and we can take the pirate out. Let's do this!"]]), {sys=missys} ) )
      misn.accept()

      misn.setTitle( _("Moving Up") )
      misn.setDesc( fmt.f( _("Chelsea needs you help them kill a wanted pirate in {sys}."), {sys=missys} ) )
      misn.setReward( fmt.credits( credits ) )
      marker = misn.markerAdd( missys, "high" )

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( _("A Great Opportunity"), _([["Ah, you're busy, eh? Oh well. Let me know if you change your mind, OK?"]]) )
      misn.finish()
   end
end


function enter ()
   if system.cur() == missys then
      spawn()
   end
end


function spawn ()
   pilot.clear()
   pilot.toggleSpawn( false )

   -- Spawn pirate
   local r = system.cur():radius()
   local x = rnd.rnd( -r, r )
   local y = rnd.rnd( -r, r )
   local p = pilot.add( "Pirate Phalanx", "Pirate", vec2.new( x, y ) )

   hook.pilot( p, "death", "pirate_death" )
   p:setHostile()
   p:setVisible( true )
   p:setHilight( true )

   fass = faction.dynAdd( "Independent", "Comingout_associates", _("Mercenary") )
   -- Spawn Chelsea
   chelsea = pilot.add( "Lancelot", fass, lastsys, _("Chelsea"), {naked=true} )
   equipopt.generic( chelsea, nil, "elite" )

   chelsea:setHealth( 100, 100 )
   chelsea:setEnergy( 100 )
   chelsea:setTemp( 0 )
   chelsea:setFuel( true )

   chelsea:setFriendly()
   chelsea:setHilight()
   chelsea:setVisible()
   chelsea:setInvincPlayer()

   hook.pilot( chelsea, "death", "chelsea_death" )
   hook.pilot( chelsea, "jump", "chelsea_leave" )
   hook.pilot( chelsea, "land", "chelsea_leave" )
end


function leave ()
   lastsys = system.cur()
   if lastsys == missys then
      fail( _("MISSION FAILED: You have abandoned the mission.") )
   end
end


function chelsea_death ()
   fail( _("MISSION FAILED: A rift in the space-time continuum causes you to have never met Chelsea in that bar.") )
end


function chelsea_leave ()
   fail( _("MISSION FAILED: Chelsea has abandoned the mission.") )
end


function pirate_death ()
   chelsea:setNoDeath( true )
   pilot.toggleSpawn( true )
   hook.timer( 1.0, "win_timer" )
end


function win_timer ()
   tk.msg( _("Death Of A Pirate"), fmt.f( _([[Chelsea pops up on your viewscreen and grins. "We did it!" they say. "Thanks for all the help, {player}. I've transferred the money into your account. See you next time with my new ship!" You say your goodbyes and go back to your own adventures.]]), {player=player.name()} ) )
   player.pay( credits )

   local t = time.get():tonumber()
   var.push( "comingout_time", t )

   srm.addComingOutLog( _([[You helped Chelsea hunt down a wanted pirate, earning a bounty for both of you and allowing Chelsea to acquire a retired Dvaered warlord's old Vigilance.]]) )

   misn.finish( true )
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("#") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "#r" .. message .. "#0" )
      end
   end
   misn.finish( false )
end
