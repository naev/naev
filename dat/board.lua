--[[
   Boarding script, now in Lua!
--]]
local luatk = require 'luatk'
local lg = require 'love.graphics'
local fmt = require 'format'
local der = require "common.derelict"

local board_lootOne -- forward-declared function, defined at bottom of file
local loot_mod
local special_col = {0.7, 0.45, 0.22} -- Dark Gold

local function slotTypeColour( stype )
   local c
   if stype=="Weapon" then
      c = "p"
   elseif stype=="Utility" then
      c = "g"
   elseif stype=="Structure" then
      c = "n"
   else
      c = "0"
   end
   return "#"..c.._(stype)
end

local function slotSizeColour( size )
   local c
   if size=="Large" then
      c = "r"
   elseif size=="Medium" then
      c = "b"
   elseif size=="Small" then
      c = "y"
   else
      c = "0"
   end
   return "#"..c.._(size)
end

local function outfit_loot( o, price )
   local name, size, prop = o:slot()
   local sprop = (prop and "\n#o".._(prop).."#0") or ""
   local stype = _(o:type())
   local sprice = ""
   if price and price > 0 then
      local bonus = price / o:price()
      local sbonus
      if bonus > 1 then
         sbonus = string.format("#r%+d", bonus*100 - 100)
      else
         sbonus = string.format("#g%+d", bonus*100 - 100)
      end
      sprice = fmt.f(_("\n#rCosts {credits} ({sbonus}%#r from crew strength) to extract!#0"), {credits=fmt.credits(price), sbonus=sbonus})
   end
   local desc = fmt.f(_("{name}{sprice}\n{slotsize} {slottype}#0 slot{sprop}\n{stype}\n{desc}"),
         { name=o:name(),
           sprice=sprice,
           desc=o:description(),
           slottype=slotTypeColour(name),
           slotsize=slotSizeColour(size),
           sprop=sprop,
           stype=stype})
   local col = nil
   if o:unique() then
      col = special_col
   end
   return {
      image = lg.newImage( o:icon() ),
      text = o:name(),
      q = nil,
      type = "outfit",
      bg = col,
      alt = desc,
      data = o,
      price = price,
   }
end

local cargo_image_generic = nil
local function cargo_loot( c, q, m )
   local icon = c:icon()
   local ispecial = m or c:price()==0
   local desc = fmt.f(_("{name}\n{desc}"), {name=c:name(),desc=_(c:description())})
   local illegalto = c:illegality()
   if #illegalto > 0 then
      desc = desc.._("\n#rIllegalized by the following factions:\n")
      for _k,f in ipairs(illegalto) do
         if f:known() then
            desc = desc..fmt.f(_("\n   - {fct}"), {fct=f})
         end
      end
      desc = desc.."#0"
   end
   return {
      image = (icon and lg.newImage(icon)) or cargo_image_generic,
      text = c:name(),
      q = math.floor( 0.5 + q*loot_mod ),
      type = "cargo",
      bg = (ispecial and special_col) or nil,
      alt = desc,
      data = c,
   }
end

local function compute_lootables ( plt )
   local ps = plt:stats()
   local pps = player.pilot():stats()
   local lootables = {}

   -- Credits
   local creds = plt:credits()
   if creds > 0 then
      table.insert( lootables, {
         image = nil,
         text = _("Credits"),
         q = math.floor( 0.5 + creds*loot_mod ),
         type = "credits",
         bg = nil,
         alt = _("Credits\nUsed as the main form of currency throughout the galaxy."),
         data = nil,
      } )
   end

   -- Fuel
   local fuel = ps.fuel
   if fuel > 0 then
      table.insert( lootables, {
         image = nil,
         text = _("Fuel"),
         q = math.floor( 0.5 + fuel*loot_mod ),
         type = "fuel",
         bg = nil,
         alt = _("Fuel\nNecessary for the activation of jump drives that allow inter-system travel."),
         data = nil,
      } )
   end

   -- Steal outfits before cargo, since they are always considered "special"
   local canstealoutfits = true
   if canstealoutfits then
      local mem = plt:memory()
      local oloot = mem.lootable_outfit
      if oloot then
         local lo = outfit_loot( oloot )
         lo.bg = special_col
         table.insert( lootables, lo )
      end

      local ocand = {}
      for _k,o in ipairs(plt:outfitsList(nil,true)) do -- Skips locked outfits
         local _name, _size, _prop, req = o:slot()
         local ot = o:tags()
         -- Don't allow looting required outfits
         if not req and not ot.noplayer and not ot.nosteal and o~=oloot then
            table.insert( ocand, o )
         end
      end
      -- Get random candidate if available
      if #ocand > 0 then
         -- TODO better criteria
         local o = ocand[ rnd.rnd(1,#ocand) ]
         local price = o:price() * (10+ps.crew) / (10+pps.crew)
         local lo = outfit_loot( o, price )
         table.insert( lootables, lo )
      end
   end

   -- Go over cargo
   local clist = plt:cargoList()
   for _k,c in ipairs(clist) do
      c.c = commodity.get(c.name)
   end
   table.sort( clist, function( a, b )
      -- Handle mission cargoes first
      if a.m and not b.m then
         return true
      elseif not a.m and b.m then
         return false
      end
      local ap = a.c:price()
      local bp = b.c:price()
      -- Look at special case of price being 0
      if ap==0 and bp~=0 then
         return true
      elseif ap~=0 and bp==0 then
         return true
      end
      return ap > bp
   end )
   for _k,c in ipairs(clist) do
      table.insert( lootables, cargo_loot( c.c, c.q, c.m ) )
   end

   -- Display nice versions of the quantity
   for _k,c in ipairs(lootables) do
      if c.q then
         c.qs = fmt.number( c.q )
      end
   end

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
   wgt.selalpha = 0
   return wgt
end
function wgtBoard:update( dt )
   dt = dt * 5
   if self.selected then
      self.selalpha = math.min( 1, self.selalpha + dt )
   else
      self.selalpha = math.max( 0, self.selalpha - dt )
   end
end
function wgtBoard:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h
   local l = self.loot
   local a = self.selalpha
   if a < 1 then
      lg.setColor( luatk.colour.outline )
      lg.rectangle( "fill", x-1, y-1, w+2, h+2 )
      lg.setColor( (l and l.bg) or {0,0,0} )
      lg.rectangle( "fill", x, y, w, h )
   end
   if a > 0 then
      lg.setColor( {0,1,1,a} )
      lg.rectangle( "fill", x-3, y-3, w+6, h+6 )
      lg.setColor( {0,0.5,0.5,a} )
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
   if txt and (self.mouseover or not img) then
      lg.setColor( luatk.colour.text )
      local font = luatk._deffont
      local _maxw, wrap = font:getWrap( txt, w )
      local th = #wrap * font:getLineHeight()
      lg.printf( txt, luatk._deffont, x, y+(h-th)/2, w, 'center' )
   end
   if l.qs then
      lg.setColor( luatk.colour.text )
      lg.printf( l.qs, luatk._deffont, x+5, y+5, w-10, 'right' )
   end
end
function wgtBoard:drawover( bx, by )
   local x, y, w = bx+self.x, by+self.y, self.w
   local l = self.loot
   if not l then return end
   local alt = l.alt
   if alt and self.mouseover then
      luatk.drawAltText( x+w, y, alt, 400 )
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
local function board_updateFreespace ()
   board_freespace:set( fmt.f(_("You have {freespace} of free space."),{freespace=fmt.tonnes(player.pilot():cargoFree())}) )
end

local function board_lootSel ()
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

local function board_lootAll ()
   for _k,w in ipairs(board_wgt) do
      if not w.loot or not w.loot.price or w.loot.price <= 0 then
         board_lootOne( w, true )
      end
   end
end

local function can_cannibalize ()
   local pp = player.pilot()
   if pp:ship():tags().cannibal then
      return true
   end
   if player.shipvarPeek("cannibal") then
      return true
   end
   for _k,o in ipairs(pp:outfitsList("all")) do
      if o:tags().cannibal then
         return true
      end
   end
   return false
end

local function board_cannibalize ()
   local armour, shield = board_plt:health(true)
   local cannibal2 = player.shipvarPeek("cannibal2")

   if armour <= 1 then
      return
   end
   local bs = board_plt:stats()

   local pp = player.pilot()
   local ps = pp:stats()
   local parmour, pshield, pstress = pp:health(true)

   local dmg = math.min( (armour-1), 2*(ps.armour-parmour) )
   if dmg <= 0 then
      return
   end

   board_plt:setHealth( 100*(armour-dmg)/bs.armour, 100*shield/bs.shield, 100 )
   local heal_armour = dmg/2
   if cannibal2 then
      heal_armour = dmg*2/3
   end
   pp:setHealth( 100*(parmour+heal_armour)/ps.armour, 100*pshield/ps.shield, pstress )
   player.msg(fmt.f(_("Your ship cannibalized {armour:.0f} armour from {plt}."),{armour=heal_armour, plt=board_plt}))
end

local function cargo_list ()
   local clist = player.fleetCargoList()
   table.sort( clist, function ( a, b )
      return a.c:price() < b.c:price()
   end )
   local cnames = {}
   for k,v in ipairs(clist) do
      cnames[k] = fmt.f(_("{cargo} ({credits}/tonne, {tonnes})"),
         {cargo=v.c, tonnes=fmt.tonnes(v.q), credits=fmt.credits(v.c:price())})
   end

   return clist, cnames
end
local cargo_btn, cargo_wdw, cargo_lst, cargo_jet -- forward declaration

local function cargo_jettison( cargo, max )
   luatk.msgFader( fmt.f(_("Jettison {cargo}"),{cargo=cargo}),
      fmt.f(_("How many tonnes of {cargo} do you wish to jettison?"),{cargo=cargo}), 1, max, 10, function( val )
      if not val then
         return
      end
      val = math.floor( val + 0.5 )
      luatk.yesno( fmt.f(_("Jettison {cargo}?"), {cargo=cargo}),
         fmt.f(_("Are you sure you want to get rid of {tonnes} of {cargo}?"),
            {tonnes=fmt.tonnes(val), cargo=cargo} ),
         function ()
            local q = player.fleetCargoJet( cargo, val )
            luatk.msg( fmt.f(_("Bye Bye {cargo}"),{cargo=cargo} ),
               fmt.f(_("You dump {tonnes} of {cargo} out the airlock of your ship."),
                  {tonnes=fmt.tonnes(q), cargo=cargo}) )
               board_updateFreespace()

               cargo_lst:destroy()
               local clist, cnames = cargo_list ()
               if #clist <= 0 then
                  cnames = { _("None") }
                  cargo_btn:disable()
                  cargo_jet:disable()
               end
               local w, h = cargo_wdw:getDimensions()
               cargo_lst = luatk.newList( cargo_wdw, 20, 65, w-40, h-130, cnames )
         end )
   end )
end

local function manage_cargo ()
   local clist, cnames = cargo_list ()
   if #cnames <= 0 then return end

   local w, h = 300, 400
   local wdw = luatk.newWindow( nil, nil, w, h )
   cargo_wdw = wdw

   luatk.newText( wdw, 0, 10, w, 20, _("Manage Cargo"), nil, "center" )

   luatk.newText( wdw, 20, 40, w-40, 20, _("Select cargo to jettison:") )
   cargo_lst = luatk.newList( wdw, 20, 65, w-40, h-130, cnames )

   luatk.newButton( wdw, w-20-120, h-20-30, 120, 30, _("Close"), function ()
      cargo_wdw:destroy()
   end )
   cargo_jet = luatk.newButton( wdw, 20, h-20-30, 120, 30, _("Jettison"), function ()
      local _c, cid = cargo_lst:get()
      local c = clist[ cid ]
      if not c then return end
      cargo_jettison( c.c, c.q )
      return true
   end )
end

local function board_close ()
   luatk.close()
   board_wdw = nil
   der.sfx.unboard:play()
end

function board( plt )
   if not plt:exists() then return end
   der.sfx.board:play()
   board_plt = plt
   loot_mod = player.pilot():shipstat("loot_mod", true)
   local lootables = compute_lootables( plt )

   local pp = player.pilot()
   if player.shipvarPeek("cannibal2") then
      pp:cooldownCycle()
   end

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
   if can_cannibalize() then
      luatk.newButton( wdw, w-20-80-350, h-20-30, 130, 30, _("Cannibalize"), board_cannibalize )
   end

   -- Add manage cargo button if applicable
   cargo_btn = luatk.newButton( wdw, w-20-120, 25, 120, 30, _("Manage Cargo"), manage_cargo )
   if #player.fleetCargoList() <= 0 then
      cargo_btn:disable()
   end

   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Boarding {plt}"), {plt=plt}), nil, "center" )
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

   wdw:setAccept( board_lootAll )
   wdw:setCancel( board_close )

   luatk.run()
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
      player.msg(fmt.f(_("You looted {creds} from {plt}."),{creds=fmt.credits(l.q), plt=board_plt}))
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
      player.msg(fmt.f(_("You looted {amount} fuel from {plt}."),{amount=q, plt=board_plt}))
      looted = true

      -- Looted all the fuel
      l.q = l.q - q
      if l.q <= 0 then
         clear = true
      end
   elseif l.type=="outfit" then
      local o = l.data
      if l.price and l.price > 0 then
         luatk.yesno(fmt.f(_("Extract {outfit}?"),{outfit=o}),
               fmt.f(_("It will cost #r{credits}#0 in repairs to successfully extract the {outfit}. You have {playercreds}. Extract the {outfit}?"),
                  {credits=fmt.credits(l.price), playercreds=fmt.credits(player.credits()), outfit=o}), function ()
            local pc = player.credits()
            if pc < l.price then
               luatk.msg(_("Insufficient Credits"), fmt.f(_("You lack #r{diff}#0 to be able to extract the {outfit}."),
                     {diff=fmt.credits(l.price-pc), outfit=o}))
               return
            end
            if board_plt:outfitRm( o ) ~= 1 then
               warn(fmt.f(_("Board script failed to remove '{outfit}' from boarded pilot '{plt}'!"),{outfit=o, plt=board_plt}))
            end
            player.pay( -l.price )
            player.outfitAdd( o )
            player.msg(fmt.f(_("You looted a {outfit} from {plt}."),{outfit=o, plt=board_plt}))
            wgt.selected = false
            wgt.loot = nil
         end )
         return false
      end
      if board_plt:outfitRm( o ) ~= 1 then
         -- Soft warning
	 print(fmt.f(_("Board script failed to remove '{outfit}' from boarded pilot '{plt}'!"),{outfit=o, plt=board_plt}))
      end
      player.outfitAdd( o )
      looted = true
      clear = true
      player.msg(fmt.f(_("You looted a {outfit} from {plt}."),{outfit=o, plt=board_plt}))
   elseif l.type=="cargo" then
      local c = l.data
      local cf = player.fleetCargoFree()
      local q = math.min( l.q, cf )
      -- Unable to loot anything
      if q <= 0 then
         if not nomsg then
            luatk.msg(_("No Space"), _("You have no free cargo space!"))
         end
         return false
      end
      board_plt:cargoRm( c, q ) -- Might be a misaligned here with loot_mod, but we sort of ignore it :/
      player.fleetCargoAdd( c, q ) -- We just use the original bonus computed with loot_mod
      player.msg(fmt.f(_("You looted {amount} of {cargo} from {plt}."),{amount=fmt.tonnes(q), cargo=c, plt=board_plt}))
      board_updateFreespace()
      looted = true

      -- Looted all the cargo
      l.q = l.q - q
      if l.q <= 0 then
         clear = true
      end
   end

   -- Clean up
   if clear then
      wgt.selected = false
      wgt.loot = nil
   else
      if l.q then
         l.qs = fmt.number( l.q )
      end
   end
   return looted
end
