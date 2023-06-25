--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Hit Intro">
 <unique />
 <priority>3</priority>
 <cond>player.outfitNum("Mercenary License") &gt; 0 or spob.cur():blackmarket() or spob.cur():tags().criminal ~= nil</cond>
 <chance>100</chance>
 <location>Bar</location>
 <faction>Wild Ones</faction>
 <faction>Black Lotus</faction>
 <faction>Raven Clan</faction>
 <faction>Dreamer Clan</faction>
 <faction>Pirate</faction>
 <faction>Independent</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[
   Pirate Hit Intro

   Introductory mission to the pirate hits, opens up the generic version of the
   missions.
--]]
local pir = require "common.pirate"
local fmt = require "format"
local pilotname = require "pilotname"
local lmisn = require "lmisn"
local vn = require "vn"
local vni = require "vnimage"

local spawn_target -- Forward-declared functions

local hunters = {}
local hunter_hits = {}

local target_faction = "Independent"
local reward = 450e3

local givername = _("Shady Individual")
mem.giverimage, mem.giverportrait = vni.generic()

function create ()
   -- Lower probability on non-pirate places
   if not pir.factionIsPirate( spob.cur():faction() ) and rnd.rnd() < 0.2 then
      misn.finish(false)
   end

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 6,
      function(s)
         local p = s:presences()[ target_faction ]
         return (p ~= nil and p > 0)
      end, nil, true )

   if #systems == 0 then
      -- No enemy presence nearby
      misn.finish( false )
   end

   mem.missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.missys ) then
      misn.finish( false )
   end

   mem.jumps_permitted = system.cur():jumpDist(mem.missys, true) + rnd.rnd( 3, 10 )
   if rnd.rnd() < 0.05 then
      mem.jumps_permitted = mem.jumps_permitted - 1
   end

   mem.name = pilotname.generic()
   mem.retpnt, mem.retsys = spob.cur()

   misn.setNPC( givername, mem.giverportrait, _("You see a shady individual who seems to be looking for pilots to do a mission for them. You're not entirely sure you want to associate with them though.") )
end

local talked = false
function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local g = vn.newCharacter( givername, { image=mem.giverimage } )
   vn.transition()
   if not talked then
      vn.na(_([[You approach the shady character who begins to speak in a low voice, almost as if they don't want to be heard.]]))
      g(fmt.f(_([["You look like a decent pilot. I represent some clients, who value pilots who can get the job done with a bit of secrecy. You know what I mean? Getting the job done means making sure nothing gets in the way. My clients want a certain pilot by the name of {name} to be responsible for their actions. It's not very fair to ask for favours and not doing anything in return right?"]]),
         {name=mem.name}))
      g(fmt.f(_([["My clients want a pilot to go find {name} and get compensation for the damages caused. However, it is past the time that the problem can be solved by paying back the favour. {name} has to be punished for their excesses through more... drastic measures. Would you be willing to meet up with {name} and express my clients' discontent with, say, the full brunt of hot plasma? You will be compensated well."
They grin.]]),
         {name=mem.name}))
      talked = true
   else
      g(fmt.f(_([["Have you changed your mind about dealing with {name}? My clients are eager to know."]]),
         {name=mem.name}))
   end
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("accept")
   g(fmt.f(_([["My clients will be pleased to hear that. {name} can be found around the {sys} system. Leaving their ship as a charred memento to their avarice will be a fitting end for them."]]),
      {name=mem.name, sys=mem.missys}))
   vn.func( function () accepted = true end )
   vn.done()

   vn.label("decline")
   vn.na(_([[You curtly decline and leave.]]))
   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   mem.state = 1

   -- Set mission details
   misn.setTitle( _("Dark Compensation") )

   misn.setDesc( fmt.f( _("A pilot known as {plt}, recently seen in the {sys} system, has to be eliminated as compensation for some unknown clients."), {plt=mem.name, sys=mem.missys } ) )

   misn.setReward( reward )
   mem.marker = misn.markerAdd( mem.missys )

   misn.osdCreate( _("Dark Compensation"), {
      fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} ),
      fmt.f( _("Kill {plt}"), {plt=mem.name} ),
      fmt.f( _("Return to {pnt} ({sys} system)"), {pnt=mem.retpnt,sys=mem.retsys} ),
   } )

   mem.last_sys = system.cur()

   hook.jumpin( "jumpin" )
   hook.jumpout( "jumpout" )
   hook.takeoff( "takeoff" )
   hook.land( "land" )
end


function jumpin ()
   -- Nothing to do.
   if system.cur() ~= mem.missys then
      return
   end

   local pos = jump.pos( system.cur(), mem.last_sys )
   local offset_ranges = { { -2500, -1500 }, { 1500, 2500 } }
   local xrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   local yrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   pos = pos + vec2.new( rnd.rnd( xrange[1], xrange[2] ), rnd.rnd( yrange[1], yrange[2] ) )
   spawn_target( pos )
end


function jumpout ()
   if mem.state ~= 1 then
      return
   end

   mem.jumps_permitted = mem.jumps_permitted - 1
   mem.last_sys = system.cur()
   if mem.last_sys == mem.missys then
      lmisn.fail( fmt.f( _("You have left the {sys} system."), {sys=mem.last_sys} ) )
   end
end


function takeoff ()
   -- Nothing to do.
   if system.cur() ~= mem.missys then
      return
   end

   spawn_target()
end

function pilot_attacked( _p, attacker, dmg )
   if attacker ~= nil then
      local found = false

      for i, j in ipairs( hunters ) do
         if j == attacker then
            hunter_hits[i] = hunter_hits[i] + dmg
            found = true
         end
      end

      if not found then
         local i = #hunters + 1
         hunters[i] = attacker
         hunter_hits[i] = dmg
      end
   end
end

function pilot_death( _p, _attacker )
   mem.state = 2
   player.msg(fmt.f(_("Target eliminated! Return to {pnt} ({sys} system)."), {pnt=mem.retpnt,sys=mem.retsys}))
   misn.osdActive( 3 )
   misn.markerMove( mem.marker, mem.retpnt )
end

function pilot_jump ()
   lmisn.fail( _("Target got away.") )
end

-- Spawn the ship at the location param.
local _target_faction
function spawn_target( param )
   if mem.state ~= 1 then
      return
   end

   if mem.jumps_permitted < 0 then
      lmisn.fail( _("Target got away.") )
      return
   end

   -- Use a dynamic faction so they don't get killed
   if not _target_faction then
      _target_faction = faction.dynAdd( target_faction, "wanted_"..target_faction, target_faction, {clear_enemies=true, clear_allies=true} )
   end

   misn.osdActive( 2 )
   local target_ship = pilot.add( "Koala", _target_faction, param, mem.name )
   target_ship:setHilight( true )
   hook.pilot( target_ship, "attacked", "pilot_attacked" )
   hook.pilot( target_ship, "death", "pilot_death" )
   hook.pilot( target_ship, "jump", "pilot_jump" )
   hook.pilot( target_ship, "land", "pilot_jump" )
   return target_ship
end


function land ()
   if mem.state ~= 2 or spob.cur() ~= mem.retpnt then
      return
   end

   vn.clear()
   vn.scene()
   local g = vn.newCharacter( givername, { image=mem.giverimage } )
   vn.transition()
   vn.na(_([[You land and find the shady individual beckoning to you from the corner.]]))
   g(_([["Good job out there. My clients are most pleased with the result. They would like to give you more job offers, here, I'll give you a secret key that will let you access more missions from the mission terminal computer when possible. It was a pleasure working with you."
They grin and then fade into the shadows.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.run()

   player.pay( reward )

   pir.addMiscLog(_("You performed a 'hit' on a debt-ridden pilot for some unknown clients. They were satisfied with your job and this opened up more similar missions at the mission computer."))

   -- Pirate rep cap increase
   pir.modReputation( 5 )
   misn.finish( true )
end
