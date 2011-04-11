--[[
   The new "slim" GUI
--]]

function create()

   --Get player
   pp = player.pilot()
   pfact = pp:faction()
   pname = player.name()
   pship = pp:ship()

   --Get sizes
   screen_w, screen_h = gfx.dim()
   deffont_h = gfx.fontSize()
   smallfont_h = gfx.fontSize(true)
   gui.viewport( 0, 28, screen_w, screen_h - 28 )

   --Colors
   col_txt_bar = colour.new( 192/255, 198/255, 217/255 )
   col_txt_top = colour.new( 148/255, 158/255, 192/255 )
   col_txt_std = colour.new( 111/255, 125/255, 169/255 )
   col_txt_wrn = colour.new( 127/255,  31/255,  31/255 )
   col_txt_enm = colour.new( 222/255,  28/255,  28/255 )
   col_txt_all = colour.new(  19/255, 152/255,  41/255 )
   col_txt_una = colour.new(  66/255,  72/255,  84/255 )
   col_shield = colour.new( 40/255,  51/255,  88/255 )
   col_armour = colour.new( 72/255,  73/255,  60/255 )
   col_energy = colour.new( 41/255,  92/255,  47/255 )
   col_speed = colour.new( 77/255,  80/255,  21/255 )
   col_speed2 = colour.new(169/255,177/255,  46/255 )
   col_ammo = colour.new(140/255,94/255,  7/255 )
   col_heat = colour.new(114/255,26/255, 14/255 )
   col_heat2 = colour.new( 222/255, 51/255, 27/255 )
   col_ready = colour.new(14/255,108/255, 114/255 )
   col_prim = colour.new(71/255,234/255, 252/255 )
   col_sec = colour.new(136/255,179/255, 255/255 )
   col_temperature = col_heat

   --Load Images
   local base = "gfx/gui/slim/"
   player_pane_t = tex.open( base .. "frame_player_top.png" )
   player_pane_m = tex.open( base .. "frame_player_middle.png" )
   player_pane_b = tex.open( base .. "frame_player_bottom.png" )
   target_pane = tex.open( base .. "frame_target.png" )
   planet_pane_t = tex.open( base .. "frame_planet_top.png" )
   planet_pane_m = tex.open( base .. "frame_planet_middle.png" )
   planet_pane_b = tex.open( base .. "frame_planet_bottom.png" )
   radar_gfx = tex.open( base .. "radar.png" )
   target_bg = tex.open( base .. "target_image.png" )
   planet_bg = tex.open( base .. "planet_image.png" )
   icon_shield = tex.open( base .. "shield.png" )
   icon_armour = tex.open( base .. "armour.png" )
   icon_energy = tex.open( base .. "energy.png" )
   icon_speed = tex.open( base .. "speed.png" )
   icon_temperature = tex.open( base .. "heat.png" )
   icon_shield_sm = tex.open( base .. "shield_sm.png" )
   icon_armour_sm = tex.open( base .. "armour_sm.png" )
   icon_energy_sm = tex.open( base .. "energy_sm.png" )
   icon_speed_sm = tex.open( base .. "speed_sm.png" )
   bg_bar = tex.open( base .. "bg_bar.png" )
   bg_bar_sm = tex.open( base .. "bg_bar_sm.png" )
   bg_bar_weapon = tex.open( base .. "bg_bar_weapon.png" )
   bg_bar_weapon_prim = tex.open( base .. "bg_bar_weapon_prim.png" )
   bg_bar_weapon_sec = tex.open( base .. "bg_bar_weapon_sec.png" )
   bg_shield = tex.open( base .. "bg_shield.png" )
   bg_armour = tex.open( base .. "bg_armour.png" )
   bg_energy = tex.open( base .. "bg_energy.png" )
   bg_speed = tex.open( base .. "bg_speed.png" )
   bg_temperature = tex.open( base .. "bg_speed.png" )
   bg_ammo = tex.open( base .. "bg_ammo.png" )
   bg_heat = tex.open( base .. "bg_heat.png" )
   bg_ready = tex.open( base .. "bg_ready.png" )
   bg_shield_sm = tex.open( base .. "bg_shield_sm.png" )
   bg_armour_sm = tex.open( base .. "bg_armour_sm.png" )
   bg_energy_sm = tex.open( base .. "bg_energy_sm.png" )
   bg_speed_sm = tex.open( base .. "bg_speed_sm.png" )
   sheen = tex.open( base .. "sheen.png" )
   sheen_sm = tex.open( base .. "sheen_sm.png" )
   sheen_weapon = tex.open( base .. "sheen_weapon.png" )
   sheen_tiny = tex.open( base .. "sheen_tiny.png" )
   bottom_bar = tex.open( base .. "bottombar.png" )
   target_dir = tex.open( base .. "dir.png" )
   warnlight1 = tex.open( base .. "warnlight1.png" )
   warnlight2 = tex.open( base .. "warnlight2.png" )
   warnlight3 = tex.open( base .. "warnlight3.png" )
   target_light_off = tex.open( base .. "targeted_off.png" )
   target_light_on =  tex.open( base .. "targeted_on.png" )
   cargo_light_off = tex.open( base .. "cargo_off.png" )
   cargo_light_on =  tex.open( base .. "cargo_on.png" )
   question = tex.open( base .. "question.png" )
   gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" ) )
   gui.targetPilotGFX(  tex.open( base .. "radar_ship.png" ) )

   --Messages
   gui.mesgInit( screen_w - 400, 20, 28+15+5 )

   --Get positions
   --Player pane
   pl_pane_w, pl_pane_h = player_pane_t:dim()
   pl_pane_w_b, pl_pane_h_b = player_pane_b:dim()
   pl_pane_x = screen_w - pl_pane_w - 16
   pl_pane_y = screen_h - pl_pane_h - 16

   --Radar
   radar_w, radar_h = radar_gfx:dim()
   radar_x = pl_pane_x - radar_w + 24
   radar_y = pl_pane_y + 31
   gui.radarInit( false, 124, 124 )

   bar_w, bar_h = bg_shield:dim()

   --Shield Bar
   x_shield = pl_pane_x + 46
   y_shield = pl_pane_y + 137

   bars = { "armour", "energy", "speed", "temperature" }
   for k,v in ipairs(bars) do
      _G["x_" .. v] = x_shield
      _G["y_" .. v] = y_shield - k * 28
   end
   bars[#bars] = "shield" -- Used later.

   --Ammo, heat and ready bars bars
   bar_weapon_w, bar_weapon_h = bg_ammo:dim()
   bar_ready_w, bar_ready_h = bg_ready:dim()
   x_ammo = pl_pane_x + 39
   y_ammo = pl_pane_y - 27

   --Target Pane
   ta_pane_w, ta_pane_h = target_pane:dim()
   ta_pane_x = screen_w - ta_pane_w - 16
   ta_pane_y = 44

   --Target image background
   ta_image_x = ta_pane_x + 14
   ta_image_y = ta_pane_y + 106
   --Target image center
   ta_image_w, ta_image_h = target_bg:dim()
   ta_center_x = ta_image_x + ta_image_w / 2
   ta_center_y = ta_image_y + ta_image_h / 2
   -- ? image
   ta_question_w, ta_question_h = question:dim()

   --Targeted icon
   ta_icon_x = ta_pane_x + 82
   ta_icon_y = ta_pane_y + 110

   --Target Faction icon
   ta_fact_x = ta_pane_x + 110
   ta_fact_y = ta_pane_y + 110

   bar_sm_w, bar_sm_h = bg_shield_sm:dim()
   --Small Shield Bar
   x_shield_sm = ta_pane_x + 13
   y_shield_sm = ta_pane_y + 71

   bars_sm = { "armour_sm", "energy_sm", "speed_sm" }
   for k,v in ipairs(bars_sm) do
      _G["x_" .. v] = x_shield_sm
      _G["y_" .. v] = y_shield_sm - k * 20
   end

   --Targeted warning light
   ta_warning_x = ta_pane_x + 82
   ta_warning_y = ta_pane_y + 110

   -- Cargo light
   ta_cargo_x = ta_pane_x + 138
   ta_cargo_y = ta_pane_y + 110

   -- Planet pane
   ta_pnt_pane_w, ta_pnt_pane_h = planet_pane_t:dim()
   ta_pnt_pane_w_b, ta_pnt_pane_h_b = planet_pane_b:dim()
   ta_pnt_pane_x = 16
   ta_pnt_pane_y = screen_h - ta_pnt_pane_h - 16

   -- Planet faction icon
   ta_pnt_fact_x = ta_pnt_pane_x + 140
   ta_pnt_fact_y = ta_pnt_pane_y + 155

   -- Planet image background
   ta_pnt_image_x = ta_pnt_pane_x + 14
   ta_pnt_image_y = ta_pnt_pane_y

   -- Planet image center
   ta_pnt_image_w, ta_pnt_image_h = planet_bg:dim()
   ta_pnt_center_x = ta_pnt_image_x + ta_pnt_image_w / 2
   ta_pnt_center_y = ta_pnt_image_y + ta_pnt_image_h / 2

   -- Set FPS
   gui.fpsPos( 20, screen_h - 48 - deffont_h )

   -- Set OSD
   gui.osdInit( 23, screen_h - 63, 150, 300 )

   -- Timer stuff
   timers = {}
   timers[1] = 0.5
   timers[2] = 0.5
   blinkcol = col_txt_enm
   gfxWarn = true

   update_target()
   update_ship()
   update_system()
   update_nav()
   update_cargo()
end

function update_target()
   ptarget = pp:target()
   if ptarget then
      ptarget_gfx = ptarget:ship():gfxTarget()
      ptarget_gfx_w, ptarget_gfx_h = ptarget_gfx:dim()
      ptargetfact = ptarget:faction()
      ptarget_target = ptarget:target()
      ta_stats = ptarget:stats()
      ta_cargo = ptarget:cargoList()

      ptarget_gfx_aspect = ptarget_gfx_w / ptarget_gfx_h

      if ptarget_gfx_aspect >= 1 and ptarget_gfx_w > 62 then
         ptarget_gfx_draw_w = 62
         ptarget_gfx_draw_h = 62 / ptarget_gfx_w * ptarget_gfx_h
      elseif ptarget_gfx_h > 62 then
         ptarget_gfx_draw_h = 62
         ptarget_gfx_draw_w = 62 / ptarget_gfx_h * ptarget_gfx_w
      end
      ptarget_faction_gfx = ptargetfact:logoTiny()
   end
end

function update_nav()
   nav_pnt, nav_hyp = pp:nav()
   autonav_hyp = player.autonavDest()
   if nav_pnt then
      pntflags = nav_pnt:services()
      n = 0
      if pntflags.land then
         services = { "land", "missions", "outfits", "shipyard", "commodity" }
         for k,v in ipairs(services) do
            if pntflags[tostring(v)] then
               n = n + 1
            end
         end
      end
      gui.osdInit( ta_pnt_pane_x + ta_pnt_pane_w + 8, screen_h - 63, 150, 300 )
      gui.fpsPos( ta_pnt_pane_x + ta_pnt_pane_w + 3, screen_h - 28 - 15 - deffont_h )

      ta_pnt_gfx = nav_pnt:gfxSpace()
      ta_pnt_gfx_w, ta_pnt_gfx_h = ta_pnt_gfx:dim()
      ta_pntfact = nav_pnt:faction()

      ta_pnt_gfx_aspect = ta_pnt_gfx_w / ta_pnt_gfx_h
      if ta_pnt_gfx_aspect >= 1 and ta_pnt_gfx_w > 140 then
         ta_pnt_gfx_draw_w = 140
         ta_pnt_gfx_draw_h = 140 / ta_pnt_gfx_aspect
      elseif ta_pnt_gfx_h > 140 then
         ta_pnt_gfx_draw_h = 140
         ta_pnt_gfx_draw_w = 140 / ta_pnt_gfx_aspect
      end
      ta_pnt_faction_gfx = nil
      if ta_pntfact then
         ta_pnt_faction_gfx = ta_pntfact:logoTiny()
      end
   else
      gui.osdInit( 23, screen_h - 63, 150, 300 )
      gui.fpsPos( 15, screen_h - 28 - 15 - deffont_h )
   end
   if nav_hyp then
      if nav_hyp:isKnown() then
         navstring = nav_hyp:name() .. " (%s)"
      else
         navstring = "Unknown (%s)"
      end
      if autonav_hyp then
         navstring = navstring:format( autonav_hyp:jumpDist() )
      end
   else
      navstring = "none"
   end
end

function update_cargo()
   cargol = pilot.cargoList(pp)
   cargo = {}
   for k,v in ipairs(cargol) do
      if v.q == 0 then
         cargo[k] = v.name
      else
         cargo[k] = string.format( "%d"  .. "t %s", v.q, v.name )
      end
      if v.m then
         cargo[k] = cargo[k] .. "*"
      end
   end
end

function update_ship()
   stats = pp:stats()
end

function update_system()
   sys = system.cur()
end

function render_bar(name, value, txt, txtcol, size, col, bgc )
   if size then
      offsets = { 22, 5, 9, 3 }
      l_bg_bar = bg_bar_sm
      l_sheen = sheen_sm
      postfix = "_sm"
   else
      offsets = { 30, 7, 15, 6 }
      l_bg_bar = bg_bar
      l_sheen = sheen
      postfix = nil
   end
   local vars = { "icon", "bg", "x", "y", "col" }
   for k,var in ipairs(vars) do
      if postfix and var ~= "col" then
         _G["l_" .. var] = _G[var .. "_" .. name .. postfix]
      else
         _G["l_" .. var] = _G[var .. "_" .. name]
      end
   end
   if col then
      l_col = col
   end
   if l_bg then
      l_bar_w, l_bar_h = l_bg:dim()
      gfx.renderTex( l_bg, l_x + offsets[1], l_y + 2)
   end
   if not value then value = 100 end
   if bgc then gfx.renderRect( l_x + offsets[1], l_y + 2, l_bar_w, l_bar_h, bgc ) end
   gfx.renderRect( l_x + offsets[1], l_y + 2, value/100. * l_bar_w, l_bar_h, l_col )
   gfx.renderTex( l_bg_bar, l_x, l_y )
   gfx.renderTex( l_icon, l_x + offsets[2], l_y + offsets[2] - 3)
   gfx.renderTex( l_sheen, l_x + offsets[1] + 1, l_y + offsets[3])

   if txt then
      small = false
      if gfx.printDim( false, txt ) > l_bar_w then
         small = true
      end
      gfx.print( small, txt, l_x + offsets[1], l_y + offsets[4], txtcol, l_bar_w, true)
   else
      gfx.print( true, "UNAVAILABLE", l_x + offsets[1], l_y + offsets[4], col_txt_una, l_bar_w, true )
   end
end

function render_ammoBar( name, x, y, value, txt, txtcol, col )
   offsets = { 2, 20, 3, 13, 22, 6 } --Bar, y of refire, sheen, y of sheen, y of refire sheen, y of text
   l_bg = _G["bg_" .. name]
   if name == "heat" then
      value[1] = value[1] / 2.
      if value[1] > .5 then
         l_col = col_heat2
      else
         l_col = _G["col_" .. name]
      end
   else
      l_col = _G["col_" .. name]
   end
   gfx.renderTex( l_bg, x + offsets[1], y + offsets[1])
   gfx.renderTex( bg_ready, x + offsets[1], y + offsets[2])
   gfx.renderRect( x + offsets[1], y + offsets[1], value[1] * bar_weapon_w, bar_weapon_h, l_col)
   gfx.renderRect( x + offsets[1], y + offsets[2], value[2] * bar_ready_w, bar_ready_h, col_ready)
   if value[3] == 2 then
      gfx.renderTex( bg_bar_weapon_sec, x, y )
   elseif value[3] == 1 then
      gfx.renderTex( bg_bar_weapon_prim, x, y )
   else
      gfx.renderTex( bg_bar_weapon, x, y )
   end
   gfx.renderTex( sheen_weapon, x + offsets[3], y + offsets[4])
   gfx.renderTex( sheen_tiny, x + offsets[3], y + offsets[5])
   gfx.print( true, txt, x + offsets[1], y + offsets[6], txtcol, bar_weapon_w, true)
end


function render( dt )

   --Values
   armour, shield = pp:health()
   energy = pp:energy()
   speed = pp:vel():dist()
   temperature = pp:temp()
   lockons = pp:lockon()
   autonav = player.autonav()
   wset_name, wset  = pp:weapset(true)
   credits = player.credits()

   --Radar
   gfx.renderTex( radar_gfx, radar_x, radar_y )
   gui.radarRender( radar_x + 2, radar_y + 2 )

   --Player pane
   gfx.renderTex( player_pane_t, pl_pane_x, pl_pane_y )
   filler_h = #wset * 28 -- extend the pane according to the number of weapon bars
   filler_h = math.max( filler_h - 6, 0 )

   gfx.renderTexRaw( player_pane_m, pl_pane_x + 33, pl_pane_y - filler_h, pl_pane_w_b, filler_h, 1, 1, 0, 0, 1, 1)
   gfx.renderTex( player_pane_b, pl_pane_x + 33, pl_pane_y - filler_h - pl_pane_h_b )

   local txt = {}
   for k,v in ipairs(bars) do
      txt[v] = string.format( "%s%% (%s)", round(_G[v]), round( stats[v] * _G[v] / 100 ) )
   end

   --Shield
   if shield == 0. then
      col = col_txt_enm
   elseif shield <= 20. then
      col = col_txt_wrn
   else
      col = col_txt_bar
   end
   render_bar( "shield", shield, txt["shield"], col )

   --Armour
   if armour <= 20. then
      col = col_txt_enm
   else
      col = col_txt_bar
   end
   render_bar( "armour", armour, txt["armour"], col )

   --Energy
   if energy == 0. then
      col = col_txt_enm
   elseif energy <= 20. then
      col = col_txt_wrn
   else
      col = col_txt_bar
   end
   render_bar( "energy", energy, txt["energy"], col )

   --Speed
   local hspeed = round(speed / stats.speed_max * 100,0)
   txt = tostring( hspeed ) .. "% (" .. tostring( round(speed)) .. ")"
   if hspeed <= 100. then
      render_bar( "speed", hspeed, txt, col_txt_bar )
   elseif hspeed <= 200. then
      render_bar( "speed", hspeed - 100, txt, col_txt_wrn, nil, col_speed2, col_speed )
   else
      timers[1] = timers[1] - dt
      if timers[1] <=0. then
         timers[1] = 0.5
         if blinkcol == col_txt_una then
            blinkcol = col_txt_enm
         else
            blinkcol = col_txt_una
         end
      end
      col = blinkcol
      render_bar( "speed", 100, txt, col, nil, col_speed2)
   end

   -- Temperature
   txt = round(temperature) .. "K"
   temperature = math.max( (temperature - 250)/1.75, 0 )
   render_bar( "temperature", temperature, txt, col_txt_bar )

   --Weapon bars
   local num = 0
   for k, weapon in ipairs(wset) do
      txt = weapon.name
      if weapon.left then -- Truncate names for readability.
         if weapon.type == "Bolt Cannon" or weapon.type == "Beam Cannon" then
            txt = string.gsub(txt,"Cannon", "C.")
         elseif weapon.type == "Bolt Turret" or weapon.type == "Beam Turret" then
            txt = string.gsub(txt,"Turret", "T.")
         elseif weapon.type == "Launcher" or weapon.type == "Turret Launcher" then
            txt = string.gsub(txt,"Launcher", "L.")
         end
      end
      if weapon.left then
         txt = txt .. " (" .. tostring( weapon.left) .. ")"
         if weapon.left == 0 then
            col = col_txt_wrn
         else
            col = col_txt_bar
         end
         values = {weapon.left_p, weapon.cooldown, weapon.level}
         render_ammoBar( "ammo", x_ammo, y_ammo - (num)*28, values, txt, col, 2, col_ammo )
      else
         col = col_txt_bar
         values = {weapon.temp, weapon.cooldown, weapon.level}
         render_ammoBar( "heat", x_ammo, y_ammo - (num)*28, values, txt, col, 2, col_heat )
      end
      num = num + 1
   end

   --Warning Light
   if lockons > 0 then
      timers[2] = timers[2] - dt
      if timers[2] <= 0. then
         if lockons < 20 then
            timers[2] = 0.5 - (0.025 * lockons)
            gfxWarn = not gfxWarn
         else
            timers[2] = 0
            gfxWarn = true
         end
      end
      if gfxWarn == true then
         gfx.renderTex( warnlight1, pl_pane_x + 6, pl_pane_y + 148 )
      end
      local length
      length = gfx.printDim( false, "Warning - Missile Lockon Detected" )
      gfx.print( false, "Warning - Missile Lockon Detected", (screen_w - length)/2, screen_h - 100, col_txt_enm )
   end
   if armour <= 20 then
      gfx.renderTex( warnlight2, pl_pane_x + 29, pl_pane_y + 3 )
   end
   if autonav then
      gfx.renderTex( warnlight3, pl_pane_x + 162, pl_pane_y + 8 )
   end

   --Target Pane
   if ptarget then
      ta_detect, ta_fuzzy = pp:inrange( ptarget )
      if ta_detect then
         --Frame
         gfx.renderTex( target_pane, ta_pane_x, ta_pane_y )
         gfx.renderTex( target_bg, ta_image_x, ta_image_y )

         if not ta_fuzzy then
            ptarget_target = ptarget:target()
            ta_armour, ta_shield, ta_disabled = ptarget:health()
            tflags = ptarget:flags()
            ta_energy = ptarget:energy()
            ta_speed = ptarget:vel():dist()

            --Render target graphic
            if ptarget_gfx_w > 62 or ptarget_gfx_h > 62 then
               gfx.renderTexRaw( ptarget_gfx, ta_center_x - ptarget_gfx_draw_w / 2, ta_center_y - ptarget_gfx_draw_h / 2, ptarget_gfx_draw_w, ptarget_gfx_draw_h, 1, 1, 0, 0, 1, 1)
            else
               gfx.renderTex( ptarget_gfx, ta_center_x - ptarget_gfx_w / 2, ta_center_y - ptarget_gfx_h / 2)
            end
         else
            --Render ?
            gfx.renderTex( question, ta_center_x - ta_question_w / 2, ta_center_y - ta_question_h / 2 )
         end

         -- Dist and dir calculated without explicit target.
         ta_pos = ptarget:pos()
         ta_dist = pp:pos():dist( ta_pos )
         ta_dir = ptarget:dir()
         ta_speed = ptarget:vel():dist()

         --Title
         gfx.print( false, "TARGETED", ta_pane_x + 14, ta_pane_y + 190, col_txt_top )

         --Text, warning light & other texts
         local htspeed = round(ta_speed / ta_stats.speed_max * 100,0)
         if not ta_fuzzy then
            --Bar Texts
            shi = tostring( round(ta_shield) ) .. "% (" .. tostring(round(ta_stats.shield  * ta_shield / 100)) .. ")"
            arm = tostring( round(ta_armour) ) .. "% (" .. tostring(round(ta_stats.armour  * ta_armour / 100)) .. ")"
            ene = tostring( round(ta_energy) ) .. "%"
            spe = tostring( htspeed ) .. "% (" .. tostring(round(ta_speed)) .. ")"

            if htspeed <= 100. then
               spetxtcol = col_txt_bar
               colspe = col_speed
               colspe2 = nil
            elseif htspeed >= 200. then
               htspeed = 100.
            else
               htspeed = htspeed - 100
               spetxtcol = col_txt_wrn
               colspe = col_speed2
               colspe2 = col_speed
            end

            --Warning Light
            if ptarget_target == pp and not ta_disabled then
               gfx.renderTex( target_light_on, ta_warning_x - 3, ta_warning_y - 3 )
            else
               gfx.renderTex( target_light_off, ta_warning_x, ta_warning_y )
            end

            --Faction Logo
            if ptarget_faction_gfx then
               gfx.renderTex( ptarget_faction_gfx, ta_fact_x, ta_fact_y )
            end

            -- Cargo light cargo_light_off
            if ta_cargo and #ta_cargo >= 1 then
               gfx.renderTex( cargo_light_on, ta_cargo_x, ta_cargo_y )
            else
               gfx.renderTex( cargo_light_off, ta_cargo_x, ta_cargo_y )
            end

            -- Status information
            if ta_disabled then
               status = "Disabled"
               gfx.print( true, status, ta_pane_x + 14, ta_pane_y + 94, col_txt_top )
            elseif tflags["boardable"] then
               status = "Boardable"
               gfx.print( true, status, ta_pane_x + 14, ta_pane_y + 94, col_txt_top )
            end

            --Pilot name
            if ta_disabled then
               col = col_txt_una
            else
               col = ptarget:colour()
            end
            gfx.print( true, ptarget:name(), ta_pane_x + 14, ta_pane_y + 176, col )
         else
            -- Unset stats.
            shi, ene, arm = nil
            ta_shield, ta_armour, ta_energy = nil

            --Bar Texts
            spe = round(ta_speed)
            colspe, colspe2 = nil
            spetxtcol = col_txt_bar
            htspeed = 0.

            --Warning light
            gfx.renderTex( target_light_off, ta_warning_x, ta_warning_y )

            -- Cargo light
            gfx.renderTex( cargo_light_off, ta_cargo_x, ta_cargo_y )

            --Pilot name
            gfx.print( true, "Unknown", ta_pane_x + 14, ta_pane_y + 176, col_txt_una )
         end

         -- Render bars.
         render_bar( "shield", ta_shield, shi, col_txt_bar, "sm")
         render_bar( "armour", ta_armour, arm, col_txt_bar, "sm")
         render_bar( "energy", ta_energy, ene, col_txt_bar, "sm")
         render_bar( "speed", htspeed, spe, spetxtcol, "sm", colspe, colspe2 )

         --Dist
         gfx.print( true, "DIST", ta_pane_x + 130, ta_pane_y + 160, col_txt_top )
         if ta_dist then
            local str = largeNumber( ta_dist, 1 )
            gfx.print( false, str, ta_pane_x + ta_pane_w - 15 - gfx.printDim(false, str), ta_pane_y +142, col_txt_std, 60, false )
         end

         --Dir
         gfx.print(true, "DIR", ta_pane_x + 86, ta_pane_y + 160, col_txt_top )

         -- Render dir sprite.
         local x, y = target_dir:spriteFromDir( ta_dir )
         gfx.renderTex( target_dir, ta_pane_x + 86, ta_pane_y + 136, x, y, col_txt_top )
      end
   end

   -- Planet pane
   if nav_pnt then
      local col = col_txt_std
      ta_pnt_pos = nav_pnt:pos()
      ta_pnt_dist = pp:pos():dist( ta_pnt_pos )
      ta_pnt_class = nav_pnt:class()

      -- Extend the pane depending on the services available.
      services_h = 44
      if pntflags.land then
         services_h = services_h + (14 * n)
      end

      -- Render background images.
      gfx.renderTex( planet_pane_t, ta_pnt_pane_x, ta_pnt_pane_y )
      gfx.renderTexRaw( planet_pane_m, ta_pnt_pane_x, ta_pnt_pane_y - services_h, ta_pnt_pane_w, services_h, 1, 1, 0, 0, 1, 1 )
      gfx.renderTex( planet_pane_b, ta_pnt_pane_x, ta_pnt_pane_y - services_h - ta_pnt_pane_h_b )
      gfx.renderTex( planet_bg, ta_pnt_image_x, ta_pnt_image_y )

      --Render planet image.
      if ta_pnt_gfx_w > 140 or ta_pnt_gfx_h > 140 then
         gfx.renderTexRaw( ta_pnt_gfx, ta_pnt_center_x - ta_pnt_gfx_draw_w / 2, ta_pnt_center_y - ta_pnt_gfx_draw_h / 2, ta_pnt_gfx_draw_w, ta_pnt_gfx_draw_h, 1, 1, 0, 0, 1, 1)
      else
         gfx.renderTex( ta_pnt_gfx, ta_pnt_center_x - ta_pnt_gfx_w / 2, ta_pnt_center_y - ta_pnt_gfx_h / 2)
      end
      gfx.print( true, "TARGETED", ta_pnt_pane_x + 14, ta_pnt_pane_y + 164, col_txt_top )
      gfx.print( true, "DISTANCE:", ta_pnt_pane_x + 35, ta_pnt_pane_y - 14, col_txt_top )
      gfx.print( true, "CLASS:", ta_pnt_pane_x + 14, ta_pnt_pane_y - 34, col_txt_top )

      if ta_pnt_faction_gfx then
         gfx.renderTex( ta_pnt_faction_gfx, ta_pnt_fact_x, ta_pnt_fact_y )
      end

      -- Colour the planet name based on friendliness.
      if ta_pntfact then
         if pfact:areEnemies( ta_pntfact ) then
            col = col_txt_enm
         elseif pfact:areAllies( ta_pntfact ) then
            col = col_txt_all
         else
            col = col_txt_std
         end
      end

      x1, y1 = vec2.get(nav_pnt:pos())
      x2, y2 = vec2.get(player.pilot():pos())
      ta_pnt_dir = math.atan2(y2 - y1, x2 - x1) + math.pi

      -- Render dir sprite.
      local x, y = target_dir:spriteFromDir( ta_pnt_dir )
      gfx.renderTex( target_dir, ta_pnt_pane_x + 12, ta_pnt_pane_y -24, x, y, col_txt_top )

      gfx.print( true, nav_pnt:class(), ta_pnt_pane_x + 130, ta_pnt_pane_y - 34, col_txt_top )
      gfx.print( true, "SERVICES:", ta_pnt_pane_x + 14, ta_pnt_pane_y - 46, col_txt_top )

      -- Space out the text.
      services_h = 60
      if pntflags.land then
         services = { "land", "missions", "outfits", "shipyard", "commodity" }
         servicesp = { "Spaceport", "Missions", "Outfits", "Shipyard", "Commodity" }
         for k,v in ipairs(services) do
            if pntflags[tostring(v)] then
               gfx.print(true, servicesp[k], ta_pnt_pane_x + 60, ta_pnt_pane_y - services_h, col_txt_top )
               services_h = services_h + 14
            end
         end
      else
         gfx.print( true, "none", ta_pnt_pane_x + 110, ta_pnt_pane_y - 46, col_txt_una )
      end

      if ta_pnt_dist then
         gfx.print( false, largeNumber( ta_pnt_dist, 1 ), ta_pnt_pane_x + 110, ta_pnt_pane_y - 15, col_txt_std, 63, false )
      end
      gfx.print( true, nav_pnt:name(), ta_pnt_pane_x + 14, ta_pnt_pane_y + 149, col )
   end

   --Bottom bar
   local length = 5, navstring, fuel, fuelstring, wsetstr
   gfx.renderTexRaw( bottom_bar, 0, 0, screen_w, 30, 1, 1, 0, 0, 1, 1 )

   fuel = player.fuel()
   if fuel > 100 then
      fuelstring = round(fuel/100.) .. " Jumps"
   elseif fuel == 100 then
      fuelstring = round(fuel/100.) .. " Jump"
   else
      fuelstring = "none"
   end

   if rdy then
      col = col_txt_std
   else
      col = col_txt_una
   end
   wsetstr = wset_name

   local bartext = { "Pilot: ", pname, "System: ", sys:name(), "Time: ", time.str(), "Credits: ",
         largeNumber( credits, 2 ), "Nav: ", navstring, "Fuel: ", fuelstring,
         "WSet: ", wsetstr, "Cargo: " }
   for k,v in ipairs(bartext) do
      if k % 2 == 1 then
         gfx.print( true, v, length, 5, col_txt_top )
         length = length + gfx.printDim( true, v )
      else
         if v == "none" or (v == wsetstr and not rdy) then
            col = col_txt_una
         else
            col = col_txt_std
         end
         gfx.print( true, v, length, 5, col )
         length = length + gfx.printDim( true, v ) + 10
      end
   end

   local cargstring = nil
   local freecargo = " (" .. pp:cargoFree() .. "t free)"
   local finallen = gfx.printDim( true, freecargo )
   local terminator = gfx.printDim( true, ", [...]" )
   if cargo and #cargo >= 1 then
      for k,v in ipairs(cargo) do
         if cargstring then
            if screen_w - length - gfx.printDim(true, cargstring .. ", " .. v) < finallen + terminator then
               cargstring = cargstring .. ", [...]"
               break
            else
               cargstring = cargstring .. ", " .. v
            end
         else
            cargstring = v
         end
      end
      gfx.print( true, cargstring, length, 6, col_txt_std )

      length = length + gfx.printDim( true, cargstring )
   else
      gfx.print( true, "none", length, 6, col_txt_una )
      length = length + gfx.printDim( true, "none" ) + 6
   end
   gfx.print( true, freecargo, length, 6, col_txt_std )
end

function largeNumber( number, idp )
   local formatted
   local units = { "K", "M", "B", "T", "Q" }
   if number < 1e4 then
      formatted = math.floor(number)
   elseif number < 1e18 then
      len = math.floor(math.log10(number))
      formatted = roundto( number / 10^math.floor(len-len%3), idp) .. units[(math.floor(len/3))]
   else
      formatted = "Too big!"
   end
   return formatted
end

function roundto(num, idp)
   return string.format("%.0" .. (idp or 0) .. "f", num)
end

function round(num)
   return math.floor( num + 0.5 )
end

function destroy()
end
