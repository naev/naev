--[[
-- Fancy GUI interface for the Crimson Gauntlet
--]]

local love = require 'love'
local luatk = require 'luatk'
local lg = require 'love.graphics'

local function button_list( wdw, captions, bx, by, bw, bh, w, h, handler )
   local btns = {}

   local fitrow = math.min( #captions, math.floor( w/(bw+10) ) )
   local offx   = (w - bw*fitrow) / (fitrow+1)

   local x = bx - bw
   local y = by
   for k,v in ipairs(captions) do
      x = x + bw + offx
      if x >= bx+w then
         x = bx + offx
         y = y + bh + 10
      end
      btns[#btns+1] = luatk.newButton( wdw, x, y, bw, bh, v, handler )
   end

   return btns, y-by+bh
end


local function gauntlet_setmodifier( wgt )
   if wgt:getCol() then
      wgt:setCol(nil)
      wgt:setFCol(nil)
   else
      wgt:setCol{ 0.8, 0.05, 0.20 }
      wgt:setFCol{ 0.9, 0.9, 0.9 }
   end
end


local function gauntlet_setoption( wgt )
   local newoption = wgt.text
   if gauntlet_option == newoption then
      return
   end
   gauntlet_option = newoption
   btn_enter:enable()

   for k,v in ipairs(btn_options) do
      v:setCol(nil)
      v:setFCol(nil)
   end
   wgt:setCol{ 0.8, 0.05, 0.20 }
   wgt:setFCol{ 0.9, 0.9, 0.9 }

   for k,v in ipairs(btn_modifiers) do
      v:destroy()
   end
   if modifiers_divider then
      modifiers_divider:destroy()
   end

   local wdw = wgt.parent
   local w, h = wdw.w, wdw.h
   modifiers_divider = luatk.newRect( wdw, 20, 189, w-40, 2, {0, 0, 0} )

   btn_modifiers, bh = button_list( wdw, {
         "Double Damage Taken (#g+50%#0)",
         "No Healing Between Waves (#g+50%)",
         "Double Enemies",
         }, 0, 205, 240, 60, w, 100, gauntlet_setmodifier )
end


local function gauntlet_settype( wgt )
   local newtype = wgt.text
   if gauntlet_type == newtype then
      return
   end
   gauntlet_type = newtype

   for k,v in ipairs(btn_types) do
      v:setCol(nil)
      v:setFCol(nil)
   end
   wgt:setCol{ 0.8, 0.05, 0.20 }
   wgt:setFCol{ 0.9, 0.9, 0.9 }

   for k,v in ipairs(btn_options) do
      v:destroy()
   end
   for k,v in ipairs(btn_modifiers) do
      v:destroy()
   end
   if options_divider then
      options_divider:destroy()
   end

   local wdw = wgt.parent
   local w, h = wdw.w, wdw.h
   if newtype == "Challenge" then
      btn_options, bh = button_list( wdw,
            {"Light", "Medium", "Heavy"},
            0, 135, 160, 40, w, 100, gauntlet_setoption )
      if not var.peek("gauntlet_unlock_medium") then
         btn_options[2]:disable()
      end
      if not var.peek("gauntlet_unlock_heavy") then
         btn_options[3]:disable()
      end
   end
   options_divider = luatk.newRect( wdw, 20, 189, w-40, 2, {0, 0, 0} )
end

function gauntlet_enter ()
   gauntlet_start = true
   love.event.quit()
end

function gauntlet_cancel ()
   gauntlet_start = false
   love.event.quit()
end

function love.load ()
   love.window.setFullscreen(true)
   love.graphics.setBackgroundColor( 0, 0, 0, 0 )

   -- Outlined fonts
   local largefont = lg.newFont(24)
   largefont:setOutline(3)
   local deffont = lg.newFont(16)
   deffont:setOutline(2)
   luatk.setDefaultFont( deffont )

   -- Clear defaults
   --gauntlet_type = nil
   --gauntlet_option = nil

   -- Window and top details
   local w, h = 800, 355
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newText( wdw, 0, 10, w, 24, "CRIMSON GAUNTLET", {0.9, 0.1, 0.25}, "center", largefont )
   luatk.newRect( wdw, 20, 49, w-40, 2, {0, 0, 0} )

   -- Tournament Types
   btn_types = button_list( wdw,
         {"Tournament", "Challenge", "Infinity Arena", "Special"},
         0, 65, 160, 40, w, h, gauntlet_settype )
   if not var.peek( "gauntlet_unlock_tournament" ) then
      btn_types[1]:disable()
   end
   if not var.peek( "gauntlet_unlock_infinity" ) then
      btn_types[3]:disable()
   end
   if not var.peek( "gauntlet_unlock_special" ) then
      btn_types[4]:disable()
   end
   luatk.newRect( wdw, 20, 119, w-40, 2, {0, 0, 0} )

   -- Tournament Options
   btn_options = {}

   -- Modifier
   btn_modifiers = {}

   -- Close or Cancel
   luatk.newRect(   wdw, 20,       h-40-35, w-40, 2, {0, 0, 0} )
   btn_enter = luatk.newButton( wdw, 250,      h-40-20, 160, 40, "ENTER ARENA", gauntlet_enter )
   btn_enter:disable()
   luatk.newButton( wdw, w-250-80, h-40-20, 120, 40, "Cancel", gauntlet_cancel )

   -- Load defaults
   if gauntlet_type then
      for k,wgt in ipairs(btn_types) do
         if wgt.text == gauntlet_type then
            gauntlet_type = nil
            gauntlet_settype( wgt )
         end
      end
   end
   if gauntlet_option then
      for k,wgt in ipairs(btn_options) do
         if wgt.text == gauntlet_option then
            gauntlet_option = nil
            gauntlet_setoption( wgt )
         end
      end
   end
end

function love.update( dt )
   luatk.update(dt)
end

function love.keypressed( key )
   if luatk.keypressed( key ) then return true end
   if key == "escape" then
      gauntlet_cancel()
   end
end

function love.draw()
   luatk.draw()
end

function love.mousepressed( x, y, button )
   if luatk.mousepressed( x, y, button ) then return true end
end

function love.mousemoved( x, y )
   if luatk.mousemoved( x, y ) then return true end
end

local gui = {}

function gui.run()
   love.run()
   if gauntlet_start then
      return gauntlet_type, gauntlet_option, {}
   end
end

return gui

