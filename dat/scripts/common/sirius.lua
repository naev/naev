--[[

   Sirius Common Functions

--]]
local srs = {}

srs.prefix = "#y".._("SIRIUS: ").."#0"

function srs.playerIsPsychic ()
   return (var.peek("sirius_psychic")==true)
end

function srs.addAcHackLog( text )
   shiplog.create( "achack", _("Academy Hack"), _("Sirius") )
   shiplog.append( "achack", text )
end

function srs.addHereticLog( text )
   shiplog.create( "heretic", _("Heretic"), _("Sirius") )
   shiplog.append( "heretic", text )
end

local ssys, sysr
function srs.obeliskEnter ()
   ssys = system.cur()
   sysr = ssys:radius()

   -- Hide rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   ssys:setHidden(false)

   -- Stop and play different music
   music.stop()
   -- TODO sound

   hook.update( "_srs_update_limits" )
end

function srs.obeliskExit ()
   ssys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
end

-- Forces the player (and other ships) to stay in the radius of the system
function _srs_update_limits ()
   for k,p in ipairs(pilot.get()) do
      local pos = p:pos()
      local d = pos:dist()
      if d > sysr then
         local _m, dir = pos:polar()
         p:setPos( vec2.newP( sysr, dir ) )
      end
   end
end

return srs
