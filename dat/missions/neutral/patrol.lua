--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Patrol">
 <priority>4</priority>
 <cond>
   return require("misn_test").mercenary()
 </cond>
 <chance>560</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Proteron</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Thurion</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Patrol

   Generalized replacement for Dvaered patrol mission. Can work with any
   faction.

--]]
local pir = require "common.pirate"
local fmt = require "format"
local vntk = require "vntk"
local lmisn = require "lmisn"

-- luacheck: globals abandon_text msg pay_text (shared with derived mission pirate.patrol)
-- luacheck: globals enter jumpout land pilot_leave timer (Hook functions passed by name)

pay_text    = {
   _("After going through some paperwork, an officer hands you your pay and sends you off."),
   _("A tired-looking officer verifies your mission log and hands you your pay."),
   _("The officer you deal with thanks you for your work, hands you your pay, and sends you off."),
   _("An officer goes through the necessary paperwork, looking bored the entire time, and hands you your fee."),
}

abandon_text    = {
   _("You are sent a message informing you that landing in the middle of a patrol mission is considered to be abandonment. As such, your contract is void and you will not receive payment."),
}

-- Messages
msg = {
   _("Point secure."),
   _("Hostiles detected. Engage hostiles."),
   _("Hostiles eliminated."),
   _("Patrol complete. You can now collect your pay."),
   _("You showed up too late."),
   _("You have left the {sys} system."),
}

mem.osd_msg = {
   _("Fly to the {sys} system"),
   "(null)",
   _("Eliminate hostiles"),
   _("Land in {fct} territory to collect your pay"),
}

mem.use_hidden_jumps = false

-- Get the number of enemies in a particular system
local function get_enemies( sys )
   local enemies = 0
   for i, j in ipairs( mem.paying_faction:enemies() ) do
      local p = sys:presences()[j:nameRaw()]
      if p ~= nil then
         enemies = enemies + p
      end
   end
   return enemies
end

function create ()
   mem.paying_faction = spob.cur():faction()

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 2,
      function(s)
         local this_faction = s:presences()[mem.paying_faction:nameRaw()]
         return this_faction ~= nil and this_faction > 0 and get_enemies(s) > 0
      end, nil, mem.use_hidden_jumps )
   if get_enemies( system.cur() ) then
      systems[ #systems + 1 ] = system.cur()
   end

   if #systems <= 0 then
      misn.finish( false )
   end

   -- Try to cache the route to make it so that the same route doesn't appear over and over
   local c = naev.cache()
   local t = time.get()
   if not c.misn_patrols then
      c.misn_patrols = {}
   end
   if c.misn_patrols._t ~= t then
      c.misn_patrols = { _t=t } -- Regenerate
   end
   local newsystems = {}
   for k,s in ipairs(systems) do
      if not c.misn_patrols[s:nameRaw()]  then
         newsystems[#newsystems+1] = s
      end
   end
   systems = newsystems

   if #systems <= 0 then
      misn.finish( false )
   end

   mem.missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.missys, true ) then misn.finish( false ) end

   local planets = mem.missys:spobs()
   local numpoints = math.min( rnd.rnd( 2, 5 ), #planets )
   mem.points = {}
   while numpoints > 0 and #planets > 0 do
      local p = rnd.rnd( 1, #planets )
      mem.points[ #mem.points + 1 ] = planets[p]
      numpoints = numpoints - 1

      local new_planets = {}
      for i, j in ipairs( planets ) do
         if i ~= p then
            new_planets[ #new_planets + 1 ] = j
         end
      end
      planets = new_planets
   end
   if #mem.points < 2 then
      misn.finish( false )
   end

   mem.jumps_permitted = math.huge --system.cur():jumpDist(mem.missys) + 3
   mem.hostiles = {}
   mem.hostiles_encountered = false

   local n_enemies = get_enemies( mem.missys )
   if n_enemies == 0 then
      misn.finish( false )
   end
   mem.credits = n_enemies * 2000
   mem.credits = mem.credits + rnd.sigma() * (mem.credits / 3)
   mem.reputation = math.floor( n_enemies / 75 )

   -- Faction prefix
   local prefix
   if mem.paying_faction:static() then
      prefix = ""
   else
      prefix = require("common.prefix").prefix(mem.paying_faction)
   end

   -- Set mission details
   misn.setTitle(prefix..fmt.f(_("Patrol of the {sys} System"),
      {sys=mem.missys}))
   local desc = fmt.f(_([[Patrol specified points in the {sys} system, eliminating any hostiles you encounter.

#nPatrol System:#0 {sys}
#nPatrol Points:#0 {amount}]]),
      {amount=#mem.points, sys=mem.missys})
   if not mem.paying_faction:static() then
      desc = desc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=mem.paying_faction})
   end
   misn.setDesc(desc)
   misn.setReward( mem.credits )
   mem.marker = misn.markerAdd( mem.missys, "computer" )

   -- Mark the system as having a patrol mission
   c.misn_patrols[mem.missys:nameRaw()] = true
end

function accept ()
   misn.accept()

   mem.osd_msg[1] = fmt.f( mem.osd_msg[1], {sys=mem.missys} )
   mem.osd_msg[2] = n_(
      "Go to indicated point (%d remaining)",
      "Go to indicated point (%d remaining)",
      #mem.points
   ):format( #mem.points )
   mem.osd_msg[4] = fmt.f( mem.osd_msg[4], {fct=mem.paying_faction} )
   misn.osdCreate( _("Patrol"), mem.osd_msg )

   mem.job_done = false

   hook.enter( "enter" )
   hook.jumpout( "jumpout" )
   hook.land( "land" )
end

function enter ()
   if system.cur() == mem.missys and not mem.job_done then
      timer()
   end
end

function jumpout ()
   if mem.mark ~= nil then
      system.markerRm( mem.mark )
      mem.mark = nil
   end

   mem.jumps_permitted = mem.jumps_permitted - 1
   local last_sys = system.cur()
   if not mem.job_done then
      if last_sys == mem.missys then
         lmisn.fail( fmt.f( msg[6], {sys=last_sys} ) )
      elseif mem.jumps_permitted < 0 then
         lmisn.fail( msg[5] )
      end
   end
end

function land ()
   if mem.mark ~= nil then
      system.markerRm( mem.mark )
      mem.mark = nil
   end

   local okspob = false
   -- Matching faction is always OK
   if spob.cur():faction() == mem.paying_faction then
      okspob = true
   -- Special case static factions we look for non-hostiles
   elseif mem.paying_faction:static() and not mem.paying_faction:areEnemies(spob.cur():faction()) then
      okspob = true
   end

   mem.jumps_permitted = mem.jumps_permitted - 1
   if mem.job_done and okspob then
      local txt = pay_text[ rnd.rnd( 1, #pay_text ) ]
      vntk.msg( _("Mission Completed"), txt )
      player.pay( mem.credits )
      if not pir.factionIsPirate( mem.paying_faction ) then
         pir.reputationNormalMission( mem.reputation )
      end
      mem.paying_faction:modPlayer( mem.reputation )
      misn.finish( true )
   elseif not mem.job_done and system.cur() == mem.missys then
      local txt = abandon_text[ rnd.rnd( 1, #abandon_text ) ]
      vntk.msg( _("Mission Abandoned"), txt )
      misn.finish( false )
   end
end

function pilot_leave ( pilot )
   local new_hostiles = {}
   for i, j in ipairs( mem.hostiles ) do
      if j ~= nil and j ~= pilot and j:exists() then
         new_hostiles[ #new_hostiles + 1 ] = j
      end
   end

   mem.hostiles = new_hostiles
end

function timer ()
   hook.rm( mem.timer_hook )

   local player_pos = player.pos()
   local enemies = pilot.get( mem.paying_faction:enemies() )

   for i, j in ipairs( enemies ) do
      if j ~= nil and j:exists() then
         local already_in = false
         for a, b in ipairs( mem.hostiles ) do
            if j == b then
               already_in = true
            end
         end
         if not already_in then
            if player_pos:dist( j:pos() ) < 1500 then
               j:setVisible( true )
               j:setHilight( true )
               j:setHostile( true )
               hook.pilot( j, "death", "pilot_leave" )
               hook.pilot( j, "jump", "pilot_leave" )
               hook.pilot( j, "land", "pilot_leave" )
               mem.hostiles[ #mem.hostiles + 1 ] = j
            end
         end
      end
   end

   if #mem.hostiles > 0 then
      if not mem.hostiles_encountered then
         player.msg( msg[2] )
         mem.hostiles_encountered = true
      end
      misn.osdActive( 3 )
   elseif #mem.points > 0 then
      if mem.hostiles_encountered then
         player.msg( msg[3] )
         mem.hostiles_encountered = false
      end
      misn.osdActive( 2 )

      local point_pos = mem.points[1]:pos()

      if mem.mark == nil then
         mem.mark = system.markerAdd( point_pos, _("Patrol Point") )
      end

      if player_pos:dist( point_pos ) < 500 then
         local new_points = {}
         for i = 2, #mem.points do
            new_points[ #new_points + 1 ] = mem.points[i]
         end
         mem.points = new_points

         player.msg( msg[1] )
         mem.osd_msg[2] = n_(
            "Go to indicated point (%d remaining)",
            "Go to indicated point (%d remaining)",
            #mem.points
         ):format( #mem.points )
         misn.osdCreate( _("Patrol"), mem.osd_msg )
         misn.osdActive(2)
         if mem.mark ~= nil then
            system.markerRm( mem.mark )
            mem.mark = nil
         end
      end
   else
      mem.job_done = true
      player.msg( msg[4] )
      misn.osdActive( 4 )
      if mem.marker ~= nil then
         misn.markerRm( mem.marker )
      end
   end

   if not mem.job_done then
      mem.timer_hook = hook.timer( 0.05, "timer" )
   end
end
