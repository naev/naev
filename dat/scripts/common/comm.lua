local vn
local lg = require 'love.graphics'

local comm = {}

-- stolen from scripts/vn.lua
local function _draw_bg( x, y, w, h, col, border_col, alpha )
   col = col or {0, 0, 0, 1}
   border_col = border_col or {0.5, 0.5, 0.5, 1}
   vn.setColor( border_col, alpha )
   lg.rectangle( "fill", x, y, w, h )
   vn.setColor( col, alpha )
   lg.rectangle( "fill", x+2, y+2, w-4, h-4 )
end

local function nameboxUpdateInternal( obj, bribed, hostile )
   local fac = obj:faction()
   local nw, _nh = naev.gfx.dim()
   vn.menu_x = math.min( -1, 500 - nw/2 )
   vn.namebox_alpha = 0
   local namebox_font = vn.namebox_font
   local faction_str
   if bribed then
      faction_str = "#g".._("Bribed").."#0"
   else
      local _std, str = fac:playerStanding()
      if hostile then
         faction_str = "#r".._("Hostile").."#0"
      elseif not fac:known() then
         faction_str = _("Unknown")
      else
         faction_str = str
      end
   end
   local facname = (fac:known() and fac:name()) or _("Unknown")
   local namebox_text = string.format("%s\n%s\n%s", facname, obj:name(), faction_str )
   local namebox_col = fac:colour()
   if namebox_col then namebox_col = {namebox_col:rgb()}
   else namebox_col = {1,1,1}
   end
   local namebox_x = math.max( 1, nw/2-600 )
   local namebox_y = vn.namebox_y + vn.namebox_h -- Correct
   local namebox_text_w, wrapped = namebox_font:getWrap( namebox_text, nw )
   local namebox_b = 20
   local namebox_w = namebox_text_w + 2*namebox_b
   local namebox_h = namebox_font:getLineHeight()*#wrapped + 2*namebox_b
   namebox_y = namebox_y - namebox_h

   -- Get the logo
   local logo = fac:logo()
   local logo_size = namebox_h - 2*namebox_b
   local logo_scale
   local logo_w, logo_h
   if logo then
      namebox_w = namebox_w + logo_size + 10
      logo = lg.newImage( logo )
      logo_w, logo_h = logo:getDimensions()
      logo_scale = logo_size / math.max(logo_w,logo_h)
   end

   local function render_namebox ()
      local bw, bh = namebox_b, namebox_b
      local x, y = namebox_x, namebox_y
      local w, h = namebox_w, namebox_h

      _draw_bg( x, y, w, h, vn.namebox_bg, nil, 1 )
      vn.setColor( namebox_col, 1 )
      lg.print( namebox_text, namebox_font, x+bw, y+bh )

      if logo then
         vn.setColor( {1, 1, 1}, 1 )
         logo:draw( x+namebox_text_w+10+bw + (logo_size-logo_w*logo_scale)*0.5, y+bh + (logo_size-logo_h*logo_scale)*0.5, 0, logo_scale )
      end
   end
   vn.setForeground( render_namebox )
end

function comm.nameboxUpdate( plt )
   return nameboxUpdateInternal( plt, plt:flags("bribed"), plt:hostile() )
end

function comm.nameboxUpdateSpob( spb, bribed )
   return nameboxUpdateInternal( spb, bribed, false )
end

function comm.newCharacter( vn_in, plt )
   vn = vn_in

   -- Shortcuts and graphics
   local shipgfx = lg.newImage( plt:ship():gfxComm() )

   -- Set up the namebox
   comm.nameboxUpdate( plt )

   return vn.newCharacter( plt:name(), { image=shipgfx, isportrait=true } )
end

function comm.newCharacterSpob( vn_in, spb, params )
   params = params or {}
   vn = vn_in

   -- Graphics
   local spbgfx = lg.newImage( spb:gfxComm() )

   -- Set up the namebox
   comm.nameboxUpdateSpob( spb, params.bribed )

   return vn.newCharacter( spb:name(), { image=spbgfx, isportrait=true } )
end

--[[--
   Sets a custom message and handler for a pilot.

   Important note: the setup function will be called from the comm enivronment. You can not call misn/evt functions or access the memory of a mission or event through this!

      @tparam Pilot plt Pilot to set up for.
      @tparam string|function menu Menu message or function returning message or nil.
      @tparam function setup Function to call to set up the vn nodes when the selected menu option is pressed. Will be passed a local version of the vn library as the first parameter, and the pilot vn character as the second parameter.
--]]
function comm.customComm( plt, menu, setup )
   local m = plt:memory()
   m.comm_custom = m.comm_custom or {}
   table.insert( m.comm_custom, {
      menu = menu,
      setup = setup,
   } )
end

return comm
