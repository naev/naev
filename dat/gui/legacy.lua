local fmt = require "format"

local armour_col, armour_h, armour_w, armour_x, armour_y
local col_console, col_gray, col_warn, col_white
local deffont_h, smallfont_h
local energy, energy_col, energy_h, energy_w, energy_x, energy_y
local frame, frame_h, frame_w, frame_x, frame_y
local fuel, fuel_col, fuel_h, fuel_max, fuel_w, fuel_x, fuel_y
local misc_cargo, misc_h, misc_w, misc_x, misc_y, nav_hyp, nav_pnt, nav_w, nav_x, nav_y, pp, ptarget
local radar_r, radar_x, radar_y, screen_h, screen_w
local shield_col, shield_h, shield_w, shield_x, shield_y
local target_fact, target_gf_h, target_gf_w, target_gfx, target_gfxFact, target_gfx_h, target_gfx_w
local target_h, target_w, target_x, target_y, weapon_w, weapon_x, weapon_y

local function relativize( x, y )
   return frame_x + x, frame_y + frame_h - y
end

--[[--
   Obligatory create function.

   Run when the GUI is loaded which is caused whenever the player gets in a different ship.
--]]
function create()
   -- Get the player
   pp = player.pilot()

   -- Get sizes
   screen_w, screen_h = gfx.dim()
   deffont_h = gfx.fontSize()
   smallfont_h = gfx.fontSize(true)

   -- FPS pos
   gui.fpsPos( 15, screen_h - 15 - deffont_h );

   -- Some colours
   col_white   = colour.new()
   col_warn    = colour.new( "Red" )
   col_gray    = colour.new( "Grey70" )
   col_console = colour.new( 0.1, 0.9, 0.1, 1.0 )
   shield_col  = colour.new( 0.2, 0.2, 0.8, 0.8 )
   armour_col  = colour.new( 0.5, 0.5, 0.5, 0.8 )
   energy_col  = colour.new( 0.2, 0.8, 0.2, 0.8 )
   fuel_col    = colour.new( 0.9, 0.1, 0.4, 0.8 )

   -- Load graphics
   local base = "gfx/gui/legacy/"
   local function tex_open( name, sx, sy )
      local t = tex.open( base .. name, sx, sy )
      t:setWrap( "clamp" )
      return t
   end
   frame    = tex_open( "minimal.png" )
   energy   = tex_open( "minimal_energy.png" )
   fuel     = tex_open( "minimal_fuel.png" )
   gui.targetSpobGFX( tex_open( "minimal_planet.png", 2, 2 ) )
   gui.targetPilotGFX( tex_open( "minimal_pilot.png", 2, 2 ) )

   -- OSD
   gui.osdInit( 30, screen_h-90, 150, 300 )

   -- Messages
   gui.mesgInit( screen_w-400, 20, 30 )

   -- Frame position
   frame_w, frame_h = frame:dim()
   frame_x  = screen_w - frame_w - 15
   frame_y  = screen_h - frame_h - 15

   -- Radar
   radar_r = 82
   radar_x, radar_y = relativize( 83, 90 )
   gui.radarInit( true, radar_r )

   -- Health position
   shield_w = 128
   shield_h = 7
   shield_x, shield_y = relativize( 43, 192 )
   armour_w = 128
   armour_h = 7
   armour_x, armour_y = relativize( 43, 206 )

   -- Fuel/energy position
   energy_x, energy_y = relativize( 97, 177 )
   energy_w, energy_h = energy:dim()
   fuel_x, fuel_y = relativize( 95, 78 )
   fuel_w, fuel_h = fuel:dim()

   -- NAV position
   nav_w = 135
   --nav_h = 40
   nav_x, nav_y = relativize( 35, 220 )

   -- Weapon position
   weapon_w = 135
   --weapon_h = 32
   weapon_x, weapon_y = relativize( 35, 294 )

   -- Target position
   target_w = 128
   target_h = 100
   target_x, target_y = relativize( 40, 350 )

   -- Misc position
   misc_w = 128
   misc_h = 104
   misc_x, misc_y = relativize( 40, 472 )

   -- Bottom bar
   --gui.viewport( 0, 20, screen_w, screen_h-20 )

   -- Overlay bounds
   gui.setMapOverlayBounds(15, 185, 15, 185)

   -- Update stuff
   update_cargo()
   update_nav()
   update_target()
   update_ship()
   update_system()
end


--[[--
   This function is run whenever the player changes nav target (be in hyperspace or planet target).
--]]
function update_nav ()
   nav_pnt, nav_hyp = pp:nav()
end


--[[--
   This function is run whenever the player changes their pilot target.
--]]
function update_target ()
   -- Set target
   ptarget = pp:target()
   target_gfxFact = nil
   if ptarget ~= nil then
      target_fact = ptarget:faction()
      target_gfx = ptarget:render():getTex()
      target_gfx_w, target_gfx_h = target_gfx:dim()
      if target_fact ~= nil and target_fact:known() then
         target_gfxFact = target_fact:logo()
         if target_gfxFact ~= nil then
            target_gf_w, target_gf_h = target_gfxFact:dim()
            local ls = 24 / math.max( target_gf_w, target_gf_h )
            target_gf_w, target_gf_h = ls*target_gf_w, ls*target_gf_h
         end
      end
   end
end


--[[--
   This function is run whenever the player modifies their ship outfits (when the ship is changed the gui is recreated).
--]]
function update_ship ()
   local stats = pp:stats()
   fuel_max = stats.fuel
end


--[[--
   This function is run whenever the player changes their cargo.
--]]
function update_cargo ()
   local cargol = pp:cargoList()
   misc_cargo = ""
   for k,v in ipairs(cargol) do
      if v.q == 0 then
         misc_cargo = misc_cargo .. _(v.name)
      else
         misc_cargo = misc_cargo .. fmt.tonnes_short( v.q ) .. " " .. _(v.name)
      end
      if v.m then
         misc_cargo = misc_cargo .. "*"
      end
      misc_cargo = misc_cargo .. "\n"
   end
end


--[[--
   This function is run whenever the player changes system (every enter).
--]]
function update_system ()
end


local function render_border ()
   --gfx.renderRect( 0, 0, screen_w/2, 20, col_white )
end


-- Renders the navigation computer
local function render_nav ()
   if nav_pnt ~= nil or nav_hyp ~= nil then
      local y = nav_y - 3 - deffont_h
      local col, str
      gfx.print( nil, _("Landing"), nav_x, y, col_console, nav_w, true )
      y = y - 5 - smallfont_h
      if nav_pnt ~= nil then
         str = nav_pnt:name()
         col = col_white
      else
         str = _("Off")
         col = col_gray
      end
      gfx.print( true, str, nav_x, y, col, nav_w, true )
      y = nav_y - 33 - deffont_h
      gfx.print( nil, _("Hyperspace"), nav_x, y, col_console, nav_w, true )
      y = y - 5 - smallfont_h
      if nav_hyp ~= nil then
         if nav_hyp:known() then
            str = nav_hyp:name()
         else
            str = _("Unknown")
         end
         col = col_white
      else
         str = _("Off")
         col = col_gray
      end
      gfx.print( true, str, nav_x, y, col, nav_w, true )
   else
      local y = nav_y - 20 - deffont_h
      gfx.print( nil, _("Navigation"), nav_x, y, col_console, nav_w, true )
      y = y - 5 - smallfont_h
      gfx.print( true, _("Off"), nav_x, y, col_gray, nav_w, true )
   end
end


-- Renders the health bars
local function render_health ()
   local arm, shi = pp:health()
   gfx.renderRect( shield_x, shield_y, shi/100.*shield_w, shield_h, shield_col )
   gfx.renderRect( armour_x, armour_y, arm/100.*armour_w, armour_h, armour_col )
   local ene = pp:energy() / 100
   gfx.renderTexRaw( energy, energy_x, energy_y, ene*energy_w, energy_h, 1, 1, 0, 0, ene, 1, energy_col )
   local fue = player.fuel() / fuel_max
   gfx.renderTexRaw( fuel, fuel_x, fuel_y, fue*fuel_w, fuel_h, 1, 1, 0, 0, fue, 1, fuel_col )
end


-- Renders the weapon systems
local function render_weapon ()
   local col = col_console
   local ws_name = pp:weapset()
   gfx.print( nil, _(ws_name), weapon_x, weapon_y-25, col, weapon_w, true )
   --[[
   local sec, amm, rdy = pp:secondary()
   if sec ~= nil then
      local col
      if rdy then
         col = col_console
      else
         col = col_gray
      end
      if amm ~= nil then
         gfx.print( nil, _(sec), weapon_x, weapon_y-17, col, weapon_w, true )
         gfx.print( true, string.format("%d", amm), weapon_x, weapon_y-32, col_gray, weapon_w, true )
      else
         gfx.print( nil, _(sec), weapon_x, weapon_y-25, col, weapon_w, true )
      end
   else
      gfx.print( nil, _("Secondary"), weapon_x, weapon_y-17, col_console, weapon_w, true )
      gfx.print( true, _("None"), weapon_x, weapon_y-32, col_gray, weapon_w, true )
   end
   --]]
end


local function render_targetnone ()
   gfx.print( false, _("No Target"), target_x, target_y-(target_h-deffont_h)/2-deffont_h, col_gray, target_w, true )
end


-- Renders the pilot target
local function render_target ()
   -- Target must exist
   if ptarget == nil then
      render_targetnone()
      return
   end

   local det, scan = pp:inrange(ptarget)

   -- Must be detected
   if not det then
      render_targetnone()
      return
   end

   local col
   local arm, shi, _stress, dis = ptarget:health()

   -- Get colour
   if dis or not scan then
      col = col_gray
   else
      col = ptarget:colour()
   end

   -- Render target graphic
   local x, y, w
   if not scan then
      local str = _("Unknown")
      w = gfx.printDim( true, str )
      x = target_x + (target_w - w)/2
      y = target_y - (target_h - smallfont_h)/2
      gfx.print( true, str, x, y-smallfont_h, col_gray, w, true )
   else
      x = target_x + (target_w - target_gfx_w)/2
      y = target_y + (target_h - target_gfx_h)/2
      gfx.renderTex( target_gfx, x, y-target_h )
   end

   -- Display name
   local name
   if not scan then
      name = _("Unknown")
   else
      name = ptarget:name()
   end
   w = gfx.printDim( nil, name )
   gfx.print( w > target_w, name, target_x, target_y-13, col, target_w )

   -- Display faction
   if scan and target_fact ~= nil and target_fact:known() then
      local faction = target_fact:name()
      gfx.print( true, faction, target_x, target_y-26, col_white, target_w )
   end

   -- Display health
   if scan then
      local str
      if dis then
         str = _("Disabled")
      elseif shi < 5 then
         str = string.format( _("Armour: %.0f%%"), arm )
      else
         str = string.format( _("Shield: %.0f%%"), shi )
      end
      gfx.print( true, str, target_x, target_y-105, col_white, target_w )
   end

   -- Render faction logo.
   if scan and target_gfxFact ~= nil then
      gfx.renderTexScale( target_gfxFact, target_x + target_w - target_gf_w/2 - 15, target_y - target_gf_h - 15, target_gf_w, target_gf_h )
   end
end


-- Renders the miscellaneous stuff
local function render_misc ()
   local _creds_num, creds = player.credits(2)
   local h = 5 + smallfont_h
   local y = misc_y - h
   gfx.print( true, _("Creds:"), misc_x, y, col_console, misc_w, false )
   local w = gfx.printDim( true, creds )
   gfx.print( true, creds, misc_x+misc_w-w-3, y, col_white, misc_w, false )
   y = y - h
   gfx.print( true, _("Cargo Free:"), misc_x, y, col_console, misc_w, false )
   local free = fmt.tonnes_short( pp:cargoFree() )
   w = gfx.printDim( true, free )
   gfx.print( true, free, misc_x+misc_w-w-3, y, col_white, misc_w, false )
   y = y - 5
   h = misc_h - 2*h - 8
   gfx.printText( true, misc_cargo, misc_x+13., y-h, misc_w-15., h, col_white )
end


-- Renders the warnings like system volatility
local function render_warnings ()
   -- Render warnings
   local sys = system.cur()
   local _nebu_dens, nebu_vol = sys:nebula()
   local y = screen_h - 50 - deffont_h
   if pp:lockon() > 0 then
      gfx.print( nil, _("LOCK-ON DETECTED"), 0, y, col_warn, screen_w, true )
      y = y - deffont_h - 10
   end
   if nebu_vol > 0 then
      gfx.print( nil, _("VOLATILE ENVIRONMENT DETECTED"), 0, y, col_warn, screen_w, true )
   end
end


--[[--
   Obligatory render function.

   Run every frame. Note that the dt will be 0. if the game is paused.

      @param dt Current deltatick in seconds since last render.
--]]
function render( _dt )
   gfx.renderTex( frame, frame_x, frame_y )
   gui.radarRender( radar_x, radar_y )
   render_border()
   render_nav()
   render_health()
   render_weapon()
   render_target()
   render_misc()
   render_warnings()
end

function update_faction()
end

local cooldown_omsg
function render_cooldown( _percent, seconds )
   local msg = _("Cooling down...\n%.1f seconds remaining"):format( seconds )
   local fail = true
   if cooldown_omsg ~= nil then
      if player.omsgChange( cooldown_omsg, msg, seconds ) then
         fail = false
      end
   end
   if fail then
      cooldown_omsg = player.omsgAdd( msg, seconds )
   end
end

function cooldown_end ()
   if cooldown_omsg ~= nil then
      player.omsgRm( cooldown_omsg )
      cooldown_omsg = nil
   end
end

function update_effects ()
end
