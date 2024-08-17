local lg = require "love.graphics"
local luatk = require "luatk"

local fctlib = {}

local fct, s

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


   local wgt_img = luatk.newImage( nil, 0, 0, s, s, lg.newImage(fct:logo()) )
   local wgt_stand = luatk.newText( nil, 0, 0, s, nil, txt, nil, "center" )
   return luatk.newContainer( nil, -10, 0, s, nil, { wgt_img, wgt_stand } )
end

function fctlib.init( fname )
   fct = faction.get( fname )
   s = 256
   return wgt
end

return fctlib
