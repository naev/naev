--[[
    Here, the AIs for the Therdin races are defined. +some other stuff
]]--

lang = naev.lang()
if lang == "es" then --you know the drill
else
   message = {}
   message[1] = "Checkpoint!"
   message[2] = "%s round(s) left."
end


function beaconSanity ( beacon_number, beacon_list ) --this function makes sure that no nonexistent beacons are referenced. if the beacon number is bigger than the total number of beacons,
   local beacon_number = beacon_number
   while beacon_number > #beacon_list do              --the function assumes it's in the next round
      beacon_number = beacon_number - #beacon_list
   end
   return beacon_number
end

function makeValidAngle ( angle ) --possibly obsolete
   while angle < 0 do
      angle = angle + 360
   end
   while angle >= 360 do
      angle = angle - 360
   end
   return angle
end


racer = {}
racer_mt = { __index = racer }


--[[
   @brief Creates a racer.

      @param pilotname Name of the pilot fleet to use (should only have one pilot).
      @param aitype AI to use for the racer.
      @param number Number of the racer in the race.
      @param beacon_list List of beacons to visit.
      @return The newly created racer.
--]]
function racer:new ( pilotname, aitype, number, beacon_list, racer_name )
   local ai    = aitype
   local beacons_done = 1

   -- Chose ai
   local name
   if ai == "player" then
      name = player.name()
      beacon_list[ beaconSanity( beacons_done +1, beacon_list ) ]:setFriendly()
   elseif ai == "perfect" then
      name = "Pete Perfect"
   elseif ai == "basic" then
      name = "Basic Butcher Ben"
   elseif ai == "fighter" then
      name = "Fighting Forklift"
   else
      error( string.format( "Invalid ai '%s' for racer", aitype ) )
   end
   if racer_name ~= nil then
      name = racer_name
   end

   -- Calculate direction to face
   local bnext = beacon_list[ beaconSanity( beacons_done + 1, beacon_list ) ]:pos()
   local bcur = beacon_list[ beacons_done ]:pos()
   local x1, y1 = bcur:get()
   local x2, y2 = bnext:get()
   local a = x2 - x1
   local b = y2 - y1
   local alpha = math.atan2( b, a )
   local sp_num = number * 2 - 1
   local c = 50 * math.sin( 0.5 * math.pi * sp_num )
   local d = 50 * number
   local e = math.sqrt( c^2 + d^2 )
   local beta = math.atan2( c, d )
   local gamma = math.pi + alpha - beta
   local f = math.sin( gamma ) * e
   local g = math.cos( gamma ) * e
   
   -- Calculate vector
   local position = vec2.add( bcur, g, f ) 

   -- Create/get pilot
   local p
   if pilotname == nil then
      p = player.pilot()
   else
      p = pilot.add( pilotname, "dummy", v )[1]
      p:rename( name )
   end
   p:setPos( position )
   p:setDir( math.deg( alpha ) )
   p:disable()

   -- Set metatable
   return setmetatable( { pilot=p, ai=ai, beacons_done=beacons_done, name=name, done=0 }, racer_mt )
end

function racer:beaconDone( beacon_list, rounds )
   --assign the next beacon
   self.beacons_done = self.beacons_done + 1
   
   --reaction to the fact that the beacon is done
   if self.ai == "player" then
      player.msg( message[1] )
      beacon_list[ beaconSanity( self.beacons_done, beacon_list ) ]:setHostile() --the friendly/hostile thing makes the next beacon visible for the player
      beacon_list[ beaconSanity( self.beacons_done +1, beacon_list ) ]:setFriendly()
   elseif self.ai == "basic" then
      self.pilot:taskClear()
      self.pilot:goto( beacon_list[ beaconSanity( self.beacons_done + 1, beacon_list ) ]:pos(), false )
   elseif self.ai == "fighter" then
      self.pilot:broadcast( "Checkpoint!", true )
   end
   
   if self.beacons_done % #beacon_list == 1 then --when round is completed
         
      if self.beacons_done == rounds * #beacon_list + 1 then --when race is completed
         self:raceDone()      
      else
         self:roundDone( beacon_list, rounds )
      end
         
   end
end

function racer:checkProx ( beacon_list, rounds )
   self.distance = vec2.dist( self.pilot:pos(), beacon_list[ beaconSanity( self.beacons_done + 1, beacon_list ) ]:pos() )
   
   -- fighter ai starts moving to the next beacon before actually reaching the the current one
   if self.ai == "fighter" and self.distance <= 200 then
      local this_beacon = beacon_list[beaconSanity(self.beacons_done+1, beacon_list)]:pos()
      local next_beacon = beacon_list[beaconSanity(self.beacons_done+2, beacon_list)]:pos()
      self.pilot:taskClear()
      self.pilot:goto( this_beacon + (next_beacon - this_beacon)*200/vec2.mod(next_beacon - this_beacon), false) --goes to a point 200 units from the next beacon (in the direction of the last one
      self.pilot:goto( beacon_list[ beaconSanity( self.beacons_done + 2, beacon_list ) ]:pos(), false )
   end
   if self.distance <= 100 then
      self:beaconDone( beacon_list, rounds )
   end
   
end

function racer:roundDone ( beacon_list )
   if self.ai == "player" then
      player.msg( string.format( message[2], (rounds * #beacon_list - ( self.beacons_done - 1 ) ) / #beacon_list ) ) -- "x rounds left"
   end
end

function racer:raceDone ()
   self.done = 1
   if self.ai == "basic" or self.ai == "fighter" then
      self.pilot:brake()
   end
end

function racer:startRace ( beacon_list )
   self.pilot:setHealth( 100, 100 )
   if self.ai == "basic" then
      self.pilot:control(true)
      self.pilot:goto( beacon_list[ beaconSanity( self.beacons_done + 1, beacon_list ) ]:pos(), false )
   elseif self.ai == "fighter" then
      self.pilot:control(true)
      local this_beacon = beacon_list[self.beacons_done]:pos()
      local next_beacon = beacon_list[beaconSanity(self.beacons_done+1, beacon_list)]:pos()
      self.pilot:goto( this_beacon + (next_beacon - this_beacon)*200/vec2.mod(next_beacon - this_beacon), false) --goes to a point 200 units from the next beacon (in the direction of the last one
   end
end

function racer:rm()
   if self.pilot ~= pilot.player() then
      self.pilot:rm()
   end
   return nil
end
