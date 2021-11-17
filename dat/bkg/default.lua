--[[
   Default background
--]]
local starfield = require "bkg.lib.starfield"
local nebula = require "bkg.lib.nebula"

local nebu_blacklist = {
   ["Mizar"] = true,
   ["Haven"] = true,
   ["Amaroq"] = true,
   ["PSO"] = true,
   ["Sultan"] = true,
   ["Faust"] = true,
}

local function nebula_add_local( cpos, sys, radius, params )
   params = params or {}
   local spos  = sys:pos()
   local d     = cpos:dist( spos )
   local scale = (radius - d) / radius
   if scale > 0 then
      params.scale = 5*scale
      params.offset = (spos-cpos)*2
      nebula.init( params )
   end
end

function background ()
   local csys = system.cur()
   local nebud, _nebuv = csys:nebula()
   if nebud > 0 then
      return
   end

   local cpos = csys:pos()
   local radius = 300
   local maxscale = 0
   for _k,s in ipairs(system.getAll()) do
      local neb = s:nebula()
      if neb > 0 and not nebu_blacklist[s:nameRaw()] then
         local d = s:pos():dist( cpos )
         local scale = (radius - d) / radius
         maxscale = math.max( maxscale, scale )
      end
   end
   if maxscale > 0 then
      nebula.init{ scale=8*maxscale, offset=(system.get("Sol"):pos()-cpos)*2 }
   end

   -- Haven
   nebula_add_local( cpos, system.get("Haven"), 150, {hue_inner=160/360, hue_outter=200/360, opacity=55 } )

   -- Mizar
   nebula_add_local( cpos, system.get("Mizar"), 150, {hue_inner=80/360, hue_outter=120/360, opacity=55 } )

   -- PSO
   nebula_add_local( cpos, system.get("PSO"), 300, {hue_inner=330/360, hue_outter=270/360, opacity=58 } )

   starfield.init()
end

renderbg = starfield.render
