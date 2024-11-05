--[[
   Boarding script, now in Lua!
--]]
local luatk = require 'luatk'
local lg = require 'love.graphics'
local fmt = require 'format'
local der = require "common.derelict"

local board_lootOne -- forward-declared function, defined at bottom of file
local board_fcthit_check
local loot_mod
local special_col = {0.7, 0.45, 0.22} -- Dark Gold
local board_close

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
      desc = desc.."#r".._("\nIllegalized by the following factions:\n")
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
         q = math.min( math.floor( 0.5 + fuel*loot_mod ), ps.fuel_max ),
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
      local numoutfits = math.floor(loot_mod)
      if rnd.rnd() < math.fmod(loot_mod,1) then
         numoutfits = numoutfits+1
      end
      for i=1,numoutfits do
         -- Get random candidate if available
         if #ocand > 0 then
            -- TODO better criteria
            local id = rnd.rnd(1,#ocand)
            local o = ocand[id]
            local price = o:price() * (10+ps.crew) / (10+pps.crew)
            local lo = outfit_loot( o, price )
            table.insert( lootables, lo )
            table.remove( ocand, id ) -- Remove from candidates
         end
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
   local a = self.selalpha
   if self.selected then
      self.selalpha = math.min( 1, self.selalpha + dt )
   else
      self.selalpha = math.max( 0, self.selalpha - dt )
   end
   if self.selalpha ~= a then
      luatk.rerender()
   end
end
function wgtBoard:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h
   local l = self.loot
   local a = self.selalpha
   if a < 1 then
      lg.setColour( luatk.colour.outline )
      lg.rectangle( "fill", x-1, y-1, w+2, h+2 )
      lg.setColour( (l and l.bg) or {0,0,0} )
      lg.rectangle( "fill", x, y, w, h )
   end
   if a > 0 then
      lg.setColour( {0,1,1,a} )
      lg.rectangle( "fill", x-3, y-3, w+6, h+6 )
      lg.setColour( {0,0.5,0.5,a} )
      lg.rectangle( "fill", x, y, w, h )
   end
   -- Ignore anything that isn't loot from now on
   if not l then return end

   local img = l.image
   if img then
      local iw, ih = img:getDimensions()
      lg.setColour( {1,1,1} )
      img:draw( x, y, 0, w / iw, h / ih )
   end
   local txt = l.text
   if txt and (self.mouseover or not img) then
      lg.setColour( luatk.colour.text )
      local font = luatk._deffont
      local _maxw, wrap = font:getWrap( txt, w )
      local th = #wrap * font:getLineHeight()
      lg.printf( txt, luatk._deffont, x, y+(h-th)/2, w, 'center' )
   end
   if l.qs then
      lg.setColour( luatk.colour.text )
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
      luatk.rerender()
      self.selected = not self.selected
   elseif btn==2 then
      luatk.rerender()
      board_lootOne( self )
   end
end

local board_wdw
local board_wgt
local board_plt
local board_fcthit
local board_freespace, board_fcthit_txt
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

   if #sel == 0 then
      return
   elseif #sel == 1 then
      board_lootOne( sel[1] )
   else
      board_fcthit_check( function ()
         for _k,w in ipairs(sel) do
            board_lootOne( w, #sel>1 )
         end
      end )
   end
end

local function board_lootAll ()
   board_fcthit_check( function ()
      for _k,w in ipairs(board_wgt) do
         if not w.loot or not w.loot.price or w.loot.price <= 0 then
            board_lootOne( w, true )
         end
      end
   end )
end

local function can_capture ()
   if player.fleetCapacity() <= 0 then
      return false
   end
   -- For now, only allow capturing one ship at a time. Potentially, make it so
   -- the player can have multiple if it fits fleet capacity, just a bit
   -- trickier to do interface-wise
   if player.evtActive("Ship Capture") then
      return false
   end
   return true
end

local function is_capturable ()
   local t = board_plt:ship():tags()
   if t.noplayer then
      return false, _("This ship is not capturable.")
   end
   if board_plt:flags("carried") then
      return false, _("You can not capture deployed fighters.")
   end
   local pm = board_plt:memory()
   if not pm.natural then
      return false, _("This ship is not capturable.")
   end
   if board_plt:flags("carried") then
      return false, _("You can not capture deployed fighters.")
   end
   local flttot, fltcur = player.fleetCapacity()
   if flttot-fltcur < board_plt:points() then
      return false, fmt.f(_("You do not have enough free fleet capacity to capture the ship. You need {needed}, but only have {have} free fleet capacity."),
         {needed=board_plt:points(), have=flttot-fltcur})
   end
   return true
end

local function board_capture ()
   local ok, msg = is_capturable()
   if not ok then
      luatk.msg(_("Unable to Capture Ship"), fmt.f(_("You are not able to capture the {shpname} for the following reason:\n\n{msg}"),
         {shpname=board_plt:name(), msg=msg}))
      return
   end

   local pp = player.pilot()
   local ps = board_plt:stats()
   local pps = pp:stats()
   -- TODO should this be affected by loot_mod and how?
   --local loot_mod = pp:shipstat("loot_mod", true)
   local bonus = (10+ps.crew) / (10+pps.crew) + 0.25
   local cost = board_plt:worth()
   local costnaked = cost
   local outfitsnaked = board_plt:outfits(nil,true) -- Get non-locked
   for k,v in pairs(outfitsnaked) do
      if v then
         -- Ignore outfits that can't be stolen
         if v:tags().nosteal then
            cost = cost - v:price()
         end
         outfitsnaked[k] = nil
         costnaked = costnaked - v:price()
      end
   end
   cost = cost * bonus
   costnaked = math.min( cost, costnaked * bonus * 1.1 ) -- Always a bit more expensive, but never more than base
   local sbonus
   if bonus > 1 then
      sbonus = string.format("#r%+d", bonus*100 - 100)
   else
      sbonus = string.format("#g%+d", bonus*100 - 100)
   end

   local fct = board_plt:faction()
   local fcthit = board_plt:points()
   local factionmsg = ""
   if not (fct:static() or fct:invisible()) then
      local rep = board_plt:reputation()
      local fcthittest = fct:hitTest( -fcthit, system.cur(), "capture" )
      if board_fcthit ~= 0 then
         fcthittest = fcthittest + fct:hitTest( -board_fcthit, system.cur(), "board" )
      end
      factionmsg = fmt.f(_(" Capturing the ship will lower your reputation with {fct} by {amount} (current standing is {current})."),
         {fct=fct, amount=fmt.number(math.abs(fcthittest)), current=rep})
      if rep+fcthittest < 0 then
         factionmsg = fmt.f(_([[{msg} This action will make you hostile with {fct}!]]),
            {msg=factionmsg, fct=fct})
      end
      factionmsg = "#r"..factionmsg.."#0"
   end

   local capturemsg = fmt.f(_([[Do you wish to capture the {shpname}? You estimate it will cost #o{credits}#0 ({sbonus}% from crew strength) in repairs to successfully restore the ship with outfits, and #o{creditsnaked}#0 without outfits. You have {playercreds}.{fctmsg}

You will still have to escort the ship and land with it to perform the repairs and complete the capture. The ship will not assist you in combat and will be lost if destroyed.]]),
      {shpname=board_plt:name(),
       credits=fmt.credits(cost),
       creditsnaked=fmt.credits(costnaked),
       playercreds=fmt.credits(player.credits()),
       fctmsg=factionmsg,
       sbonus=sbonus})

   luatk.yesno( _("Capture Ship?"), capturemsg,
      function ()
         luatk.msg(_([[Ship Taken Over]]),fmt.f(_([[You have taken over the {shp}. You will still have to escort the ship to an spaceport with refueling capabilities to complete the capture and then pay the reparation fee of {amount}.]]),
            {shp=board_plt:name(),amount=fmt.credits(cost)}))

         -- Faction hit
         local realhit = fct:hit( -fcthit, system.cur(), "capture" )
         player.msg("#r"..fmt.f(_("You lost {amt} reputation with {fct}."),{amt=realhit,fct=fct}).."#0")

         -- Start capture script
         local nc = naev.cache()
         nc.capture_pilot = {
            pilot=board_plt,
            cost=cost,
            costnaked=costnaked,
            outfitsnaked=outfitsnaked,
         }
         naev.eventStart("Ship Capture")
         board_close()
      end )
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
   player.msg(fmt.f(_("Your ship cannibalized {armour} armour from {plt}."),{armour=fmt.number(heal_armour), plt=board_plt}))
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

function board_close ()
   if board_wdw then
      board_wdw:destroy()
   end
   board_wdw = nil

   -- Player stole something to make it not spaceworthy, sorry bud, you're
   -- not waking up.
   if not board_plt:spaceworthy() then
      board_plt:setDisable(false) -- Permanently disable
   end

   der.sfx.unboard:play()
end

function board( plt )
   if not plt:exists() then return end

   if plt:flags("carried") then
      player.msg( "#r".._("You can not board deployed fighters.").."#0" )
      return
   end

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

   local fct = board_plt:faction()
   board_fcthit = math.max( 0, board_plt:points() )
   if fct:static() or fct:invisible() then
      board_fcthit = 0
   end

   local w, h = 570,350
   local wdw = luatk.newWindow( nil, nil, w, h )
   board_wdw = wdw
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Boarding {plt}"), {plt=plt}), nil, "center" )

   local x = w-20-80
   luatk.newButton( wdw, x, h-20-30, 80, 30, _("Close"), board_close )
   x = x-100
   luatk.newButton( wdw, x, h-20-30, 80, 30, _("Loot"), board_lootSel )
   x = x-100
   luatk.newButton( wdw, x, h-20-30, 80, 30, _("Loot All"), board_lootAll )
   x = x-100
   if can_capture() then
      local btn_capture = luatk.newButton( wdw, x, h-20-30, 80, 30, _("Capture"), board_capture )
      x = x-100
      local ok, msg = is_capturable()
      if not ok then
         btn_capture:disable()
         btn_capture:setAlt( msg )
      end
   end
   if can_cannibalize() then
      luatk.newButton( wdw, x, h-20-30, 130, 30, _("Cannibalize"), board_cannibalize )
      --x = x-100
   end

   -- Display about faction hits
   local fctmsg
   if board_fcthit > 0 then
      local loss = fct:hitTest( -board_fcthit, system.cur(), "board" )
      fctmsg = fmt.f(_("Looting anything from the ship will lower your reputation with {fct} by {fcthit}, and may anger nearby ships."),
            {fct=fct,fcthit=fmt.number(math.abs(loss))})
   else
      fctmsg = _("Looting anything from the ship may anger nearby ships.")
   end
   board_fcthit_txt = luatk.newText( wdw, 20, 40, w-40, 20, fctmsg )
   local fh = board_fcthit_txt:height()
   local dfh = luatk._deffont:getLineHeight()
   if fh > dfh then
      h = h + fh-(30-dfh)
      wdw:resize( w, h )
   end

   -- Add manage cargo button if applicable
   cargo_btn = luatk.newButton( wdw, w-20-120, 70, 120, 30, _("Manage Cargo"), manage_cargo )
   if #player.fleetCargoList() <= 0 then
      cargo_btn:disable()
      cargo_btn:setAlt(_("You have no cargo to manage."))
   end
   board_freespace = luatk.newText( wdw, 20, 80, w-40, 20, "" )
   board_updateFreespace()

   local y = 115
   local b = 80
   local m = 10
   local nrows = 2
   local id = 1
   board_wgt = {}
   for j=1,nrows do
      for i=1,6 do
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

function board_fcthit_check( func )
   local fct = board_plt:faction()
   local std = board_plt:reputation()
   local fcthittest = fct:hitTest( -board_fcthit, system.cur(), "board" )
   if (std>=0) and (std-fcthittest<0) then
      local msg = fmt.f(_("Looting anything from the ship will lower your reputation with {fct} by {amount} (current standing is {current}). #rThis action will make you hostile with {fct}!#0"),
         {fct=fct, amount=fmt.number(math.abs(fcthittest)), current=std})
      luatk.yesno(fmt.f(_([[Offend {fct}?]]),{fct=fct}), msg, function ()
         func()
      end )
   else
      func()
   end
end

local function board_fcthit_apply ()
   -- Piss off nearby ships
   board_plt:distress( player.pilot() )

   if board_fcthit <= 0 then
      return
   end
   local fct = board_plt:faction()
   local loss = fct:hit( -board_fcthit, system.cur(), "board" )
   board_fcthit = 0
   if loss <= 0 then
      -- No loss actually happened
      return
   end

   local msg = fmt.f(_("You have lost {fcthit} reputation with {fct} for looting this ship!"),
      {fcthit=fmt.number(loss),fct=fct})
   player.msg( "#r"..msg.."#0" )
   board_fcthit_txt:set( msg )
end

-- Frontend to do checks when looting
local board_lootOneDo
function board_lootOne( wgt, nomsg )
   local l = wgt.loot
   if not l then return end

   if nomsg then
      board_lootOneDo( wgt, nomsg )
   else
      board_fcthit_check( function ()
         board_lootOneDo( wgt, nomsg )
      end )
   end
end
-- Helper function that actually loots
function board_lootOneDo( wgt, nomsg )
   local looted = false
   local clear = false
   local l = wgt.loot

   -- Moolah
   if l.type=="credits" then
      board_plt:credits( -l.q ) -- Will go negative with loot_mod
      player.pay( l.q )
      looted = true
      clear = true
      player.msg(fmt.f(_("You looted {creds} from {plt}."),{creds=fmt.credits(l.q), plt=board_plt}))
      board_fcthit_apply()
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
      board_fcthit_apply()
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
            board_fcthit_apply()
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
      board_fcthit_apply()
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
      board_fcthit_apply()
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
