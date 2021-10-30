--[[
   Boarding script, now in Lua!
--]]
local luatk = require 'luatk'
local lg = require 'love.graphics'
local fmt = require 'format'

local loot_mod

local function outfit_loot( o )
   return {
      image = lg.newImage( o:icon() ),
      text = o:name(),
      q = nil,
      type = "outfit",
      bg = nil,
      alt = o:description(),
      data = o,
   }
end

local cargo_image_generic = nil
local function cargo_loot( c, q )
   local icon = c:icon()
   return {
      image = (icon and lg.newImage(icon)) or cargo_image_generic,
      text = c:name(),
      q = math.floor( 0.5 + q*loot_mod ),
      type = "cargo",
      bg = nil,
      alt = c:description(),
      data = c,
   }
end

local lootables
function compute_lootables ( plt )
   lootables = {}

   -- Credits
   local creds = plt:credits()
   if creds > 0 then
      table.insert( lootables, {
         image = nil,
         text = _("Credits"),
         q = math.floor( 0.5 + creds*loot_mod ),
         type = "credits",
         bg = nil,
         alt = nil,
         data = nil,
      } )
   end

   -- Fuel
   local fuel = plt:stats().fuel
   if fuel > 0 then
      table.insert( lootables, {
         image = nil,
         text = _("Fuel"),
         q = math.floor( 0.5 + fuel*loot_mod ),
         type = "fuel",
         bg = nil,
         alt = nil,
         data = nil,
      } )
   end

   -- Go over cargo
   for _k,c in ipairs(plt:cargoList()) do
      table.insert( lootables, cargo_loot( commodity.get(c.name), c.q ) )
   end

   --table.insert( lootables, outfit_loot(outfit.get("Laser Cannon MK1")) )

   return lootables
end

local wgtBoard = {}
setmetatable( wgtBoard, { __index = luatk.Widget } )
local wgtBoard_mt = { __index = wgtBoard }
function wgtBoard.new( parent, x, y, w, h, loot )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, wgtBoard_mt )
   wgt.loot = loot
   wgt.selected = false
   return wgt
end
function wgtBoard:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h
   local l = self.loot
   if self.selected then
      lg.setColor( {0,1,1} )
      lg.rectangle( "fill", x-3, y-3, w+6, h+6 )
      lg.setColor( {0,0.5,0.5} )
      lg.rectangle( "fill", x, y, w, h )
   else
      lg.setColor( luatk.colour.outline )
      lg.rectangle( "fill", x-1, y-1, w+2, h+2 )
      lg.setColor( {0,0,0} )
      lg.rectangle( "fill", x, y, w, h )
   end
   -- Ignore anything that isn't loot from now on
   if not l then return end

   local img = l.image
   if img then
      local iw, ih = img:getDimensions()
      lg.setColor( {1,1,1} )
      img:draw( x, y, 0, w / iw, h / ih )
   end
   local txt = l.text
   if txt and self.mouseover then
      lg.setColor( luatk.colour.text )
      local font = luatk._deffont
      local maxw, wrap = font:getWrap( txt, w )
      local th = #wrap * font:getLineHeight()
      lg.printf( txt, luatk._deffont, x, y+(h-th)/2, w, 'center' )
   end
   if l.q then
      lg.setColor( luatk.colour.text )
      lg.printf( string.format("%d",l.q), luatk._deffont, x+5, y+5, w-10, 'right' )
   end
end
function wgtBoard:clicked( _mx, _my, btn )
   if not self.loot then return end
   
   if btn==1 then
      self.selected = not self.selected
   elseif btn==2 then
      board_lootOne( self )
   end
end

local board_wdw
local board_wgt
local board_plt
local board_freespace
function board( plt )
   board_plt = plt
   loot_mod = player.pilot():shipstat("loot_mod", true)
   loot = compute_lootables( plt )

   -- Destroy if exists
   if board_wdw then
      board_wdw:destroy()
   end

   local font = lg.newFont(naev.conf().font_size_def)
   font:setOutline(1)
   luatk.setDefaultFont( font )

   local w, h = 480,310 
   local wdw = luatk.newWindow( nil, nil, w, h )
   board_wdw = wdw

   luatk.newButton( wdw, w-20-80, h-20-30, 80, 30, _("Close"), board_close )
   luatk.newButton( wdw, w-20-80-100, h-20-30, 80, 30, _("Loot"), board_lootSel )
   luatk.newButton( wdw, w-20-80-200, h-20-30, 80, 30, _("Loot All"), board_lootAll )

   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Boarding {shipname}"),{shipname=plt:name()}), nil, "center" )
   board_freespace = luatk.newText( wdw, 20, 40, w-40, 20, "" )
   board_updateFreespace()

   local y = 70
   local b = 80
   local m = 10
   local nrows = 2
   local id = 1
   board_wgt = {}
   for j=1,nrows do
      for i=1,5 do
         local l = lootables[id]
         local wx, wy = 20+(i-1)*(m+b), y+(j-1)*(m+b)
         local wgt = wgtBoard.new( wdw, wx, wy, b, b, l )
         id = id+1
         table.insert( board_wgt, wgt )
      end
   end

   luatk.run()
end

function board_updateFreespace ()
   board_freespace:set( fmt.f(_("You have {freespace} of free space."),{freespace=fmt.tonnes(player.pilot():cargoFree())}) )
end

function board_lootOne( wgt, nomsg )
   local looted = false
   local clear = false
   local l = wgt.loot
   if not l then return end
   -- Moolah
   if l.type=="credits" then
      board_plt:credits( -l.q ) -- Will go negative with loot_mod
      player.pay( l.q )
      looted = true
      clear = true
      player.msg(fmt.f(_("You looted {creds} from {plt}."),{creds=fmt.credits(l.q), plt=board_plt:name()}))
   elseif l.type=="fuel" then
      local pp = player.pilot()
      local ps = pp:stats()
      local pf = ps.fuel_max - ps.fuel
      local q = math.min( l.q, pf )
      -- Unable to loot anything
      if q <= 0 then
         if not nomsg then
            luatk.msg(_("Fuel Tank Full"), _("Your fuel tanks are already full!"))
         end
         return false
      end
      board_plt:setFuel( board_plt:stats().fuel - q )
      pp:setFuel( ps.fuel + q )
      player.msg(fmt.f(_("You looted {amount} fuel from {plt}."),{amount=q, plt=board_plt:name()}))
      looted = true

      -- Looted all the fuel
      l.q = l.q - q
      if l.q <= 0 then
         clear = true
      end
   elseif l.type=="outfit" then
      local o = l.data
      if board_plt:outfitRm( o ) ~= 1 then
         warn(fmt.f(_("Board script failed to remove '{outfit}' from boarded pilot '{plt}'!"),{outfit=o:name(), plt=boarded_plt:name()}))
      end
      player.outfitAdd( o )
      looted = true
      clear = true
      player.msg(fmt.f(_("You looted a {outfit} from {plt}."),{outfit=o:name(), plt=board_plt:name()}))
   elseif l.type=="cargo" then
      local c = l.data
      local pp = player.pilot()
      local cf = pp:cargoFree()
      local q = math.min( l.q, cf )
      -- Unable to loot anything
      if q <= 0 then
         if not nomsg then
            luatk.msg(_("No Space"), _("You have no free cargo space!"))
         end
         return false
      end
      local qr = board_plt:cargoRm( c, q ) -- Might be a misaligned here with loot_mod, but we sort of ignore it :/
      pp:cargoAdd( c, q )
      player.msg(fmt.f(_("You looted {amount} of {cargo} from {plt}."),{amount=fmt.tonnes(q), cargo=c:name(), plt=board_plt:name()}))
      board_updateFreespace()
      looted = true

      -- Looted all the cargo
      l.q = l.q - q
      if l.q <= 0 then
         clear = true
      end
   end

   -- Clean up
   if looted then
      wgt.selected = false
      wgt.loot = nil
   end
   return looted
end

function board_lootSel ()
   local sel = {}
   for _k,w in ipairs(board_wgt) do
      if w.selected then
         table.insert( sel, w )
      end
   end
   for _k,w in ipairs(sel) do
      board_lootOne( w, #sel>1 )
   end
end

function board_lootAll ()
   for _k,w in ipairs(board_wgt) do
      board_lootOne( w, true )
   end
end

function board_close ()
   luatk.close()
   board_wdw = nil
end
