local vn = require 'vn'
local fmt = require "format"
local ccomm = require "common.comm"
local lg = require "love.graphics"
local luaspob = {}

function luaspob.init( spb, init_params )
   mem.spob = spb
   mem.params = init_params or {}
   mem.std_land = mem.params.std_land or 0 -- Needed for can_land
end

local function _draw_bg( x, y, w, h, col, border_col, alpha )
   col = col or {0, 0, 0, 1}
   border_col = border_col or {0.5, 0.5, 0.5, 1}
   vn.setColor( border_col, alpha )
   lg.rectangle( "fill", x, y, w, h )
   vn.setColor( col, alpha )
   lg.rectangle( "fill", x+2, y+2, w-4, h-4 )
end

function luaspob.customNeutralSpob(vn_in, spb)
   local spbgfx = lg.newImage( spb:gfxComm() )
   --The following section creates a neutral namebox, always
   local fac = spb:faction()
   local nw, _nh = naev.gfx.dim()
   vn_in.menu_x = math.min( -1, 500 - nw/2 )
   vn.namebox_alpha = 0
   local namebox_font = vn.namebox_font
   local true_standing, _unused = fac:playerStanding()
   if true_standing<0 then
   fac:setPlayerStanding(0)
   end
   local faction_str
   local _std, str = fac:playerStanding()
   if not fac:known() then
      faction_str = _("Unknown")
   else
      faction_str = str
   end
   local facname = (fac:known() and fac:name()) or _("Unknown")
   local namebox_text = string.format("%s\n%s\n%s", facname, spb:name(), faction_str )
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
   fac:setPlayerStanding(true_standing)

	return vn.newCharacter( spb:name(), { image=spbgfx, isportrait=true } )
end

function luaspob.load ()
   -- Basic stuff
   local fct = mem.spob:faction()
   mem.bribed = false

   mem.bribe_cost_function = mem.params.bribe_cost or function ()
      local std = fct:playerStanding()
      return (mem.std_land-std) * 1e3 * player.pilot():ship():size() + 5e3
   end

   mem.std_land = mem.params.std_land or 0
   mem.std_bribe = mem.params.std_bribe or -30
   mem.std_dangerous = mem.params.std_dangerous or -30

   mem.msg_bribed = mem.params.msg_bribed or {
      _([["Make it quick."]]),
      _([["Don't let anyone see you."]]),
      _([["Be quiet about this."]]),
   }
   mem.msg_denied = mem.params.msg_denied or {
      _([["Landing request denied."]]),
      _([["Landing not authorized."]]),
      _([["Landing denied."]]),
   }
   mem.msg_notyet = mem.params.msg_notyet or mem.msg_denied
   mem.msg_granted = mem.params.msg_granted or {
      _([["Permission to land granted."]]),
      _([["You are clear to land."]]),
      _([["Proceed to land."]]),
      _([["Landing authorized."]]),
   }
   mem.msg_cantbribe = mem.params.msg_cantbribe or {
      _([["We do not accept bribes."]]),
   }
   mem.msg_trybribe = mem.params.msg_trybribe or {
      _([["I'll let you land for the modest price of {credits}."

Pay {credits}?]]),
      _([["Some {credits} would make me reconsider letting you land."

Pay {credits}?]]),
   }
   mem.msg_dangerous = mem.params.msg_dangerous or {
      _([["I'm not dealing with dangerous criminals like you!"]]),
   }

   -- Randomly choose
   local function choose( tbl )
      local msg = tbl[ rnd.rnd(1,#tbl) ]
      if type(tbl)=='function' then
         msg = msg()
      end
      return msg
   end
   mem.msg_bribed     = choose( mem.msg_bribed )
   mem.msg_denied     = choose( mem.msg_denied )
   mem.msg_notyet     = choose( mem.msg_notyet )
   mem.msg_granted    = choose( mem.msg_granted )
   mem.msg_cantbribe  = choose( mem.msg_cantbribe )
   mem.msg_trybribe   = choose( mem.msg_trybribe )
   mem.msg_dangerous  = choose( mem.msg_dangerous )
end

function luaspob.unload ()
   mem.bribed = false
end

function luaspob.can_land ()
   local s = mem.spob:services()
   if not s.land then
      return false,nil -- Use default landing message
   end
   if mem.bribed or mem.spob:getLandOverride() then
      return true, mem.msg_granted
   end
   local fct = mem.spob:faction()
   if not fct then
      return true,nil -- Use default landing message
   end
   local std = fct:playerStanding()
   if std < 0 then
      return false, mem.msg_denied
   end
   if std < mem.std_land then
      return false, mem.msg_notyet
   end
   return true, mem.msg_granted
end

function luaspob.comm ()
   local s = mem.spob:services()
   if not s.inhabited then
      return false
   end

   local fct = mem.spob:faction()
   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, mem.spob, {
      bribed = mem.bribed,
   } )
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with the authorities at {spb}."),
      {spb=mem.spob}))

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         { _("Close"), "leave" }
      }
      local std = fct:playerStanding()
      if std < mem.std_land and not mem.bribed then
         table.insert( opts, 1, { _("Bribe"), "bribe" } )
      end
      return opts
   end )

   local bribe_cost
   vn.label("bribe")
   vn.func( function ()
      local std = fct:playerStanding()
      if std < mem.std_dangerous then
         vn.jump("dangerous")
         return
      end
      if std < mem.std_bribe then
         vn.jump("nobribe")
         return
      end
      bribe_cost = mem.bribe_cost_function( mem.spob )
   end )
   spb( function ()
      return fmt.f( mem.msg_trybribe, {credits=fmt.credits( bribe_cost )} )
   end )
   vn.menu( function ()
      return {
         { fmt.f(_("Pay {credits}"),{credits=fmt.credits( bribe_cost )}), "bribe_yes" },
         { _("Refuse"), "bribe_no" },
      }
   end )

   vn.label("bribe_yes")
   vn.func( function ()
      if bribe_cost > player.credits() then
         vn.jump("player_broke")
         return
      end
      player.pay( -bribe_cost )
      mem.bribed = true
      ccomm.nameboxUpdateSpob( mem.spob, mem.bribed )
   end )
   spb( mem.msg_bribed )
   vn.jump("menu")

   vn.label("player_broke")
   vn.na( function ()
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( bribe_cost - player.credits() )
      return fmt.f(_("You only have {credits} credits. You need #r{cdif}#0 more to be able to afford the bribe!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   vn.label("bribe_no")
   vn.na(_("You refuse to pay the bribe."))
   vn.jump("menu")

   vn.label("nobribe")
   spb( mem.msg_cantbribe )
   vn.jump("menu")

   vn.label("dangerous")
   spb( mem.msg_dangerous )
   vn.jump("menu")

   vn.label("leave")
   vn.run()

   mem.spob:canLand() -- forcess a refresh of condition
   return true
end

return luaspob
