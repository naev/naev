--[[
   Some stuff for helping with player escorts.
--]]
local luatk = require "luatk"

local lib = {}

function lib.command_menu( plt )
   local function msg_escort( order, msg )
      return function ()
         player.pilot():msg( plt, order )
         plt:comm( player.pilot(), msg )
      end
   end
   local COMMANDS = {
      {  caption = _("Hold Formation"),
         order = msg_escort("e_hold", _("Holding formation.")), },
      {  caption = _("Return to Ship"),
         order = msg_escort("e_return", _("Returning to ship.")), },
      {  caption = _("Clear Orders"),
         order = msg_escort("e_clear", _("Clearing orders.")), },
   }

   local bw = 150
   local bh = 30
   local w = 260
   local h = 90 + (bh+20)*(#COMMANDS+1)
   local title = _( "Escort Orders" )
   local msg = _( "Select the order to give to this escort." )

   local wdw = luatk.newWindow( nil, nil, w, h )
   local function wdw_done( dying_wdw )
      dying_wdw:destroy()
      return true
   end
   wdw:setCancel( wdw_done )
   luatk.newText( wdw, 0, 10, w, 20, title, nil, "centre" )
   luatk.newText( wdw, 20, 40, w-40, h, msg )
   local y = h-20-30
   luatk.newButton( wdw, (w-bw)*0.5, y, bw, 30, _("Cancel"), function( wgt )
      wgt.parent:destroy()
   end )
   for k,cmd in ipairs(COMMANDS) do
      y = y - (bh+20)
      luatk.newButton( wdw, (w-bw)*0.5, y, bw, 30, cmd.caption, function( wgt )
         cmd.order()
         wgt.parent:destroy()
      end )
   end
   luatk.run()
end

return lib
