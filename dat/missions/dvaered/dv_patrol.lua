--[[

   Handles random Dvaered Patrol missions.

   Stage 1: GOTO
   Stage 2: HOLD
   Stage 3: CLEAR

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = {}
   misn_desc[1] = "Patrol %d points in %d system%s for hostiles: "
   misn_desc[2] = "Return to %s in the %s system for payment."
   misn_reward = "%s credits"
   title = {}
   title[1] = "DV: Routine %d sector patrol"
   title[2] = "DV: Patrol %d sectors"
   title[3] = "DV: Scan of %d sectors"
   accept_title = "Mission Accepted"
   msg_title = {}
   msg_msg = {}
   msg_title[1] = "Mission Success"
   msg_msg[1] = "You are greeted by a Dvaered official and receive your payment of %s credits for your contribution in keeping Dvaered systems clean."
   msg_msg[2] = "DV: Arrived at point. Hold point."
   msg_msg[3] = "DV: Point insecure. Engage hostiles!"
   msg_msg[4] = "DV: Hostiles eliminated. Hold point."
   msg_msg[5] = "DV: Patrol finished. Return to base."
   msg_msg[6] = "DV: Left point. Return to point."
   msg_msg[7] = "DV: Point secure. Proceed to next point."
   osd_msg = {}
   osd_msg[1] = "Go to %s"
   osd_msg[2] = "Hold %s"
   osd_msg[3] = "Clear %s of hostiles"
   osd_msg[4] = "Return to %s in the %s system"
   osd_msg[5] = "Next: %s"
   osd_msg[6] = "Jump to the %s system"
end


include("scripts/proximity.lua")
include("scripts/numstring.lua")

-- Mission parameters
chk_range      = math.pow( 2000, 2 ) -- Radius within target
chk_range_buf  = math.pow( 2500, 2 ) -- Range at which player has left (has buffer zone)
chk_time       = 10 -- Time in seconds to hold position

-- Saves function calls
f_dvaered = faction.get("Dvaered")

function filter( func, tbl )
   local ntbl = {}
   for i=1,#tbl do
      if func( tbl[i] ) then
         ntbl[ #ntbl+1 ] = tbl[i]
      end
   end
   return ntbl
end

-- Creates a target
function create_target( sys, target, type )
   return { sys = sys, data = target, type = type }
end

-- Checks to see if jump target is interesting
function check_jmp_target( jmp )
   return jmp:presences( f_dvaered ) ~= nil
end

-- Checks to see if planet target is interesting
function check_pnt_target( pnt )
   return pnt:faction() == f_dvaered or rnd.rnd() > 0.5
end

-- Compares two targets
function tgt_cmp( a, b )
   if a.type ~= b.type then
      return false
   end
   return a.data == b.data
end

-- Checks to see if a target is in a list
function tgt_inList( list, a )
   for _,v in ipairs(list) do
      if tgt_cmp( v, a ) then
         return true
      end
   end
   return false
end

-- Gets the available targets
function get_avail_targets( from, list )
   -- Get local point
   local sys   = from.sys

   -- Create possible targets
   local jumps   = filter( check_jmp_target, sys:adjacentSystems() )
   local planets = filter( check_pnt_target, sys:planets() )

   -- Create list
   local targets = {}
   for _,v in ipairs(jumps) do
      local t = create_target( sys, v, "jump" )
      if not tgt_inList( list, t ) then
         targets[ #targets+1 ] = t
      end
   end
   for _,v in ipairs(planets) do
      local t = create_target( sys, v, "planet" )
      if not tgt_inList( list, t ) then
         targets[ #targets+1 ] = t
      end
   end

   -- Special case it's a jump, we can look at the other side
   if from.type == "jump" then
      local t = create_target( from.data, sys, "jump" )
      if not tgt_inList( list, t ) then
         targets[ #targets+1 ] = t
      end
   end

   return targets
end

-- Prints a target when debugging
function tgt_print( tgt )
   print( string.format( "Target: type=%s, target=%s, sys=%s",
         tgt.type, tostring(tgt.data), tostring(tgt.sys) ) )
end

-- Gets a string for the target
function tgt_str( tgt, simple )
   local typename
   if tgt.type == "jump" then
      typename = "Jump Point to"
   else
      -- Numeric classes are for stations.
      if tonumber( tgt.data:class() ) then
         typename = "Station"
      else
         typename = "Planet"
      end
   end
   if simple then
      return string.format( "%s %s", typename, tgt.data:name() )
   else
      return string.format( "%s %s in the %s system", typename,
            tgt.data:name(), tgt.sys:name() )
   end
end

-- Creates a path, includes the starting point
function create_path( from, n )
   local list = { from }
   for i=1,n do
      local tgts = get_avail_targets( from, list )
      if #tgts == 0 then
         return list
      end
      list[ #list+1 ] = tgts[ rnd.rnd( 1, #tgts ) ]
      from = list[ #list ]
   end
   return list
end

function filter( func, tbl )
   local ntbl = {}
   for i=1,#tbl do
      if func( tbl[i] ) then
         ntbl[ #ntbl+1 ] = tbl[i]
      end
   end
   return ntbl
end

function tgt_get_sys( tgt_list )
   local tgt_sys = {}
   for _,v in ipairs(tgt_list) do
      local sys = v.sys
      local found = false
      for _,s in ipairs(tgt_sys) do
         if s == sys then
            found = true
            break
         end
      end
      if not found then
         tgt_sys[ #tgt_sys+1 ] = sys
      end
   end
   return tgt_sys
end

function tgt_getPos( tgt )
   if tgt.type == "jump" then
      return jump.pos(system.cur(), tgt.data)
   else
      return tgt.data:pos()
   end
end

-- Create the mission
function create ()
   -- Note: this mission makes no system claims.

   -- Get systems to patrol
   base, base_sys = planet.cur()
   local from = create_target( base_sys, base, "planet" )
   num_patrol = rnd.rnd( 3, 5 )
   tgt_list = create_path( from, num_patrol )
   tgt_list["__save"] = true -- Save the systems
   -- We must remove the initial element
   for i=1,#tgt_list-1 do
      tgt_list[i] = tgt_list[i+1]
   end
   tgt_list[ #tgt_list ] = nil
   -- Make sure it has al least some elements
   num_patrol = #tgt_list
   if num_patrol < 3 then
      misn.finish(false)
   end

   -- Get number of systems to visit
   tgt_sys = tgt_get_sys( tgt_list )
   mission_marker = misn.markerAdd( tgt_sys[1], "computer" )

   -- Create the description.
   if #tgt_sys > 1 then
      plural = "s"
   else
      plural = ""
   end
   desc = string.format( misn_desc[1], num_patrol, #tgt_sys, plural ) .. tgt_sys[1]:name()
   for i=2, #tgt_sys-1 do
      desc = desc .. ", " .. tgt_sys[i]:name()
   end
   if #tgt_sys > 1 then
      desc = desc .. " and " .. tgt_sys[#tgt_sys]:name()
   end
   for i=1,#tgt_list do
      desc = desc .. "\n   " .. tgt_str( tgt_list[i] )
   end

   -- Set mession stage
   misn_stage  = 1
   visited     = 0

   -- Calculate reward
   reward = 10000
   for i=1,num_patrol do -- Base price for spots to check
      reward = reward + 15000 + 5000 * rnd.twosigma()
   end
   for i=2,#tgt_sys do -- Bonus for jumps
      reward = reward + 20000 + 5000 * rnd.twosigma()
   end
   for i=1,#tgt_sys do -- Adjust for dvaered enemy presence
      local enemy = faction.enemies( "Dvaered" )
      local presence = 0
      for _,v in ipairs(enemy) do
         presence = presence + tgt_sys[i]:presence( v )
      end
      presence = presence / #tgt_sys
      reward = reward * math.min( 1.5, presence/ 100 )
   end

   -- Must have minimum reward
   if reward <= 10000 then
      misn.finish(false)
   end

   -- Set some details.
   misn.setTitle( string.format( title[rnd.int(1,3)], num_patrol ) )
   misn.setDesc( desc )
   misn.setReward( string.format( misn_reward, numstring(reward) ) )
end

-- Mission is accepted
function accept ()
   if misn.accept() then

      -- Set the OSD
      set_osd()

      -- Set the hooks
      hook.land( "land" )
      hook.enter( "enter" )
   end
end

-- Sets and updates the osd
function set_osd ()
   -- Set the OSD
   local osd_table = {}

   -- Only list targets when still have to travel
   if visited < num_patrol then

      -- Indicate player has to jump
      local tgt_next = tgt_list[ visited+1 ]
      if tgt_next.sys ~= system.cur() then
         osd_table[ #osd_table+1 ] = string.format( osd_msg[6], tgt_next.sys:name() )
         osd_table[ #osd_table+1 ] = "..."
      else

         -- List targets
         osd_table[ #osd_table+1 ] = string.format( osd_msg[misn_stage], tgt_str( tgt_next, true ) )
         if visited+2 <= num_patrol then
            osd_table[ #osd_table+1 ] = string.format( osd_msg[5], tgt_str( tgt_list[visited+2], true ) )
            if visited+3 <= num_patrol then
               osd_table[ #osd_table+1 ] = "..."
            end
         end
      end
   end

   -- List return
   osd_table[ #osd_table+1 ] = string.format( osd_msg[4], base:name(), base_sys:name() )

   -- Create the osd
   misn.osdCreate( "DV Patrol", osd_table )
end

-- Enter hook, just starts up our timer
function enter ()
   -- Reset goal
   misn_stage = 1
   set_osd()

   -- We start the update goal timer when we're in the proper system
   if visited+1 <= num_patrol and system.cur() == tgt_list[ visited+1 ].sys then
      updateGoal() -- Will set timer
      local vec = tgt_getPos( tgt_list[ visited+1 ] )
      misn_mrk = system.mrkAdd( "Patrol Point", vec )
   end
end

function updateGoal ()
   if system.cur() ~= tgt_list[ visited+1 ].sys then
      return
   end
   local vec = tgt_getPos( tgt_list[ visited+1 ] )
   local dist2 = vec:dist2( player.pilot():pos() )

   -- Going to position
   if misn_stage == 1 then
      misn_counter = 0

      -- Go to next stage when in range
      if dist2 < chk_range then
         misn_stage = 2
         player.msg( msg_msg[2] )
         misn_counter = 0
         set_osd()
      end

   -- Holding position
   elseif misn_stage == 2 then

      -- Make sure we're still in position
      if dist2 > chk_range_buf then
         misn_stage = 1
         player.msg( msg_msg[6] )
         set_osd()

      else
         -- Check for hostiles
         if checkHostiles( vec ) then
            misn_stage = 3
            player.msg( msg_msg[3] )
            set_osd()

         else
            -- Increment counter
            misn_counter = misn_counter + 1

            -- Check if we finished checking out the area
            if misn_counter > chk_time then
               misn_stage = 1
               visited = visited + 1
               set_osd()

               -- Remove in system marker
               if misn_mrk ~= nil then
                  system.mrkRm( misn_mrk )
               end

               -- Update marker
               if visited >= num_patrol then
                  misn.setDesc( string.format( misn_desc[2], base:name(), base_sys:name() ) )
                  misn.markerMove( mission_marker, base_sys )
                  player.msg( msg_msg[5] )
                  return -- No need to restart timer
               else

                  -- Set system marker and message
                  misn.markerMove( mission_marker, tgt_list[ visited+1 ].sys )
                  player.msg( msg_msg[7] )

                  -- If not the same system kill the timer
                  local tgt_next = tgt_list[ visited+1 ]
                  if system.cur() ~= tgt_next.sys then
                     return -- kill the timer
                  end

                  -- Set in system timer
                  local vec = tgt_getPos( tgt_next )
                  misn_mrk = system.mrkAdd( "Patrol Point", vec )
               end
            end
         end
      end
   -- Clear hostiles
   else
      misn_counter = 0

      -- Make sure we're still in position
      if dist2 > chk_range_buf then
         misn_stage = 1 -- Out of range
         player.msg( msg_msg[6] )
         set_osd()
      else

         -- Check for hostiles
         if not checkHostiles( vec ) then
            misn_stage = 2 -- Hostiles dead
            player.msg( msg_msg[4] )
            set_osd()
         end
      end
   end

   -- Set goal again
   hook.timer( 1000, "updateGoal" )
end

-- Checks for nearby hostiles
function checkHostiles( vec )
   local p = pilot.get( f_dvaered:enemies() ) -- Gets all enemies
   local ret = false
   for _,v in ipairs(p) do
      if vec:dist2( v:pos() ) < chk_range then
         v:setHilight(true)
         v:setHostile(true)
         ret = true
      end
      v:setHilight(false)
   end
   return ret
end

-- Land hook
function land ()
   landed = planet.cur()
   if visited == num_patrol and landed == base then
      player.pay( reward )
      tk.msg( msg_title[1], string.format( msg_msg[1], numstring(reward) ))

      -- increase dvaered patrol mission counter
      n = var.peek("dv_patrol")
      if n ~= nil then
         var.push("dv_patrol", n+1)
      else
         var.push("dv_patrol", 1)
      end      

      -- modify the faction standing
      faction.modPlayerSingle("Dvaered", rnd.rnd(1, num_patrol/2) )
      faction.modPlayerSingle("FLF", -3)

      misn.finish(true)
   end
end

