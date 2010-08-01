

--[[
   @brief Obligatory create function.

   Run when the GUI is loaded.
--]]
function create()
   -- Get the player
   pp = player.pilot()

   -- Get screen size
   screen_w, screen_h = gfx.dim()

   -- Load graphics
   local base = "gfx/gui/"
   frame    = tex.open( base .. "minimal.png" )
   energy   = tex.open( base .. "minimal_energy.png" )
   fuel     = tex.open( base .. "minimal_fuel.png" )

   -- Calculate positions
   frame_w, frame_h = frame:dim()
   frame_x  = screen_w - frame_w - 15
   frame_y  = screen_h - frame_h - 15
   shield_w = 128
   shield_h = 7
   shield_x, shield_y = relativize( 43, 185 )
   armour_w = 128
   armour_h = 7
   armour_x, armour_y = relativize( 43, 199 )
end

function relativize( x, y )
   return frame_x + x, frame_y + frame_h - y
end


--[[
   @brief Obligatory render function.

   Run every frame.
--]]
function render()

   -- Frame
   gfx.renderTex( frame, frame_x, frame_y )

   -- Render health
   local arm, shi = pp:health()
   gfx.renderRect( shield_x, shield_y, shi*shield_w, shield_h )
   gfx.renderRect( armour_x, armour_y, arm*armour_w, armour_h )

end


--[[
   @brief Optional destroy function.

   Run when exitting the game on changing GUI.
--]]
function destroy()
end

