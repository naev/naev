--[[
-- @brief Adds an outfit from a list of outfits to the pilot.
--
-- Outfits parameter should be something like:
--
-- outfits = {
--    "Laser Cannon",
--    { "Seeker Launcher", { "Seeker Missile", 10 } }
-- }
-- 
--    @param p Pilot to add outfits to.
--    @param o Table to add what was added.
--    @param outfits Table of outfits to add.  Look at description for special formatting.
--]]
function pilot_outfitAdd( p, o, outfits )
   r = rnd.rnd(1, #outfits)
   if type(outfits[r]) == "string" then
      q = p:addOutfit( outfits[r], 1 )
      _insertOutfit( o, outfits[r], q )
   elseif type(outfits[r]) == "table" then
      q = p:addOutfit( outfits[r][2][1], outfits[r][2][2] )
      _insertOutfit( o, outfits[r][2][1], q )
      q = p:addOutfit( outfits[r][1], 1 ) -- Add launcher later.
      _insertOutfit( o, outfits[r][1], q )
   end
end
function _insertOutfit( o, name, q )
   -- Must have quantity
   if q <= 0 then
      return
   end
   -- Try to find in table first
   for k,v in ipairs(o) do
      if v[1] == name then
         v[2] = v[2] + q
         return
      end
   end
   -- Insert
   table.insert( o, { name, q } )
end
