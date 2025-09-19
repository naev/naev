local lg = require "love.graphics"
local luatk = require "luatk"

local fctlib = {}

local fct

local function wgt ()
   local val = fct:reputationGlobal()
   local str = fct:reputationText( val )
   local c = "N"
   local fp = faction.player()
   if fct:areEnemies(fp) then
      c = 'H'
   elseif fct:areAllies(fp) then
      c = 'F'
   end
   local txt = string.format( p_("standings", "#%c%+d%%#0   [ %s ]"), c:byte(), val, str )

   local wgtlist = {}
   table.insert( wgtlist, luatk.newImage( nil, 0, 0, 192, 192, lg.newImage(fct:logo()) ) )
   table.insert( wgtlist, luatk.newText( nil, 0, 0, nil, nil, txt, nil, "centre" ) )

   local function filter_known( lst )
      local newlst = {}
      for k,v in ipairs(lst) do
         if v:known() and not v:invisible() then
            table.insert( newlst, v )
         end
      end
      return newlst
   end

   local allies = filter_known( fct:allies() )
   local t = ""
   if #allies > 0 then
      if t ~= "" then
         t = t.."\n"
      end
      t = t.."#F".._("Allies:").."#0"
      for k,v in ipairs(allies) do
         t = t.."\n・"..v:longname()
      end
   end
   local enemies = filter_known( fct:enemies() )
   if #enemies > 0 then
      if t ~= "" then
         t = t.."\n"
      end
      t = t.."#H".._("Enemies:").."#0"
      for k,v in ipairs(enemies) do
         t = t.."\n・"..v:longname()
      end
   end
   if t ~= "" then
      table.insert( wgtlist, luatk.newText( nil, 0, 0, nil, nil, t ) )
   end
   return luatk.newContainer( nil, -10, 0, nil, nil, wgtlist, {
      centre = true
   } )
end

function fctlib.init( fname )
   fct = faction.get( fname )
   return wgt
end

return fctlib
