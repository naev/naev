--[[
   The revamped version of the slim GUI
]]--

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
   gui.viewport( 0, 0, screen_w, screen_h )

   --Colors
   col_txt_std = colour.new( 192/255, 198/255, 217/255 )
   col_txt_wrn = colour.new( 127/255,  31/255,  31/255 )
   col_txt_enm = colour.new( 222/255,  28/255,  28/255 )
   col_txt_all = colour.new(  19/255, 152/255,  41/255 )
   bar_shield_col = colour.new( 40/255,  51/255,  88/255 )
   bar_armour_col = colour.new( 72/255,  73/255,  60/255 )
   bar_energy_col = colour.new( 41/255,  92/255,  47/255 )
   bar_speed_col = colour.new( 77/255,  80/255,  21/255 )
   bar_speed2_col = colour.new( 169/255,177/255,  46/255 )
   bar_heat_col = colour.new(114/255,26/255, 14/255 )
   bar_heat2_col = colour.new( 222/255, 51/255, 27/255 )
   bar_stress_col = colour.new( 42/255,  43/255,  120/255 )
   bar_fuel_col = colour.new( 92/255, 41/255, 41/255 )
   col_black = colour.new( 0, 0, 0 )
   col_slot_bg = colour.new( 12/255, 14/255, 20/255 )
   col_slot_heat = colour.new( 108/255, 25/255, 13/255, 200/255 )
   
   --Load Images
   local base = "gfx/gui/slimv2/"
   player_pane = tex.open( base .. "main.png" )
   bar_sheen = tex.open( base .. "sheen.png" )
   bar_bg = tex.open( base .. "bar_bg.png" )
   bar_armour_icon = tex.open( base .. "armour.png" )
   bar_energy_icon = tex.open( base .. "energy.png" )
   bar_fuel_icon = tex.open( base .. "fuel.png" )
   bar_heat_icon = tex.open( base .. "heat.png" )
   bar_shield_icon = tex.open( base .. "shield.png" )
   bar_speed_icon = tex.open( base .. "speed.png" )
   slotA = tex.open( base .. "slot1-3.png" )
   slotAend = tex.open( base .. "slot1-3end.png" )
   slotB = tex.open( base .. "slot4.png" )
   slotBend = tex.open( base .. "slot4end.png" )
   slotC = tex.open( base .. "slot.png" )
   slotCend = tex.open( base .. "slotend.png" )
   slotAe = tex.open( base .. "slot1e.png" )
   slotBe = tex.open( base .. "slot2-3e.png" )
   slotCe = tex.open( base .. "slot4e.png" )
   warnlight1 = tex.open( base .. "warnlight1.png" )
   warnlight2 = tex.open( base .. "warnlight2.png" )
   warnlight3 = tex.open( base .. "warnlight3.png" )
   cooldown = tex.open( base .. "cooldown.png", 6, 6 )
   lockonA = tex.open( base .. "padlockA.png" )
   lockonB = tex.open( base .. "padlockB.png" )
   active =  tex.open( base .. "active.png" )
   
   gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" ) )
   gui.targetPilotGFX(  tex.open( base .. "radar_ship.png" ) )
   
   --Get positions
   --Radar
   radar_w = 126
   radar_h = 118
   radar_x = screen_w/2 - radar_w/2
   radar_y = 5
   gui.radarInit( false, radar_w, radar_h )
   
   --Player pane
   pl_pane_w, pl_pane_h = player_pane:dim()
   pl_pane_x = screen_w/2 - pl_pane_w/2
   pl_pane_y = 0
   
   slot_w, slot_h = slotA:dim()
   max_slots = math.floor(screen_w/2 / slot_w)
   slot_start_x = screen_w/2 + radar_w/2 + 5
   
   slote_y = slot_h - 2
   
   slot_img_offs_x = 1
   slot_img_offs_y = 5
   
   slot_txt_offs_x = slot_img_offs_x + 24
   slot_txt_offs_y = 9
   slot_txt_w = 40
   
   slot_img_w = 64
   
   lockon_w, lockon_h = lockonA:dim()
   
   slotA_w, slotA_h = slotA:dim()
   slotAend_w, slotAend_h = slotAend:dim()
   slotB_w, slotB_h = slotB:dim()
   slotBend_w, slotBend_h = slotBend:dim()
   slotC_w, slotC_h = slotC:dim()
   slotCend_w, slotCend_h = slotCend:dim()
   slotAe_w, slotAe_h = slotAe:dim()
   slotBe_w, slotBe_h = slotBe:dim()
   slotCe_w, slotCe_h = slotCe:dim()

   --Bars
   bar_w = 86
   bar_h = 22
   
   bar_bg_w, bar_bg_h = bar_bg:dim()
   
   bar_armour_x = pl_pane_x + 127
   bar_armour_y = 103
   
   bar_shield_x = bar_armour_x-- Missile lock warning
   missile_lock_text = "Warning - Missile Lock-on Detected"
   missile_lock_length = gfx.printDim( false, missile_lock_text )
   bar_shield_y = 75
   
   bar_fuel_x = pl_pane_x + 7
   bar_fuel_y = bar_shield_y
   
   bar_energy_x = pl_pane_x + 409
   bar_energy_y = bar_armour_y
   
   bar_heat_x = bar_energy_x
   bar_heat_y = bar_shield_y
   
   bar_speed_x = pl_pane_x + 529
   bar_speed_y = bar_shield_y

   -- Cooldown pane.
   cooldown_sheen = tex.open( "gfx/gui/slim/cooldown-sheen.png" )
   cooldown_bg = tex.open( "gfx/gui/slim/cooldown-bg.png" )
   cooldown_frame = tex.open( "gfx/gui/slim/cooldown-frame.png" )
   cooldown_panel = tex.open( "gfx/gui/slim/cooldown-panel.png" )
   cooldown_frame_w, cooldown_frame_h = cooldown_frame:dim()
   cooldown_frame_x = (screen_w - cooldown_frame_w)/2.
   cooldown_frame_y = (screen_h - cooldown_frame_h)/2.
   cooldown_panel_x = cooldown_frame_x + 8
   cooldown_panel_y = cooldown_frame_y + 8
   cooldown_bg_x = cooldown_panel_x + 30
   cooldown_bg_y = cooldown_panel_y + 2
   cooldown_bg_w, cooldown_bg_h = cooldown_bg:dim()
   cooldown_sheen_x = cooldown_bg_x
   cooldown_sheen_y = cooldown_bg_y + 12
   
   -- Missile lock warning
   missile_lock_text = "Warning - Missile Lock-on Detected"
   missile_lock_length = gfx.printDim( false, missile_lock_text )
   
   --Lock-on warning light
   warning_lockon_c_x = pl_pane_x + 125 + 9
   warning_lockon_c_y = 130 + 9
   
   --Autonav warning light
   warning_autonav_c_x = pl_pane_x + 145 + 9
   warning_autonav_c_y = warning_lockon_c_y
   
   --Low armour warning light
   warning_armour_c_x = pl_pane_x + 167 + 16
   warning_armour_c_y = warning_lockon_c_y
   
   -- Set FPS
   gui.fpsPos( 20, screen_h - 20 - deffont_h )

   -- Set OSD
   gui.osdInit( 20, screen_h - 63, 150, 500 )
   
   --Credits
   credits_x = screen_w/2 - 86
   credits_y = 134
   credits_w = 52
   
   --Cargo
   cargo_x = screen_w/2 -2
   cargo_y = 134
   cargo_w = 38
   
   --Time
   time_x = screen_w/2 + 67
   time_y = 134
   time_w = 114
   
   
   --Messages
   gui.mesgInit( 600, 20, pl_pane_h + 20 )
   
   -- Timer stuff
   timers = {}
   timers[1] = 0.5
   blinkcol = col_txt_enm
   
   update_target()
   update_ship()
   --update_system()
   --update_nav()
   --update_faction()
   update_cargo()
   update_wset()
end

function update_ship()
   stats = pp:stats()
end

function update_target()
   ptarget = pp:target()
end

function update_system()
end

function update_nav()
end

function update_faction()
end

function update_cargo()
   cargo_free = tostring(pp:cargoFree()) .. "t"
end

function update_wset()
   wset_name, wset  = pp:weapset()
   weap_icons = {}
   
   for k, v in ipairs( wset ) do
      if k > max_slots then
         break
      end
      weap_icons[k] = outfit.get( v.name ):icon()
   end
   
   aset = pp:actives()
   active_icons = {}
   
   for k, v in ipairs( aset ) do
      if k > max_slots then
         break
      end
      active_icons[k] = outfit.get( v.name ):icon()
    end
end


function render_cooldown( percent, seconds )
   gfx.renderTex( cooldown_frame, cooldown_frame_x, cooldown_frame_y )
   gfx.renderTex( cooldown_bg, cooldown_bg_x, cooldown_bg_y )
   gfx.renderRect( cooldown_bg_x, cooldown_bg_y, percent * cooldown_bg_w, cooldown_bg_h, bar_heat_col )
   gfx.renderTex( cooldown_sheen, cooldown_sheen_x, cooldown_sheen_y )
   gfx.renderTex( cooldown_panel, cooldown_panel_x, cooldown_panel_y )
   gfx.print(false, "Cooling down...", cooldown_frame_x,
         cooldown_bg_y + cooldown_bg_h + 8, col_txt_std, cooldown_frame_w, true )
end


function render_bar( left, name, value, text, txtcol, stress )
   --stress is only used for armour
   --Get values
   values = { "x", "y", "icon", "col" }
   for k, v in ipairs(values) do
      _G[ "s_" .. v ] = _G[ "bar_" .. name .. "_" .. v ]
   end
   
   if name == "heat" and value > 0.8 then
      s_col = bar_heat2_col
   elseif name == "speed" and value > 1. then
      gfx.renderRect( s_x, s_y, bar_w, bar_h, bar_speed_col )
      s_col = bar_speed2_col
      value = value - 1
   end
   
   if value > 1. then
      value = 1
   end
   
   --Draw bar
   if name ~= "armour" then
      if left then
         gfx.renderRect( s_x + bar_w * (1-value), s_y, bar_w * value, bar_h, s_col )
      else
         gfx.renderRect( s_x, s_y, bar_w * value, bar_h, s_col )
      end
   else
      gfx.renderRect( s_x + bar_w * (1-value), s_y, bar_w * value, bar_h, s_col )
      gfx.renderRect( s_x + bar_w * (1-stress), s_y, bar_w * stress, bar_h, bar_stress_col )
   end
   --Draw border
   scal = 1
   bg_x = s_x - 30
   if left then
      scal = -1
      bg_x = s_x + bar_w + 30
   end
   gfx.renderTexRaw( bar_bg, bg_x, s_y-2, bar_bg_w * scal, bar_bg_h, 1, 1, 0, 0, 1, 1 )
   --Icon
   ic_w, ic_h = s_icon:dim()
   ic_c_x = s_x - 15
   if left then
      ic_c_x = s_x + bar_w + 15
   end
   ic_c_y = s_y + bar_h/2
   gfx.renderTex( s_icon, math.floor(ic_c_x - ic_w/2), math.floor(ic_c_y - ic_h/2), 1, 1)
   --Sheen
   gfx.renderTex( bar_sheen, s_x+1, s_y+13, 1, 1)
   --Text
   small = false
   if gfx.printDim(small, text) >= bar_w then
      small = true
   end
   gfx.print( small, text, s_x, s_y + 6, txtcol, bar_w, true )
end

function largeNumber( number, idp )
   local formatted
   local units = { "k", "M", "G", "T", "P" }
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

function render( dt, dt_mod )
   
   --Values
   armour, shield, stress = pp:health()
   energy = pp:energy()
   speed = pp:vel():dist()
   heat = pp:temp()
   fuel, consumption = player.fuel()
   fuel_max = pp:stats().fuel_max
   jumps = player.jumps()
   lockons = pp:lockon()
   autonav = player.autonav()
   credits = player.credits()
   update_wset() --Rather hacky, waiting for fix
   
   --Radar
   gfx.renderRect( radar_x, radar_y, radar_w, radar_h, col_black )
   gui.radarRender( radar_x, radar_y )
   
   gfx.renderTex( player_pane, pl_pane_x, pl_pane_y, 1, 1 )
   
   --Slots
   --Right side
   local i = 1
   while i <= math.max( 4, math.min( max_slots, #wset )) do
      
      local slot_x = slot_start_x + (i-1) * slot_w
      if i <= #wset then
         --There is something in this slot
         gfx.renderRect( slot_x, 0, slot_w, slot_h, col_slot_bg ) --Background
         
         gfx.renderTexRaw( weap_icons[i], slot_x + slot_img_offs_x, slot_img_offs_y, slot_img_w, slot_img_w, 1, 1, 0, 0, 1, 1 ) --Image 
         
         if wset[i].temp > 0 then
            gfx.renderRect( slot_x + slot_img_offs_x, slot_img_offs_y, slot_img_w, slot_img_w * wset[i].temp/2, col_slot_heat ) --Heat
         end
         
         --Cooldown
         local coolinglevel = wset[i].cooldown
         if wset[i].charge then
            coolinglevel = wset[i].charge
         end
         if coolinglevel ~= nil and coolinglevel < 1. then
            local texnum = round((1-coolinglevel)*35) --Turn the 0..1 cooldown number into a 0..35 tex id where 0 is ready. Also, reversed
            gfx.renderTex( cooldown, slot_x + slot_img_offs_x, slot_img_offs_y, (texnum % 6) + 1, math.floor( texnum / 6 ) + 1 )
            
            --A strange thing: The texture at 6,6 is never drawn, the one at 5,6 only about 50% of the time. Otherwise, they're skipped
            --is this an error in my code or bobbens' ?
         end
         
         --Ammo
         if wset[i].left then
            local txtcol = col_txt_std
            if wset[i].left_p <= .2 then
               txtcol = col_txt_wrn
            end
            gfx.print( true, tostring( wset[i].left), slot_x + slot_txt_offs_x, slot_txt_offs_y, txtcol, slot_txt_w, true )
         end
         
         --Lock-on
         if wset[i].lockon ~= nil and ptarget and wset[i].lockon > 0. then
            if wset[i].lockon < 1. then
               local iconcol = colour.new( 1, 1, 1, wset[i].lockon )
               gfx.renderTex( lockonA, slot_x + slot_img_offs_x + slot_img_w/2 - lockon_w/2, slot_img_offs_y + slot_img_w/2 - lockon_h/2, 1, 1, iconcol )
            else
               gfx.renderTex( lockonB, slot_x + slot_img_offs_x + slot_img_w/2 - lockon_w/2, slot_img_offs_y + slot_img_w/2 - lockon_h/2, 1, 1 )
            end
         end
         
         
         --Frame
         local postfix = ""
         if i+1 > #wset then
            postfix = "end"
         end
         if i <= 3 then
            gfx.renderTex( _G["slotA" .. postfix], slot_x, 0, 1, 1 )
         elseif i == 4 then
            gfx.renderTex( _G["slotB" .. postfix], slot_x, 0, 1, 1 )
         else
            gfx.renderTex( _G["slotC" .. postfix], slot_x, 0, 1, 1 )
         end
               
      else
         if i == 1 then
            gfx.renderTex( slotAe, slot_x, 0, 1, 1 )
         elseif i <= 3 then
            gfx.renderTex( slotBe, slot_x, slote_y, 1, 1 )
         else
            gfx.renderTex( slotCe, slot_x, slote_y, 1, 1 )
         end
      end
      i = i + 1
   end
   
   --Left side
   i = 1
   while i <= math.max( 4, math.min( max_slots, #aset )) do
   
      local slot_x = screen_w - slot_start_x - i * slot_w
      if i <= #aset then
         --There is something in this slot
         gfx.renderRect( slot_x, 0, slot_w, slot_h, col_slot_bg ) --Background

         -- Draw a heat background for certain outfits. TODO: detect if the outfit is heat based somehow!
         if aset[i].type == "Afterburner" then
            gfx.renderRect( slot_x + slot_img_offs_x, slot_img_offs_y, slot_img_w, slot_img_w * aset[i].temp, col_slot_heat ) -- Background (heat)
         end

         gfx.renderTexRaw( active_icons[i], slot_x + slot_img_offs_x, slot_img_offs_y, slot_img_w, slot_img_w, 1, 1, 0, 0, 1, 1 ) --Image 

         if aset[i].state == "on" then
            gfx.renderTex( active, slot_x + slot_img_offs_x, slot_img_offs_y )
         elseif aset[i].state == "cooldown" then
         --Cooldown
            local texnum = round(aset[i].cooldown*35) --Turn the 0..1 cooldown number into a 0..35 tex id where 0 is ready.
            gfx.renderTex( cooldown, slot_x + slot_img_offs_x, slot_img_offs_y, (texnum % 6) + 1, math.floor( texnum / 6 ) + 1 )
            
            --A strange thing: The texture at 6,6 is never drawn, the one at 5,6 only about 50% of the time. Otherwise, they're skipped
            --is this an error in my code or bobbens' ?
         elseif aset[i].state == "on" then
            --"Heat"
            gfx.renderRect( slot_x + slot_img_offs_x, slot_img_offs_y, slot_img_w, slot_img_w * (1-aset[i].duration), col_slot_heat )
         end
         
         --Frame
         local postfix = ""
         if i+1 > #aset then
            postfix = "end"
         end
         if i <= 3 then
            gfx.renderTexRaw( _G["slotA" .. postfix], slot_x + slot_w, 0, -1*_G["slotA"..postfix.."_w"], _G["slotA"..postfix.."_h"], 1, 1, 0, 0, 1, 1 )
         elseif i == 4 then
            gfx.renderTexRaw( _G["slotB" .. postfix], slot_x + slot_w, 0, -1*_G["slotB"..postfix.."_w"], _G["slotB"..postfix.."_h"], 1, 1, 0, 0, 1, 1 )
         else
            gfx.renderTexRaw( _G["slotC" .. postfix], slot_x + slot_w, 0, -1*_G["slotC"..postfix.."_w"], _G["slotC"..postfix.."_h"], 1, 1, 0, 0, 1, 1 )
         end
               
      else
         if i == 1 then
            gfx.renderTexRaw( slotAe, slot_x + slot_w, 0, -1*slotAe_w, slotAe_h, 1, 1, 0, 0, 1, 1 )
         elseif i <= 3 then
            gfx.renderTex( slotBe, slot_x, slote_y, 1, 1 )
         else
            gfx.renderTexRaw( slotCe, slot_x + slot_w, 0, -1*slotCe_w, slotCe_h, 1, 1, 0, 0, 1, 1 )
         end
      end
      i = i + 1
   end


   --Bars
   --Fuel
   txt = tostring(round( fuel )) .. " (" .. tostring(jumps) .. " jumps)"
   col = col_txt_std
   if jumps == 1 then
      col = col_txt_wrn
   elseif fuel == 0. then
      col = col_txt_enm
   end
   render_bar( true, "fuel", fuel/fuel_max, txt, col )
   
   --Armour
   txt = string.format( "%s%% (%s)", round( armour ), round( stats.armour * armour / 100 ) )
   col = col_txt_std
   if armour <= 20. then
      col = col_txt_enm
   end
   render_bar( true, "armour", armour/100, txt, col, stress/100 )
   
   --Shield
   txt = string.format( "%s%% (%s)", round( shield ), round( stats.shield * shield / 100 ) )
   col = col_txt_std
   if shield <= 20. then
      col = col_txt_wrn
   elseif shield == 0. then
      col = col_txt_enm
   end
   render_bar( true, "shield", shield/100, txt, col )
   
   --Energy
   txt = string.format( "%s%% (%s)", round( energy ), round( stats.energy * energy / 100 ) )
   col = col_txt_std
   if energy <= 20. then
      col = col_txt_wrn
   elseif energy == 0. then
      col = col_txt_enm
   end
   render_bar( false, "energy", energy/100, txt, col )
   
   --Heat
   txt = round(heat) .. "K"
   heat = math.max( math.min( (heat - 250)/1.75, 100 ), 0 )
   col = col_txt_std
   if heat >= 80. then
      col = col_txt_wrn
   elseif heat == 100. then
      col = col_txt_enm
   end
   render_bar( false, "heat", heat/100, txt, col )
   
   --Speed
   local hspeed
   if stats.speed_max <= 0 then hspeed = 0
   else hspeed = round(speed / stats.speed_max * 100) end
   txt = tostring( hspeed ) .. "% (" .. tostring( round(speed)) .. ")"
   col = col_txt_std
   if hspeed >= 200. then
      timers[1] = timers[1] - dt / dt_mod
      if timers[1] <=0. then
         timers[1] = 0.5
         if blinkcol == col_txt_std then
            blinkcol = col_txt_enm
         else
            blinkcol = col_txt_std
         end
      end
      col = blinkcol
   elseif hspeed >= 101. then
      col = col_txt_wrn
   end
   render_bar( false, "speed", hspeed/100, txt, col )
   
   --Cargo
   gfx.print( true, cargo_free, cargo_x, cargo_y, col_txt_std, cargo_w, true)
   
   --Money
   gfx.print( true, largeNumber( player.credits(), 2 ), credits_x, credits_y, col_txt_std, credits_w, true )
   
   --Time
   gfx.print( true, time.str( time.get(), 2 ), time_x, time_y, col_txt_std, time_w, true )
   
end
