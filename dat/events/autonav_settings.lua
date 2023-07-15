--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Autonav Settings">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
local luatk = require "luatk"

local autonav_gui, uselanes_jump, uselanes_spob

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
   player.infoButtonRegister( _("Autonav Settings"), autonav_gui, 2, "A" )
end

function autonav_gui ()
   local chk_uselanes_jump, chk_uselanes_spob

   local w, h = 600, 420
   local wdw = luatk.newWindow( nil, nil, w, h )
   wdw:setCancel( luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Autonav Settings"), nil, "center" )

   local y = 40
   chk_uselanes_jump = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("#nUse Lanes:#0 autonav will travel on patrol lanes when jumping"), nil, uselanes_jump )
   y = y + 30
   chk_uselanes_spob = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("#nUse Lanes:#0 autonav will travel on patrol lanes when travelling to a space object"), nil, uselanes_spob )
   luatk.newButton( wdw, -20, -20, 80, 40, _("Close"), luatk.close )
   luatk.run()

   -- Save as variables
   uselanes_jump = chk_uselanes_jump:get()
   uselanes_spob = chk_uselanes_spob:get()
   var.push( "autonav_uselanes_jump", uselanes_jump )
   var.push( "autonav_uselanes_spob", uselanes_spob )
end
