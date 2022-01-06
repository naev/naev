--[[
-- Fancy GUI interface for the Crimson Gauntlet
--]]

local luatk = require 'luatk'
local lg = require 'love.graphics'

local gauntlet_option, gauntlet_start, gauntlet_type
local btn_enter, btn_modifiers, btn_options, btn_types, headerh, modifiers_divider, options_divider

local gauntlet_modifiers = {
   { id = "doubledmgtaken", str = _("Double Damage Enemies (#g+50%#0)"), var = "gauntlet_unlock_doubledmgtaken", enabled = false },
   { id = "nohealing", str = _("No Healing Between Waves (#g+25%#0)"), var = "gauntlet_unlock_nohealing", enabled = false },
   { id = "doubleenemy", str = _("Double Enemies"), var = nil, enabled = false },
}

local function button_list( wdw, captions, bx, by, bw, bh, w, _h, handler )
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
   local state
   if wgt:getCol() then
      wgt:setCol(nil)
      wgt:setFCol(nil)
      state = false
   else
      wgt:setCol{ 0.8, 0.05, 0.20 }
      wgt:setFCol{ 0.9, 0.9, 0.9 }
      state = true
   end
   for k,v in ipairs(gauntlet_modifiers) do
      if v.str == wgt.text then
         v.enabled = state
      end
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
   local w = wdw.w
   modifiers_divider = luatk.newRect( wdw, 20, headerh+144, w-40, 2, {0, 0, 0} )

   local strlist = {}
   for k,v in ipairs(gauntlet_modifiers) do
      table.insert( strlist, v.str )
   end
   btn_modifiers = button_list( wdw, strlist,
         0, headerh+160, 240, 60, w, 100, gauntlet_setmodifier )
   for k,v in ipairs(gauntlet_modifiers) do
      if v.var and not var.peek(v.var) then
         btn_modifiers[k]:disable()
      end
   end
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
   local w = wdw.w
   if newtype == "Challenge" then
      btn_options = button_list( wdw,
            {_("Skirmisher"), _("Warrior"), _("Warlord")},
            0, headerh+90, 160, 40, w, 100, gauntlet_setoption )
      if not var.peek("gauntlet_unlock_warrior") then
         btn_options[2]:disable()
      end
      if not var.peek("gauntlet_unlock_warlord") then
         btn_options[3]:disable()
      end
   end
   options_divider = luatk.newRect( wdw, 20, headerh+144, w-40, 2, {0, 0, 0} )
end

local function gauntlet_enter ()
   gauntlet_start = true
   luatk.close()
end

local function gauntlet_cancel ()
   gauntlet_start = false
   luatk.close()
end

local function gauntlet_gui ()
   -- Outlined fonts
   local largefont = lg.newFont(24)
   largefont:setOutline(3)
   local deffont = lg.newFont(16)
   deffont:setOutline(2)
   luatk.setDefaultFont( deffont )

   -- Clear defaults
   local cache = naev.cache()
   gauntlet_type = cache.gauntlet_type
   gauntlet_option = cache.gauntlet_option
   cache.gauntlet_modifiers = cache.gauntlet_modifiers or {}
   for k,v in ipairs(cache.gauntlet_modifiers) do
      gauntlet_modifiers[k].enabled = v
   end

   -- Window and top details
   local w, h = 800, 400
   local wdw = luatk.newWindow( nil, nil, w, h )
   wdw:setCancel( luatk.close )
   local icon = lg.newImage("gfx/misc/crimson_gauntlet.webp")
   local iw, ih = icon:getDimensions()
   iw = iw * 80 / ih
   ih = 80
   local y = 5
   luatk.newImage( wdw, (w-iw)/2, y, iw, ih, icon )
   luatk.newText( wdw, 0, (ih-24)/2+5, (w-iw)/2-10, 24, _("CRIMSON"), {0.9, 0.1, 0.25}, "right", largefont )
   luatk.newText( wdw, (w-iw)/2+iw+10, (ih-24)/2+5, w-(w-iw)/2-iw-10, 24, _("GAUNTLET"), {0.9, 0.1, 0.25}, "left", largefont )
   headerh = ih+10
   luatk.newRect( wdw, 20, headerh+4, w-40, 2, {0, 0, 0} )

   -- Tournament Types
   btn_types = button_list( wdw,
         {_("Tournament"), _("Challenge"), _("Infinity Arena"), _("Special")},
         0, headerh+20, 160, 40, w, h, gauntlet_settype )
   if not var.peek( "gauntlet_unlock_tournament" ) then
      btn_types[1]:disable()
   end
   if not var.peek( "gauntlet_unlock_infinity" ) then
      btn_types[3]:disable()
   end
   if not var.peek( "gauntlet_unlock_special" ) then
      btn_types[4]:disable()
   end
   luatk.newRect( wdw, 20, headerh+74, w-40, 2, {0, 0, 0} )

   -- Tournament Options
   btn_options = {}

   -- Modifier
   btn_modifiers = {}

   -- Close or Cancel
   luatk.newRect(   wdw, 20,       h-40-35, w-40, 2, {0, 0, 0} )
   btn_enter = luatk.newButton( wdw, 250,      h-40-20, 160, 40, _("ENTER ARENA"), gauntlet_enter )
   btn_enter:disable()
   luatk.newButton( wdw, w-250-80, h-40-20, 120, 40, _("Cancel"), gauntlet_cancel )

   -- Load defaults
   if gauntlet_type then
      for k,wgt in ipairs(btn_types) do
         if wgt.text == gauntlet_type then
            gauntlet_type = nil
            gauntlet_settype( wgt )
         end
      end
      if gauntlet_option then
         for k,wgt in ipairs(btn_options) do
            if wgt.text == gauntlet_option then
               gauntlet_option = nil
               gauntlet_setoption( wgt )
            end
         end
         for mk,mv in ipairs(gauntlet_modifiers) do
            for k,wgt in ipairs(btn_modifiers) do
               if wgt.text == gauntlet_modifiers[mk].str then
                  local gm = gauntlet_modifiers[mk]
                  if gm.enabled then
                     gm.enabled = false
                     gauntlet_setmodifier( wgt )
                  end
               end
            end
         end
      end
   end

   luatk.run()
end

local gui = {}

function gui.run()
   gauntlet_gui()

   if gauntlet_start then
      local mods = {}
      local cache = naev.cache()
      cache.gauntlet_type = gauntlet_type
      cache.gauntlet_option = gauntlet_option
      for k,v in ipairs(gauntlet_modifiers) do
         cache.gauntlet_modifiers[k] = v.enabled
         mods[v.id] = v.enabled
      end
      return gauntlet_type, gauntlet_option, mods
   end
   -- return nils if cancelled
end

return gui
