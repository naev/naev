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

function racer:new ( pilot, aitype, number, beacon_list )
   local pilot = pilot
   local ai = aitype
   local beacons_done = 1
   if ai == "player" then
      local name = player.name()
      beacon_list[ beaconSanity( beacons_done +1, beacon_list ) ]:setFriendly()
   elseif ai == "perfect" then
      local name = "Pete Perfect"
   elseif ai == "basic" then
      local name = "Basic Butcher Ben"
   end
   local x, y = beacon_list[ beaconSanity( beacons_done +1, beacon_list ) ]:pos():sub( beacon_list[ beaconSanity( beacons_done, beacon_list ) ]:pos() ):get()
   local angle = math.atan( y/x )
   local sinevalue = 50 * math.sin( 0.5 * math.pi * ( number * 2 - 1 ) )
   pilot:setPos( beacon_list[ beacons_done ]:pos():add( vec2.new( math.cos( angle ) * ( -50 ) * number + math.sin( angle ) * sinevalue, math.sin( angle ) * ( -50 ) * number + math.cos( angle ) * sinevalue ) ) )
   pilot:setDir( math.deg( angle ) )
   pilot:disable()
   return setmetatable( { pilot=pilot, ai=ai, beacons_done=beacons_done, name=name, done=0 }, racer_mt )
end

function racer:beaconDone( beacon_list, rounds )
   self.beacons_done = self.beacons_done + 1
   
   if self.ai == "player" then
      player.msg( message[1] )
      beacon_list[ beaconSanity( self.beacons_done, beacon_list ) ]:setHostile()
      beacon_list[ beaconSanity( self.beacons_done +1, beacon_list ) ]:setFriendly()
   elseif self.ai == "basic" then
      self.pilot:goto( beacon_list[ beaconSanity( self.beacons_done + 1, beacon_list ) ]:pos(), false )
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
   if vec2.dist( self.pilot:pos(), beacon_list[ beaconSanity( self.beacons_done + 1, beacon_list ) ]:pos() ) <= 100 then
      self:beaconDone( beacon_list, rounds )
   end
end

function racer:roundDone ( beacon_list )
   if self.ai == "player" then
      player.msg( string.format( message[2], (rounds * #beacon_list - ( self.beacons_done - 1 ) ) / #beacon_list ) )
   end
end

function racer:raceDone ()
   self.done = 1
   if self.ai == "basic" then
      self.pilot:brake()
   end
end

function racer:startRace ( beacon_list )
   self.pilot:setHealth( 100, 100 )
   if self.ai == "basic" then
      self.pilot:control(true)
      self.pilot:goto( beacon_list[ beaconSanity( self.beacons_done + 1, beacon_list ) ]:pos(), false )
   end
end

function racer:rm()
   self.pilot:rm()
end