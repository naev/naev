local lg = require "love.graphics"
local le = require "love.event"
local mg = {}

local function _( x )
   return x
end
local format = {}
function format.f( str, tab )
   return (str:gsub("%b{}", function(block)
      local key, fmt = block:match("{(.*):(.*)}")
      key = key or block:match("{(.*)}")
      key = tonumber(key) or key  -- Support {1} for printing the first member of the table, etc.
      local val = tab[key]
      if val==nil then
         warn(string.format(_("fmt.f: string '%s' has '%s'==nil!"), str, key))
      end
      return fmt and string.format('%'..fmt, val) or tostring(val)
   end))
end
local fmt = format

local function inlist( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return true
      end
   end
   return false
end
local function getpos( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return k
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

local font, keyset, sol, guess, tries, game, selected

function mg.load ()
   font = lg.newFont( 16 )

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
   selected = 1
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
      selected = 1
      guess = { k }
      elseif not inlist( guess, k ) then
         guess[selected] = k
         for i=1,#sol do
            if not guess[i] then
               selected = i
               break
            end
         end
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

   if key == "left" then
      selected = math.max( selected-1, 1 )
   elseif key == "right" then
      selected = math.min( selected+1, #sol )
   elseif key == "down" then
      local p = getpos( keyset, guess[selected] ) or 0
      for i=p+1, #keyset do
         if not inlist( guess, keyset[i] ) then
            guess[selected] = keyset[i]
            break
         end
      end
   elseif key == "up" then
      local p = getpos( keyset, guess[selected] ) or #sol+1
      for i=p-1,1,-1 do
         if not inlist( guess, keyset[i] ) then
            guess[selected] = keyset[i]
            break
         end
      end
   end
end

local function drawglyph( g, f, x, y, w, h, col )
   col = col or {0, 0.2, 0.8, 0.6}
   lg.setColor( col )
   lg.rectangle( "fill", x, y, w, h )
   lg.setColor{ 1, 1, 1 }
   local fh = f:getHeight()
   lg.printf( g, f, x, y+(h-fh)*0.5, w, "center" )
end

function mg.draw ()
   local bx, by = 0, 0
   local x, y

   -- Draw glyph bar
   x = 20
   y = 30
   lg.setColor{ 1, 1, 1, 1 }
   lg.rectangle( "line", bx+x, by+y, 30, 30*#keyset )
   for k,v in ipairs(keyset) do
      local col
      if inlist( guess, v ) then
         col = { 0, 0.8, 0.8, 0.6 }
      else
         col = nil
      end
      drawglyph( v, font, bx+x+5, by+y+k*30-25, 20, 20, col )
   end

   x = 60
   lg.print( fmt.f(_("Input the code ({tries} tries left):"),{tries=tries}), font, bx+x, by+y )

   x = 70
   y = 70
   lg.setColor{ 1, 1, 1, 1 }
   lg.rectangle( "line", bx+x-5, by+y-5, 50*#sol+10, 50+10 )
   for i=1,#sol do
      local v = guess[i] or ""
      local col
      if i==selected then
         col = { 0, 0.8, 0.8, 0.6 }
      else
         col = nil
      end
      drawglyph( v, font, bx+x+i*50-45, by+y+5, 40, 40, col )
   end


--[[

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
--]]
end

function mg.update( _dt )
   return false -- true to finish
end

return mg
