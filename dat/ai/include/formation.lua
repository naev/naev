local function cmd_pos(p, pos)
   ai.pilot():msg(p, "form-pos", pos)
end


local function formation(pilots, formation)
   --setup

   local angle, radius, flip

   local class_count = {}
   for i, p in ipairs(pilots) do
      if class_count[p:ship():class()] == nil then
         class_count[p:ship():class()] = 1
      else
         class_count[p:ship():class()] = class_count[p:ship():class()] + 1
      end
   end

  --cross formation
  if formation == "cross" then
      -- Cross logic. Forms an X.
      angle = 45 -- Spokes start rotated at a 45 degree angle.
      radius = 100 -- First ship distance.
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius}) -- Store as polar coordinates.
         angle = (angle + 90) % (360) -- Rotate spokes by 90 degrees.
         radius = 100 * (math.floor(i / 4) + 1) -- Increase the radius every 4 positions.
      end
      
      
   --buffer formation
   elseif formation == "buffer" then
      -- Buffer logic. Consecutive arcs eminating from the fleader. Stored as polar coordinates.
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
         cmd_pos(p, {angle = angle, radius = radius})
      end


   --vee formation
   elseif formation == "vee" then
      -- The vee formation forms a v, with the fleader at the apex, and the arms extending in front.
      angle = 45 -- Arms start at a 45 degree angle.
      radius = 100 -- First ship distance.
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius}) -- Store as polar coordinates.
         angle = angle * -1 -- Flip the arms between -45 and 45 degrees.
         radius = 100 * (math.floor(i / 2) + 1) -- Increase the radius every 2 positions.
      end
   
   --wedge formation
   elseif formation == "wedge" then
      -- The wedge formation forms a v, with the fleader at the apex, and the arms extending out back.
      flip = -1
      angle = (flip * 45) + 180
      radius = 100 -- First ship distance.
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius}) -- Store as polar coordinates.
         flip = flip * -1
         angle = (flip * 45) + 180 -- Flip the arms between 135 and 215 degrees.
         radius = 100 * (math.floor(i / 2) + 1) -- Increase the radius every 2 positions.
      end
      
   elseif formation == "echelon left" then
      --This formation forms a "/", with the fleader in the middle.
      radius = 100
      flip = -1
      angle = (45 * 3) + (90 * flip)  --Flip between 45 degrees and 225 degrees.
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius})
         flip = flip * -1
         angle = (45 * 3) + (90 * flip)
         radius = 100 * (math.ceil(i / 2)) -- Increase the radius every 2 positions
      end

   elseif formation == "echelon right" then
      --This formation forms a "\", with the fleader in the middle.
      radius = 100
      flip = 1
      angle = (45 * 5) + (90 * flip) --Flip between 315 degrees, and 135 degrees
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius})
         flip = flip * -1
         angle = (45 * 5) + (90 * flip)
         radius = 100 * (math.ceil(i / 2))
      end

   elseif formation == "column" then
      --This formation is a simple "|", with fleader in the middle.
      radius = 100
      flip = -1
      angle = 90 + (90 * flip)  --flip between 0 degrees and 180 degrees
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius})
         flip = flip * -1
         angle = 90 + (90 * flip)
         radius = 100 * (math.ceil(i/2)) --Increase the radius every 2 ships.
      end

   elseif formation == "wall" then
      --This formation is a "-", with the fleader in the middle.
      radius = 100
      flip = -1
      angle = 180 + (90 * flip) --flip between 90 degrees and 270 degrees
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius})
         flip = flip * -1
         angle = 180 + (90 * flip)
         radius = 100 * (math.ceil(i/2)) --Increase the radius every 2 ships.
      end

   elseif formation == "fishbone" then
      radius = 500
      flip = -1
      orig_radius = radius
      angle = (22.5 * flip) / (radius / orig_radius)
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius})
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


   elseif formation == "chevron" then
      radius = 500
      flip = -1
      orig_radius = radius
      angle = (22.5 * flip) / (radius / orig_radius)
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle, radius = radius})
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

   elseif formation == "circle" or formation == nil then
      -- Default to circle.
      angle = 360 / #pilots -- The angle between each ship, in radians.
      radius = 80 + #pilots * 25 -- Pulling these numbers out of my ass. The point being that more ships mean a bigger circle.
      for i, p in ipairs(pilots) do
         cmd_pos(p, {angle = angle * i, radius = radius}) -- Store as polar coordinates.
      end
   end
end

return formation
