--[[

   Za'lek Black hole Common Functions

--]]
local vn = require "vn"
local ferals = require "common.ferals"

local zbh = {}

zbh.sfx = ferals.sfx

-- Zach Xiao
zbh.zach = {
   portrait = "zach.webp",
   image = "zach.webp",
   name = _("Zach"),
   colour = nil,
   transition = "hexagon",
   description = _("Zach looks like he is idle at the bar. You wonder what he's thinking about."),
}
zbh.icarus = {
   name = _("Icarus"),
   portrait = nil,
   -- TODO change image to proper one when Nohinohi graphics are changed
   image = 'gfx/ship/reaver/reaver_comm.webp',
   colour = nil,
}
zbh.pi = {
   portrait = "zalek_thug1.png",
   image = "zalek_thug1.png",
   name = _("Za'lek PI"),
   colour = nil,
}

function zbh.vn_zach( params )
   return vn.Character.new( zbh.zach.name,
         tmerge( {
            image=zbh.zach.image,
            colour=zbh.zach.colour,
         }, params) )
end

function zbh.vn_icarus( params )
   return vn.Character.new( zbh.icarus.name,
         tmerge( {
            image=zbh.icarus.image,
            colour=zbh.icarus.colour,
         }, params) )
end

function zbh.vn_pi( params )
   return vn.Character.new( zbh.pi.name,
         tmerge( {
            image=zbh.pi.image,
            colour=zbh.pi.colour,
         }, params) )
end

function zbh.log( text )
   shiplog.create( "zlk_blackhole", _("Black Hole"), _("Za'lek") )
   shiplog.append( "zlk_blackhole", text )
end

zbh.unidiff_list = {
   "sigma13_fixed1",
   "sigma13_fixed2",
}

--[[
   Evil Principal Investigator (PI) Faction
--]]
function zbh.evilpi ()
   local id = "zbh_evilpi"
   local f = faction.exists( id )
   if f then
      return f
   end
   return faction.dynAdd( "Za'lek", id, _("Evil PI"),
         {clear_enemies=true, clear_allies=true} )
end

function zbh.fzach ()
   local id = "zbh_zach"
   local f = faction.exists( id )
   if f then
      return f
   end
   return faction.dynAdd( "Za'lek", id, _("Za'lek"),
         {clear_enemies=true, clear_allies=true} )
end

function zbh.feralbioship ()
   local id = "zbh_feralbioship"
   local f = faction.exists( id )
   if f then
      return f
   end
   return faction.dynAdd( nil, id, _("Feral Bioship"), {ai="feralbioship"} )
end

function zbh.plt_icarus( pos )
   return pilot.add( "Nohinohi", zbh.feralbioship(), pos, _("Icarus") )
end

function zbh.unidiff( diffname )
   for _k,d in ipairs(zbh.unidiff_list) do
      if diff.isApplied(d) then
         diff.remove(d)
      end
   end
   diff.apply( diffname )
end

zbh.rewards = {
   zbh01 = 200e3,
   zbh02 = 300e3,
   zbh03 = 400e3,
   zbh04 = 300e3,
   zbh05 = 600e3,
   zbh06 = 500e3,
   --zbh07 = 0, -- Just a cutscene
   zbh08 = 500e3,
   zbh09 = 600e3,
   zbh10 = 700e3,
}

zbh.fctmod = {
   zbh01 = 2,
   zbh02 = 2,
   zbh03 = 2,
   zbh04 = 2,
   zbh05 = 3,
   zbh06 = 2,
   --zbh07 = 0, -- Just a cutscene
   zbh08 = 2,
   zbh09 = 3,
   zbh10 = 3,
   zbh11 = 2,
}

return zbh
