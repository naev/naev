local equipopt = require "equipopt"
local pir = require "common.pirate"

local function add_treasure_map( p )
   local pm = p:memory()
   if not pm.lootables then
      pm.lootables = {}
   end
   pm.lootables["treasure_map"] = 1
end

return function ()
   local pers = {}

   local scur = system.cur()
   local pirpres = pir.systemPresence( scur )
   if pirpres <= 0 then
      return nil -- Need at least some presence
   end

   local fpir = faction.get("Pirate")
   local enemies = 0
   for k,v in pairs(scur:presences()) do
      local f = faction.get(k)
      if not f:tags().civilian and fpir:areEnemies( f ) then
         enemies = enemies + v
      end
   end

   -- Few enemies, spawn special ships
   --if enemies <= 100 then
   --end

   -- Larger ships can be there
   if pirpres > 50 and enemies < 600 then
      for k,v in ipairs{
         { -- Anchovy Brothers
            spawn = function ()
               local pos, vel
               local function anchovy_spawn( name, greet, taunt )
                  local p = pilot.add("Pirate Shark", "Pirate", pos, name, {naked=true, ai="pers_pirate"})
                  p:outfitAddIntrinsic("Escape Pod")
                  equipopt.pirate( p, {
                     outfits_add={"Emergency Stasis Inducer"},
                     prefer={["Emergency Stasis Inducer"] = 100}} )
                  local m = p:memory()
                  m.capturable = true
                  m.comm_greet = greet
                  m.taunt = taunt
                  if not pos then
                     pos = p:pos()
                     vel = p:vel()
                  else
                     p:setVel( vel )
                  end
                  add_treasure_map( p )
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
                  p:outfitAddIntrinsic("Escape Pod")
                  equipopt.pirate( p, {
                     outfits_add={"Emergency Stasis Inducer"},
                     prefer={["Emergency Stasis Inducer"] = 100}} )
                  local m = p:memory()
                  m.capturable = true
                  m.comm_greet = greet
                  m.taunt = taunt
                  if not pos then
                     pos = p:pos()
                     vel = p:vel()
                  else
                     p:setVel( vel )
                  end
                  add_treasure_map( p )
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

   if pirpres > 400 then
      for k,v in ipairs {
         {
            spawn = function ()
               local p = pilot.add("Pirate Kestrel", "Pirate", nil, _("Flying Dutchman"), {naked= true, ai="pers_pirate"})
               --p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet( "shield_mod", 80 )
               p:intrinsicSet( "armour_mod", 60 )
               p:intrinsicSet( "shield_regen_mod", 50 )
               p:intrinsicSet( "tur_damage", 25 )
               p:setNoDisable(true) -- Can't board or capture
               equipopt.pirate( p ) -- So intrinsics affect
               local m = p:memory()
               --m.capturable = true
               m.comm_greet = _([[You hear the sound of oceans and wild over the communication channel.]])
               m.taunt = nil
               m.bribe_no = _([[The wind is howling over the communication channel.]])
               -- TODO add higher quality map
               add_treasure_map( p )
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   local pres = scur:presences()

   local wildones = pres["Wild Ones"] or 0
   if wildones > 50  and enemies <= 600 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Revenant", "Wild Ones", nil, _("The Beast"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.pirate( p, { bioship_stage=6,
                  bioship_skills={
                     "bite1", "bite2", "bite3",
                     "attack1", "attack2", "attack3",
                  } } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([[You hear a weird sort of growling over the communication channel.]])
               m.taunt = _("Grrrrrrrawwwwwrrrr!")
               m.bribe_no = _([[You hear more weird growling sounds over the communication channel. It doesn't look like they are interested in discussion.]])
               add_treasure_map( p )
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   local ravenclan = pres["Raven Clan"] or 0
   if ravenclan > 150 and enemies < 900 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Zebra", "Raven Clan", nil, _("Raven's Talon"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.pirate( p, {fighterbay=5, bolt=0.1} )
               p:intrinsicSet( "fbay_reload", 100 )
               p:intrinsicSet( "fbay_rate", 50 )
               p:intrinsicSet( "fbay_movement", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["When I get sick of hauling cargo for the Raven Clan, nothing helps me unwind like some good old fashion piracy."]])
               m.taunt = _("Fighters, engage!")
               add_treasure_map( p )
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   local dreamerclan = pres["Dreamer Clan"] or 0
   if dreamerclan > 100 and enemies <= 700 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Starbridge", "Dreamer Clan", nil, _("Shroomancer"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.sirius( p, {
                  prefer={
                     ["Large Flow Amplifier"] = 100,
                     ["Medium Flow Resonator"] = 100,
                  },
                  type_range = {
                     ["Flow Modifier"] = { max=1 },
                  },
                  flow_ability=outfit.get("House of Mirrors"),
               } )
               p:intrinsicSet( "shield_mod", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["Did you know that there are over 5,000 types of psychedelic mushrooms grown on Sanchez alone?"]])
               m.taunt = _("You're ruining my vibes, soul!")
               add_treasure_map( p )
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   local blacklotus = pres["Black Lotus"] or 0
   if blacklotus > 50 and enemies <= 700 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Shark", "Black Lotus", nil, _("Feldspar"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.sirius( p, {noflow=true} )
               p:intrinsicSet( "shield_mod", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["I've been working overtime to get my rank up in the Black Lotus. I won't be the underdog forever!"]])
               m.taunt = _("You're my ticket to a promotion. Die!")
               add_treasure_map( p )
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Pirate Starbridge", "Black Lotus", nil, _("Lapis Lazuli"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.pirate( p, {
                  launcher = 2,
                  ["Launcher"] = { max=4 },
               } )
               p:intrinsicSet( "shield_mod", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["With so many careless merchants flying around, business has never been better. I always like to get my hands dirty."]])
               m.taunt = _("Let me add you to my trophy collection!")
               add_treasure_map( p )
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Pirate Starbridge", "Black Lotus", nil, _("Opal"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet( "shield_mod", 25 )
               equipopt.dvaered( p )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["It's hard being a rising star in the Black Lotus. They expect so much from you. Good thing I'm an overachiever."]])
               m.taunt = _("It's my time to shine!")
               add_treasure_map( p )
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end
   if blacklotus > 125 and enemies <= 1000 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Pirate Kestrel", "Black Lotus", nil, _("Emerald"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.zalek( p, {fighterbay=5} )
               p:intrinsicSet( "shield_mod", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["Paperwork is such a bore. I escaped from the Empire to join the Black Lotus only to find I have to fill out more and more paperwork as I rise in rank. Maybe I should have joined the Wild Ones instead?"]])
               m.taunt = _("Time for some fun!")
               add_treasure_map( p )
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Pirate Kestrel", "Black Lotus", nil, _("Topaz"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.soromid( p, {fighterbay=5} )
               p:intrinsicSet( "shield_mod", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _("(ΦωΦ)")
               m.taunt = _([["(｢・ω・)｢"]])
               add_treasure_map( p )
               return p
            end,
         },
      } do
         table.insert( pers, v )
      end
   end
   if blacklotus > 200 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Dealbreaker", "Black Lotus", nil, _("Diamond"), {naked=true, ai="pers_pirate"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.pirate( p, {fighterbay=5, beam=0.1} )
               p:intrinsicSet( "shield_mod", 25 )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["It's nice being near the top of the pecking order. It's fun to give contradictory orders to your subordinates and see them run around in chaos."]])
               m.taunt = _("Time to snare another bonus!")
               add_treasure_map( p )
               return p
            end,
         },
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
