--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Autonav Settings">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
local luatk = require "luatk"

local autonav_gui, use_lanes

function create ()
   -- Load variables
   use_lanes = var.peek( "autonav_uselanes" )
   if use_lanes == nil then
      -- Not set so default to true
      use_lanes = true
      var.push( "autonav_uselanes", true )
   end

   -- Set an info button up
   player.infoButtonRegister( _("Autonav Settings"), autonav_gui, 2, "A" )
end

function autonav_gui ()
   local chk_uselanes

   local w, h = 600, 420
   local wdw = luatk.newWindow( nil, nil, w, h )
   wdw:setCancel( luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Autonav Settings"), nil, "center" )

   local y = 40
   chk_uselanes = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("#nUse Lanes:#0 autonav will try to use non-hostile lanes"), nil, use_lanes )
   --y = y + 30
   luatk.newButton( wdw, -20, -20, 80, 40, _("Close"), luatk.close )
   luatk.run()

   -- Save as variables
   var.push( "autonav_uselanes", chk_uselanes:get() )
end
