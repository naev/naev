local equipopt = require "equipopt"
local pir = require "common.pirate"

return function ()
   local pers = {}

   local scur = system.cur()
   local pres = pir.systemPresence( scur )
   if pres <= 0 then
      return nil -- Need at least some presence
   end

   -- Larger ships can be there
   if pres > 50 then
      for k,v in ipairs{
         { -- Anchovy Brothers
            spawn = function ()
               local pos, vel
               local function anchovy_spawn( name, greet, taunt )
                  local p = pilot.add("Pirate Shark", "Pirate", pos, name, {naked=true, ai="pers_pirate"})
                  equipopt.pirate( p, {
                     outfits_add={"Emergency Stasis Inducer"},
                     prefer={["Emergency Stasis Inducer"] = 100}} )
                  local m = p:memory()
                  m.comm_greet = greet
                  m.taunt = taunt
                  if not pos then
                     pos = p:pos()
                     vel = p:vel()
                  else
                     p:setVel( vel )
                  end
                  return p
               end
               local p = {}
               p[1] = anchovy_spawn( _("Anchovy Cyko"),
                  _([["It's tough being an anchovy in the big fish world."]]),
                  _("Anchovy Brothers, attack!") )
               p[2] = anchovy_spawn( _("Anchovy Nikola"),
                  _([["The Anchovy Brothers bow down to no one!"]]),
                  _("Prepare to become fish food!") )
               p[3] = anchovy_spawn( _("Anchovy Azerty"),
                  _([["Enough anchovies together can take down any big fish!"]]),
                  _("Feel our anchovy wrath!") )
               p[2]:setLeader( p[1] )
               p[3]:setLeader( p[1] )
               return p
            end,
            w = 0.5,
         }, { -- Sardine Sisters
            spawn = function ()
               local pos, vel
               local function sardine_spawn( name, greet, taunt )
                  local p = pilot.add("Pirate Vendetta", "Pirate", pos, name, {naked=true, ai="pers_pirate"})
                  equipopt.pirate( p, {
                     outfits_add={"Emergency Stasis Inducer"},
                     prefer={["Emergency Stasis Inducer"] = 100}} )
                  local m = p:memory()
                  m.comm_greet = greet
                  m.taunt = taunt
                  if not pos then
                     pos = p:pos()
                     vel = p:vel()
                  else
                     p:setVel( vel )
                  end
                  return p
               end
               local p = {}
               p[1] = sardine_spawn( _("Sardine Addy"),
                  _([["Never underestimate the power of a determined school of sardines!"]]),
                  _("Sardine Sisters, attack!") )
               p[2] = sardine_spawn( _("Sardine Milli"),
                  _([["The Sardine Sisters bow down to no one!"]]),
                  _("Prepare to become fish food!") )
               p[3] = sardine_spawn( _("Sardine Alva"),
                  _([["Enough sardines together can take down any big fish!"]]),
                  _("Feel our sardine wrath!") )
               p[2]:setLeader( p[1] )
               p[3]:setLeader( p[1] )
               return p
            end,
            w = 0.5,
         },
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
