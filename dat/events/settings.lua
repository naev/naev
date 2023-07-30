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

local settings, uselanes_jump, uselanes_spob, pick_gui
local reset_dist, reset_shield

local AUTONAV_MAX_DIST = 10e3

function create ()
   -- Load variables
   uselanes_jump = var.peek( "autonav_uselanes_jump" )
   uselanes_spob = var.peek( "autonav_uselanes_spob" )
   if uselanes_jump == nil then
      -- Not set so default to true
      uselanes_jump = true
      var.push( "autonav_uselanes_jump", true )
   end
   reset_shield = var.peek("autonav_reset_shield")
   reset_dist = var.peek("autonav_reset_dist")

   -- Set an info button up
   player.infoButtonRegister( _("Settings"), settings, 1, "A" )
end

function settings ()
   local txt_autonav, fad_autonav
   local chk_uselanes_jump, chk_uselanes_spob

   local w, h = 600, 420
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
         msg = msg.._("Armour Damage")
      end
      txt_autonav:set( msg )
   end
   local autonav_value
   if reset_dist > 0 then
      autonav_value = math.min( 1, 0.5 + 0.5 * reset_dist / AUTONAV_MAX_DIST )
   else
      autonav_value = math.max( 0, 0.5 * reset_shield )
   end

   local y = 40
   luatk.newText( wdw, 20, y, w-40, 20, "#n".._("Autonav") )
   y = y + 20
   txt_autonav = luatk.newText( wdw, 20, y, w-40, 20 )
   y = y + 20
   fad_autonav = luatk.newFader( wdw, 20, y, w-40, 20, 0, 1, nil, update_autonav )
   fad_autonav:set( autonav_value )
   y = y + 40
   chk_uselanes_jump = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Use patrol lanes when jumping"), nil, uselanes_jump )
   y = y + 30
   chk_uselanes_spob = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Use patrol lanes when travelling to a space object"), nil, uselanes_spob )
   y = y + 40

   luatk.newText( wdw, 20, y, w-40, 20, "#n".._("GUI") )
   y = y + 20
   luatk.newButton( wdw, 20, y, 80, 40, _("Pick GUI"), pick_gui )

   luatk.newButton( wdw, -20, -20, 80, 40, _("Close"), luatk.close )
   luatk.run()

   -- Save as variables
   uselanes_jump = chk_uselanes_jump:get()
   uselanes_spob = chk_uselanes_spob:get()
   var.push( "autonav_uselanes_jump", uselanes_jump )
   var.push( "autonav_uselanes_spob", uselanes_spob )
   var.push( "autonav_reset_dist", reset_dist )
   var.push( "autonav_reset_shield", reset_shield )
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
