local utility = {}

--[[
-- @brief Encodes a number as a roman numeral.
--
--    @param k Number to encode.
--    @return String representing the number as a roman numeral.
--]]
function utility.roman_encode( k )
   local romans = {
      {1000, "M"},
      {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
      {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"},
      {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"} }

   local s = ""
   for _, v in ipairs(romans) do --note that this is -not- ipairs.
      local val, let = table.unpack(v)
      while k >= val do
         k = k - val
         s = s .. let
      end
   end
   return s
end

return utility
