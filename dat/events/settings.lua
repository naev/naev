--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Settings">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[
   General settings.
--]]
local luatk = require "luatk"

local settings, uselanes_jump, uselanes_spob, pick_gui

function create ()
   -- Load variables
   uselanes_jump = var.peek( "autonav_uselanes_jump" )
   uselanes_spob = var.peek( "autonav_uselanes_spob" )
   if uselanes_jump == nil then
      -- Not set so default to true
      uselanes_jump = true
      var.push( "autonav_uselanes_jump", true )
   end

   -- Set an info button up
   player.infoButtonRegister( _("Settings"), settings, 1, "A" )
end

function settings ()
   local chk_uselanes_jump, chk_uselanes_spob

   local w, h = 600, 420
   local wdw = luatk.newWindow( nil, nil, w, h )
   wdw:setCancel( luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Player Settings"), nil, "center" )

   local y = 40
   luatk.newText( wdw, 20, y, w-40, 20, "#n".._("Autonav") )
   y = y + 20
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
