local luatk = require "luatk"
local wgtEquipment = require "luatk.equipment"
local fmt = require "format"

function scan ()
   local plt = player.pilot():target()

   local w, h = 500, 600
   local wdw = luatk.newWindow( nil, nil, w, h )
   local function wdw_close () return wdw:destroy() end
   wdw:setCancel( wdw_close )
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Scanning {plt}"), {plt=plt}), nil, "centre" )

   -- Equipment can determine height
   local weq = wgtEquipment.new( wdw, 20, 40, 200, 500, plt )
   h = math.max( 300, 40+weq:height()+20+30+20 )
   wdw:resize( w, h )

   -- Buttons
   luatk.newButton( wdw, w-20-80, h-20-30, 80, 30, _("Close"), wdw_close )
   --[[ Doesn't work unless support for multiple dialogues is implemented...
   luatk.newButton( wdw, w-2*(20+80), h-20-30, 80, 30, _("Hail"), function ()
      luatk.close() -- Need to make sure it is closed
      player.commOpen( plt )
   end )
   --]]

   local x = 20+weq:width()+20
   local y = 40

   -- Fleet
   if plt:flags("carried") then
      local wcarried = luatk.newText( wdw, x, y, w-x-20, 30, _("This ship is a deployed fighter.") )
      y = y+wcarried:height()+20
   end

   local fleet
   local leader = plt:leader()
   if leader ~= nil then
      fleet = leader:followers(true)
   else
      fleet = plt:followers(true)
   end
   if #fleet > 0 then
      luatk.newText( wdw, x, y, w-x-20, 30, "#n".._("Ships in the same fleet:") )
      y = y+25
      local fleetstr = ""
      for k,p in ipairs(fleet) do
         if p~=plt then
            if fleetstr~="" then
               fleetstr = fleetstr.."\n"
            end
            fleetstr = fleetstr..p:name()
         end
      end
      local wfleet = luatk.newText( wdw, x, y, w-x-20, 200, fleetstr )
      y = y+wfleet:height()+20
   end

   -- Cargo
   luatk.newText( wdw, x, y, w-x-20, 30, "#n".._("Cargo:") )
   y = y+25
   local cargostr = ""
   for k,c in ipairs(plt:cargoList()) do
      if cargostr~="" then
         cargostr = cargostr.."\n"
      end
      cargostr = cargostr..fmt.f(_("{amount} of {cargo}"), {
         amount=fmt.tonnes(c.q),
         cargo=c.c,
      })
   end
   if cargostr=="" then
      cargostr = _("No cargo detected.")
   end
   local wcargo = luatk.newText( wdw, x, y, w-x-20, 200, cargostr )
   y = y+wcargo:height()+20

   -- Intrinsic Outfits
   local ointrinsic = plt:outfitsList("intrinsic")
   luatk.newText( wdw, x, y, w-x-20, 30, "#n".._("Intrinsic Outfits:") )
   y = y+25
   local intrinsicstr = ""
   for k,v in ipairs(ointrinsic) do
      if intrinsicstr~="" then
         intrinsicstr = intrinsicstr.."\n"
      end
      intrinsicstr = intrinsicstr..tostring(v)
   end
   if intrinsicstr=="" then
      intrinsicstr = _("No intrinsic outfits detected.")
   end
   luatk.newText( wdw, x, y, w-x-20, 200, intrinsicstr )

   luatk.run()
end
