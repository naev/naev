-- Some compatibility with Lua 5.2
table.pack = function ( ... ) -- luacheck: globals table (Pairity with newer versions)
   return {...}
end
math.mod = nil -- luacheck: globals math (Pairity with newer versions)
string.gfind = nil -- luacheck: globals string (Pairity with newer versions)

--[[--
Checks to see if an element is in a table. Uses comparison operator.

   @tparam table tbl Table to check to see if element is in it.
   @param elm Element to check to see if it is in tbl.
   @treturn boolean true if elm is in tbl, false otherwise.
--]]
function inlist( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return true
      end
   end
   return false
end

--[[--
Does a shallow copy of a table.

   @tparam table tbl Table to copy.
   @tparam[opt={}] copy Initial table to use as a base and add elements of tbl to.
   @treturn table A new table that is a shallow copy of tbl, or copy if not nil.
--]]
function tcopy( tbl, copy )
   copy = copy or {}
   for k,v in pairs(tbl) do
      copy[k] = v
   end
   return copy
end

--[[--
   Merges src table into dest. Does not recurse into subtables. This functino is for unordered tables.

   @tparam table dest Destination table. Must not be nil.
   @tparam[opt={}] table src Source table to be merged into dest.
   @treturn table Returns dest.
--]]
function tmerge( dest, src )
   src = src or {}
   for k,v in pairs(src) do
      dest[k] = v
   end
   return dest
end

--[[--
   Merges src table into dest. Does not recurse into subtables. This function is for ordered tables.

   @tparam table dest Destination table. Must not be nil.
   @tparam[opt={}] table src Source table to be merged into dest.
   @treturn table Returns dest.
--]]
function tmergei( dest, src )
   src = src or {}
   for k,v in ipairs(src) do
      table.insert( dest, v )
   end
   return dest
end

--[[--
   Merges src table into dest recursively.

   @tparam table dest Destination table. Must not be nil.
   @tparam[opt={}] table src Source table to be merged into dest.
   @treturn table Returns dest.
--]]
function tmerge_r( dest, src )
   src = src or {}
   for k,v in pairs(src) do
      if type(v)=="table" then
         dest[k] = tmerge_r( dest[k] or {}, v )
      else
         dest[k] = v
      end
   end
   return dest
end

--[[--
Repeats an ordered table a number of tables.

If you have a table {1,2,3}, and you repeat it 2 times, you will get the table {1,1,2,2,3,3}.

   @tparam table tbl Table to repeat.
   @tparam number num Number of times to repeat the table.
   @treturn table A new table that is tbl repeated num times.
--]]
function trepeat( tbl, num )
   local t = {}
   for k,v in ipairs(tbl) do
      for i=1,num do
         table.insert( t, v )
      end
   end
   return t
end

--[[--
Reverses a table. Only works when using contiguous numbers as keys.

   @tparam table tbl Table to reverse.
   @treturn table A new table with the elements of tbl in reverse order.
--]]
function treverse( tbl )
   local t = {}
   for i = #tbl, 1, -1 do
      table.insert( t, tbl[i] )
   end
   return t
end
