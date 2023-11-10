local equipopt = require "equipopt"
local pir = require "common.pirate"

return function ()
   local pers = {}

   local scur = system.cur()
   local pirpres = pir.systemPresence( scur )
   if pirpres <= 0 then
      return nil -- Need at least some presence
   end

   -- Larger ships can be there
   if pirpres > 50 then
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

   local pres = scur:presences()

   local wildones = pres["Wild Ones"] or 0
   if wildones > 50 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Revenant", "Wild Ones", nil, _("The Beast"), {naked=true, ai="pers_pirate"})
               equipopt.pirate( p, { bioship_stage=6,
                  bioship_skills={
                     "bite1", "bite2", "bite3",
                     "attack1", "attack2", "attack3",
                  } } )
               local m = p:memory()
               m.comm_greet = _([[You hear a weird sort of growling over the communication channel.]])
               m.taunt = _("Grrrrrrrawwwwwrrrr!")
               m.bribe_no = _([[You hear more weird growling sounds over the communication channel. It doesn't look like they are interested in discussion.]])
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   local ravenclan = pres["Raven Clan"] or 0
   if ravenclan > 150 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Zebra", "Raven Clan", nil, _("Raven's Talon"), {naked=true, ai="pers_pirate"})
               equipopt.pirate( p, {fighterbay=5, beam=5} )
               p:intrinsicSet( "fbay_reload", 100 )
               p:intrinsicSet( "fbay_rate", 50 )
               p:intrinsicSet( "fbay_movement", 25 )
               local m = p:memory()
               m.comm_greet = _([["When I get sick of hauling cargo for the Raven Clan, nothing helps me unwind like some good old fashion piracy."]])
               m.taunt = _("Fighters, engage!")
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
