--[[
   The new "brushed" UI.
--]]

playerform = include "dat/scripts/playerform.lua"

function create()

   --Get Player
   pp = player.pilot()
   pp = player.pilot()
   pfact = pp:faction()
   pname = player.name()
   pship = pp:ship()

   --Get sizes
   screen_w, screen_h = gfx.dim()
   deffont_h = gfx.fontSize()
   smallfont_h = gfx.fontSize(true)

   --Colors
   col_shield = colour.new(  24/255,  31/255,  80/255 )
   col_armour = colour.new(  52/255,  52/255,  52/255 )
   col_energy = colour.new(  23/255,  80/255,  33/255 )
   col_fuel   = colour.new(  80/255,  24/255,  37/255 )
   col_heat   = colour.new(  80/255,  27/255,  24/255 )
   col_heat2  = colour.new( 181/255,  34/255,  26/255 )
   col_stress = colour.new(  24/255,  27/255,  80/255 )
   col_ammo   = colour.new( 159/255,  93/255,  15/255 )
   col_top_shield = colour.new(  88/255,  96/255, 156/255 )
   col_top_armour = colour.new( 122/255, 122/255, 122/255 )
   col_top_energy = colour.new(  52/255, 172/255,  71/255 )
   col_top_fuel   = colour.new( 156/255,  88/255, 104/255 )
   col_top_heat   = colour.new( 188/255,  63/255,  56/255 )
   col_top_heat2  = colour.new( 238/255, 143/255, 138/255 )
   col_top_stress = colour.new(  56/255,  88/255, 156/255 )
   col_top_ammo   = colour.new( 233/255, 131/255,  21/255 )
   col_text = colour.new( 203/255, 203/255, 203/255 )
   col_unkn = colour.new( 130/255, 130/255, 130/255 )
   col_lgray = colour.new( 160/255, 160/255, 160/255 )

   --Images
   local base = "dat/gfx/gui/brushed/"
   main = tex.open( base .. "main.png" )
   ext_right = tex.open( base .. "extRight.png" )
   end_right = tex.open( base .. "endRight.png" )
   popup_bottom = tex.open( base .. "popupBottom.png" )
   popup_bottom_side_left = tex.open( base .. "tooltipRightSideLeft.png" )
   popup_bottom2 = tex.open( base .. "tooltipRightBottom.png" )
   popup_body = tex.open( base .. "tooltipRight.png" )
   popup_top = tex.open( base .. "tooltipRightTop.png" )
   popup_empty = tex.open( base .. "tooltipEmpty.png" )
   popup_pilot = tex.open( base .. "pilotFrame.png" )
   bar_bg = tex.open( base .. "barBg.png" )
   bar_frame_light = tex.open( base .. "barFrameLight.png" )
   bar_frame = tex.open( base .. "barFrame.png" )
   bar_light = tex.open( base .. "light.png" )
   bar_sheen = tex.open( base .. "barSheen.png" )
   bar_light = tex.open( base .. "light.png" )
   bar_lock = tex.open( base .. "lock.png" )
   planet_pane_t = tex.open( base .. "frame_planet_top.png" )
   planet_pane_m = tex.open( base .. "frame_planet_middle.png" )
   planet_pane_b = tex.open( base .. "frame_planet_bottom.png" )
   planet_bg = tex.open( base .. "planet_image.png" )
   icon_shield = tex.open( base .. "iconShield.png" )
   icon_armour = tex.open( base .. "iconArmour.png" )
   icon_energy = tex.open( base .. "iconEnergy.png" )
   icon_fuel = tex.open( base .. "iconFuel.png" )
   icon_Kinetic = tex.open( base .. "kinetic.png" )
   icon_Radiation = tex.open( base .. "nuclear.png" )
   icon_EMP = tex.open( base .. "ion.png" )
   icon_Energy = tex.open( base .. "plasma.png" )
   icon_Ion = tex.open( base .. "laser.png" )
   icon_missile = tex.open( base .. "missile.png" )
   icon_projectile = tex.open( base .. "projectile.png" )
   icon_beam = tex.open( base .. "beam.png" )
   icon_weapon2 = tex.open( base .. "weapon2.png" )
   icon_weapon1 = tex.open( base .. "weapon1.png" )
   icon_outfit = tex.open( base .. "outfit.png" )
   icon_pnt_target = tex.open( base .. "iconPntTarg.png" )
   icon_nav_target = tex.open( base .. "iconNavTarg.png" )
   icon_money = tex.open( base .. "iconMoney.png" )
   icon_cargo = tex.open( base .. "iconCargo.png" )
   icon_missions = tex.open( base .. "iconMissions.png" )
   icon_ship = tex.open( base .. "iconShip.png" )
   icon_weapons = tex.open( base .. "iconWeaps.png" )
   icon_autonav = tex.open( base .. "A.png" )
   icon_lockon = tex.open( base .. "lockon.png" )
   icon_refire = tex.open( base .. "refireCircle.png" )
   icon_lockon2 = tex.open( base .. "lockonCircle.png" )
   field_bg_left = tex.open( base .. "fieldBgLeft1.png" )
   field_bg_center1 = tex.open( base .. "fieldBgCenter1.png" )
   field_bg_center2 = tex.open( base .. "fieldBgCenter2.png" )
   field_bg_right1 = tex.open( base .. "fieldBgRight1.png" )
   field_bg_right2 = tex.open( base .. "fieldBgRight2.png" )
   field_frame_left = tex.open( base .. "fieldFrameLeft.png" )
   field_frame_center = tex.open( base .. "fieldFrameCenter.png" )
   field_frame_right = tex.open( base .. "fieldFrameRight.png" )
   field_sheen_left = tex.open( base .. "fieldSheenLeft.png" )
   field_sheen = tex.open( base .. "fieldSheen.png" )
   target_bg = tex.open( base .. "targetBg.png" )
   target_frame = tex.open( base .. "targetFrame.png" )
   target_sheen = tex.open( base .. "targetSheen.png" )
   question = tex.open( base .. "question.png" )
   speed_light = tex.open( base .. "speedOn.png" )
   speed_light_double = tex.open( base .. "speedDouble.png" )
   speed_light_off = tex.open( base .. "speedOff.png" )
   top_bar = tex.open( base .. "topbar.png" )
   top_bar_center = tex.open( base .. "topbarCenter.png" )
   top_bar_center_sheen = tex.open( base .. "topbarSheen.png" )
   top_bar_center_sheen2 = tex.open( base .. "topbarSheen2.png" )
   button_normal = tex.open( base .. "button.png" )
   button_hilighted = tex.open( base .. "buttonHil2.png" )
   button_mouseover = tex.open( base .. "buttonHil.png" )
   button_pressed = tex.open( base .. "buttonPre.png" )
   button_disabled = tex.open( base .. "buttonDis.png" )
   gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" ) )
   gui.targetPilotGFX(  tex.open( base .. "radar_ship.png" ) )


   --Positions
   --Main is at 0,0

   --Radar
   radar_x = 263
   radar_y = 5
   radar_w = 114
   radar_h = 120
   gui.radarInit( false, radar_w, radar_h )

   bar_y = 2
   bar_x = 46
   bar_w, bar_h = bar_bg:dim()
   bars = { "shield", "armour", "energy", "fuel" }
   for k,v in ipairs( bars ) do
      _G[ "x_" .. v ] = bar_x + (k-1)*(bar_w + 6)
      _G[ "y_" .. v ] = bar_y
   end

   pl_speed_x = 38
   pl_speed_y = 2

   target_bar_x = 57
   target_bar_y = 92
   bars_target = { "target_shield", "target_armour", "target_energy" }
   for k, v in ipairs( bars_target ) do
      _G[ "x_" .. v ] = target_bar_x + (k-1)*(bar_w + 6)
      _G[ "y_" .. v ] = target_bar_y
   end

   target_image_x = 53
   target_image_y = 172
   target_image_w, target_image_h = target_bg:dim()
   question_w, question_h = question:dim()

   x_name = 102
   y_name = 197

   field_w, field_h = field_bg_left:dim()

   x_dist = 102
   y_dist = 172

   x_speed = 189
   y_speed = 173

   right_side_x = 406
   left_side_w, left_side_h = main:dim()
   end_right_w, end_right_h = end_right:dim()
   popup_left_x = 42
   popup_left_y = 88
   popup_right_x = 432
   popup_right_y = 88

   weapbars = math.max( 3, math.floor((screen_w - left_side_w - end_right_w + 10)/(bar_w + 6)) ) --number of weapon bars that can fit on the screen (minimum 3)

   circle_w, circle_h = icon_refire:dim()

   tbar_center_x = screen_w/2
   tbar_center_w, tbar_center_h = top_bar_center:dim()
   tbar_w, tbar_h = top_bar:dim()
   tbar_y = screen_h - tbar_h

   gui.viewport( 0, 0, screen_w, screen_h )

   fields_y = tbar_y + 15
   if screen_w <=1024 then
      fields_w = (screen_w-tbar_center_w)/4-8
      fields_x = 0
   else
      fields_w = (1024-tbar_center_w)/4-8
      fields_x = (screen_w - 1024)/2
   end

   buttons_y = screen_h - 34
   buttons_w, buttons_h = button_normal:dim()
   buttontypes = { "missions", "cargo", "ship", "weapons" }
   buttons = {}
   for k, v in ipairs(buttontypes) do
      buttons[v] = { x=tbar_center_x-116+(k-1)*60, y=buttons_y, w=buttons_w, h=buttons_h, state="default", icon=_G[ "icon_" .. v ], action=_G["action_" .. v ] }
      buttons[v]["icon_w"], buttons[v]["icon_h"] = _G[ "icon_" .. v]:dim()
   end

   -- Messages
   mesg_x = left_side_w
   mesg_y = end_right_h + 10
   mesg_w = screen_w - mesg_x - 10
   gui.mesgInit( mesg_w, mesg_x, mesg_y )

   -- Planet pane
   ta_pnt_pane_w, ta_pnt_pane_h = planet_pane_t:dim()
   ta_pnt_pane_w_b, ta_pnt_pane_h_b = planet_pane_b:dim()
   ta_pnt_pane_x = math.max( screen_w - ta_pnt_pane_w - 16, tbar_center_x + tbar_center_w/2 - 10 )
   ta_pnt_pane_y = screen_h - ta_pnt_pane_h - 32

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
   gui.fpsPos( 10, left_side_h )

   -- Set OSD
   local osd_w = 225
   local osd_h = screen_h - 275
   gui.osdInit( 30, screen_h - 50, osd_w, osd_h )

   first_time = { true, 2 }
   navstring = _("none")

   gui.mouseClickEnable(true)
   gui.mouseMoveEnable(true)

   update_target()
   update_ship()
   update_system()
   update_nav()
   update_cargo()
end

function roundto(num, idp)
   return string.format("%.0" .. (idp or 0) .. "f", num)
end

function round(num)
   return math.floor( num + 0.5 )
end

function largeNumber( number, idp )
   local formatted
   local units = { "k", "M", "B", "T", "Q" }
   if number < 1e4 then
      formatted = math.floor(number)
   elseif number < 1e18 then
      len = math.floor(math.log10(number))
      formatted = roundto( number / 10^math.floor(len-len%3), idp) .. units[(math.floor(len/3))]
   else
      formatted = _("Too big!")
   end
   return formatted
end

function update_target()
   ptarget = pp:target()
   if ptarget ~= nil then
      ta_dir = ptarget:dir()
      ta_gfx = ptarget:ship():gfx()
      ta_sx, ta_sy = ta_gfx:spriteFromDir( ta_dir )
      ta_gfx_w, ta_gfx_h, ta_gfx_sw, ta_gfx_sh = ta_gfx:dim()
      ta_fact = ptarget:faction()
      ta_stats = ptarget:stats()

      ta_gfx_aspect = ta_gfx_sw / ta_gfx_sh

      if ta_gfx_aspect >= 1 then
         if ta_gfx_sw > target_image_w then
            ta_gfx_draw_w = target_image_w
            ta_gfx_draw_h = target_image_w / ta_gfx_sw * ta_gfx_sh
         else
            ta_gfx_draw_w = ta_gfx_sw
            ta_gfx_draw_h = ta_gfx_sh
         end
      else
         if ta_gfx_sh > target_h then
            ta_gfx_draw_h = target_image_w
            ta_gfx_draw_w = target_image_w / ta_gfx_sh * ta_gfx_sw
         else
            ta_gfx_draw_w = ta_gfx_sw
            ta_gfx_draw_h = ta_gfx_sh
         end
      end
   end
end

function update_nav()
   planet = {}
   nav_pnt, nav_hyp = pp:nav()
   autonav_hyp = player.autonavDest()
   if nav_pnt then
      pntflags = nav_pnt:services()

      ta_pnt_gfx = nav_pnt:gfxSpace()
      ta_pnt_gfx_w, ta_pnt_gfx_h = ta_pnt_gfx:dim()
      ta_pntfact = nav_pnt:faction()

      ta_pnt_gfx_aspect = ta_pnt_gfx_w / ta_pnt_gfx_h
      if math.max( ta_pnt_gfx_w, ta_pnt_gfx_h ) > 140 then
         ta_pnt_gfx_draw_w = math.min( 140, 140 * ta_pnt_gfx_aspect )
         ta_pnt_gfx_draw_h = math.min( 140, 140 / ta_pnt_gfx_aspect )
      end

      ta_pnt_faction_gfx = nil
      if ta_pntfact then
         ta_pnt_faction_gfx = ta_pntfact:logoTiny()
      end

      planet = { -- Table for convenience.
         name = nav_pnt:name(),
         pos = nav_pnt:pos(),
         class = nav_pnt:class(),
         col = nav_pnt:colour(),
         services = {}
      }

      if pntflags.land then
         services = { "missions", "outfits", "shipyard", "commodity" }

         -- "Spaceport" is nicer than "Land"
         table.insert( planet.services, "Spaceport" )
         for k,v in ipairs(services) do
            table.insert( planet.services, pntflags[v] )
         end
         planet.nservices = #planet.services
      end
   end
   if nav_hyp then
      if nav_hyp:known() then
         navstring = nav_hyp:name()
      else
         navstring = _("Unknown")
      end
      if autonav_hyp then
         navstring = (navstring .. " (%s)"):format( autonav_hyp:jumpDist() )
      end
   else
      navstring = _("none")
   end
end

function update_faction()
end

function update_cargo()
   cargo = pp:cargoFree()
   cargolist = pp:cargoList()

   if not first_time[1] then
      if #cargolist == 0 then
         buttons["cargo"].state = "disabled"
      else
         if buttons["cargo"].state ~= "mouseover" then buttons["cargo"].state = "hilighted" end
      end
   else
      first_time[1] = false
   end
end

function update_ship()
   stats = pp:stats()

   if not first_time[2] then
      if buttons["ship"].state ~= "mouseover" then
         buttons["ship"].state = "hilighted"
      end
      if buttons["weapons"].state ~= "mouseover" then
         buttons["weapons"].state = "hilighted"
      end
   else
      first_time[2] = first_time[2] - 1
   end
end

function update_system()
end

function renderBar( name, value, light, locked, prefix, mod_x, heat, stress )
   local offsets = { 2, 2, 4, 54, 12, -2 } --Bar/Icon x, Bar y, Sheen x, Sheen y, light x, light y

   local vars = { "col", "col_top", "x", "y", "icon" }
   for k, v in ipairs( vars ) do
      if (v == "x" or v == "y") and prefix ~= nil then
         _G[ "l_" .. v ] = _G[ v .. "_" .. prefix .. "_" .. name ]
      else
         _G[ "l_" .. v ] = _G[ v .. "_" .. name ]
      end
   end
   l_x = l_x + mod_x
   icon_w, icon_h = l_icon:dim()

   if locked == true then
      gfx.renderTex( bar_lock, l_x + offsets[1], l_y + offsets[2] ) --Lock
   else
      gfx.renderTex( bar_bg, l_x + offsets[1], l_y + offsets[2] ) --Background
      gfx.renderRect( l_x + offsets[1], l_y + offsets[2], bar_w, value/100. * bar_h, l_col ) --Bar

      -- Heat bar (only if heat is specified)
      if heat ~= nil then
         if heat <= 1 then
            heatcol = col_heat
            heatcol_top = col_top_heat
         else
            heatcol = col_heat2
            heatcol_top = col_top_heat2
         end
         gfx.renderRect( l_x + offsets[1], l_y + offsets[2], bar_w/2, heat/2 * bar_h * (value/100.), heatcol ) --Heat bar
         if heat < 2 then
            gfx.renderRect( l_x + offsets[1], l_y + offsets[2] + heat/2 * bar_h * (value/100.), bar_w/2, 1, heatcol_top ) --top bit
         end
      end

      -- Stress (disable) bar (only if stress is specified)
      if stress ~= nil then
         gfx.renderRect( l_x + offsets[1] + bar_w/2, l_y + offsets[2], bar_w/2, (stress/100.) * bar_h * (value/100.), col_stress ) --Stress bar
         if stress < 100 then
            gfx.renderRect( l_x + offsets[1] + bar_w/2, l_y + offsets[2] + (stress/100.) * bar_h * value/100., bar_w/2, 1, col_top_stress ) --top bit
         end
      end

      if value < 100 then
         gfx.renderRect( l_x + offsets[1], l_y + offsets[2] + value/100. * bar_h, bar_w, 1, l_col_top ) --lighter area
      end
   end
   gfx.renderTex( l_icon, l_x + offsets[1], l_y + offsets[2] + bar_h/2 - icon_h/2 ) --Icon
   if light ~= false then
      gfx.renderTex( bar_frame_light, l_x, l_y ) --Frame
      local show_light = false
      if name == "fuel" then
         show_light = player.jumps() <= 0
         if autonav_hyp ~= nil then
            show_light = show_light or player.jumps() < autonav_hyp:jumpDist()
         end
      else
         show_light = value < 20
      end
      if show_light then
         gfx.renderTex( bar_light, l_x + offsets[5], l_y + offsets[6] ) --Warning light
      end
   else
      gfx.renderTex( bar_frame, l_x, l_y ) --Frame
   end
   gfx.renderTex( bar_sheen, l_x + offsets[3], l_y + offsets[4] ) --Sheen
end

function renderWeapBar( weapon, x, y )
   local offsets = { 2, 2, 4, 54, 13, 23, 47 } --third last is y of icon_weapon1, last two are the centers of the two weapon icons
   local outfit_yoffset = 31
   local name_offset = 17
   if weapon ~= nil then
      if weapon.ammo ~= nil then
         width = bar_w/2
      else
          width = bar_w
      end
      if weapon.temp <= 1 then
         heatcol = col_heat
         heatcol_top = col_top_heat
      else
         heatcol = col_heat2
         heatcol_top = col_top_heat2
      end

      if weapon.is_outfit then
         icon = outfit.get( weapon.name ):icon()
         icon_w, icon_h = icon:dim()

         if weapon.type == "Afterburner" then
            weap_heat = weapon.temp * 2
         elseif weapon.duration ~= nil then
            weap_heat = (1 - weapon.duration) * 2
         elseif weapon.cooldown ~= nil then
            weap_heat = weapon.cooldown * 2
         else
            weap_heat = 0
         end
      else
         if weapon.dtype ~= nil and weapon.dtype ~= "Unknown" and _G[ "icon_" .. weapon.dtype ]~= nil then
            top_icon = _G[ "icon_" .. weapon.dtype ]
         else
            top_icon = icon_Kinetic
         end

         if weapon.type == "Bolt Cannon" or weapon.type == "Bolt Turret" then
            bottom_icon = icon_projectile
         elseif weapon.type == "Beam Cannon" or weapon.type == "Beam Turret" then
            bottom_icon = icon_beam
         elseif weapon.type == "Launcher" or weapon.type == "Turret Launcher" then
            bottom_icon = icon_missile
         elseif weapon.type == "Fighter Bay" then
            bottom_icon = icon_ship
         end

         top_icon_w, top_icon_h = top_icon:dim()
         bottom_icon_w, bottom_icon_h = bottom_icon:dim()

         weap_heat = weapon.temp
      end

      gfx.renderTex( bar_bg, x + offsets[1], y + offsets[2] ) --Background
      gfx.renderRect( x + offsets[1], y + offsets[2], width, weap_heat/2 *bar_h, heatcol ) --Heat bar, mandatory
      if weap_heat < 2 then
         gfx.renderRect( x + offsets[1], y + offsets[2] + weap_heat/2 * bar_h, width, 1, heatcol_top ) --top bit
      end

      if weapon.is_outfit then
         gfx.renderTex( icon_outfit, x + offsets[1], y + offsets[5] )
         gfx.renderTexRaw( icon, x + offsets[1] + bar_w/2 - 20, y + offsets[2] + outfit_yoffset, 40, 40, 1, 1, 0, 0, 1, 1 )
         if weapon.weapset then
            gfx.print( false, weapon.weapset, x + offsets[1], y + offsets[2] + name_offset, col_text, 40, true )
         end
      else
         local col = nil
         if weapon.ammo ~= nil then
            gfx.renderRect( x + offsets[1] + width, y + offsets[2], width, weapon.left_p * bar_h, col_ammo ) --Ammo bar, only if applicable
            if weapon.left_p < 1 then
               gfx.renderRect( x + offsets[1] + width, y + offsets[2] + weapon.left_p * bar_h, width, 1, col_top_ammo ) --top bit
            end
            if not weapon.in_arc and player.pilot():target() ~= nil then
               col = col_lgray
            end

            if weapon.lockon ~= nil then
               gfx.renderTexRaw( icon_lockon2, x + offsets[1] + bar_w/2 - circle_w/2, y + offsets[2] + offsets[6] - circle_h/2, circle_w, circle_h * weapon.lockon, 1, 1, 0, 0, 1, weapon.lockon) --Lockon indicator
            end
            gfx.renderTexRaw( icon_refire, x + offsets[1] + bar_w/2 - circle_w/2, y + offsets[2] + offsets[7] - circle_h/2, circle_w, circle_h * weapon.cooldown, 1, 1, 0, 0, 1, weapon.cooldown) --Cooldown indicator
            --Icon
            gfx.renderTex( icon_weapon2, x + offsets[1], y + offsets[2] )
         else
            --Icon
            gfx.renderTexRaw( icon_refire, x + offsets[1] + bar_w/2 - circle_w/2, y + offsets[2] + offsets[7] - circle_h/2, circle_w, circle_h * weapon.cooldown, 1, 1, 0, 0, 1, weapon.cooldown) --Cooldown indicator
            gfx.renderTex( icon_weapon1, x + offsets[1], y + offsets[5] )
         end

         --Weapon-specific Icon
         gfx.renderTex( top_icon, x + offsets[1] + bar_w/2 - top_icon_w/2, y + offsets[2] + offsets[7] - top_icon_h/2 )
         gfx.renderTex( bottom_icon, x + offsets[1] + bar_w/2 - bottom_icon_w/2, y + offsets[2] +  offsets[6] - bottom_icon_h/2, col )
      end
      if weapon.is_outfit then
         gfx.renderTex( bar_frame_light, x, y ) -- Frame with light
         if weapon.state == "on" then
            gfx.renderTex( bar_light, x + 12, y - 2 ) -- Active light
         end
      else
         gfx.renderTex( bar_frame, x, y ) --Frame
      end
   else
      gfx.renderTex( bar_lock, x + offsets[1], y + offsets[2] )
      gfx.renderTex( bar_frame, x, y ) --Frame
   end

   gfx.renderTex( bar_sheen, x + offsets[3], y + offsets[4] )
end

function renderField( text, x, y, w, col, icon )
   local offsets = { 3, 14, 6 } --Sheen x and y, Icon x
   local onetwo = 1

   gfx.renderTex( field_bg_left, x, y )
   drawn_w = 14
   while drawn_w < w - 14 do
      gfx.renderTex( _G[ "field_bg_center" .. tostring(onetwo) ], x + drawn_w, y )
      if onetwo == 1 then
         onetwo = 2
      else
         onetwo = 1
      end
      drawn_w = drawn_w + 2
   end
   gfx.renderTex( _G[ "field_bg_right" .. tostring(onetwo) ], x + w - 14, y )

   if icon ~= nil then
      local icon_w, icon_h = icon:dim()
      gfx.renderTex( icon, x + offsets[3], y + 11 - icon_h/2 )
      gfx.print( true, text, x+offsets[3]+icon_w+2, y+field_h/2-smallfont_h/2, col, w-(offsets[3]+icon_w+2), true )
   else
      gfx.print( true, text, x, y + field_h/2 - smallfont_h/2, col, w, true )
   end

   --gfx.renderTex( field_frame, x, y ) --Frame

   gfx.renderTex( field_frame_left, x, y )
   if w > 28 then
      gfx.renderTexRaw( field_frame_center, x+14, y, w-28, field_h, 1, 1, 0, 0, 1, 1 )
   end
   if w >= 28 then
      gfx.renderTex( field_frame_right, x+w-14, y )
   else
      gfx.renderTex( field_frame_right, x+14, y )
   end

   --gfx.renderTex( field_sheen, x + offsets[1], y + offsets[2] ) --Sheen
   gfx.renderTex( field_sheen_left, x + offsets[1], y + offsets[2] )
   gfx.renderTexRaw( field_sheen, x + offsets[1] + 6, y + offsets[2], w - (2*offsets[1]+6), 6, 1, 1, 0, 0, 1, 1 )
end

function renderButton( button )
   local v_button = buttons[button]

   if v_button.state == "hilighted" then
      gfx.renderTex( button_hilighted, v_button.x, v_button.y )
   elseif v_button.state == "mouseover" then
      gfx.renderTex( button_mouseover, v_button.x, v_button.y )
   elseif v_button.state == "disabled" then
      gfx.renderTex( button_disabled, v_button.x, v_button.y )
   elseif v_button.state == "pressed" then
      gfx.renderTex( button_pressed, v_button.x, v_button.y )
   else
      gfx.renderTex( button_normal, v_button.x, v_button.y )
   end

   gfx.renderTex( v_button.icon, v_button.x+v_button.w/2-v_button.icon_w/2, v_button.y+v_button.h/2-v_button.icon_h/2 )
end

function render( dt )

   --Values
   armour, shield, stress = pp:health()
   energy = pp:energy()
   fuel = player.fuel() / stats.fuel_max * 100
   heat = math.max( math.min( (pp:temp() - 250)/87.5, 2 ), 0 )
   wset_name, wset = pp:weapset( true )
   aset = pp:actives( true )
   table.sort( aset, function(v) return v.weapset end )

   for k, v in ipairs( wset ) do
      v.is_outfit = false
   end
   for k, v in ipairs( aset ) do
      v.is_outfit = true
      wset[ #wset + 1 ] = v
   end

   credits, credits_h = player.credits(2)
   autonav = player.autonav()
   lockons = pp:lockon()

   --Main window right
   if #wset > weapbars then
      wbars_right = weapbars
   else
      wbars_right = #wset
   end
   right_side_w = (bar_w + 6)*wbars_right - 1
   gui_w = right_side_w + left_side_w - 10
   mod_x = math.max( 0, math.min( screen_w - math.max( gui_w, 1024 ), math.floor( (screen_w - gui_w)/3 ) ) )
   gfx.renderTexRaw( ext_right, left_side_w - 10 + mod_x, 0, right_side_w, end_right_h, 1, 1, 0, 0, 1, 1 )
   gfx.renderTex( end_right, right_side_x + right_side_w + mod_x, 0 )

   right_side_h = end_right_h
   for k=1,wbars_right do
      renderWeapBar( wset[k], right_side_x + 6 + (k-1)*(bar_w + 6) + mod_x, bar_y )
   end
   if wbars_right ~= #wset then
      --Draw a popup of (theoretically) arbitrary size.
      amount = #wset - wbars_right
      height = math.ceil(amount/3. ) * (bar_h+6) - 3
      right_side_h = right_side_h + height
      gfx.renderTex( popup_bottom2, popup_right_x + mod_x, popup_right_y )
      gfx.renderTex( popup_top, popup_right_x + mod_x, popup_right_y + 6 + height )
      gfx.renderTexRaw( popup_body, popup_right_x + mod_x, popup_right_y + 6, 165, height, 1, 1, 0, 0, 1, 1 )
      gfx.renderTex( popup_bottom_side_left, popup_right_x + 7 + mod_x, popup_right_y )
      gfx.renderTexRaw( popup_bottom_side_left, popup_right_x + 158 + mod_x, popup_right_y, -3, 19, 1, 1, 0, 0, 1, 1 )

      local drawn
      for i=1, (amount+1) do
         local x = (i-1) % 3 * (bar_w+6) + popup_right_x + 14
         local y = math.floor( (i-1) / 3. ) * (bar_h+6) + 3 + popup_right_y
         renderWeapBar( wset[ wbars_right + i ], x + mod_x, y )
      end
      for i=(amount+1), math.ceil( amount/3. )*3 do
         local x = (i-1) % 3 * (bar_w+6) + popup_right_x + 14
         local y = math.floor( (i-1) / 3. ) * (bar_h+6) + 3 + popup_right_y
         renderWeapBar( nil, x + mod_x, y )
      end
      gfx.renderTex( popup_bottom, popup_right_x + mod_x, popup_right_y - 5 )
   end

   -- Messages
   local new_mesg_x = left_side_w + mod_x
   local new_mesg_y = right_side_h + 10
   local new_mesg_w = screen_w - new_mesg_x - 10
   if mesg_x ~= new_mesg_x or mesg_y ~= new_mesg_y or mesg_w ~= new_mesg_w then
      mesg_x = new_mesg_x
      mesg_y = new_mesg_y
      mesg_w = new_mesg_w
      gui.mesgInit( mesg_w, mesg_x, mesg_y )
   end

   --Main window left
   gfx.renderTex( main, mod_x, 0 )
   gui.radarRender( radar_x + mod_x, radar_y )

   if lockons > 0 then
      gfx.renderTex( icon_lockon, 378 + mod_x, 50 )
   end
   if autonav then
      gfx.renderTex( icon_autonav, 246 + mod_x, 52 )
   end

   for k, v in ipairs( bars ) do --bars = { "shield", "armour", "energy", "fuel" }, remember?
      local ht = nil
      local st = nil
      if v == "armour" then
         ht = heat
         st = stress
      end
      renderBar( v, _G[v], _G[v .. "_light"], nil, nil, mod_x, ht, st )
   end

   --Speed Lights
   local nlights = 11
   local value = round( pp:vel():mod() * nlights / stats.speed_max )
   if value > nlights * 2 then value = nlights * 2 end
   for i=1, value do
      if i <= nlights then
         gfx.renderTex( speed_light, pl_speed_x - 5 + mod_x, pl_speed_y - 3 + (i-1)*6 )
      else
         local imod = i % nlights
         gfx.renderTex( speed_light_double, pl_speed_x - 5 + mod_x, pl_speed_y - 3 + (imod-1)*6 )
      end
   end
   if value < nlights then
      for i=value+1, nlights do
      gfx.renderTex( speed_light_off, pl_speed_x + mod_x, pl_speed_y + (i-1)*6 )
      end
   end


   --Popup left
   if ptarget ~= nil then
      ta_detect, ta_scanned = pp:inrange(ptarget)

      if ta_detect then
         gfx.renderTex( popup_pilot, popup_left_x + mod_x, popup_left_y ) --Frame

         --Target Image
         gfx.renderTex( target_bg, target_image_x + mod_x, target_image_y )
         ta_dist = pp:pos():dist(ptarget:pos())
         if ta_scanned then
            if ta_dir ~= ptarget:dir() then
               update_target()
            end
            ta_armour, ta_shield, ta_stress = ptarget:health()
            ta_heat = math.max( math.min( (ptarget:temp() - 250)/87.5, 2 ), 0 )
            ta_energy = ptarget:energy()
            ta_name = ptarget:name()
            gfx.renderTexRaw( ta_gfx, target_image_x + target_image_w/2 - ta_gfx_draw_w/2 + mod_x, target_image_y + target_image_h/2 - ta_gfx_draw_h/2, ta_gfx_draw_w, ta_gfx_draw_h, ta_sx, ta_sy, 0, 0, 1, 1 )
            renderBar( "shield", ta_shield, false, false, "target", mod_x )
            renderBar( "armour", ta_armour, false, false, "target", mod_x, ta_heat, ta_stress )
            renderBar( "energy", ta_energy, false, false, "target", mod_x )
            renderField( ta_name, x_name + mod_x, y_name, 86, col_text )
            renderField( tostring( math.floor(ta_dist) ), x_dist + mod_x, y_dist, 86,col_text )
         else
            gfx.renderTex( question, target_image_x + target_image_w/2 - question_w/2 + mod_x, target_image_y + target_image_h/2 - question_h/2 )
            renderBar( "shield", 0, false, true, "target", mod_x )
            renderBar( "armour", 0, false, true, "target", mod_x )
            renderBar( "energy", 0, false, true, "target", mod_x )
            renderField( _("Unknown"), x_name + mod_x, y_name, 86, col_unkn )
            renderField( tostring( math.floor(ta_dist) ), x_dist + mod_x, y_dist, 86, col_text )
         end

         gfx.renderTex( target_frame, target_image_x + mod_x, target_image_y )
         gfx.renderTex( target_sheen, target_image_x + 3 + mod_x, target_image_y + 32 )

         --Speed Lights
         local nlights = 7
         local value = round( ptarget:vel():mod() * nlights / ta_stats.speed_max )
         if value > nlights * 2 then value = nlights * 2 end
         for i=1, value do
            if i <= nlights then
               gfx.renderTex( speed_light, x_speed - 5 + mod_x, y_speed - 3 + (i-1)*6 )
            else
               local imod = i % nlights
               gfx.renderTex( speed_light_double, pl_speed_x - 5 + mod_x, pl_speed_y - 3 + (imod-1)*6 )
            end
         end
         if value < nlights then
            for i=value+1, nlights do
               gfx.renderTex( speed_light_off, x_speed + mod_x, y_speed + (i-1)*6 )
            end
         end
      else
         gfx.renderTex( popup_empty, popup_left_ + mod_xx, popup_left_y )
      end
   else
      gfx.renderTex( popup_empty, popup_left_x + mod_x, popup_left_y )
   end
   gfx.renderTex( popup_bottom, popup_left_x + mod_x, popup_left_y - 5 )

   --Top Bar
   gfx.renderTexRaw( top_bar, 0, tbar_y, screen_w, tbar_h, 1, 1, 0, 0, 1, 1 )

   if nav_pnt ~= nil then
      renderField( nav_pnt:name(), fields_x + 4, fields_y, fields_w, col_text, icon_pnt_target )
   else
      renderField( "None", fields_x + 4, fields_y, fields_w, col_unkn, icon_pnt_target )
   end
   if autonav_hyp ~= nil then
      renderField( autonav_hyp:name() .. " (" .. tostring(autonav_hyp:jumpDist()) .. ")", fields_x + fields_w + 12, fields_y, fields_w, col_text, icon_nav_target )
   else
      renderField( _("None"), fields_x + fields_w + 12, fields_y, fields_w, col_unkn, icon_nav_target )
   end
   renderField( credits_h, tbar_center_x + tbar_center_w/2 + 4, fields_y, fields_w, col_text, icon_money )
   renderField( tostring(cargo) .. "t", tbar_center_x + tbar_center_w/2 + fields_w + 12, fields_y, fields_w, col_text, icon_cargo )

   --Center
   gfx.renderTex( top_bar_center, tbar_center_x - tbar_center_w/2, screen_h - tbar_center_h )

   --Time
   local time_str = time.str(time.get())
   local time_str_w = gfx.printDim(false, time_str)
   gfx.print( false, time_str, screen_w/2 - 78, screen_h - tbar_center_h + 55, col_text, 156, true )
   gfx.renderTex( top_bar_center_sheen, screen_w/2 - 77, screen_h - 56 )

   --System name
   local sysname = system.cur():name()
   local sysname_w = gfx.printDim(false, sysname)
   gfx.print( false, sysname, screen_w/2 - 67, screen_h - tbar_center_h + 19, col_text, 132, true )
   gfx.renderTex( top_bar_center_sheen2, screen_w/2 - 66, screen_h - 92 )

   for k, v in ipairs(buttontypes) do
      renderButton( v )
   end

   -- Formation selection button
   if #pp:followers() ~= 0 then
      local x = 0
      local _, height = field_frame_center:dim()
      local width = gfx.printDim(false, "Set formation")
      local y = tbar_y - height

      if buttons["formation"] == nil then
          buttons["formation"] = {}
      end

      local button = buttons["formation"]
      button.x = x
      button.y = y
      button.w = width
      button.h = height
      button.action = playerform

      local col = col_text
      if button.state == "mouseover" then
          col = col_lgray
      end

      renderField( "Set formation", x, y, width, col )
   end

   -- Planet pane
   if nav_pnt then
      ta_pnt_dist = pp:pos():dist( planet.pos )

      -- Extend the pane depending on the services available.
      services_h = 44
      if pntflags.land then
         services_h = services_h + (14 * planet.nservices)
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
      gfx.print( true, _("TARGETED"), ta_pnt_pane_x + 14, ta_pnt_pane_y + 164, col_text )
      gfx.print( true, _("DISTANCE:"), ta_pnt_pane_x + 35, ta_pnt_pane_y - 14, col_text )
      gfx.print( true, _("CLASS:"), ta_pnt_pane_x + 14, ta_pnt_pane_y - 34, col_text )

      if ta_pnt_faction_gfx then
         gfx.renderTex( ta_pnt_faction_gfx, ta_pnt_fact_x, ta_pnt_fact_y )
      end

      x1, y1 = vec2.get(planet.pos)
      x2, y2 = vec2.get(player.pilot():pos())
      ta_pnt_dir = math.atan2(y2 - y1, x2 - x1) + math.pi

      gfx.print( true, planet.class, ta_pnt_pane_x + 130, ta_pnt_pane_y - 34, col_text )
      gfx.print( true, _("SERVICES:"), ta_pnt_pane_x + 14, ta_pnt_pane_y - 46, col_text )

      -- Space out the text.
      services_h = 60
      if pntflags.land then
         local services_h = 60
         for k,v in ipairs(planet.services) do
            gfx.print(true, v, ta_pnt_pane_x + 60, ta_pnt_pane_y - services_h, col_text )
            services_h = services_h + 14
         end
      else
         gfx.print( true, _("none"), ta_pnt_pane_x + 110, ta_pnt_pane_y - 46, col_text )
      end

      gfx.print( false, largeNumber( ta_pnt_dist, 1 ), ta_pnt_pane_x + 110, ta_pnt_pane_y - 15, col_text, 63, false )
      gfx.print( true, planet.name, ta_pnt_pane_x + 14, ta_pnt_pane_y + 149, planet.col )

      gfx.renderTex( top_bar_center_sheen, ta_pnt_pane_x + 11, ta_pnt_pane_y + ta_pnt_pane_h - 15 )
   end
end

function mouse_click( button, x, y, state )
   if button ~= 2 then
      return false
   else
      lmouse = state
      pressed = mouseInsideButton( x, y )

      if pressed == nil then
         if not state then
            for k, v in pairs(buttons) do
               if v.state ~= "disabled" and v.state ~= "hilighted" then
                  v.state = "default"
               end
            end
         end
         return false
      else
         if state then
            if pressed.state ~= "disabled" then
               pressed.state = "pressed"
            end
            return true
         else
            if pressed.state ~= "disabled" then
               pressed.state = "default"
               pressed.action()
            end
            return true
         end
      end
   end
end

function mouse_move( x, y )
   pressed = mouseInsideButton( x, y )
   if pressed ~= nil then
      if pressed.state ~= "disabled" and not lmouse then
         pressed.state = "mouseover"
      elseif pressed.state ~= "disabled" and lmouse then
         pressed.state = "pressed"
      end
   else
      for k, v in pairs(buttons) do
         if v.state ~= "disabled" and v.state ~= "hilighted" then
            v.state = "default"
         end
      end
   end
end

function mouseInsideButton( x, y )
   for k, v in pairs(buttons) do
      if x > v.x and x < v.x+v.w and y > v.y and y < v.y+v.h then
         return v
      end
   end
   return nil
end

function action_missions()
   gui.menuInfo( "missions" )
end

function action_cargo()
   gui.menuInfo( "cargo" )
end

function action_ship()
   gui.menuInfo( "ship" )
end

function action_weapons()
   gui.menuInfo( "weapons" )
end

function render_cooldown( percent, seconds )
   local msg = _("Cooling down...\n%.1f seconds remaining"):format( seconds )
   local fail = true
   if cooldown_omsg ~= nil then
      if player.omsgChange( cooldown_omsg, msg, 1 ) then
         fail = false
      end
   end
   if fail then
      cooldown_omsg = player.omsgAdd( msg, 1 )
   end
end
