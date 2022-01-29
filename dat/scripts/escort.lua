--[[
   Library for dealing with escorts in missions
--]]
-- luacheck: globals _escort_jumpin _escort_jumpout _escort_land _escort_failure _escort_e_death _escort_e_land _escort_e_jump _escort_e_attacked(Hook functions passed by name)
local escort = {}

local lmisn = require "lmisn"
local fmt = require "format"
local fleet = require "fleet"
local vntk = require "vntk"

function escort.init( ships, params )
   params = params or {}
   mem._escort = {
      params = params,
      ships_orig = ships,
      ships = tcopy(ships),
      faction = params.faction or "independent",
      hooks = {
         jumpin = hook.jumpin( "_escort_jumpin" ),
         jumpout = hook.jumpout( "_escort_jumpout" ),
         land = hook.land( "_escort_land" ),
      },
   }
end

local function clear_hooks ()
   if mem._escort.hooks.jumpin then
      hook.rm( mem._escort.hooks.jumpin )
   end
   if mem._escort.hooks.jumpout then
      hook.rm( mem._escort.hooks.jumpout )
   end
   if mem._escort.hooks.land then
      hook.rm( mem._escort.hooks.land )
   end
end

function escort.exit ()
   clear_hooks()
   mem._escort = nil
end

function escort.num_alive ()
   return #mem._escort.ships
end

local _escort_convoy
function escort.pilots ()
   return _escort_convoy
end

function escort.setDest( dest, success, failure )
   if dest.system then
      mem._escort.destspob = nil
      mem._escort.destsys = dest
   else
      mem._escort.destspob = dest
      mem._escort.destsys = dest:system()
   end
   mem._escort.func_failure = failure
   mem._escort.func_success = success
end

local exited
local function run_success ()
   clear_hooks()
   _G[mem._escort.func_success]()
end

local function control_ai( l )
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

function _escort_e_death( p )
   for k,v in ipairs(_escort_convoy) do
      if v==p then
         player.msg( "#r"..fmt.f(_("{plt} has been lost!"), {plt=p} ).."#0" )

         table.remove(_escort_convoy, k)
         table.remove(mem._escort.ships, k)

         if escort.num_alive() <= 0 then
            lmisn.fail( n_("The escort has been lost!","All escorts have been lost!",#mem._escort.ships_orig) )
         end

         -- Set a new leader and tell them to move on
         if k==1 then
            local l = _escort_convoy[1]
            for i,e in ipairs(_escort_convoy) do
               if i~=1 then
                  e:setLeader( l )
               end
            end
            control_ai( l )
         end
         return
      end
   end
end

function _escort_e_attacked( _p )
end

function _escort_e_land( p, landed_spob )
   if landed_spob == mem._escort.destspob then
      table.insert( exited, p )
      if p:exists() then
         player.msg( "#g"..fmt.f(_("{plt} has landed on {pnt}."), {plt=p, pnt=landed_spob} ).."#0" )
      end
   else
      _escort_e_death( p )
   end
end

function _escort_e_jump( p, j )
   if j:dest() == lmisn.getNextSystem( system.cur(), mem.destsys ) then
      table.insert( exited, p )
      if p:exists() then
         player.msg( "#g"..fmt.f(_("{plt} has jumped to {sys}."), {plt=p, sys=j:dest()} ).."#0" )
      end
   else
      _escort_e_death( p )
   end
end

function _escort_jumpin()
   if system.cur() ~= mem._escort.nextsys then
      lmisn.fail( _("You jumped into the wrong system.") )
      return
   end

   -- Set up the new convoy for the new system
   exited = {}
   _escort_convoy = fleet.add( 1, mem._escort.ships, mem._escort.faction )

   -- See if we have a post-processing function
   local fcreate
   if mem._escort.params.func_ship_create then
      fcreate = _G[mem._escort.params.func_ship_create]
   end

   -- Some post-processing for the convoy
   local minspeed = math.huge
   for k,p in ipairs(_escort_convoy) do
      p:setHilight(true)
      p:setInvincPlayer(true)

      hook.pilot( p, "death", "_escort_e_death" )
      hook.pilot( p, "attacked", "_escort_e_attacked" )
      hook.pilot( p, "land", "_escort_e_land" )
      hook.pilot( p, "jump", "_escort_e_jump" )

      if fcreate then
         fcreate( p )
      end

      minspeed = math.min( p:stats().speed_max, minspeed )
   end

   -- Have the leader move as slow as the slowest ship
   local l = _escort_convoy[1]
   l:setSpeedLimit( minspeed )
   -- Moving to system
   control_ai( l )
end

local function update_left ()
   local ships_alive = {}
   for i,p in ipairs(exited) do
      for j,v in ipairs(_escort_convoy) do
         if v==p then
            table.insert( ships_alive, mem._escort.ships[j] )
         end
      end
   end
   mem._escort.ships = ships_alive
end

function _escort_jumpout()
   update_left ()
   if #exited <= 0 then
      lmisn.fail( _("You jumped before the convoy you were escorting.") )
   else
      -- Treat those that didn't exit as dead
      mem._escort.alive = math.min( mem._escort.alive, mem._escort.exited )
   end
   mem._escort.origin = system.cur()
   mem._escort.nextsys = lmisn.getNextSystem(system.cur(), mem._escort.destsys)
end

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
   if spob.cur() ~= mem._escort.dest or escort.num_alive() <= 0 then
      _G[mem._escort.func_failure]()
   else
      run_success()
   end
end

return escort
