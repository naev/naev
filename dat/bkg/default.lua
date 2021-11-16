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
         maxscale = math.max( scale, maxscale )
      end
   end
   if maxscale > 0 then
      nebula.init{ scale=8*maxscale }
   end

   starfield.init()
end

renderbg = starfield.render
