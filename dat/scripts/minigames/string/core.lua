local lg = require "love.graphics"
local le = require "love.event"
local mg = {}

local function inlist( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return true
      end
   end
   return false
end

local function shuffle(tbl)
   for i = #tbl, 2, -1 do
      local j = math.random(i)
      tbl[i], tbl[j] = tbl[j], tbl[i]
   end
   return tbl
end

local font, fontlh, keyset, sol, guess, tries, game

function mg.load ()
   font = lg.newFont( 32 )
   fontlh = 40

   keyset = {"Q","W","E","R","T","Y"}
   local rndset = {}
   for k,v in ipairs(keyset) do
      table.insert( rndset, v )
   end
   rndset = shuffle( rndset )
   --[[
   rndset = rnd.permutation( keyset )
   --]]
   sol = {}
   for i=1,3 do
      table.insert( sol, rndset[i] )
   end

   guess = {}
   tries = 6
   game  = 0
end

local matches_exact, matches_fuzzy
function mg.keypressed( key )
   if key == "escape" then
      le.quit()
   end

   if game ~= 0 then
      return
   end

   local k = string.upper(key)
   if inlist( keyset, k ) then
      if #guess >= #sol then
      guess = { k }
      elseif not inlist( guess, k ) then
         table.insert( guess, k )
         if #guess >= #sol then
            matches_exact = 0
            matches_fuzzy = 0
            for i,v in ipairs(guess) do
               if v==sol[i] then
                  matches_exact = matches_exact+1
               elseif inlist( sol, v ) then
                  matches_fuzzy = matches_fuzzy+1
               end
            end

            tries = tries - 1
            if matches_exact == 3 then
               game = 1
            elseif tries <= 0 then
               game = -1
            end
         end
      end
   end
end

function mg.draw ()
   local x, y = 40, 40
   local s

   s = "ATTEMPTS LEFT: "..tostring(tries)
   lg.print( s, font, x, y )

   y = y + fontlh
   s = "SYMBOL SET: "
   for k,v in ipairs( keyset ) do
      s = s .. v .. " "
   end
   lg.print( s, font, x, y )

   y = y + fontlh
   s = "GUESS: "
   for k,v in ipairs( guess ) do
      s = s .. v .. " "
   end
   lg.print( s, font, x, y )

   if #guess >= #sol then
      y = y + fontlh
      s = "RESULT: "
      for i=1,matches_fuzzy do
         s = s .. "? "
      end
      for i=1,matches_exact do
         s = s .. "! "
      end
      lg.print( s, font, x, y )
   end

   y = y + fontlh
   if game > 0 then
      lg.print( "YOU WIN!", font, x, y )
   elseif game < 0 then
      lg.print( "YOU LOSE!", font, x, y )
   end
end

return mg
