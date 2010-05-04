--[[
    Here, the AIs for the Therdin races are defined. +some other stuff
]]--

function beaconSanity ( beacon_number, beacon_list ) --this function makes sure that no nonexistent beacons are referenced. if the beacon number is bigger than the total number of beacons,
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
   local x, y = beacon_list[ beaconSanity( beacons_done +1, beacon_list ) ]:pos():get()
   local angle = math.deg( math.atan( y/x ) )
   pilot:setPos( vec2.new( ( 2 * number - 1 ) * math.cos( math.rad( angle ) ) + math.cos( math.rad( angle + 90 ) ) * 50 * math.sin( ( 2 * number - 1 ) ), ( 2 * number - 1 ) * math.sin( math.rad( angle ) ) + math.sin( math.rad( angle + 90 ) ) * 50 * math.sin( ( 2 * number - 1 ) ) ) )
   pilot:setDir( angle )
   pilot:disable()
   return setmetatable( { pilot=pilot, ai=ai, beacons_done=beacons_done }, racer_mt )
end

function racer:nextBeacon( beacon_list )
   if self.ai == "player" then
      beacon_list[ beaconSanity( self.beacons_done, beacon_list ) ]:setHostile()
      beacon_list[ beaconSanity( self.beacons_done +1, beacon_list ) ]:setFriendly()
   end
end