--[[
   The new "slim" GUI
--]]
local flow = require "ships.lua.lib.flow"
local fmt = require "format"
local playerform = require "playerform"

local radar_gfx, radar_x, radar_y, screen_h, screen_w
local aset, cargo, nav_spob, nav_pnt, pp, ptarget, slot_w, slot_h, slot_y, timers
local ta_pane_w, ta_pane_x, ta_pane_y, ta_pnt_pane_x, ta_pnt_pane_y, pl_pane_x, pl_pane_y
-- This script has a lot of globals. It really loves them.
-- The below variables aren't part of the GUI API and aren't accessed via _G:
-- luacheck: globals active active_icons aset bardata bar_offsets bar_ready_h bar_ready_w bars bar_weapon_h bar_weapon_w blinkcol bottom_bar buttons cargofree cargofreel cargo_light_off cargo_light_on cargoterml cooldown cooldown_bg cooldown_bg_h cooldown_bg_w cooldown_bg_x cooldown_bg_y cooldown_frame cooldown_frame_h cooldown_frame_w cooldown_frame_x cooldown_frame_y cooldown_panel cooldown_panel_x cooldown_panel_y cooldown_sheen cooldown_sheen_x cooldown_sheen_y deffont_h gfxWarn lmouse missile_lock_length missile_lock_text nav_hyp navstring planet_bg planet_pane_b planet_pane_m planet_pane_t player_pane_b player_pane_m player_pane_t pl_pane_h_b pl_pane_w_b pname pntflags ptarget_faction_gfx ptarget_gfx ptarget_gfx_draw_h ptarget_gfx_draw_w ptarget_gfx_h ptarget_gfx_w ptarget_target question sheen sheen_sm sheen_tiny sheen_weapon slot slotend slotend_h slotend_w slot_img_offs_x slot_img_offs_y slot_img_w slot_start_x stats sysname ta_cargo ta_cargo_x ta_cargo_y ta_center_x ta_center_y ta_fact_x ta_fact_y ta_image_x ta_image_y ta_pane_h ta_pnt_center_x ta_pnt_center_y ta_pnt_faction_gfx ta_pnt_fact_x ta_pnt_fact_y ta_pnt_gfx ta_pnt_gfx_draw_h ta_pnt_gfx_draw_w ta_pnt_gfx_h ta_pnt_gfx_w ta_pnt_image_x ta_pnt_image_y ta_pnt_pane_h ta_pnt_pane_h_b ta_pnt_pane_w ta_pnt_pane_w_b ta_question_h ta_question_w target_bg target_dir target_light_off target_light_on target_pane ta_speed ta_stats ta_warning_x ta_warning_y tflags track_h tracking_light track_w warnlight1 warnlight2 warnlight3 warnlight4 warnlight5 wset wset_name x_ammo y_ammo
-- Unfortunately, it is an error to make any function a closure over more than 60 variables.
-- Caution: the below **are** accessed via _G.
-- luacheck: globals armour energy shield speed stress (_G[v])
local fps_y = 32

-- Namespaces
local bgs = {}
local cols = {}
local icons = {}
local has_flow
function create()

   --Get player
   pp = player.pilot()
   pname = player.name()

   --Get sizes
   screen_w, screen_h = gfx.dim()
   deffont_h = gfx.fontSize()
   gui.viewport( 0, 28, screen_w, screen_h - 28 )

   --Colors
   cols.txt_bar = colour.new( 192/255, 198/255, 217/255 )
   cols.txt_top = colour.new( 148/255, 158/255, 192/255 )
   cols.txt_std = colour.new( 111/255, 125/255, 169/255 )
   cols.txt_wrn = colour.new( 127/255,  31/255,  31/255 )
   cols.txt_enm = colour.new( 222/255,  28/255,  28/255 )
   --cols.txt_res = colour.new(     1.0,     0.6,     0.0 )
   cols.txt_una = colour.new(  66/255,  72/255,  84/255 )
   cols.shield = colour.new( 40/255,  51/255,  88/255 )
   cols.armour = colour.new( 72/255,  73/255,  60/255 )
   cols.stress = colour.new( 42/255,  43/255,  120/255 )
   cols.energy = colour.new( 41/255,  92/255,  47/255 )
   cols.speed = colour.new( 77/255,  80/255,  21/255 )
   cols.speed2 = colour.new(169/255,177/255,  46/255 )
   cols.ammo = colour.new(140/255,94/255,  7/255 )
   cols.heat = colour.new(114/255,26/255, 14/255 )
   cols.heat2 = colour.new( 222/255, 51/255, 27/255 )
   cols.afb = colour.new(cols.heat)
   cols.afb:setAlpha(.5)
   cols.ready = colour.new( 14/255,108/255, 114/255 )
   cols.temperature = cols.heat
   cols.flow = colour.new( 189/255, 166/255, 85/255 )
   cols.missile = colour.new(cols.txt_enm)

   -- Active outfit bar
   cols.slot_bg = colour.new( 12/255, 14/255, 20/255 )

   --Load Images
   local base = "gfx/gui/slim/"
   local function tex_open( name, sx, sy )
      local t = tex.open( base .. name, sx, sy )
      t:setWrap( "clamp" )
      return t
   end
   player_pane_t = tex_open( "frame_player_top.png" )
   player_pane_m = tex_open( "frame_player_middle.webp" )
   player_pane_b = tex_open( "frame_player_bottom.png" )
   target_pane = tex_open( "frame_target.png" )
   planet_pane_t = tex_open( "frame_planet_top.png" )
   planet_pane_m = tex_open( "frame_planet_middle.webp" )
   planet_pane_b = tex_open( "frame_planet_bottom.png" )
   radar_gfx = tex_open( "radar.png" )
   target_bg = tex_open( "target_image.png" )
   planet_bg = tex_open( "planet_image.png" )
   icons.shield = tex_open( "shield.png" )
   icons.armour = tex_open( "armour.png" )
   icons.energy = tex_open( "energy.webp" )
   icons.speed = tex_open( "speed.png" )
   icons.temperature = tex_open( "heat.png" )
   icons.flow = tex_open( "energy.webp" ) -- TODO
   icons.shield_sm = tex_open( "shield_sm.png" )
   icons.armour_sm = tex_open( "armour_sm.png" )
   icons.energy_sm = tex_open( "energy_sm.png" )
   icons.speed_sm = tex_open( "speed_sm.png" )
   bgs.bar = tex_open( "bg_bar.png" )
   bgs.bar_sm = tex_open( "bg_bar_sm.png" )
   bgs.bar_weapon = tex_open( "bg_bar_weapon.png" )
   bgs.bar_weapon_prim = tex_open( "bg_bar_weapon_prim.png" )
   bgs.bar_weapon_sec = tex_open( "bg_bar_weapon_sec.png" )
   bgs.shield = tex_open( "bg_shield.png" )
   bgs.armour = tex_open( "bg_armour.png" )
   bgs.energy = tex_open( "bg_energy.png" )
   bgs.speed = tex_open( "bg_speed.png" )
   bgs.temperature = bgs.speed
   bgs.flow = bgs.speed -- TODO
   bgs.ammo = tex_open( "bg_ammo.png" )
   bgs.heat = tex_open( "bg_heat.png" )
   bgs.ready = tex_open( "bg_ready.png" )
   bgs.shield_sm = tex_open( "bg_shield_sm.png" )
   bgs.armour_sm = tex_open( "bg_armour_sm.png" )
   bgs.energy_sm = tex_open( "bg_energy_sm.png" )
   bgs.speed_sm = tex_open( "bg_speed_sm.png" )
   sheen = tex_open( "sheen.png" )
   sheen_sm = tex_open( "sheen_sm.png" )
   sheen_weapon = tex_open( "sheen_weapon.png" )
   sheen_tiny = tex_open( "sheen_tiny.png" )
   bottom_bar = tex_open( "bottombar.png" )
   target_dir = tex_open( "dir.png", 6, 6 )
   warnlight1 = tex_open( "warnlight1.png" )
   warnlight2 = tex_open( "warnlight2.png" )
   warnlight3 = tex_open( "warnlight3.png" )
   warnlight4 = tex_open( "warnlight4.png" )
   warnlight5 = tex_open( "warnlight5.png" )
   tracking_light = tex_open( "track.png" )
   target_light_off = tex_open( "targeted_off.png" )
   target_light_on =  tex_open( "targeted_on.png" )
   cargo_light_off = tex_open( "cargo_off.png" )
   cargo_light_on =  tex_open( "cargo_on.png" )
   question = tex_open( "question.png" )
   gui.targetSpobGFX( tex_open( "radar_planet.png", 2, 2 ) )
   gui.targetPilotGFX(  tex_open( "radar_ship.png", 2, 2 ) )

   -- Active outfit list.
   slot = tex_open( "slot.png" )
   slotend = tex_open( "slotend.png" )
   cooldown = tex_open( "cooldown.png", 6, 6 )
   active =  tex_open( "active.png" )

   -- Active outfit bar
   slot_w, slot_h = slot:dim()
   slot_y = screen_h - slot_h - 16
   slot_img_offs_x = 4
   slot_img_offs_y = 6
   slot_img_w = 48
   slotend_w, slotend_h = slotend:dim()

   -- Cooldown pane.
   cooldown_sheen = tex_open( "cooldown-sheen.png" )
   cooldown_bg = tex_open( "cooldown-bg.png" )
   cooldown_frame = tex_open( "cooldown-frame.png" )
   cooldown_panel = tex_open( "cooldown-panel.png" )
   cooldown_frame_w, cooldown_frame_h = cooldown_frame:dim()
   cooldown_frame_x = (screen_w - cooldown_frame_w)/2.
   cooldown_frame_y = math.min( slot_y - cooldown_frame_h - 10, (screen_h - cooldown_frame_h)/2. + 150 )
   cooldown_panel_x = cooldown_frame_x + 8
   cooldown_panel_y = cooldown_frame_y + 8
   cooldown_bg_x = cooldown_panel_x + 30
   cooldown_bg_y = cooldown_panel_y + 2
   cooldown_bg_w, cooldown_bg_h = cooldown_bg:dim()
   cooldown_sheen_x = cooldown_bg_x
   cooldown_sheen_y = cooldown_bg_y + 12

   --Messages
   gui.mesgInit( screen_w - 400, 20, 28+15+5 )

   --Get positions
   --Player pane
   local pl_pane_w, pl_pane_h = player_pane_t:dim()
   pl_pane_w_b, pl_pane_h_b = player_pane_b:dim()
   pl_pane_x = screen_w - pl_pane_w - 16
   pl_pane_y = screen_h - pl_pane_h - 16

   --Radar
   local radar_w = radar_gfx:dim()
   radar_x = pl_pane_x - radar_w + 24
   radar_y = pl_pane_y + 31
   gui.radarInit( false, 124, 124 )

   --Shield Bar
   local x_shield = pl_pane_x + 46
   local y_shield = pl_pane_y + 137

   has_flow = (flow.max( pp ) > 0)

   bardata = {}

   -- Initialize bar data
   local types = { "shield", "armour", "energy", "speed", "temperature" }
   if has_flow then
      table.insert( types, "flow" )
   end
   for k,v in ipairs(types) do
      local bgw, bgh = bgs[v]:dim()
      bardata[v] = {
         icon = icons[v],
         col  = cols[v],
         bg   = bgs[v],
         x = x_shield,
         y = y_shield - (k-1) * 28,
         w = bgw,
         h = bgh
      }
   end

   bars = { "armour", "energy", "speed", "shield" }

   --Ammo, heat and ready bars
   bar_weapon_w, bar_weapon_h = bgs.ammo:dim()
   bar_ready_w, bar_ready_h = bgs.ready:dim()
   track_w, track_h = tracking_light:dim()
   x_ammo = pl_pane_x + 39
   y_ammo = pl_pane_y - 27

   -- Missile lock warning
   missile_lock_text = _("Warning - Missile Lock-on Detected")
   missile_lock_length = gfx.printDim( false, missile_lock_text )

   --Target Pane
   ta_pane_w, ta_pane_h = target_pane:dim()
   ta_pane_x = screen_w - ta_pane_w - 16
   ta_pane_y = 44

   --Target image background
   ta_image_x = ta_pane_x + 14
   ta_image_y = ta_pane_y + 106
   --Target image center
   local ta_image_w, ta_image_h = target_bg:dim()
   ta_center_x = ta_image_x + ta_image_w / 2
   ta_center_y = ta_image_y + ta_image_h / 2
   -- ? image
   ta_question_w, ta_question_h = question:dim()

   --Target Faction icon center
   ta_fact_x = ta_pane_x + 122
   ta_fact_y = ta_pane_y + 122

   --Small Shield Bar
   local x_shield_sm = ta_pane_x + 13
   local y_shield_sm = ta_pane_y + 71

   -- Initialize small bar data
   local smtypes = { "shield", "armour", "energy", "speed" }
   for k,v in ipairs(smtypes) do
      local bgw, bgh = bgs[v .. "_sm"]:dim()
      bardata[v .. "_sm"] = {
         icon = icons[v .. "_sm"],
         col  = cols[v],
         bg   = bgs[v .. "_sm"],
         x = x_shield_sm,
         y = y_shield_sm - (k-1) * 20,
         w = bgw,
         h = bgh
      }
   end

   bar_offsets = {
      normal = {
         30, -- Bar X, relative to frame
          7, -- Icon X
         15, -- Sheen Y
          6  -- Text Y
      },
      small = { 22, 5, 9, 3 }, -- See above.
      ammo = {
          2, -- Bar
         20, -- Refire indicator Y
          3, -- Sheen
         13, -- Sheen Y
         22, -- Refire sheen Y
          6, -- Text Y
          2, -- Tracking icon X
          5  -- Tracking icon Y
      }
   }

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
   local ta_pnt_image_w, ta_pnt_image_h = planet_bg:dim()
   ta_pnt_center_x = ta_pnt_image_x + ta_pnt_image_w / 2
   ta_pnt_center_y = ta_pnt_image_y + ta_pnt_image_h / 2

   -- Set FPS
   gui.fpsPos( 15, screen_h - fps_y )

   -- Set OSD
   gui.osdInit( 23, screen_h - 63, 150, 500 )

   -- Timer stuff
   timers = {}
   timers[1] = 0.5
   timers[2] = 0.5
   timers[3] = 0.5
   blinkcol = cols.txt_enm
   gfxWarn = true

   buttons = {}

   gui.mouseClickEnable(true)
   gui.mouseMoveEnable(true)

   update_target()
   update_ship()
   update_system()
   update_nav()
   update_faction()
   update_cargo()
   update_effects()
end

function update_target()
   ptarget = pp:target()
   if ptarget then
      --ptarget_gfx = ptarget:ship():gfxTarget()
      ptarget_gfx = ptarget:render():getTex()
      ptarget_gfx_w, ptarget_gfx_h = ptarget_gfx:dim()
      local ptargetfact = ptarget:faction()
      ptarget_target = ptarget:target()
      ta_stats = ptarget:stats()
      ta_cargo = ptarget:cargoList()

      local ptarget_gfx_aspect = ptarget_gfx_w / ptarget_gfx_h
      if math.max( ptarget_gfx_w, ptarget_gfx_h ) > 62 then
         ptarget_gfx_draw_w = math.min( 62, 62 * ptarget_gfx_aspect )
         ptarget_gfx_draw_h = math.min( 62, 62 / ptarget_gfx_aspect )
      end

      if ptargetfact ~= nil and ptargetfact:known() then
         ptarget_faction_gfx = ptargetfact:logo()
      else
         ptarget_faction_gfx = nil
      end
   end
end

function update_nav()
   nav_spob = {}
   nav_pnt, nav_hyp = pp:nav()
   local autonav_hyp, jumps = player.autonavDest()
   if nav_pnt then
      pntflags = nav_pnt:services()
      gui.osdInit( ta_pnt_pane_x + ta_pnt_pane_w + 8, screen_h - 63, 150, 500 )
      gui.fpsPos( ta_pnt_pane_x+ta_pnt_pane_w+3, screen_h - fps_y )

      ta_pnt_gfx = nav_pnt:gfxSpace()
      ta_pnt_gfx_w, ta_pnt_gfx_h = ta_pnt_gfx:dim()
      local ta_pntfact = nav_pnt:faction()

      local ta_pnt_gfx_aspect = ta_pnt_gfx_w / ta_pnt_gfx_h
      if math.max( ta_pnt_gfx_w, ta_pnt_gfx_h ) > 140 then
         ta_pnt_gfx_draw_w = math.min( 140, 140 * ta_pnt_gfx_aspect )
         ta_pnt_gfx_draw_h = math.min( 140, 140 / ta_pnt_gfx_aspect )
      end

      ta_pnt_faction_gfx = nil
      if ta_pntfact and ta_pntfact:known() then
         ta_pnt_faction_gfx = ta_pntfact:logo()
      end

      nav_spob = { -- Table for convenience.
         name = nav_pnt:name(),
         pos = nav_pnt:pos(),
         class = _(nav_pnt:class()),
         col = nav_pnt:colour(),
         services = {}
      }
      nav_spob.class_w = gfx.printDim( nil, nav_spob.class )

      if pntflags.land then
         local services = { "refuel", "bar", "missions", "outfits", "shipyard", "commodity" }

         -- "Spaceport" is nicer than "Land"
         table.insert( nav_spob.services, N_("Spaceport") )
         for k,v in ipairs(services) do
            table.insert( nav_spob.services, pntflags[v] )
         end
         nav_spob.nservices = #nav_spob.services
      end
   else
      gui.osdInit( 23, screen_h - 63, 150, 500 )
      gui.fpsPos( 15, screen_h - fps_y )
   end
   if nav_hyp then
      if nav_hyp:known() then
         navstring = nav_hyp:name()
      else
         navstring = _("Unknown")
      end
      if autonav_hyp then
         navstring = (navstring .. " (%s)"):format( jumps )
      end
   else
      navstring = _("none")
   end
end

function update_faction()
   if nav_pnt then -- Colour the planet name based on friendliness.
      nav_spob.col = nav_pnt:colour()
   end
end

function update_cargo()
   local cargol = pp:cargoList()
   cargofree = string.format( _(" (%s free)"), fmt.tonnes_short( pp:cargoFree() ) )
   cargofreel = gfx.printDim( true, cargofree )
   cargoterml = gfx.printDim( true, ", [...]" )
   cargo = {}
   for k,v in ipairs(cargol) do
      if v.q == 0 then
         cargo[k] = _(v.name)
      else
         cargo[k] = fmt.tonnes_short(v.q) .. " " .. _(v.name)
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
   sysname = system.cur():name()
end

local effects = {}
function update_effects()
   local buff_col = colour.new("Friend")
   local debuff_col = colour.new("Hostile")
   effects = {}
   local effects_added = {}
   for k,e in ipairs(pp:effectGet()) do
      local a = effects_added[ e.name ]
      if not a then
         a = #effects+1
         effects[ a ] = e
         e.n = 0
         effects_added[ e.name ] = a

         if e.buff then
            e.col = buff_col
         elseif e.debuff then
            e.col = debuff_col
         end
      end
      effects[ a ].n = effects[ a ].n + 1
   end
end

local function update_wset()
   wset_name, wset  = pp:weapset()

-- Currently unused.
--[[
   weap_icons = {}

   for k, v in ipairs( wset ) do
      weap_icons[k] = outfit.get( v.name ):icon()
   end
--]]

   aset = pp:actives(true)
   active_icons = {}

   for k, v in ipairs( aset ) do
      active_icons[k] = outfit.get( v.name ):icon()
   end
   slot_start_x = screen_w/2 - #aset/2 * slot_w
end


function render_cooldown( percent, _seconds )
   gfx.renderTex( cooldown_frame, cooldown_frame_x, cooldown_frame_y )
   gfx.renderTex( cooldown_bg, cooldown_bg_x, cooldown_bg_y )
   gfx.renderRect( cooldown_bg_x, cooldown_bg_y, percent * cooldown_bg_w, cooldown_bg_h, cols.temperature )
   gfx.renderTex( cooldown_sheen, cooldown_sheen_x, cooldown_sheen_y )
   gfx.renderTex( cooldown_panel, cooldown_panel_x, cooldown_panel_y )
   gfx.print(false, _("Cooling down..."), cooldown_frame_x,
         cooldown_bg_y + cooldown_bg_h + 8, cols.txt_bar, cooldown_frame_w, true )
end


local function render_bar( data, value, txt, txtcol, size, col, bgc )
   local offsets, l_bg_bar, l_sheen
   if size then
      offsets = bar_offsets['small']
      l_bg_bar = bgs.bar_sm
      l_sheen = sheen_sm
   else
      offsets = bar_offsets['normal']
      l_bg_bar = bgs.bar
      l_sheen = sheen
   end

   if not col then
      col = data.col
   end

   if data.bg then
      gfx.renderTex( data.bg, data.x + offsets[1], data.y + 2)
   end

   if not value then value = 100 end
   if bgc then gfx.renderRect( data.x + offsets[1], data.y + 2, data.w, data.h, bgc ) end

   if value > 0 then
      gfx.renderRect( data.x + offsets[1], data.y + 2, value/100. * data.w, data.h, col )
   end

   gfx.renderTex( l_bg_bar, data.x, data.y )
   gfx.renderTex( data.icon, data.x + offsets[2], data.y + offsets[2] - 3)
   gfx.renderTex( l_sheen, data.x + offsets[1] + 1, data.y + offsets[3])

   if txt then
      local small = false
      if gfx.printDim( false, txt ) > data.w then
         small = true
      end
      gfx.print( small, txt, data.x + offsets[1], data.y + offsets[4], txtcol, data.w, true)
   else
      gfx.print( true, _("UNAVAILABLE"), data.x + offsets[1], data.y + offsets[4], cols.txt_una, data.w, true )
   end
end

local function render_armourBar( data, value, stress_value, txt, txtcol, size, col, bgc )
   local offsets, l_bg_bar, l_sheen
   if size then
      offsets = bar_offsets['small']
      l_bg_bar = bgs.bar_sm
      l_sheen = sheen_sm
   else
      offsets = bar_offsets['normal']
      l_bg_bar = bgs.bar
      l_sheen = sheen
   end

   if not col then
      col = data.col
   end

   if data.bg then
      gfx.renderTex( data.bg, data.x + offsets[1], data.y + 2)
   end

   if not value then value = 100 end
   if not stress_value then stress_value = 0 end

   if bgc then gfx.renderRect( data.x + offsets[1], data.y + 2, data.w, data.h, bgc ) end
   gfx.renderRect( data.x + offsets[1], data.y + 2, value/100. * data.w, data.h, col )

   if stress_value > 0 then
      gfx.renderRect( data.x + offsets[1], data.y + 2, (stress_value/100) * (value/100) * data.w, data.h, cols.stress )
   end

   gfx.renderTex( l_bg_bar, data.x, data.y )
   gfx.renderTex( data.icon, data.x + offsets[2], data.y + offsets[2] - 3)
   gfx.renderTex( l_sheen, data.x + offsets[1] + 1, data.y + offsets[3])

   if txt then
      local small = false
      if gfx.printDim( false, txt ) > data.w then
         small = true
      end
      gfx.print( small, txt, data.x + offsets[1], data.y + offsets[4], txtcol, data.w, true)
   else
      gfx.print( true, _("UNAVAILABLE"), data.x + offsets[1], data.y + offsets[4], cols.txt_una, data.w, true )
   end
end

local function render_ammoBar( name, x, y, value, txt, txtcol )
   local offsets = bar_offsets['ammo']
   local l_bg = bgs[name]
   local l_col
   gfx.renderTex( l_bg, x + offsets[1], y + offsets[1])
   gfx.renderTex( bgs.ready, x + offsets[1], y + offsets[2])

   -- Overheat or ammo capacity
   if value[1] > 0 then
      if name == "heat" then
         value[1] = value[1] / 2.
         if value[1] > .5 then
            l_col = cols.heat2
         else
            l_col = cols.heat
         end
      else
         l_col = cols[name]
      end

      gfx.renderRect( x + offsets[1], y + offsets[1], value[1] * bar_weapon_w, bar_weapon_h, l_col)
   end

   -- Refire indicator
   gfx.renderRect( x + offsets[1], y + offsets[2], value[2] * bar_ready_w, bar_ready_h, value[6])

   if value[3] == 1 then
      gfx.renderTex( bgs.bar_weapon_prim, x, y )
   elseif value[3] == 2 then
      gfx.renderTex( bgs.bar_weapon_sec, x, y )
   else
      gfx.renderTex( bgs.bar_weapon, x, y )
   end

   local textoffset = 0
   local trackcol
   if value[4] then
      if value[4] == -1 or ptarget == nil then
         trackcol = cols.txt_una
      elseif value[5] then -- Handling missile lock-on.
         if value[4] < 1. then
            local h, s, v = cols.txt_una:hsv()
            trackcol = colour.new( cols.txt_una )
            trackcol:setHSV( h, s, v + value[4] * (1-v))
         else
            trackcol = colour.new( "Green" )
         end
      else -- Handling turret tracking.
         trackcol = colour.new(1-value[4], value[4], 0)
      end
      gfx.renderTex( tracking_light, x + offsets[7], y + offsets[8], trackcol )
      textoffset = track_w + 2
   end
   gfx.renderTex( sheen_weapon, x + offsets[3], y + offsets[4])
   gfx.renderTex( sheen_tiny, x + offsets[3], y + offsets[5])
   gfx.print( true, txt, x + offsets[1] + textoffset, y + offsets[6], txtcol, bar_weapon_w - textoffset, true)
end


local function round(num)
   return math.floor( num + 0.5 )
end

local function roundto(num, idp)
   return string.format("%.0" .. (idp or 0) .. "f", num)
end

local function largeNumber( number, idp )
   local formatted
   local units = { "k", "M", "B", "T", "Q" }
   if number < 1e4 then
      formatted = math.floor(number)
   elseif number < 1e18 then
      local len = math.floor(math.log10(number))
      formatted = roundto( number / 10^math.floor(len-len%3), idp) .. units[(math.floor(len/3))]
   else
      formatted = _("Too big!")
   end
   return formatted
end


function render( dt, dt_mod )
   --Values
   armour, shield, stress = pp:health()
   energy = pp:energy()
   speed = pp:vel():mod()
   local temperature = pp:temp()
   local lockons = pp:lockon()
   local autonav = player.autonav()
   local credits = player.credits()
   update_wset() -- Ugly.

   --Radar
   gfx.renderTex( radar_gfx, radar_x, radar_y )
   gui.radarRender( radar_x + 2, radar_y + 2 )

   -- Effects
   local ex, ey = radar_x-48, screen_h-32-32
   for k,e in ipairs(effects) do
      gfx.renderTexRaw( e.icon, ex, ey, 32, 32, 1, 1, 0, 0, 1, 1, e.col )
      if e.n > 1 then
         gfx.print( true, tostring(e.n), ex+24, ey+24, cols.txt_bar )
      end
      ex = ex - 48
   end

   --Player pane
   gfx.renderTex( player_pane_t, pl_pane_x, pl_pane_y )
   local filler_h = #wset * 28 -- extend the pane according to the number of weapon bars
   filler_h = math.max( filler_h - 6, 0 )

   gfx.renderTexRaw( player_pane_m, pl_pane_x + 33, pl_pane_y - filler_h, pl_pane_w_b, filler_h)
   gfx.renderTex( player_pane_b, pl_pane_x + 33, pl_pane_y - filler_h - pl_pane_h_b )

   local txt = {}
   for k,v in ipairs(bars) do
      txt[v] = round(_G[v]) .. "% (" .. round( stats[v] * _G[v] / 100 ) .. ")"
   end

   --Shield
   local col
   if shield == 0. then
      col = cols.txt_enm
   elseif shield <= 20. then
      col = cols.txt_wrn
   else
      col = cols.txt_bar
   end
   render_bar( bardata['shield'], shield, txt["shield"], col )

   --Armour
   if armour <= 20. then
      col = cols.txt_enm
   else
      col = cols.txt_bar
   end
   render_armourBar( bardata['armour'], armour, stress, txt["armour"], col )

   --Energy
   if energy == 0. then
      col = cols.txt_enm
   elseif energy <= 20. then
      col = cols.txt_wrn
   else
      col = cols.txt_bar
   end
   render_bar( bardata['energy'], energy, txt["energy"], col )

   --Speed
   local hspeed
   if stats.speed_max <= 0 then hspeed = 0
   else hspeed = round(speed / stats.speed_max * 100) end
   txt = hspeed .. "% (" .. round(speed) .. ")"
   if hspeed <= 100. then
      render_bar( bardata['speed'], hspeed, txt, cols.txt_bar )
   elseif hspeed <= 200. then
      render_bar( bardata['speed'], hspeed - 100, txt, cols.txt_wrn, nil, cols.speed2, cols.speed )
   else
      timers[1] = timers[1] - dt / dt_mod
      if timers[1] <=0. then
         timers[1] = 0.5
         if blinkcol == cols.txt_una then
            blinkcol = cols.txt_enm
         else
            blinkcol = cols.txt_una
         end
      end
      col = blinkcol
      render_bar( bardata['speed'], 100, txt, col, nil, cols.speed2)
   end

   -- Temperature
   txt = round(temperature) .. "K"
   temperature = math.max( math.min( (temperature - 250)/1.75, 100 ), 0 )
   render_bar( bardata['temperature'], temperature, txt, cols.txt_bar )

   -- Sirius Flow
   if has_flow then
      local f = flow.get(pp)
      local fm = flow.max(pp)
      txt = string.format("%.0f / %.0f", f, fm )
      render_bar( bardata['flow'], f / fm * 100, txt, cols.txt_bar )
   end

   --Weapon bars
   for num, weapon in ipairs(wset) do
      txt = _(weapon.name)
      local values
      if weapon.left then -- Truncate names for readability.
         if weapon.type == "Bolt Cannon" or weapon.type == "Beam Cannon" then
            txt = string.gsub(txt,"Cannon", "C.")
         elseif weapon.type == "Bolt Turret" or weapon.type == "Beam Turret" then
            txt = string.gsub(txt,"Turret", "T.")
         elseif weapon.type == "Launcher" or weapon.type == "Turret Launcher" then
            txt = string.gsub(txt,"Launcher", "L.")
         end

         txt = txt .. " (" .. weapon.left .. ")"
         if weapon.left == 0 then
            col = cols.txt_wrn
         else
            col = cols.txt_bar
         end
         if not weapon.in_arc and ptarget ~= nil then
            col = cols.txt_una
         end
         values = {weapon.left_p, weapon.cooldown, weapon.level,
               weapon.track or weapon.lockon, weapon.lockon, cols.ready }
         render_ammoBar( "ammo", x_ammo, y_ammo - (num-1)*28, values, txt, col)
      else
         col = cols.txt_bar
         values = {weapon.temp, weapon.cooldown, weapon.level, weapon.track, nil, cols.ready}

         if weapon.charge then
            values[2] = weapon.charge
            if weapon.charge == 1 or weapon.cooldown == 0 then
               values[6] = cols.energy
            else
               values[6] = cols.txt_wrn
            end
         end

         render_ammoBar( "heat", x_ammo, y_ammo - (num-1)*28, values, txt, col )
      end
   end

   -- Formation selection button
   if #pp:followers() ~= 0 then
      local x = x_ammo
      local y = y_ammo - #wset * 28 - 15
      local width, height = bgs.bar_weapon:dim()

      if buttons["formation"] == nil then
          buttons["formation"] = {}
      end

      local button = buttons["formation"]
      button.x = x
      button.y = y
      button.w = width
      button.h = height
      button.action = playerform

      col = colour.new( .10, .10, .10 )
      if button.state == "mouseover" then
          col = colour.new( .25, .25, .25 )
      end

      gfx.renderRect( x, y, width, height, col)
      gfx.renderTex( bgs.bar_weapon, x, y )
      gfx.print( true, _("Set formation"), x, y + 8, cols.txt_bar, width, true )
   end

   --Warning Light
   if lockons > 0 then
      timers[2] = timers[2] - dt / dt_mod
      timers[3] = timers[3] - dt / dt_mod
      if timers[2] <= 0. then
         if lockons < 20 then
            timers[2] = 0.5 - (0.025 * lockons)
            gfxWarn = not gfxWarn
         else
            timers[2] = 0
            gfxWarn = true
         end
      end
      if gfxWarn then
         gfx.renderTex( warnlight2, pl_pane_x + 29, pl_pane_y + 7 )
      end
      if timers[3] <= -0.5 then
         timers[3] = 0.5
      end
      colour.setAlpha( cols.missile, math.abs(timers[3]) * 1.2 + .4 )
      gfx.print( false, missile_lock_text, (screen_w - missile_lock_length)/2, screen_h - 100, cols.missile )
   end

   if armour <= 20 then
      gfx.renderTex( warnlight1, pl_pane_x + 6, pl_pane_y + 148 )
   elseif shield <= 50 or armour <= 50 then
      gfx.renderTex( warnlight4, pl_pane_x + 6, pl_pane_y + 148 )
   else
      gfx.renderTex( warnlight5, pl_pane_x + 6, pl_pane_y + 148 )
   end

   if autonav then
      gfx.renderTex( warnlight3, pl_pane_x + 162, pl_pane_y + 12 )
   end

   -- Active outfits
   if #aset > 0 then
      -- Draw the left-side bar cap.
      gfx.renderTexRaw( slotend, slot_start_x - slotend_w, slot_y, slotend_w, slotend_h, 1, 1, 0, 0, -1, 1 )

      gfx.renderRect( slot_start_x, slot_y, slot_w * #aset, slot_h, cols.slot_bg ) -- Background for all slots.
      for i=1,#aset do
         local slot_x = screen_w - slot_start_x - i * slot_w

         -- Draw a heat background for certain outfits. TODO: detect if the outfit is heat based somehow!
         if aset[i].type == "Afterburner" then
            gfx.renderRect( slot_x, slot_y, slot_w, slot_h * aset[i].temp, cols.heat ) -- Background (heat)
         end

         gfx.renderTexRaw( active_icons[i], slot_x + slot_img_offs_x, slot_y + slot_img_offs_y + 2, slot_img_w, slot_img_w, 1, 1, 0, 0, 1, 1 ) --Image

         if aset[i].type == "Afterburner" then
            gfx.renderRect( slot_x, slot_y, slot_w, slot_h * aset[i].temp, cols.afb ) -- Foreground (heat)
         end

         if aset[i].state == "on" then
            gfx.renderTex( active, slot_x + slot_img_offs_x, slot_y + slot_img_offs_y )
         elseif aset[i].state == "cooldown" then
            local texnum = round(aset[i].cooldown*35) --Turn the 0..1 cooldown number into a 0..35 tex id where 0 is ready.
            gfx.renderTex( cooldown, slot_x + slot_img_offs_x, slot_y + slot_img_offs_y, (texnum % 6) + 1, math.floor( texnum / 6 ) + 1 )
         end

         if aset[i].weapset then
            gfx.print( true, _(aset[i].weapset), slot_x + slot_img_offs_x + 5,
                  slot_y + slot_img_offs_y + 5, cols.txt_bar, slot_w, false )
         end

         gfx.renderTex( slot, slot_x, slot_y ) -- Frame
      end

      -- Draw the right-side bar cap.
      gfx.renderTex( slotend, slot_start_x + #aset * slot_w, slot_y )
   end

   --Target Pane
   if ptarget then
      local ta_detect, ta_scanned = pp:inrange( ptarget )
      if ta_detect then
         --Frame
         gfx.renderTex( target_pane, ta_pane_x, ta_pane_y )
         gfx.renderTex( target_bg, ta_image_x, ta_image_y )

         local ta_armour, ta_shield, ta_stress, ta_disabled, ta_energy
         if ta_scanned then
            ptarget_target = ptarget:target()
            ta_armour, ta_shield, ta_stress, ta_disabled = ptarget:health()
            tflags = ptarget:flags()
            ta_energy = ptarget:energy()

            --Render target graphic
            if ptarget_gfx_w > 62 or ptarget_gfx_h > 62 then
               gfx.renderTexRaw( ptarget_gfx, ta_center_x - ptarget_gfx_draw_w / 2, ta_center_y - ptarget_gfx_draw_h / 2, ptarget_gfx_draw_w, ptarget_gfx_draw_h, 1, 1, 0, 0, 1, -1)
            else
               gfx.renderTexRaw( ptarget_gfx, ta_center_x - ptarget_gfx_w / 2, ta_center_y - ptarget_gfx_h / 2, ptarget_gfx_w, ptarget_gfx_h, 1, 1, 0, 0, 1, -1)
            end
         else
            --Render ?
            gfx.renderTex( question, ta_center_x - ta_question_w / 2, ta_center_y - ta_question_h / 2 )
         end

         -- Dist and dir calculated without explicit target.
         local ta_pos = ptarget:pos()
         local ta_dist = pp:pos():dist( ta_pos )
         local ta_dir = ptarget:dir()
         ta_speed = ptarget:vel():mod()

         --Title
         gfx.print( false, _("TARGETED"), ta_pane_x + 14, ta_pane_y + 190, cols.txt_top )

         --Text, warning light & other texts
         local htspeed = round(ta_speed / ta_stats.speed_max * 100,0)
         local shi, ene, arm, spe, colspe, colspe2, spetxtcol
         if ta_scanned then
            --Bar Texts
            shi = round(ta_shield) .. "% (" .. round(ta_stats.shield  * ta_shield / 100) .. ")"
            arm = round(ta_armour) .. "% (" .. round(ta_stats.armour  * ta_armour / 100) .. ")"
            ene = round(ta_energy) .. "%"
            if ta_stats.speed_max < 1 then
               spe = round(ta_speed)
               spetxtcol = cols.txt_bar
            else
               spe = htspeed .. "% (" .. round(ta_speed) .. ")"
                  if htspeed <= 100. then
                  spetxtcol = cols.txt_bar
                  colspe = cols.speed
               else
                  htspeed = math.min( htspeed - 100, 100 )
                  spetxtcol = cols.txt_wrn
                  colspe = cols.speed2
                  colspe2 = cols.speed
               end
            end


            --Warning Light
            if ptarget_target == pp and not ta_disabled then
               gfx.renderTex( target_light_on, ta_warning_x - 3, ta_warning_y - 3 )
            else
               gfx.renderTex( target_light_off, ta_warning_x, ta_warning_y )
            end

            --Faction Logo
            if ptarget_faction_gfx then
               local lw, lh = ptarget_faction_gfx:dim()
               local ls = 24 / math.max( lw, lh )
               gfx.renderTexScale( ptarget_faction_gfx, ta_fact_x - ls*lw/2, ta_fact_y - ls*lh/2, ls*lw, ls*lh )
            end

            -- Cargo light cargo_light_off
            if ta_cargo and #ta_cargo >= 1 then
               gfx.renderTex( cargo_light_on, ta_cargo_x, ta_cargo_y )
            else
               gfx.renderTex( cargo_light_off, ta_cargo_x, ta_cargo_y )
            end

            -- Status information
            local status
            if ta_disabled then
               status = _("Disabled")
            elseif tflags["boardable"] then
               status = _("Boardable")
            elseif ptarget:cooldown() then
               status = _("Cooling Down")
            end

            if status then
               gfx.print( true, status, ta_pane_x + 14, ta_pane_y + 94, cols.txt_top )
            end

            --Pilot name
            if ta_disabled then
               col = cols.txt_una
            else
               col = ptarget:colour()
            end
            gfx.print( true, ptarget:name(), ta_pane_x + 14, ta_pane_y + 176, col, ta_pane_w - 28 )
         else
            --Bar Texts
            spe = round(ta_speed)
            spetxtcol = cols.txt_bar
            htspeed = 0.

            --Warning light
            gfx.renderTex( target_light_off, ta_warning_x, ta_warning_y )

            -- Cargo light
            gfx.renderTex( cargo_light_off, ta_cargo_x, ta_cargo_y )

            --Pilot name
            gfx.print( true, _("Unknown"), ta_pane_x + 14, ta_pane_y + 176, cols.txt_una )
         end

         -- Render bars.
         render_bar( bardata['shield_sm'], ta_shield, shi, cols.txt_bar, "sm")
         render_armourBar( bardata['armour_sm'], ta_armour, ta_stress, arm, cols.txt_bar, "sm")
         render_bar( bardata['energy_sm'], ta_energy, ene, cols.txt_bar, "sm")
         render_bar( bardata['speed_sm'], htspeed, spe, spetxtcol, "sm", colspe, colspe2 )

         --Dist
         gfx.print( true, _("DIST"), ta_pane_x + 130, ta_pane_y + 160, cols.txt_top )
         if ta_dist then
            local str = largeNumber( ta_dist, 1 )
            gfx.print( false, str, ta_pane_x + ta_pane_w - 15 - gfx.printDim(false, str), ta_pane_y +142, cols.txt_std, 60, false )
         end

         --Dir
         gfx.print(true, _("DIR"), ta_pane_x + 86, ta_pane_y + 160, cols.txt_top )

         -- Render dir sprite.
         local x, y = target_dir:spriteFromDir( ta_dir )
         gfx.renderTex( target_dir, ta_pane_x + 86, ta_pane_y + 136, x, y, cols.txt_top )
      end
   end

   -- Planet pane
   local services_h
   if nav_pnt then
      local ta_pnt_dist = pp:pos():dist( nav_spob.pos )

      -- Extend the pane depending on the services available.
      services_h = 46
      if pntflags.land then
         services_h = services_h + (14 * nav_spob.nservices)
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
      gfx.print( true, _("TARGETED"), ta_pnt_pane_x + 14, ta_pnt_pane_y + 164, cols.txt_top )
      gfx.print( true, _("DISTANCE:"), ta_pnt_pane_x + 35, ta_pnt_pane_y - 14, cols.txt_top )
      gfx.print( true, _("CLASS:"), ta_pnt_pane_x + 14, ta_pnt_pane_y - 34, cols.txt_top )

      if ta_pnt_faction_gfx then
         local lw, lh = ta_pnt_faction_gfx:dim()
         local ls = 24 / math.max( lw, lh )
         gfx.renderTexScale( ta_pnt_faction_gfx, ta_pnt_fact_x - ls*lw/2, ta_pnt_fact_y - ls*lh/2, ls*lw, ls*lh )
      end

      local x1, y1 = vec2.get(nav_spob.pos)
      local x2, y2 = vec2.get(player.pos())
      local ta_pnt_dir = math.atan2(y2 - y1, x2 - x1) + math.pi

      -- Render dir sprite.
      local x, y = target_dir:spriteFromDir( ta_pnt_dir )
      gfx.renderTex( target_dir, ta_pnt_pane_x + 12, ta_pnt_pane_y -24, x, y, cols.txt_top )

      gfx.print( true, nav_spob.class, ta_pnt_pane_x + 150 - nav_spob.class_w, ta_pnt_pane_y - 34, cols.txt_top )
      gfx.print( true, _("SERVICES:"), ta_pnt_pane_x + 14, ta_pnt_pane_y - 48, cols.txt_top )

      -- Space out the text.
      if pntflags.land then
         services_h = 62
         for k,v in ipairs(nav_spob.services) do
            gfx.print(true, _(v), ta_pnt_pane_x + 60, ta_pnt_pane_y - services_h, cols.txt_top )
            services_h = services_h + 14
         end
      else
         gfx.print( true, _("none"), ta_pnt_pane_x + 110, ta_pnt_pane_y - 48, cols.txt_una )
      end

      gfx.print( false, largeNumber( ta_pnt_dist, 1 ), ta_pnt_pane_x + 110, ta_pnt_pane_y - 15, cols.txt_std, 63, false )
      gfx.print( true, nav_spob.name, ta_pnt_pane_x + 14, ta_pnt_pane_y + 149, nav_spob.col )
   end

   --Bottom bar
   local length = 5
   local fuelstring
   gfx.renderTexRaw( bottom_bar, 0, 0, screen_w, 30, 1, 1, 0, 0, 1, 1 )

   local jumps = player.jumps()
   local fuel = player.fuel()

   if fuel > 0 then
      fuelstring = string.format( "%d (%s)", fuel, fmt.jumps(jumps) )
   else
      fuelstring = _("none")
   end

   local bartext = { _("Pilot:"), pname, _("System:"), sysname, _("Time:"), time.str(), _("Credits:"),
         largeNumber( credits, 2 ), _("Nav:"), navstring, _("Fuel:"), fuelstring,
         _("WSet:"), wset_name, _("Cargo:") }
   for k,v in ipairs(bartext) do
      if k % 2 == 1 then
         gfx.print( true, v, length, 5, cols.txt_top )
         length = length + gfx.printDim( true, v .. " " )
      else
         if v == "none" then
            col = cols.txt_una
         else
            col = cols.txt_std
         end
         gfx.print( true, v, length, 5, col )
         length = length + gfx.printDim( true, v ) + 10
      end
   end

   local cargstring = nil
   if cargo and #cargo >= 1 then
      for k,v in ipairs(cargo) do
         if cargstring then
            if screen_w - length - gfx.printDim(true, cargstring .. ", " .. v) < cargofreel + cargoterml then
               cargstring = cargstring .. ", [...]"
               break
            else
               cargstring = cargstring .. ", " .. v
            end
         else
            cargstring = v
         end
      end
      gfx.print( true, cargstring, length, 6, cols.txt_std )

      length = length + gfx.printDim( true, cargstring )
   else
      gfx.print( true, _("none"), length, 6, cols.txt_una )
      length = length + gfx.printDim( true, _("none") ) + 6
   end
   gfx.print( true, cargofree, length, 6, cols.txt_std )
end

local function mouseInsideButton( x, y )
   for _, v in pairs(buttons) do
      if x > v.x and x < v.x+v.w and y > v.y and y < v.y+v.h then
         return v
      end
   end
   return nil
end

function mouse_click( button, x, y, state )
   if button ~= 2 then
      return false
   else
      lmouse = state
      local pressed = mouseInsideButton( x, y )

      if pressed == nil then
         if not state then
            for _,v in pairs(buttons) do
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
   local pressed = mouseInsideButton( x, y )
   if pressed ~= nil then
      if pressed.state ~= "disabled" and not lmouse then
         pressed.state = "mouseover"
      elseif pressed.state ~= "disabled" and lmouse then
         pressed.state = "pressed"
      end
   else
      for _,v in pairs(buttons) do
         if v.state ~= "disabled" and v.state ~= "hilighted" then
            v.state = "default"
         end
      end
   end
end

function cooldown_end ()
end
