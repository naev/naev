--[[
-- FLF patrol elimination missions.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
end


function pilot_attacked_dv ()
   for i, j in ipairs( fleetDV ) do
      if j:exists() then
         j:setHostile()
      end
   end
end


-- Get a system for the Dvaered patrol.
-- These are systems which have both FLF and Dvaered presence.
function patrol_getTargetSystem ()
   local choices = { "Surano", "Zylex", "Arcanis", "Sonas", "Raelid", "Toaxis", "Tau Prime", "Zacron", "Tuoladis", "Doranthex", "Torg", "Tarsus", "Klantar", "Verex", "Dakron", "Theras", "Gilligan's Light", "Haleb", "Slaccid", "Norpin", "Triap", "Brimstone" }
   return system.get( choices[ rnd.rnd( 1, #choices ) ] )
end


-- Spawn a Dvaered patrol with n ships.
function patrol_spawnDV( n )
   pilot.clear()
   pilot.toggleSpawn( false )
   player.pilot():setVisible( true )
   local r = system.cur():radius()
   fleetDV = {}
   for i = 1, n do
      local x = rnd.rnd( -r, r )
      local y = rnd.rnd( -r, r )
      local shipnames = {"Dvaered Vendetta", "Dvaered Ancestor"}
      local shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      local pstk = pilot.add( shipname, "dvaered_norun", vec2.new( x, y ) )
      local p = pstk[1]
      p:rename( dv_ship_name )
      hook.pilot( p, "attacked", "pilot_attacked_dv" )
      hook.pilot( p, "death", "pilot_death_dv" )
      p:setVisible( true )
      p:setHilight( true )
      fleetDV[i] = p
      dv_ships_left = dv_ships_left + 1
   end
end

