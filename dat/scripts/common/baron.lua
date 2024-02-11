--[[

   Baron Common Functions

--]]
local vn = require "vn"
local utf8 = require "utf8"

local baron = {}

-- Function for adding log entries for miscellaneous one-off missions.
function baron.addLog( text )
   shiplog.create( "baron", _("Baron"), _("Baron") )
   shiplog.append( "baron", text )
end

function baron.vn_baron( params )
   return vn.Character.new( _("Baron Sauterfeldt"),
         tmerge( {
            portrait="neutral/unique/baron_sauterfeldt.webp",
            image="neutral/unique/baron_sauterfeldt.webp",
         }, params) )
end

baron.rewards = {
   baron = 300e3,
   prince = 500e3, -- The price of each artefact will always be 15% of this, so at most the player will be paid 85% and at least 55%.
}

-- Function that tries to misspell whatever string is passed to it.
function baron.mangle( intext )
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
      for c=1,1 do--math.ceil((len-1)/6) do
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

return baron
