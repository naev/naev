--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Settings">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[
   General per-pilot settings.
--]]
local luatk = require "luatk"
local fmt = require "format"

local settings, uselanes_jump, uselanes_spob, uselanes_thr, pick_gui
local reset_dist, reset_shield, compr_speed, compr_max, follow_jump, brake_pos

local AUTONAV_MAX_DIST  = 10e3 -- quite a reasonable distance
local COMPR_SPEED_MIN   = 1e3 -- Old default was 5e3
local COMPR_SPEED_MAX   = 10e3
local COMPR_MIN         = 10 -- Old default was 200
local COMPR_MAX         = 200

function create ()
   -- Load variables
   uselanes_jump = var.peek( "autonav_uselanes_jump" )
   uselanes_spob = var.peek( "autonav_uselanes_spob" )
   if uselanes_jump == nil then
      -- Not set so default to true
      uselanes_jump = true
      var.push( "autonav_uselanes_jump", true )
   end
   uselanes_thr = var.peek("autonav_uselanes_thr") or 2
   reset_shield = var.peek("autonav_reset_shield") or 1
   reset_dist = var.peek("autonav_reset_dist") or 3e3
   compr_speed = var.peek("autonav_compr_speed") or 5e3
   compr_max = var.peek("autonav_compr_max") or 50
   follow_jump = var.peek("autonav_follow_jump")
   brake_pos = var.peek("autonav_brake_pos")

   -- Set an info button up
   player.infoButtonRegister( _("Settings"), settings, 1, "S" )
end

function settings ()
   local txt_autonav, fad_autonav, txt_compr
   local chk_uselanes_jump, chk_uselanes_spob, chk_follow_jump, chk_brake_pos

   local w, h = 600, 500
   local wdw = luatk.newWindow( nil, nil, w, h )
   wdw:setCancel( luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Player Settings"), nil, "center" )

   local function update_autonav( _fad, value )
      local msg = _("Stop Speedup At: ")
      if value >= 1 then
         reset_dist = math.huge
         reset_shield = 1
         msg = msg.._("Enemy Presence")
      elseif value > 0.5 then
         reset_dist = (value-0.5) / 0.5 * AUTONAV_MAX_DIST
         reset_shield = 1
         msg = msg..fmt.f(_("Enemy within {dist} distance"),{dist=fmt.number(reset_dist)})
      elseif value > 0 then
         reset_dist = -1
         reset_shield = value / 0.5
         msg = msg..fmt.f(_("{shield:.0f}% Shield"),{shield=reset_shield*100})
      else
         reset_dist = -1
         reset_shield = 0
         msg = msg..p_("settings", "Armour Damage")
      end
      txt_autonav:set( msg )
   end
   local autonav_value
   local function update_autonav_value ()
      if reset_dist > 0 then
         autonav_value = math.min( 1, 0.5 + 0.5 * reset_dist / AUTONAV_MAX_DIST )
      else
         autonav_value = math.max( 0, 0.5 * reset_shield )
      end
   end
   update_autonav_value()
   local function update_ship_tc ()
      local pp = player.pilot()
      local tc_max = compr_speed / pp:speedMax()
      tc_max = math.max( 1, math.min( tc_max, compr_max ) )
      local msg = fmt.f(_("Your current ship will have a time compression factor of #o{tc_max:.1f}x#0."),{tc_max=tc_max})
      txt_compr:set( msg )
   end

   local y = 40
   luatk.newText( wdw, 20, y, w-40, 20, "#n".._("Autonav") )
   y = y + 20
   txt_autonav = luatk.newText( wdw, 20, y, w-40, 20 )
   y = y + 20
   fad_autonav = luatk.newFader( wdw, 20, y, w-40, 20, 0, 1, nil, update_autonav )
   fad_autonav:set( autonav_value )
   y = y + 40
   local txt_lanes_thr = luatk.newText( wdw, 20, y, w-40, 20, _("Lanes preference:") )
   local tw = txt_lanes_thr:dimensions()
   local fad_thr = luatk.newFader( wdw, 40+tw, y, w-70-tw, 30, 1, 5, uselanes_thr, function ( _fdr, val )
      uselanes_thr = val
   end, {labels=true} )
   y = y + 40
   local txt_compr_speed = luatk.newText( wdw, 20, y, w-40, 20, _("Compression velocity target:") )
   tw = txt_compr_speed:dimensions()
   local fad_compr_speed = luatk.newFader( wdw, 40+tw, y, w-70-tw, 30, COMPR_SPEED_MIN, COMPR_SPEED_MAX, compr_speed, function ( _fdr, val )
      compr_speed = val
      update_ship_tc()
   end, {labels=true} )
   y = y + 40
   local txt_compr_max = luatk.newText( wdw, 20, y, w-40, 20, _("Maximum compression:") )
   tw = txt_compr_max:dimensions()
   local fad_compr_max = luatk.newFader( wdw, 40+tw, y, w-70-tw, 30, COMPR_MIN, COMPR_MAX, compr_max, function ( _fdr, val )
      compr_max = val
      update_ship_tc()
   end, {labels=true} )
   y = y + 40
   txt_compr = luatk.newText( wdw, 20, y, w-40, 20 )
   update_ship_tc()
   y = y + 30
   chk_uselanes_jump = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Use patrol lanes when jumping"), nil, uselanes_jump )
   y = y + 30
   chk_uselanes_spob = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Use patrol lanes when travelling to a space object"), nil, uselanes_spob )
   y = y + 30
   chk_follow_jump = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Jump to follow pilots"), nil, follow_jump )
   y = y + 30
   chk_brake_pos = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Brake when heading to a position"), nil, brake_pos )
   y = y + 40

   luatk.newText( wdw, 20, y, w-40, 20, "#n".._("GUI") )
   y = y + 20
   luatk.newButton( wdw, 20, y, 80, 40, _("Pick GUI"), pick_gui )

   luatk.newButton( wdw, -20-80-20, -20, 80, 40, _("Defaults"), function ()
      luatk.yesno(_("Reset Player Settings?"), _("Are you sure you want to reset the player settings?"), function ()
         uselanes_jump = true
         uselanes_spob = true
         uselanes_thr = 2
         reset_shield = 1
         reset_dist = 3e3
         compr_speed = 5e3
         compr_max = 50
         follow_jump = false
         brake_pos = false
         update_autonav_value()
         chk_uselanes_jump:set( uselanes_jump )
         chk_uselanes_spob:set( uselanes_spob )
         chk_follow_jump:set( follow_jump )
         chk_brake_pos:set( brake_pos )
         fad_thr:set( uselanes_thr )
         fad_autonav:set( autonav_value )
         fad_compr_speed:set( compr_speed )
         fad_compr_max:set( compr_max )
         var.push( "autonav_uselanes_jump", uselanes_jump )
         var.push( "autonav_uselanes_spob", uselanes_spob )
         var.push( "autonav_uselanes_thr", uselanes_thr )
         var.push( "autonav_reset_shield", reset_shield )
         var.push( "autonav_reset_dist", reset_dist )
         var.push( "autonav_compr_speed", compr_speed )
         var.push( "autonav_compr_max", compr_max )
         var.push( "autonav_follow_jump", follow_jump )
         var.push( "autonav_brake_pos", brake_pos )

         -- Also set GUI
         player.guiSet( player.start().gui ) -- Default setting GUI
      end )
   end )
   luatk.newButton( wdw, -20, -20, 80, 40, _("Close"), luatk.close )
   luatk.run()

   -- Save as variables
   uselanes_jump = chk_uselanes_jump:get()
   uselanes_spob = chk_uselanes_spob:get()
   follow_jump = chk_follow_jump:get()
   brake_pos = chk_brake_pos:get()
   var.push( "autonav_uselanes_jump", uselanes_jump )
   var.push( "autonav_uselanes_spob", uselanes_spob )
   var.push( "autonav_uselanes_thr", uselanes_thr )
   var.push( "autonav_reset_dist", reset_dist )
   var.push( "autonav_reset_shield", reset_shield )
   var.push( "autonav_compr_speed", compr_speed )
   var.push( "autonav_compr_max", compr_max )
   var.push( "autonav_follow_jump", follow_jump )
   var.push( "autonav_brake_pos", brake_pos)
end

function pick_gui ()
   local w, h = 220, 300
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newText( wdw, 0, 10, w, 20, _("Select GUI"), nil, "center" )

   local lst_guis = luatk.newList( wdw, 20, 40, w-40, h-120, player.guiList() )
   lst_guis:setItem( player.gui() )

   local function wdw_load ()
      player.guiSet( lst_guis:get() )
      wdw:destroy()
   end
   local function wdw_close ()
      wdw:destroy()
   end
   wdw:setAccept( wdw_load )
   wdw:setCancel( wdw_close )
   luatk.newButton( wdw, -20, -20, 80, 40, _("Close"), wdw_close )
   luatk.newButton( wdw, 20, -20, 80, 40, _("Load"), wdw_load )
end
