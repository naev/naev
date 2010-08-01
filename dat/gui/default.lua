

--[[
   @brief Obligatory create function.

   Run when the GUI is loaded.
--]]
function create()
   -- Get the player
   pp = player.pilot()

   -- Get sizes
   screen_w, screen_h = gfx.dim()
   deffont_h = gfx.fontSize()
   smallfont_h = gfx.fontSize(true)

   -- Some colours
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

   -- Frame position
   frame_w, frame_h = frame:dim()
   frame_x  = screen_w - frame_w - 15
   frame_y  = screen_h - frame_h - 15

   -- Radar
   radar_r = 82
   radar_x, radar_y = relativize( 83, 90 )
   --radar = gui.radarCreate( radar_r )

   -- Health position
   shield_w = 128
   shield_h = 7
   shield_x, shield_y = relativize( 43, 192 )
   armour_w = 128
   armour_h = 7
   armour_x, armour_y = relativize( 43, 206 )

   -- Fuel/energy position
   energy_x, energy_y = relativize( 97, 177 )
   fuel_x, fuel_y = relativize( 95, 78 )

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
   misc_w = 135
   misc_h = 104
   misc_x, misc_y = relativize( 35, 475 )
end

function relativize( x, y )
   return frame_x + x, frame_y + frame_h - y
end


--[[
   @brief Obligatory render function.

   Run every frame.
--]]
function render()

   -- Render warnings
   local sys = system.cur()
   local nebu_dens, nebu_vol = sys:nebula()
   if nebu_vol > 0 then
      gfx.print( nil, 0, screen_h-40, col_warn, screen_w, true )
   end

   -- Frame
   gfx.renderTex( frame, frame_x, frame_y )

   -- Render radar
   --gui.radarRender( radar, radar_x, radar_y )

   -- NAV

   -- Render health
   local arm, shi = pp:health()
   gfx.renderRect( shield_x, shield_y, shi/100.*shield_w, shield_h, shield_col )
   gfx.renderRect( armour_x, armour_y, arm/100.*armour_w, armour_h, armour_col )
   -- energy
   -- fuel

   -- Weapon
   gfx.print( nil, "Secondary", weapon_x, weapon_y-17, col_console, weapon_w, true )
   gfx.print( true, "None", weapon_x, weapon_y-32, col_gray, weapon_w, true )

   -- Target
   local targ = pp:target()
   if targ ~= pp then
      local col, shi, arm, dis
      arm, shi, dis = targ:health()

      -- Get colour
      if dis then
         col = col_gray
      else
         local tfact = targ:faction()
         local pfact = pp:faction()
         if pfact:areEnemies( tfact ) then
            col = col_warn
         elseif pfact:areAllies( tfact ) then
            col = col_console
         else
            col = col_neut
         end
      end

      -- Display name
      local name = targ:name()
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

      -- Render faction graphic and target graphic
   else
      gfx.print( false, "No Target", target_x, target_y-(target_h-deffont_h)/2, col_gray, target_w, true )
   end

end


--[[
   @brief Optional destroy function.

   Run when exitting the game on changing GUI.
--]]
function destroy()
end

