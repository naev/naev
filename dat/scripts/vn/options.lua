local luatk = require "luatk"
local fmt = require "format"
local lg = require "love.graphics"

local opt = {}

function opt.mousepressed( mx, my, btn )
   return luatk.mousepressed( mx, my, btn )
end
function opt.mousereleased( mx, my, btn )
   return luatk.mousereleased( mx, my, btn )
end
function opt.mousemoved( mx, my )
   return luatk.mousemoved( mx, my )
end
function opt.keypressed( key )
   return luatk.keypressed( key )
end
function opt.draw ()
   luatk.draw()
end
function opt.update( dt )
   luatk.update( dt )
   return opt.running
end

function opt.open( vn )
   local font = lg.newFont(naev.conf().font_size_def)
   font:setOutline(1)
   luatk.setDefaultFont( font )

   local w = 400
   local h = 200
   local wdw = luatk.newWindow( nil, nil, w, h )

   -- Translator note VN stands for Visual Novel
   luatk.newText( wdw, 0, 10, w, h, _("VN Options"), luatk.colour.text, "center" )

   local y = 40
   local autoscrollval = (var.peek("vn_autoscroll")==true)
   local autoscrollchk = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Enable autoscroll to end of text"), function ( wgt )
      var.push( "vn_autoscroll", wgt:get() )
   end, autoscrollval )
   y = y + 30

   local nobounceval = (var.peek("vn_nobounce")==true)
   local nobouncechk = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("Disable bounce when NPC speaks"), function ( wgt )
      var.push( "vn_nobounce", wgt:get() )
   end, nobounceval )
   y = y + 40

   local txtspeed = var.peek("vn_textspeed") or 0.025
   local txtspeedtxt = luatk.newText( wdw, 20, y, w-40, 20, nil, luatk.colour.text )
   local txtspeedfad = luatk.newFader( wdw, 20, y+30, w-80, 20, 0, 0.1, txtspeed, function ( _wgt, val )
      txtspeedtxt:set(fmt.f(_("#nText Speed#0: {val:.3f} seconds per character"),{val=val}))
      var.push( "vn_textspeed", txtspeed )
   end )
   txtspeedfad:set( txtspeed )

   luatk.newButton( wdw, w-120-20, h-40-20, 120, 40, _("Done"), function ()
      vn.speed = txtspeedfad:get()
      vn.autoscroll = autoscrollchk:get()
      vn.nobounce = nobouncechk:get()
      var.push( "vn_textspeed", vn.speed )
      var.push( "vn_autoscroll", vn.autoscroll )
      var.push( "vn_nobounce", vn.nobounce )
      luatk.close()
      opt.running = false
      return true
   end )

   opt.running = true
end

return opt
