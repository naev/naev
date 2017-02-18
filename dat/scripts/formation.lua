local function count_classes(pilots)
   local class_count = {}
   for i, p in ipairs(pilots) do
      if class_count[p:ship():class()] == nil then
         class_count[p:ship():class()] = 1
      else
         class_count[p:ship():class()] = class_count[p:ship():class()] + 1
      end
   end
   return class_count
end

formations = {}

function formations.cross(leader)
   local pilots = leader:followers()
   -- Cross logic. Forms an X.
   local angle = 45 -- Spokes start rotated at a 45 degree angle.
   local radius = 100 -- First ship distance.
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      angle = (angle + 90) % (360) -- Rotate spokes by 90 degrees.
      radius = 100 * (math.floor(i / 4) + 1) -- Increase the radius every 4 positions.
   end
end

function formations.buffer(leader)
   -- Buffer logic. Consecutive arcs eminating from the fleader. Stored as polar coordinates.
   local pilots = leader:followers()
   local class_count = count_classes(pilots)
   local angle, radius
   local radii = {Scout = 1200, Fighter = 900, Bomber = 850, Corvette = 700, Destroyer = 500, Cruiser = 350, Carrier = 250} -- Different radii for each class.
   local count = {Scout = 1, Fighter = 1, Bomber = 1, Corvette = 1, Destroyer = 1, Cruiser = 1, Carrier = 1} -- Need to keep track of positions already iterated through.
   for i, p in ipairs(pilots) do
      ship_class = p:ship():class() -- For readability.
      if class_count[ship_class] == 1 then -- If there's only one ship in this specific class...
         angle = 0 --The angle needs to be zero.
      else -- If there's more than one ship in each class...
         angle = ((count[ship_class]-1)*(90/(class_count[ship_class]-1)))-45 -- ..the angle rotates from -45 degrees to 45 degrees, assigning coordinates at even intervals.
         count[ship_class] = count[ship_class] + 1 --Update the count
      end
      radius = radii[ship_class] --Assign the radius, defined above.
      leader:msg(p, "form-pos", {angle, radius})
   end
end

function formations.vee(leader)
   -- The vee formation forms a v, with the fleader at the apex, and the arms extending in front.
   local pilots = leader:followers()
   local angle = 45 -- Arms start at a 45 degree angle.
   local radius = 100 -- First ship distance.
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      angle = angle * -1 -- Flip the arms between -45 and 45 degrees.
      radius = 100 * (math.floor(i / 2) + 1) -- Increase the radius every 2 positions.
   end
end

function formations.wedge(leader)
   -- The wedge formation forms a v, with the fleader at the apex, and the arms extending out back.
   local pilots = leader:followers()
   local flip = -1
   local angle = (flip * 45) + 180
   local radius = 100 -- First ship distance.
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      flip = flip * -1
      angle = (flip * 45) + 180 -- Flip the arms between 135 and 215 degrees.
      radius = 100 * (math.floor(i / 2) + 1) -- Increase the radius every 2 positions.
   end
end
      
function formations.echelon_left(leader)
   --This formation forms a "/", with the fleader in the middle.
   local pilots = leader:followers()
   local radius = 100
   local flip = -1
   local angle = 135 + (90 * flip)  --Flip between 45 degrees and 225 degrees.
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      flip = flip * -1
      angle = 135 + (90 * flip)
      radius = 100 * (math.ceil(i / 2)) -- Increase the radius every 2 positions
   end
end

function formations.echelon_right(leader)
   --This formation forms a "\", with the fleader in the middle.
   local pilots = leader:followers()
   local radius = 100
   local flip = 1
   local angle = 225 + (90 * flip) --Flip between 315 degrees, and 135 degrees
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      flip = flip * -1
      angle = 225 + (90 * flip)
      radius = 100 * (math.ceil(i / 2))
   end
end

function formations.column(leader)
   --This formation is a simple "|", with fleader in the middle.
   local pilots = leader:followers()
   local radius = 100
   local flip = -1
   local angle = 90 + (90 * flip)  --flip between 0 degrees and 180 degrees
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      flip = flip * -1
      angle = 90 + (90 * flip)
      radius = 100 * (math.ceil(i/2)) --Increase the radius every 2 ships.
   end
end

function formations.wall(leader)
   --This formation is a "-", with the fleader in the middle.
   local pilots = leader:followers()
   local radius = 100
   local flip = -1
   local angle = 180 + (90 * flip) --flip between 90 degrees and 270 degrees
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      flip = flip * -1
      angle = 180 + (90 * flip)
      radius = 100 * (math.ceil(i/2)) --Increase the radius every 2 ships.
   end
end

function formations.fishbone(leader)
   local pilots = leader:followers()
   local radius = 500
   local flip = -1
   local orig_radius = radius
   local angle = (22.5 * flip) / (radius / orig_radius)
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      if flip == 0 then
         flip = -1
         radius = (orig_radius * (math.ceil(i/3))) + ((orig_radius * (math.ceil(i/3))) / 30)
      elseif flip == -1 then
         flip = 1
      elseif flip == 1 then
         flip = 0
         radius = orig_radius * (math.ceil(i/3))
      end
      angle = (22.5 * flip) / (radius / orig_radius)
   end
end

function formations.chevron(leader)
   local pilots = leader:followers()
   local radius = 500
   local flip = -1
   local orig_radius = radius
   local angle = (22.5 * flip) / (radius / orig_radius)
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle, radius})
      if flip == 0 then
         flip = -1
         radius = (orig_radius * (math.ceil(i/3))) - ((orig_radius * (math.ceil(i/3))) / 20)
      elseif flip == -1 then
         flip = 1
      elseif flip == 1 then
         flip = 0
         radius = orig_radius * (math.ceil(i/3))
      end
      angle = (22.5 * flip) / (radius / orig_radius)
   end
end

function formations.circle(leader)
   -- Default to circle.
   local pilots = leader:followers()
   local angle = 360 / #pilots -- The angle between each ship, in radians.
   local radius = 80 + #pilots * 25 -- Pulling these numbers out of my ass. The point being that more ships mean a bigger circle.
   for i, p in ipairs(pilots) do
      leader:msg(p, "form-pos", {angle * i, radius, "absolute"})
   end
end

local keys = {}
for k, _ in pairs(formations) do
   keys[#keys+1] = k
end

formations.keys = keys

-- Clear formation; not really a 'formation' so it is not in keys
function formations.clear(leader)
   leader:msg(leader:followers(), "form-pos", nil)
end

function formations.random_key()
   return keys[rnd.rnd(1, #keys)]
end


return formations
