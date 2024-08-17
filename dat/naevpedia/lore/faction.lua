local lg = require "love.graphics"
local luatk = require "luatk"

local fctlib = {}

local fct

local function wgt ()
   local val, str = fct:playerStanding()
   local c = "N"
   local fp = faction.player()
   if fct:areEnemies(fp) then
      c = 'H'
   elseif fct:areAllies(fp) then
      c = 'F'
   end
   local txt = string.format( p_("standings", "#%c%+d%%#0   [ %s ]"), c:byte(), val, str )

   local wgt_img = luatk.newImage( nil, 0, 0, 192, 192, lg.newImage(fct:logo()) )
   local wgt_stand = luatk.newText( nil, 0, 0, nil, nil, txt, nil, "center" )
   return luatk.newContainer( nil, -10, 0, nil, nil, { wgt_img, wgt_stand }, {
      center = true
   } )
end

function fctlib.init( fname )
   fct = faction.get( fname )
   return wgt
end

return fctlib
