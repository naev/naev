

--[[
   @brief Obligatory create function.

   Run when the GUI is loaded.
--]]
function create()
   screen_w, screen_h = gfx.dim()

   tex = tex.open( "gfx/NAEV.png" )
end


--[[
   @brief Obligatory render function.

   Run every frame.
--]]
function render()
   gfx.renderTex( tex, 0., 0. )
end


--[[
   @brief Optional destroy function.

   Run when exitting the game on changing GUI.
--]]
function destroy()
end

