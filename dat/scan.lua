local luatk = require "luatk"
local wgtEquipment = require "luatk.equipment"
local fmt = require "format"

function scan ()
   local plt = player.pilot():target()

   local w, h = 500, 600
   local wdw = luatk.newWindow( nil, nil, w, h )
   local function wdw_close () return wdw:destroy() end
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Scanning {plt}"), {plt=plt}), nil, "center" )

   luatk.newButton( wdw, w-20-80, h-20-30, 80, 30, _("Close"), wdw_close )

   wgtEquipment.new( wdw, 20, 40, 200, 500, plt )

   wdw:setCancel( wdw_close )

   luatk.run()
end
