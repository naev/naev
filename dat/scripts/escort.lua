--[[--
Library for dealing with escorts in missions.

Can be used either to provide the player with a few perishable escorts, or provide a set of escorts the player has to follow and guard to the destination.

Below is a simple example of a convoy that the player has to protect while it goes on to some spob.
```
-- This is an example of using the API to provide a fleet the player has to
-- guard starting from the start_convoy function that sets it up
-- The mission will automatically fail if all the escorts die
function start_convoy ()
   -- Initialize the library with the pilots the player has to escort
   escort.init( {"Koala", "Llama"}, {faction=faction.get("Independent")} )
   -- Set the destination target, the escorts will try to go there automagically
   escort.setDest( spob.get("Darkshed"), "convoy success" )
end
-- This function will be run  when the player lands on the destination
function convoy_success ()
   local alive = escort.num_alive()
   -- Do something based on the number of escorts that are alive
   do_something( alive )
   -- Cleans up the library
   escort.exit()
end
```

Below is a more complex example where the player becomes the leader and the escorts follow and protect the player.
```
-- Initialize the library and give the player two escorts that will guard them / help them out
function start_convoy ()
   escort.init( {"Lancelot", "Lancelot"}, {faction=faction.get("Mercenary"), nofailifdead=true, func_pilot_death="escort_died"} )
end
-- The escort died
function escort_died( _p )
   if escort.num_alive() <= 0 then
      -- All the escorts are dead
   end
end
```

@module escort
--]]
local escort = {}

local lmisn = require "lmisn"
local fmt = require "format"
local vntk = require "vntk"
local aisetup = require "ai.core.setup"

local DISTANCE_THRESHOLD2 = 2000^2
local HEARTBEAT_TIMER = 1

local escort_outfits

--[[--
Initializes the library by setting all the necessary hooks.

   @tparam table ships List of ships to add.
   @tparam[opt] table params List of optional parameters.
--]]
function escort.init( ships, params )
   params = params or {}
   mem._escort = {
      convoy = {},
      params = params,
      ships_orig = ships,
      ships = tcopy(ships),
      faction = params.faction or faction.get("Independent"),
      nofollowplayer = params.nofollowplayer,
      nofailifdead = params.nofailifdead,
      followorders = params.followorders,
      hooks = {
         jumpin   = hook.jumpin(  "_escort_jumpin" ),
         jumpout  = hook.jumpout( "_escort_jumpout" ),
         land     = hook.land(    "_escort_land" ),
         takeoff  = hook.takeoff( "_escort_takeoff"),
      },
   }

   -- Initialize to wherever the player is
   if player.isLanded() then
      mem._escort.origin = spob.cur()
   else
      mem._escort.origin = system.cur()
   end
end

local heartbeat
local function clear_hooks ()
   if mem._escort.hooks.jumpin then
      hook.rm( mem._escort.hooks.jumpin )
      mem._escort.hooks.jumpin = nil
   end
   if mem._escort.hooks.jumpout then
      hook.rm( mem._escort.hooks.jumpout )
      mem._escort.hooks.jumpout = nil
   end
   if mem._escort.hooks.land then
      hook.rm( mem._escort.hooks.land )
      mem._escort.hooks.land = nil
   end
   if mem._escort.hooks.takeoff then
      hook.rm( mem._escort.hooks.takeoff )
      mem._escort.hooks.takeoff = nil
   end
   if heartbeat then
      hook.rm( heartbeat )
      heartbeat = nil
   end
end

--[[--
Cleans up the escort framework when done, eliminating all hooks.
--]]
function escort.exit ()
   clear_hooks()
   mem._escort = nil
end

--[[--
Gets the number of escorts that are still alive.

   @treturn number Number of escorts still alive.
--]]
function escort.num_alive ()
   return #mem._escort.ships
end

--[[--
Gets the list of pilots.

   @treturn table Table containing the existing pilots. The first will be the leader.
--]]
function escort.pilots ()
   return mem._escort.convoy
end

--[[--
Sets the destination of the escort convoy.

Disables the escorts from following the player.

   @tparam system|spob dest Destination of the escorts.
   @tparam string success Name of the global function to call on success.
   @tparam[opt] string failure Name of the global function to call on failure. Default will give a vntk message and fail the mission.
--]]
function escort.setDest( dest, success, failure )
   if dest==nil then
      mem._escort.destspob = nil
      mem._escort.destsys = nil
      mem._escort.route = nil
      mem._escort.current = nil
      mem._escort.nextsys = nil
   end

   if dest.system then -- Check to see if it's a spob
      mem._escort.destspob = dest
      mem._escort.destsys = dest:system()
   else
      mem._escort.destspob = nil
      mem._escort.destsys = dest
   end
   mem._escort.func_success = success
   mem._escort.func_failure = failure or "_escort_failure"

   -- Figure out the route and how to get there
   mem._escort.route = lmisn.getRoute( system.cur(), mem._escort.destsys )
   mem._escort.current = 1
   mem._escort.nextsys = mem._escort.route[ mem._escort.current+1 ]
end

local exited
local function run_success ()
   _G[mem._escort.func_success]()
   clear_hooks()
end

function escort.reset_ai ()
   local followorders = mem._escort.followorders

   -- Clear speed limits and such
   for k,p in ipairs(mem._escort.convoy) do
      if p:exists() then
         local pm = p:memory()
         if (not pm._escort_jump) and (not pm._escort_land) then
            p:setSpeedLimit(0)
            p:control(false)
            p:setNoJump(false)
            p:setNoLand(false)
            p:taskClear()
            p:memory().ignoreorders = not followorders
         end
      end
   end

   if mem._escort.nofollowplayer then
      if not mem._escort.destsys then
         error("Escorts don't follow player and have no destination!")
      end

      -- Find the leader
      local l
      for k,v in ipairs(mem._escort.convoy) do
         if v:exists() then
            l = v
            break
         end
      end
      if not l then return end

      -- Find and limit max speed
      local minspeed = player.pilot():stats().speed_max * 0.9
      for k,p in ipairs(mem._escort.convoy) do
         if p:exists() then
            minspeed = math.min( p:stats().speed_max * 0.95, minspeed )
         end
      end
      l:setSpeedLimit( minspeed )

      -- Control leader and set new target
      l:control(true)
      local scur = system.cur()
      if scur == mem._escort.destsys then
         if mem._escort.destspob then
            l:land( mem._escort.destspob )
         else
            run_success()
         end
      else
         l:hyperspace( lmisn.getNextSystem( scur, mem._escort.destsys ) )
      end
   end
end

function escort.update_leader ()
   local l
   for k,v in ipairs(mem._escort.convoy) do
      if v:exists() then
         local pm = v:memory()
         if (not pm._escort_jump) and (not pm._escort_land) then
            l = v
            break
         end
      end
   end
   if not l then return end

   l:setLeader()
   l:setHilight(true)
   for k,v in ipairs(mem._escort.convoy) do
      if v~=l and v:exists() then
         local pm = v:memory()
         if (not pm._escort_jump) and (not pm._escort_land) then
            v:taskClear()
         end
         v:setLeader(l)
      end
   end

   if not l:flags("manualcontrol") then
      escort.reset_ai()
   end
end
function _escort_update_leader ()
   escort.update_leader()
end

function _escort_e_death( p )
   for k,v in ipairs(mem._escort.convoy) do
      if v==p then
         player.msg( "#r"..fmt.f(_("{plt} has been lost!"), {plt=p} ).."#0" )

         table.remove(mem._escort.convoy, k)
         table.remove(escort_outfits, k)
         table.remove(mem._escort.ships, k)

         if escort.num_alive() <= 0 then
            local msg
            if #mem._escort.ships_orig==1 then
               msg = _("The escort has been lost!")
            else
               msg = _("All escorts have been lost!")
            end
            if mem._escort.nofailifdead then
               player.msg("#r"..msg.."#0")
            else
               lmisn.fail( msg )
            end
         else
            -- Set a new leader and tell them to move on
            if mem._escort.nofollowplayer then
               if k==1 then
                  hook.safe("_escort_update_leader")
               end
            end
         end

         if mem._escort.params.func_pilot_death then
            _G[mem._escort.params.func_pilot_death]( p )
         end
         return
      end
   end
end

function _escort_e_attacked( p, attacker )
   -- If they are not following the player, the player should protect them
   if mem._escort.nofollowplayer then
      player.autonavReset( 3 )
      attacker:setHostile(true) -- Make attacker hostile
   end
   if mem._escort.params.func_pilot_attacked then
      _G[mem._escort.params.func_pilot_attacked]( p, attacker )
   end
end

local function escorts_left ()
   local left = 0
   for k,v in ipairs(escort.pilots()) do
      if v:exists() then
         left = left+1
      end
   end
   return left
end

function _escort_e_land( p, landed_spob )
   if landed_spob == mem._escort.destspob then
      table.insert( exited, p )
      if p:exists() then
         player.msg( "#g"..fmt.f(_("{plt} has landed on {pnt}."), {plt=p, pnt=landed_spob} ).."#0" )
      end

      if escorts_left() <= 1 then
         player.msg("#g"..fmt.f(_("All escorts have landed on {pnt}."), {pnt=landed_spob}).."#0")
      else
         hook.safe("_escort_update_leader")
      end
   else
      _escort_e_death( p )
   end
end

function _escort_e_jump( p, j )
   if mem._escort.destsys and j:dest() == mem._escort.nextsys then
      table.insert( exited, p )
      if p:exists() then
         player.msg( "#g"..fmt.f(_("{plt} has jumped to {sys}."), {plt=p, sys=j:dest()} ).."#0" )
      end

      if escorts_left() <= 1 then
         player.msg("#g"..fmt.f(_("All escorts have jumped to {sys}. Follow them."), {sys=j:dest()}).."#0")
      else
         hook.safe("_escort_update_leader")
      end
   else
      _escort_e_death( p )
   end
end

--[[--
Spawns the escorts at location. This can be useful at the beginning if you want them to jump in or take of while in space. It is handled automatically when the player takes off or jumps into a system.

   @tparam Vector|Spob|System pos Position to spawn the fleet at. The argument is directly passed to pilot.add.
   @return table Table of newly created pilots.
--]]
function escort.spawn( pos )
   local have_outfits = (escort_outfits ~= nil)
   local pp = player.pilot()
   pos = pos or mem._escort.origin
   if heartbeat then
      hook.rm( heartbeat )
      heartbeat = nil
   end

   -- Set up the new convoy for the new system
   exited = {}
   mem._escort.convoy = {}
   if not have_outfits then
      escort_outfits = {}
   end
   local l
   if not mem._escort.nofollowplayer then
      l = pp
   end
   for k,s in ipairs( mem._escort.ships ) do
      local params = tcopy( mem._escort.params.pilot_params or {} )
      local donaked = params.naked -- Probably the script is using fcreate to do something
      if have_outfits then
         params.naked = true
      end
      local p = pilot.add( s, mem._escort.faction, pos, nil, params )
      if not l then
         l = p
      else
         p:setLeader( l )
      end
      mem._escort.convoy[k] = p
      if not donaked then
         if have_outfits then
            p:outfitsEquip( escort_outfits[k] )
         else
            escort_outfits[k] = p:outfits()
         end
      end

      -- Ensure they have at least one jump of fuel
      p:setFuel( math.max( p:fuel(), p:stats().fuel_consumption ) )
   end

   -- See if we have a post-processing function
   local fcreate
   if mem._escort.params.func_pilot_create then
      fcreate = _G[mem._escort.params.func_pilot_create]
   end

   -- Some post-processing for the convoy
   for k,p in ipairs(mem._escort.convoy) do
      p:setInvincPlayer(true)
      p:setFriendly(true)

      hook.pilot( p, "death", "_escort_e_death" )
      hook.pilot( p, "attacked", "_escort_e_attacked" )
      hook.pilot( p, "land", "_escort_e_land" )
      hook.pilot( p, "jump", "_escort_e_jump" )

      if fcreate then
         fcreate( p )
         aisetup.setup( p )
      end
   end

   if mem._escort.destsys then
      local scur = system.cur()
      -- Reached destination system, we are done here
      if not mem._escort.destspob and mem._escort.destsys==scur then
         run_success()
         return mem._escort.convoy
      end

      -- Have the leader move as slow as the slowest ship
      l:setHilight(true)
      -- Moving to system
      escort.reset_ai()

      -- Mark destination
      if mem._escort.nextsys then
         system.markerAdd( jump.get( scur, mem._escort.nextsys ):pos() )
      else
         if mem._escort.destspob then
            system.markerAdd( mem._escort.destspob:pos() )
         end
      end

      -- Have to run logic
      if not mem._escort.nofollowplayer then
         heartbeat = hook.timer( HEARTBEAT_TIMER, "_escort_heartbeat" )
      end
   end

   return mem._escort.convoy
end

-- Logic to make the pilots automatically jump or land when near the target
function _escort_heartbeat ()
   if not mem._escort then return end
   local destspob = mem._escort.destspob
   local nextsys = mem._escort.nextsys
   local doland = (system.cur() == mem._escort.destsys) and (destspob ~= nil)
   local nextjump
   if nextsys then
      nextjump = jump.get( system.cur(), nextsys )
   end
   for k,p in ipairs(mem._escort.convoy) do
      if p:exists() then
         if doland then
            if destspob:pos():dist2( p:pos() ) <= DISTANCE_THRESHOLD2 then
               local pm = p:memory()
               if (not pm._escort_land) or (not p:task()) then
                  p:control(true)
                  p:land( destspob )
                  pm._escort_land = true
               end
            end
         elseif nextjump then
            if nextjump:pos():dist2( p:pos() ) <= DISTANCE_THRESHOLD2 then
               local pm = p:memory()
               if (not pm._escort_jump) or (not p:task()) then
                  p:control(true)
                  p:setNoJump(false)
                  p:hyperspace( nextjump )
                  pm._escort_jump = true
               end
            end
         end
      end
   end
   heartbeat = hook.timer( HEARTBEAT_TIMER, "_escort_heartbeat" )
end

function _escort_spawn ()
   escort.spawn()
end

function _escort_takeoff ()
   -- We want to defer it one frame in case an enter hook clears all pilots
   hook.safe( "_escort_spawn" )
end

function _escort_jumpin ()
   if mem._escort.nextsys and system.cur() ~= mem._escort.nextsys then
      lmisn.fail( _("You jumped into the wrong system.") )
      return
   end
   if mem._escort.destsys then
      -- Progress
      mem._escort.current = mem._escort.current+1
      mem._escort.nextsys = mem._escort.route[ mem._escort.current+1 ]
   end
   -- We want to defer it one frame in case an enter hook clears all pilots
   hook.safe( "_escort_spawn" )
end

local function update_left ()
   local ships_outfits = {}
   local ships_alive = {}
   if mem._escort.destsys then
      for j,v in ipairs(mem._escort.convoy) do
         for i,p in ipairs(exited) do
            if v==p then
               table.insert( ships_alive, mem._escort.ships[j] )
               table.insert( ships_outfits, escort_outfits[j] )
            end
         end
      end
   else
      for j,p in ipairs(mem._escort.convoy) do
         if p:exists() then
            table.insert( ships_alive, mem._escort.ships[j] )
            table.insert( ships_outfits, escort_outfits[j] )
         end
      end
   end
   mem._escort.ships = ships_alive
   escort_outfits = ships_outfits
end

function _escort_jumpout()
   -- We'll be nice and mark escorts that are currently jumping as jumped out too
   for j,p in ipairs(mem._escort.convoy) do
      if p:exists() and p:flags("jumpingout") then
         table.insert( exited, p )
      end
   end

   -- Update and report
   update_left ()
   if not mem._escort.nofailifdead and #exited <= 0 then
      lmisn.fail( _("You jumped before the convoy you were escorting.") )
   end
   mem._escort.origin = system.cur()
end

-- luacheck: globals _escort_failure
function _escort_failure ()
   local n = #mem._escort.ships_orig
   if escort.num_alive() <= 0 then
      vntk.msg( n_("You landed before your escort!","You landed before your escorts!", n),
            n_([[You landed at the planet before ensuring that your escort was safe. You have abandoned your duties, and failed your mission.]],
               [[You landed at the planet before ensuring that your escorts were safe. You have abandoned your duties, and failed your mission.]], n))
   else
      vntk.msg(_("You abandoned your mission!"),
         n_("You have landed, abandoning your mission to protect your escort.",
            "You have landed, abandoning your mission to protect your escorts.", n))
   end
   misn.finish(false)
end

function _escort_land()
   update_left ()
   if not mem._escort.nofailifdead and (spob.cur() ~= mem._escort.destspob or escort.num_alive() <= 0) then
      _G[mem._escort.func_failure]()
   elseif spob.cur()==mem._escort.destspob then
      run_success()
   end
end

--[[--
Gets all the escort pilots from all the available escort missions.
--]]
function escort.all_mission_pilots()
   local plts = {}
   for k,m in ipairs(player.missions()) do
      local e = m.memory._escort
      if e then
         for i,p in ipairs(e.convoy) do
            if p:exists() then
               table.insert( plts, p )
            end
         end
      end
   end
   return plts
end

function escort.all_mission_target()
   for k,m in ipairs(player.missions()) do
      local e = m.memory._escort
      if e and e.destsys then
         if e.nextsys then
            return e.nextsys
         elseif system.cur()==e.destsys and e.destspob then
            return e.destspob
         else
            return e.destsys
         end
      end
   end
end

return escort
