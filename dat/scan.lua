local luatk = require "luatk"
local wgtEquipment = require "luatk.equipment"
local fmt = require "format"

function scan ()
   local plt = player.pilot():target()

   local w, h = 500, 600
   local wdw = luatk.newWindow( nil, nil, w, h )
   local function wdw_close () return wdw:destroy() end
   wdw:setCancel( wdw_close )
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Scanning {plt}"), {plt=plt}), nil, "center" )

   -- Equipment can determine height
   local weq = wgtEquipment.new( wdw, 20, 40, 200, 500, plt )
   h = math.max( 300, 40+weq:height()+20+30+20 )
   wdw:resize( w, h )

   -- Buttons
   luatk.newButton( wdw, w-20-80, h-20-30, 80, 30, _("Close"), wdw_close )

   -- Cargo
   local x = 20+weq:width()+20
   local y = 40
   luatk.newText( wdw, x, y, w-x-20, 30, "#n".._("Cargo:") )
   y = y+25
   local cargostr = ""
   for k,c in ipairs(plt:cargoList()) do
      cargostr = cargostr..fmt.f(_("{amount} of {cargo}").."\n", {
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
      intrinsicstr = intrinsicstr..tostring(v).."\n"
   end
   if intrinsicstr=="" then
      intrinsicstr = _("No intrinsic outfits detected.")
   end
   luatk.newText( wdw, x, y, w-x-20, 200, intrinsicstr )

   luatk.run()
end
