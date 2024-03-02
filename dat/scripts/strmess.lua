--[[
   @brief Library to mess up strings.
--]]
local utf8 = require "utf8"

local strmess = {}

local function _split( str, sep, n )
   local t = {}
   --local i = {}
   local o = {}
   local s, e
   local position = 1
   n = n or #sep
   s, e = utf8.find(str, sep, position)
   while s do
      --i[#i+1] = #t+1
      local si = utf8.sub(str, position, s-1)
      if #si > 0 then
         o[#t+1] = true
         t[#t+1] = si
      end
      t[#t+1] = utf8.sub(str, s, s+n-1 )
      position = e + 1
      s, e = utf8.find(str, sep, position)
   end
   s = utf8.sub(str, position)
   if #s>0 then
      --i[#i+1] = #t+1
      o[#t+1] = true
      t[#t+1] = s
   end
   return t, o
end

local function _wordsplit( str )
   local t = {}
   local x = {}
   local ls, lo = _split(str,"#%w",2)
   for i,is in ipairs(ls) do
      if lo[i] then
         local ss, so = _split(is," ")
         for j,js in ipairs(ss) do
            if so[j] then
               x[#x+1] = #t+1
            end
            t[#t+1] = js
         end
      else
         t[#t+1] = is
      end
   end
   return t, x
end

local function _uniquechar( str )
   local uniquesymbols = {}
   for pos, code in utf8.next, str do
      uniquesymbols[ utf8.char(code) ] = 1
   end
   local symbols = {}
   for k,v in pairs(uniquesymbols) do
      symbols[ #symbols+1 ] = k
   end
   return symbols
end

--[[-
   @brief Mangles a string
      @tparam string str String to mangle.
      @tparam number strength A 0 to 1 value which represents the chance of mangling a character.
--]]
function strmess.messup( str, strength )
   strength = strength or 0.1
   -- Find all unique symbols
   local symbols = _uniquechar( str )

   -- Split into words and try to mess them up
   local t,idx = _wordsplit(str)
   for k,v in ipairs(idx) do
      local s = t[v]
      local l = utf8.len(s)
      if l > 1 then
         for i=1,l do
            if rnd.rnd() < strength then
               local r = rnd.rnd()
               if l>2 and r < 1/3 then
                  -- random removal
                  local p = rnd.permutation(l-1)
                  local pre = utf8.sub( s, 1, p[1] )
                  local pos = utf8.sub( s, p[1]+2, l )
                  s = pre..pos
               elseif r < 2/3 then
                  -- random swap
                  local p = rnd.permutation(l-1)
                  local pre = utf8.sub( s, 1, p[1] )
                  local pos = utf8.sub( s, p[1]+2, l )
                  local sym
                  if rnd.rnd() < 0.5 then
                     sym = symbols[ rnd.permutation(#symbols)[1] ]
                  else
                     local ssym = _uniquechar(s)
                     sym = ssym[ rnd.permutation(#ssym)[1] ]
                  end
                  s = pre..sym..pos
               else
                  -- random add
                  local p = rnd.permutation(l)
                  local pre = utf8.sub( s, 1, p[1] )
                  local pos = utf8.sub( s, p[1]+1, l )
                  local sym
                  if rnd.rnd() < 0.5 then
                     sym = symbols[ rnd.permutation(#symbols)[1] ]
                  else
                     local ssym = _uniquechar(s)
                     sym = ssym[ rnd.permutation(#ssym)[1] ]
                  end
                  s = pre..sym..pos
               end
            end
         end
      end
      t[v] = s
   end
   return table.concat(t,"")
end

--[[-
   @brief Mangles a word a certain amount of times
      @tparam string intext word or string to mess with
      @tparam number amount Number of manglements to apply
--]]
function strmess.mangle( intext, amount )
   local outtext = intext

   local vowels = {"a", "e", "i", "o", "u", "y"}
   local consonants = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "z"}

   local i = 1
   local found = false

   -- Try to find a triplet of
   while i < #intext-1 do
      if inlist(consonants, utf8.lower(utf8.sub(intext,i,i))) and
            inlist(vowels, utf8.lower(utf8.sub(intext,i+1,i+1))) and
            inlist(consonants, utf8.lower(utf8.sub(intext,i+2,i+2))) then
         found = true
         break
      end
      i = i+1
   end

   -- Mess up the first pair
   if found then
      local first = consonants[rnd.rnd(1, #consonants)]
      local second = vowels[rnd.rnd(1, #vowels)]
      -- Preserve case
      if utf8.upper(utf8.sub(intext,i,i))==utf8.sub(intext,i,i) then
         first = utf8.upper(first)
      end
      if utf8.upper(utf8.sub(intext,i+1,i+1))==utf8.sub(intext,i,i) then
         second = utf8.upper(second)
      end
      outtext = utf8.sub(intext,-#intext,-(#intext-i+2)) .. first .. second .. utf8.sub(intext,i+2)
   end

   -- Our ASCII based mangling failed, so we need to do an alternative
   local len = utf8.len(intext)
   if outtext==intext and len>1 then
      local ct = {}
      local lower = {}
      for j=1,len do
         ct[j] = j
         local c = utf8.sub(intext,j,j)
         lower[j] = utf8.lower(c)==c
      end
      for c=1,amount do
         local p = rnd.permutation(len)
         local t = ct[ p[1] ]
         ct[ p[1] ] = ct[ p[2] ]
         ct[ p[2] ] = t
      end
      outtext = ""
      for j=1,len do
         local c = utf8.sub(intext,ct[j],ct[j])
         if lower[j] then
            c = utf8.lower(c)
         else
            c = utf8.upper(c)
         end
         outtext = outtext..c
      end
   end

   return outtext
end

return strmess
