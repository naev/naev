--[[
Control fleets, assign formations, have ships stick together.
Created by Loki and BTAxis

Usage:
First create a fleet with addShips() (from fleethelper).
Then pass that to the Forma:new() function.
Then use Forma:setTask() to control the fleader
-usage is: 
      Forma:new(fleet,"formation",combat_dist, preferred_fleet_leader)
--combat_dist is the distance from the fleet leader to an enemy before the formation breaks and ships attack.
--preferred_fleet_leader is the ship you want to be leader. if that ship dies, the scripts hands leadership to another ship.
-if formation or distance isn't provided, formation will be "circle" and distance will be 3000
Control the fleet's movements by controlling the fleet leader, or "fleader".

example:
my_fleet = addShips( 4, "Hyena", "Pirate", _("Pirate Hyena") )
my_fleet = Forma:new(my_fleet,"echelon left",1500)
my_fleet:setTask("moveto",vec2.new(0,0))

Current formations are:
buffer            echelon left
cross             echelon right
wedge             wall
vee               column
circle            fishbone
chevron

]]--

-- First define the base object. There are no default values, I only wrote in the variables for reader reference.
Forma = {
         fleet = nil,
         formation = nil,
         fleader = nil,
         fleetspeed = nil,
         thook = nil,
         incombat = nil,
         class_count = nil,
         combat_dist = nil,
         lead_ship = nil,
         d1 = {},
         d2 = {},
         d3 = {},
         task = nil,
}

-- The functions below that start with Forma: are all elements of the Forma table above.
-- Every such function is automatically passed the Forma object they are called on.
-- This object is referenced as "self".

-- Create a new object based on the Forma "class".
-- Usage: forma = Forma:new(table_of_pilots, "formation_name")
function Forma:new(fleet, formation, combat_dist, lead_ship)
   -- We want to make sure a fleet was specified. The formation can be nil.
   if not fleet then
      error( _("Forma must have a fleet.") )
      return
   end

   if combat_dist == nil then
      combat_dist = 3000
   end
   
   -- Fleets need at least two ships.
   if #fleet <= 1 then
      error( _("Fleets need at least 2 ships") )
      return
   end

   -- Create the object.
   local forma = {fleet = fleet, formation = formation, combat_dist = combat_dist, lead_ship = lead_ship}
   setmetatable(forma, self) -- Metatable fanciness.
   self.__index = self -- This means the object will look for its functions in "Forma" if it doesn't have them itself.

   -- Set up stuff and start control loop.
   for _, p in ipairs(forma.fleet) do
      -- We pass the Forma object itself to the hooks.
      self.d1[p] = hook.pilot(p, "death", "dead", forma)
      self.d2[p] = hook.pilot(p, "jump", "jumper", forma)
      self.d3[p] = hook.pilot(p, "land", "lander", forma)
   end
   
   hook.pilot(player.pilot(),"jump","jumper",forma)
   hook.pilot(player.pilot(),"land","lander",forma)

   forma:reorganize()
   forma:control() -- This is sadly the only time we can do this.
   return forma, self.fleader --return the fleader so control of the fleet can be handled in a script.
end

-- Cleans up a forma object.
function Forma:destroy()
   if self.thook then
      hook.rm(self.thook)
   end

   -- Remove any pilots that might still be around.
   for _, p in ipairs(self.fleet) do
      p:rm()
   end

   -- Finally, remove the object itself.
   self = nil
end

function Forma:disband()
   if self.thook then
      hook.rm(self.thook)
   end

   --Remove the land/jump/death hooks over the pilots
   for i, p in ipairs(self.fleet) do
      hook.rm(self.d1[p])
      hook.rm(self.d2[p])
      hook.rm(self.d3[p])
   end
   
   self = nil
end

-- Reorganizes a formation.
-- Finds the slowest ship in a fleet and assigns it the leader role.
function Forma:reorganize()
   local minspeed = nil -- Holds the LOWEST speed we know.
   local pspeed = nil -- Holds the speed for the current pilot.
   local fleader = nil -- Holds the pilot with the lowest speed.

   for _, p in ipairs(self.fleet) do -- We will assume there are no gaps in the table, so we don't need to check if any pilots exist. See dead() for why.
      pspeed = p:stats().speed_max
      if not minspeed or minspeed > pspeed then
         minspeed = pspeed
         fleader = self.lead_ship or p
      end
   end

   if self.formation == "buffer" then
      self:shipCount()
   end
 

   for _,p in ipairs(self.fleet) do
      if p ~= fleader then
         p:setFaction(fleader:faction())
      end
   end

   self.fleader = fleader
   self.fleetspeed = minspeed
   self.incombat = false --Combat flag, used in the controlling function.
end

-- Death function. Removes a pilot from a fleet and reorganizes the fleet if necessary.
function Forma:dead(victim)
   for i, p in ipairs(self.fleet) do
      if p == victim then
         table.remove(self.fleet, i) -- This will automatically reorganize the table, so there won't be any entries for pilots that have been removed.
         break -- Don't keep iterating after a table.remove, as it can cause trouble.
      end
   end

   --can't have a fleet with one ship.
   if #self.fleet == 1 then
      self:disband()
      return
   end
   
   -- Now, if our fleet has been completely wiped out, we need to clean up.
   if #self.fleet == 0 then
      self:destroy()
      return
   end

   -- if the scripter set a lead ship which dies, set the lead_ship to nil.
   if victim == self.lead_ship then
      self.lead_ship = nil
   end

   -- We need to update the fleet's organization if the leader got killed.
   if victim == self.fleader then
      self:reorganize()
   end

   -- Update the ship count if the formation is "buffer".
   if self.formation == "buffer" then
      self:shipCount()
   end
end

function Forma:shipCount()
   self.class_count = {}
   for i, p in ipairs(self.fleet) do
      if self.class_count[p:ship():class()] == nil then
         self.class_count[p:ship():class()] = 1
      else
         self.class_count[p:ship():class()] = self.class_count[p:ship():class()] + 1
      end
   end
end
   

-- Jump hook. Ensures the entire fleet jumps if the fleader jumps.
-- Takes an ID that tells this function which formation the pilot belonged to.
function Forma:jumper(jumper, jumpoint)
   if jumper == self.fleader then
      self:dead(jumper) -- Jumping out is the same as dying, for our purpose; we need to not run this before the if statement.
      self.fleader:setSpeedLimit(0)
      for _, p in ipairs(self.fleet) do
         --Make the whole fleet use the jump.
         p:control() -- control pilots or clear their orders.
         p:hyperspace(jumpoint:dest())
      end

      -- Stop the control loop, or it will override our hyperspace() order.
      if self.thook then
         hook.rm(self.thook)
      end
   elseif jumper == player.pilot() then
      self:destroy()
   else
      self:dead(jumper) --we need to not run this before the if statement.
   end
end

-- Land hook. Ensures the entire fleet lands if the fleader lands.
-- Takes an ID that tells this function which formation the pilot belonged to.
function Forma:lander(lander, planet)
   if lander == self.fleader then
      self:dead(lander) -- Landing is the same as dying, for our purpose.
      self.fleader:setSpeedLimit(0)
      for _, p in ipairs(self.fleet) do
         --Have the fleet land on the asset.
         p:control() -- control pilots or clear their orders.
         p:land(planet)
      end

      -- Stop the control loop, or it will override our land() order.
      if self.thook then
         hook.rm(self.thook)
      end
   elseif lander == player.pilot() then
      self:destroy()
   else
      self:dead(lander)
   end
end

-- Hooks. The hook functions are just wrappers for the Forma functions.
-- We need these because hooks can't reference table elements as their callbacks.
function dead(victim, killer, forma)
   forma:dead(victim)
end

function jumper(jumper, jumpoint, forma)
   forma:jumper(jumper, jumpoint)
end

function lander(lander, planet, forma)
   forma:lander(lander, planet)
end

--Used in formation creation, this creates vec2s for each ship to follow.
-- Defined on a formation table.
-- This is where formations will ultimately be defined.
function Forma:assignCoords()
   --setup

   local posit = {} --position array. I'm using this twice, once for polar coordinates and once for absolute vec2s.
   local numships = #self.fleet -- Readability.
   local offset, x, y, angle, radius, flip

   if numships <= 1 then
      return {} -- A fleet of 1 ship or less has no business flying in formation.
   end

  --cross formation
  if self.formation == "cross" then
      -- Cross logic. Forms an X.
      angle = math.pi / 4 -- Spokes start rotated at a 45 degree angle.
      radius = 100 -- First ship distance.
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius} -- Store as polar coordinates.
         angle = (angle + math.pi / 2) % (math.pi * 2) -- Rotate spokes by 90 degrees.
         radius = 100 * (math.floor(i / 4) + 1) -- Increase the radius every 4 positions.
      end
      
      
   --buffer formation
   elseif self.formation == "buffer" then
      -- Buffer logic. Consecutive arcs emanating from the fleader. Stored as polar coordinates.
      local radii = {Scout = 1200, Fighter = 900, Bomber = 850, Corvette = 700, Destroyer = 500, Cruiser = 350, Carrier = 250} -- Different radii for each class.
      local count = {Scout = 1, Fighter = 1, Bomber = 1, Corvette = 1, Destroyer = 1, Cruiser = 1, Carrier = 1} -- Need to keep track of positions already iterated through.
      for i, p in ipairs(self.fleet) do
         ship_class = p:ship():class() -- For readability.
         if self.class_count[ship_class] == 1 then -- If there's only one ship in this specific class...
            angle = 0 --The angle needs to be zero.
         else -- If there's more than one ship in each class...
            angle = ((count[ship_class]-1)*((math.pi/2)/(self.class_count[ship_class]-1)))-(math.pi/4) -- ..the angle rotates from -45 degrees to 45 degrees, assigning coordinates at even intervals.
            count[ship_class] = count[ship_class] + 1 --Update the count
         end
         radius = radii[ship_class] --Assign the radius, defined above.
         posit[i] = {angle = angle, radius = radius}
      end


   --vee formation
   elseif self.formation == "vee" then
      -- The vee formation forms a v, with the fleader at the apex, and the arms extending in front.
      angle = math.pi / 4 -- Arms start at a 45 degree angle.
      radius = 100 -- First ship distance.
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius} -- Store as polar coordinates.
         angle = angle * -1 -- Flip the arms between -45 and 45 degrees.
         radius = 100 * (math.floor(i / 2) + 1) -- Increase the radius every 2 positions.
      end
   
   --wedge formation
   elseif self.formation == "wedge" then
      -- The wedge formation forms a v, with the fleader at the apex, and the arms extending out back.
      flip = -1
      angle = (flip * (math.pi / 4)) + math.pi
      radius = 100 -- First ship distance.
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius} -- Store as polar coordinates.
         flip = flip * -1
         angle = (flip * (math.pi / 4)) + math.pi-- Flip the arms between 135 and 215 degrees.
         radius = 100 * (math.floor(i / 2) + 1) -- Increase the radius every 2 positions.
      end
      
   elseif self.formation == "echelon left" then
      --This formation forms a "/", with the fleader in the middle.
      radius = 100
      flip = -1
      angle = ((math.pi / 4) * 3) + ((math.pi / 2) * flip)  --Flip between 45 degrees and 225 degrees.
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius}
         flip = flip * -1
         angle = ((math.pi / 4) * 3) + ((math.pi / 2) * flip)
         radius = 100 * (math.ceil(i / 2)) -- Increase the radius every 2 positions
      end

   elseif self.formation == "echelon right" then
      --This formation forms a "\", with the fleader in the middle.
      radius = 100
      flip = 1
      angle = ((math.pi / 4) * 5) + ((math.pi / 2) * flip) --Flip between 315 degrees, and 135 degrees
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius}
         flip = flip * -1
         angle = ((math.pi / 4) * 5) + ((math.pi / 2) * flip)
         radius = 100 * (math.ceil(i / 2))
      end

   elseif self.formation == "column" then
      --This formation is a simple "|", with fleader in the middle.
      radius = 100
      flip = -1
      angle = (math.pi / 2) + ((math.pi / 2) * flip)  --flip between 0 degrees and 180 degrees
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius}
         flip = flip * -1
         angle = (math.pi / 2) + ((math.pi / 2) * flip)
         radius = 100 * (math.ceil(i/2)) --Increase the radius every 2 ships.
      end

   elseif self.formation == "wall" then
      --This formation is a "-", with the fleader in the middle.
      radius = 100
      flip = -1
      angle = math.pi + ((math.pi / 2) * flip) --flip between 90 degrees and 270 degrees
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius}
         flip = flip * -1
         angle = math.pi + ((math.pi / 2) * flip)
         radius = 100 * (math.ceil(i/2)) --Increase the radius every 2 ships.
      end

   elseif self.formation == "fishbone" then
      radius = 500
      flip = -1
      orig_radius = radius
      angle = ((math.pi / 8) * flip) / (radius / orig_radius)
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius}
         if flip == 0 then
            flip = -1
            radius = (orig_radius * (math.ceil(i/3))) + ((orig_radius * (math.ceil(i/3))) / 30)
         elseif flip == -1 then
            flip = 1
         elseif flip == 1 then
            flip = 0
            radius = orig_radius * (math.ceil(i/3))
         end
         angle = ((math.pi / 8) * flip) / (radius / orig_radius)
      end


   elseif self.formation == "chevron" then
      radius = 500
      flip = -1
      orig_radius = radius
      angle = ((math.pi / 8) * flip) / (radius / orig_radius)
      for i = 1, numships do
         posit[i] = {angle = angle, radius = radius}
         if flip == 0 then
            flip = -1
            radius = (orig_radius * (math.ceil(i/3))) - ((orig_radius * (math.ceil(i/3))) / 20)
         elseif flip == -1 then
            flip = 1
         elseif flip == 1 then
            flip = 0
            radius = orig_radius * (math.ceil(i/3))
         end
         angle = ((math.pi / 8) * flip) / (radius / orig_radius)
      end

   elseif self.formation == "circle" or self.formation == nil then
      -- Default to circle.
      angle = math.pi * 2 / numships -- The angle between each ship, in radians.
      radius = 80 + numships * 25 -- Pulling these numbers out of my ass. The point being that more ships mean a bigger circle.
      for i = 1, numships do
         posit[i] = {angle = angle * i, radius = radius} -- Store as polar coordinates.
      end
   end

   for i, position in ipairs(posit) do
      -- We want a fleet formed in positions relative to the direction the
      -- captain is facing, not static offsets.
      offset = self.fleader:dir() / 180 * math.pi -- convert to radians
      x = position.radius * math.cos(position.angle + offset) --x coordinate assignment.
      y = position.radius * math.sin(position.angle + offset) --y coordinate assignment.
      posit[i] = self.fleader:pos() + vec2.new(x, y) -- You can add and subtract vec2s as you would expect to, no need to bend over backwards.
   end
   return posit
end

-- Formation control function. A wrapper again.
function toRepeat (forma)
   forma:control()
end

function Forma:control()
   local inrange = false --Using false instead of nil for readability.

   -- A little unconventional, re-set the timer hook at the start of the function. This is because execution might not reach the end.
   self.thook = hook.timer(100, "toRepeat", self) -- Call the wrapper, not this function.

   --combat. mmmm.
   local enemies = pilot.get(self.fleader:faction():enemies())

   if self.fleader:hostile() then
      enemies[#enemies+1] = player.pilot() -- Includes the player as an enemy to be iterated over.
   end

   for _, enemy in ipairs(enemies) do --Iterate over enemies to see if at least one is in range.
      if enemy:pos():dist(self.fleader:pos()) < self.combat_dist then
         inrange = true --The inrange variable was already defaulted to false.
         break --If an enemy is in range, no need to continue looping.
      end
   end
   
   --We want to separate out the for loop that finds enemies and the if functions that control combat, so that all of the enemy pilots are iterated over to see if one is in range before controlling the pilots.
   --It's also easier on my brain. </lazy>
   
   if inrange then
      if not self.incombat then --If baddies are in range and the fleet isn't set to do combat yet, then...
         self.fleader:setSpeedLimit(0)
         for _, p in ipairs(self.fleet) do
            if p ~= player.pilot() then
               p:control(false) -- ...cut 'em loose.
            end
         end
         self.incombat = true
         return -- No need to iterate further. No need to do the rest of this function either!
      else
         return --If baddies are in range, and the fleet was already in combat, no need to de-control them again. Prevents spam broadcasting.
      end
   else --If no badguys are in range...
      if self.incombat then --...and the fleet is no longer in combat, then...
         self.incombat = false --...flip the combat flag, and continue the function.
         self:manageTask()     --give the fleader his orders back
      end
   end
      
   
   -- At this point we know we're not in combat (well, the leader isn't).
   -- Assign the coordinates.
   local posit = self:assignCoords()

   -- Remember, there is no need to check if a pilot exists, because we've already made sure all pilots exist.
   for i, p in ipairs(self.fleet) do
      if not (p == self.fleader) then

         p:control() -- Clear orders.

         local cons = (posit[i]-p:pos())*10 + (self.fleader:vel()-p:vel())*20  --Computing the direction using a pd controller
         local goal = cons + p:pos()

         if cons:mod() >= 300 then
            p:moveto(goal, false, false)
            else
            p:face(goal)
         end

      else
         if p ~= player.pilot() then
            p:setSpeedLimit(self.fleetspeed) -- Make mon capitan travel at 5 below the slow speed, so other ships can catch up.
         end
         -- Logic for fleet leader goes here. For now, let's allow the fleader to act according to the regular AI.
      end
   end
end

--Task management
function Forma:manageTask()
   if self.task and self.task[1] then 
      self.fleader:control()
      if self.task[1] == "moveto" then
         self.fleader:moveto(self.task[2])
      elseif self.task[1] == "land" then
         self.fleader:land(self.task[2])
      elseif self.task[1] == "hyperspace" then
         self.fleader:hyperspace(self.task[2])
      elseif self.task[1] == "follow" then
         if self.task[2]:exists() then
            self.fleader:follow(self.task[2])
         end
      elseif self.task[1] == "attack" then
         if self.task[2]:exists() then
            self.fleader:attack(self.task[2])
         end
      elseif self.task[1] == "runaway" then
         if self.task[2]:exists() then
            self.fleader:runaway(self.task[2])
         end
      elseif self.task[1] == "brake" then
         self.fleader:brake()
      else
         error( _("task unknown") )
      end
   end
end

function Forma:setTask(title, arg)
   self.task = {title, arg}

   if not self.incombat then
      self:manageTask()
   end
end
