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
      params.size = 3000*scale
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
   local prng = require("prng").new()
   prng:setSeed( system.cur():nameRaw() )

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
      nebula.init{ prng=prng, size=5000*maxscale, offset=(system.get("Sol"):pos()-cpos)*2, movemod=0.2 }
   end

   -- Haven
   nebula_add_local( cpos, system.get("Haven"), 150, { prng=prng, hue_inner=160/360, hue_outter=200/360, opacity=55, granularity=0.3 } )

   -- Mizar
   nebula_add_local( cpos, system.get("Mizar"), 150, { prng=prng, hue_inner=80/360, hue_outter=120/360, opacity=55, granularity=0.3 } )

   -- PSO
   nebula_add_local( cpos, system.get("PSO"), 300, { prng=prng, hue_inner=330/360, hue_outter=270/360, opacity=58, granularity=0.5, movemod=0.5 } )

   starfield.init()
end

renderbg = starfield.render
