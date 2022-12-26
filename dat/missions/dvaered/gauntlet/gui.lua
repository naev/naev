--[[
-- Fancy GUI interface for the Crimson Gauntlet
--]]
local luatk = require 'luatk'
local lg = require 'love.graphics'
local fmt = require "format"

local gauntlet_option, gauntlet_start, gauntlet_type, gauntlet_subtype
local btn_enter, btn_modifiers, btn_options, btn_subtypes, btn_types, headerh, modifiers_divider, options_divider, subtypes_divider

local gauntlet_modifiers = {
   { id = "doubledmgtaken",
    str = fmt.f(_("Double Damage Enemies ({bonus})"), {bonus="#g+50%#0"}),
    var = "gauntlet_unlock_doubledmgtaken",
    enabled = false },
   { id = "nohealing",
    str = fmt.f(_("No Healing Between Waves ({bonus})"),{bonus="#g+25%#0"}),
    var = "gauntlet_unlock_nohealing",
    enabled = false },
   { id = "doubleenemy",
    str = _("Double Enemies"),
    var = nil,
    enabled = false },
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
      local b = luatk.newButton( wdw, x, y, bw, bh, _(v), handler )
      b.gauntlet = v
      btns[#btns+1] = b
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
      if v.id == wgt.gauntlet then
         v.enabled = state
      end
   end
end

local function gauntlet_setsubtype( wgt )
   local newsubtype = wgt.gauntlet
   --[[
   if gauntlet_subtype == newsubtype then
      return
   end
   --]]
   gauntlet_subtype = newsubtype
   btn_enter:enable()

   for k,v in ipairs(btn_subtypes) do
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
   modifiers_divider = luatk.newRect( wdw, 20, headerh+214, w-40, 2, {0, 0, 0} )

   local strlist = {}
   for k,v in ipairs(gauntlet_modifiers) do
      table.insert( strlist, v.str )
   end
   btn_modifiers = button_list( wdw, strlist,
         0, headerh+230, 240, 60, w, 100, gauntlet_setmodifier )
   for k,v in ipairs(gauntlet_modifiers) do
      btn_modifiers[k].gauntlet = v.id -- Overwrite and use id instead of string
      if v.var and not var.peek(v.var) then
         btn_modifiers[k]:disable()
      end
   end
end

local function gauntlet_setoption( wgt )
   local newoption = wgt.gauntlet
   if gauntlet_option == newoption then
      return
   end
   gauntlet_option = newoption
   btn_enter:disable()

   for k,v in ipairs(btn_options) do
      v:setCol(nil)
      v:setFCol(nil)
   end
   wgt:setCol{ 0.8, 0.05, 0.20 }
   wgt:setFCol{ 0.9, 0.9, 0.9 }

   for k,v in ipairs(btn_subtypes) do
      v:destroy()
   end
   if subtypes_divider then
      subtypes_divider:destroy()
   end

   for k,v in ipairs(btn_modifiers) do
      v:destroy()
   end
   if modifiers_divider then
      modifiers_divider:destroy()
   end

   local wdw = wgt.parent
   local w = wdw.w
   if gauntlet_type == "Challenge" then
      btn_subtypes = button_list( wdw,
            {N_("Independent")},
            0, headerh+160, 160, 40, w, 100, gauntlet_setsubtype )
      --[[
      if not var.peek("gauntlet_unlock_warrior") then
         btn_options[2]:disable()
      end
      --]]
   end
   subtypes_divider = luatk.newRect( wdw, 20, headerh+214, w-40, 2, {0, 0, 0} )

   local enabled = 0
   local subtype_def = nil
   for k,v in ipairs( btn_subtypes ) do
      if not v.disabled then
         enabled = enabled+1
         subtype_def = v
      end
   end
   if enabled == 1 then
      gauntlet_setsubtype( subtype_def )
   end
end


local function gauntlet_settype( wgt )
   local newtype = wgt.gauntlet
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
   for k,v in ipairs(btn_subtypes) do
      v:destroy()
   end
   for k,v in ipairs(btn_modifiers) do
      v:destroy()
   end
   if subtypes_divider then
      subtypes_divider:destroy()
   end
   if options_divider then
      options_divider:destroy()
   end

   local wdw = wgt.parent
   local w = wdw.w
   if newtype == "Challenge" then
      btn_options = button_list( wdw,
            {N_("Skirmisher"), N_("Warrior"), N_("Warlord")},
            0, headerh+90, 160, 40, w, 100, gauntlet_setoption )
      if not var.peek("gauntlet_unlock_warrior") then
         btn_options[2]:disable()
      end
      if not var.peek("gauntlet_unlock_warlord") then
         btn_options[3]:disable()
      end
   end
   options_divider = luatk.newRect( wdw, 20, headerh+144, w-40, 2, {0, 0, 0} )

   local enabled = 0
   local option_def = nil
   for k,v in ipairs( btn_options ) do
      if not v.disabled then
         enabled = enabled+1
         option_def = v
      end
   end
   if enabled == 1 then
      gauntlet_setoption( option_def )
   end
end

local function gauntlet_enter ()
   local worthy, reason = player.pilot():spaceworthy()
   if not worthy then
      luatk.msg(_("Not Spaceworthy!"), _("Your ship is not spaceworthy and can not participate in the Crimson Gauntlet right now for the following reasons:\n\n")..reason)
   else
      gauntlet_start = true
      luatk.close()
   end
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
   gauntlet_subtype = cache.gauntlet_subtype
   cache.gauntlet_modifiers = cache.gauntlet_modifiers or {}
   for k,v in ipairs(cache.gauntlet_modifiers) do
      gauntlet_modifiers[k].enabled = v
   end

   -- Window and top details
   local w, h = 800, 470
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
         {N_("Tournament"), N_("Challenge"), N_("Infinity Arena"), N_("Special")},
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

   -- Subtypes
   btn_subtypes = {}

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
         if wgt.gauntlet == gauntlet_type then
            gauntlet_type = nil
            gauntlet_settype( wgt )
         end
      end
      if gauntlet_option then
         for k,wgt in ipairs(btn_options) do
            if wgt.gauntlet == gauntlet_option then
               gauntlet_option = nil
               gauntlet_setoption( wgt )
            end
         end
         if gauntlet_subtype then
            for k,wgt in ipairs(btn_subtypes) do
               if wgt.gauntlet == gauntlet_subtype then
                  gauntlet_subtype = nil
                  gauntlet_setsubtype( wgt )
               end
            end
            for mk,mv in ipairs(gauntlet_modifiers) do
               for k,wgt in ipairs(btn_modifiers) do
                  if wgt.gauntlet == gauntlet_modifiers[mk].id then
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
   else
      local enabled = 0
      local type_def = nil
      for k,v in ipairs( btn_types ) do
         if not v.disabled then
            enabled = enabled+1
            type_def = v
         end
      end
      if enabled == 1 then
         gauntlet_settype( type_def )
      end
   end

   luatk.run()
end

local gui = {}

function gui.run()
   gauntlet_gui()

   if not gauntlet_start then
      return -- Return nothing if cancelled
   end

   local mods = {}
   local cache = naev.cache()
   cache.gauntlet_type = gauntlet_type
   cache.gauntlet_subtype = gauntlet_subtype
   cache.gauntlet_option = gauntlet_option
   for k,v in ipairs(gauntlet_modifiers) do
      cache.gauntlet_modifiers[k] = v.enabled
      mods[v.id] = v.enabled
   end
   return gauntlet_type, gauntlet_option, gauntlet_subtype, mods
end

return gui
