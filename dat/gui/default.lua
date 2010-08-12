

--[[
   @brief Obligatory create function.

   Run when the GUI is loaded which is caused whenever the player gets in a different ship.
--]]
function create()
   -- Get the player
   pp = player.pilot()
   pfact = pp:faction()

   -- Get sizes
   screen_w, screen_h = gfx.dim()
   deffont_h = gfx.fontSize()
   smallfont_h = gfx.fontSize(true)

   -- Some colours
   col_white   = colour.new()
   col_warn    = colour.new( "Red" )
   col_gray    = colour.new( "Grey70" )
   col_neut    = colour.new( 0.9, 1.0, 0.3, 1.0 )
   col_console = colour.new( 0.1, 0.9, 0.1, 1.0 )
   shield_col  = colour.new( 0.2, 0.2, 0.8, 0.8 )
   armour_col  = colour.new( 0.5, 0.5, 0.5, 0.8 )
   energy_col  = colour.new( 0.2, 0.8, 0.2, 0.8 )
   fuel_col    = colour.new( 0.9, 0.1, 0.4, 0.8 )

   -- Load graphics
   local base = "gfx/gui/"
   frame    = tex.open( base .. "minimal.png" )
   energy   = tex.open( base .. "minimal_energy.png" )
   fuel     = tex.open( base .. "minimal_fuel.png" )
   gui.targetPlanetGFX( tex.open( base .. "minimal_planet.png" ) )
   gui.targetPilotGFX( tex.open( base .. "minimal_pilot.png" ) )

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
   nav_h = 40
   nav_x, nav_y = relativize( 35, 220 )

   -- Weapon position
   weapon_w = 135
   weapon_h = 32
   weapon_x, weapon_y = relativize( 35, 294 )

   -- Target position
   target_w = 128
   target_h = 128
   target_x, target_y = relativize( 40, 350 )

   -- Misc position
   misc_w = 128
   misc_h = 104
   misc_x, misc_y = relativize( 40, 472 )

   -- Update stuff
   update_cargo()
   update_nav()
   update_target()
   update_ship()
   update_system()
end

function relativize( x, y )
   return frame_x + x, frame_y + frame_h - y
end


--[[
-- @brief This function is run whenever the player changes nav target (be in hyperspace or planet target).
--]]
function update_nav ()
   nav_pnt, nav_hyp = pp:nav()
end


--[[
-- @brief This function is run whenever the player changes his pilot target.
--]]
function update_target ()
   -- Set target
   ptarget = pp:target()
   if ptarget ~= nil then
      target_fact = ptarget:faction()
      target_gfx = ptarget:ship():gfxTarget()
      target_gfx_w, target_gfx_h = target_gfx:dim()
      target_gfxFact = target_fact:logoTiny()
      if target_gfxFact ~= nil then
         target_gf_w, target_gf_h = target_gfxFact:dim()
         target_gf_w = ( target_gf_w + 24 ) / 2
         target_gf_h = ( target_gf_h + 24 ) / 2
      end
   end
end


--[[
-- @brief This function is run whenever the player modifies his ship outfits (when the ship is changed the gui is recreated).
--]]
function update_ship ()
   stats = pp:stats()
   fuel_max = stats.fuel
end


--[[
-- @brief This function is run whenever the player changes his cargo.
--]]
function update_cargo ()
   cargol = player.cargoList()
   misc_cargo = ""
   for _,v in ipairs(cargol) do
      if v.q > 0 then
         misc_cargo = misc_cargo .. v.name
      else
         misc_cargo = misc_cargo .. string.format( "%d %s", v.q, v.name )
      end
      if v.m then
         misc_cargo = misc_cargo .. "*"
      end
      misc_cargo = misc_cargo .. "\n"
   end
end


--[[
-- @brief This function is run whenever the player changes system (every enter).
--]]
function update_system ()
end


--[[
   @brief Obligatory render function.

   Run every frame. Note that the dt will be 0. if the game is paused.

      @param dt Current deltatick in seconds since last render.
--]]
function render( dt )

   -- Render warnings
   local sys = system.cur()
   local nebu_dens, nebu_vol = sys:nebula()
   if nebu_vol > 0 then
      gfx.print( nil, 0, screen_h-40, col_warn, screen_w, true )
   end

   -- Frame
   gfx.renderTex( frame, frame_x, frame_y )

   -- Render radar
   gui.radarRender( radar_x, radar_y )

   -- NAV
   if nav_pnt ~= nil or nav_hyp ~= nil then
      local y = nav_y - 3 - deffont_h
      local str
      gfx.print( nil, "Landing", nav_x, y, col_console, nav_w, true )
      y = y - 5 - smallfont_h
      if nav_pnt ~= nil then
         str = nav_pnt:name()
         col = col_white
      else
         str = "Off"
         col = col_gray
      end
      gfx.print( true, str, nav_x, y, col, nav_w, true )
      y = nav_y - 33 - deffont_h
      gfx.print( nil, "Hyperspace", nav_x, y, col_console, nav_w, true )
      y = y - 5 - smallfont_h
      if nav_hyp ~= nil then
         str = nav_hyp:name()
         col = col_white
      else
         str = "Off"
         col = col_gray
      end
      gfx.print( true, str, nav_x, y, col, nav_w, true )
   else
      local y = nav_y - 20 - deffont_h
      gfx.print( nil, "Navigation", nav_x, y, col_console, nav_w, true )
      y = y - 5 - smallfont_h
      gfx.print( true, "Off", nav_x, y, col_gray, nav_w, true )
   end


   -- Render health
   local arm, shi = pp:health()
   gfx.renderRect( shield_x, shield_y, shi/100.*shield_w, shield_h, shield_col )
   gfx.renderRect( armour_x, armour_y, arm/100.*armour_w, armour_h, armour_col )
   local ene = pp:energy() / 100
   gfx.renderTexRaw( energy, energy_x, energy_y, ene*energy_w, energy_h, 1, 1, 0, 0, ene, 1, energy_col )
   local fue = player.fuel() / fuel_max
   gfx.renderTexRaw( fuel, fuel_x, fuel_y, fue*fuel_w, fuel_h, 1, 1, 0, 0, fue, 1, fuel_col )

   -- Weapon
   local sec, amm, rdy = pp:secondary()
   if sec ~= nil then
      local col
      if rdy then
         col = col_console
      else
         col = col_gray
      end
      if amm ~= nil then
         gfx.print( nil, sec, weapon_x, weapon_y-17, col, weapon_w, true )
         gfx.print( true, string.format("%d", amm), weapon_x, weapon_y-32, col_gray, weapon_w, true )
      else
         gfx.print( nil, "Secondary", weapon_x, weapon_y-25, col_console, weapon_w, true )
      end
   else
      gfx.print( nil, "Secondary", weapon_x, weapon_y-17, col_console, weapon_w, true )
      gfx.print( true, "None", weapon_x, weapon_y-32, col_gray, weapon_w, true )
   end

   -- Target
   if ptarget ~= nil then
      local col, shi, arm, dis
      arm, shi, dis = ptarget:health()

      -- Get colour
      if dis then
         col = col_gray
      else
         if pfact:areEnemies( target_fact ) then
            col = col_warn
         elseif pfact:areAllies( target_fact ) then
            col = col_console
         else
            col = col_neut
         end
      end

      -- Display name
      local name = ptarget:name()
      local w = gfx.printDim( nil, name )
      gfx.print( w > target_w, name, target_x, target_y-13, col, target_w )

      -- Display health
      if dis then
         str = "Disabled"
      elseif shi < 5 then
         str = string.format( "Armour: %.0f%%", arm )
      else
         str = string.format( "Shield: %.0f%%", shi )
      end
      gfx.print( true, str, target_x, target_y-100, col, target_w )

      -- Render target graphic
      local x, y
      x = target_x + (target_w - target_gfx_w)/2
      y = target_y - (target_h - target_gfx_h)/2 + 5
      gfx.renderTex( target_gfx, x, y - target_gfx_h )

      -- Render faction logo.
      if target_gfxFact ~= nil then
         gfx.renderTex( target_gfxFact, target_x + target_w - target_gf_w - 3, target_y - target_gf_h - 3 )
      end
   else
      gfx.print( false, "No Target", target_x, target_y-(target_h-deffont_h)/2, col_gray, target_w, true )
   end

   -- Misc
   _, creds = player.credits(2)
   h = 5 + smallfont_h
   y = misc_y - h
   gfx.print( true, "Creds:", misc_x, y, col_console, misc_w, false )
   w = gfx.printDim( true, creds )
   gfx.print( true, creds, misc_x+misc_w-w-3, y, col_white, misc_w, false )
   y = y - h
   gfx.print( true, "Cargo Free:", misc_x, y, col_console, misc_w, false )
   local free = string.format("%d Tn", player.cargoFree())
   w = gfx.printDim( true, free )
   gfx.print( true, free, misc_x+misc_w-w-3, y, col_white, misc_w, false )
   y = y - 5
   h = misc_h - 2*h - 8
   gfx.printText( true, misc_cargo, misc_x+13., y-h, misc_w-15., h, col_console )
end


--[[
   @brief Optional destroy function.

   Run when exitting the game on changing GUI. Graphics and stuff are cleaned up automatically.
--]]
function destroy()
end


