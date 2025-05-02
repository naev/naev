return {
   -- Priority of the songs here. Lower priority will precede. Defaults to 5 if unspecified.
   priority = 10,
   loading_songs = {
      "machina.ogg",
   },
   intro_songs = {
      "intro.ogg",
   },
   credits_songs = {
      "empire1.ogg",
   },
   -- Songs chosen randomly when taking off
   takeoff_songs = {
      "liftoff.ogg",
      "launch2.ogg",
      "launch3chatstart.ogg",
   },
   -- Neutral ambient songs
   ambient_songs_func = function ()
      local sys = system.cur()
      local tags = sys:tags()
      local nebu = sys:nebula() > 0
      if tags.wildspace then
         return {
            "wild_space.ogg",
         }
      elseif nebu then
         return {
            "ambient1.ogg",
            "ambient3.ogg",
            "dreamy_homage.ogg",
            "mellow_suspension.ogg",
         }
      end
      return {
         "ambient2.ogg",
         "ambient2_5.ogg",
         "ambient4.ogg",
         "peace1.ogg",
         "peace2.ogg",
         "peace4.ogg",
         "peace6.ogg",
         "mission.ogg",
         "void_sensor.ogg",
         "ambiphonic.ogg",
         "terminal.ogg",
         "eureka.ogg",
         "78pulse.ogg",
         "therewillbestars.ogg",
      }
   end,
   -- Factional songs. Systems default to songs of dominant factions.
   factional_songs = {
      -- TODO disco_melody.ogg for dreamer clan
      Collective = { "collective1.ogg", "automat.ogg" },
      Pirate     = { "pirate1_theme1.ogg", "pirates_orchestra.ogg", "ambient4.ogg",
                     "terminal.ogg" },
      Empire     = { "empire1.ogg", "empire2.ogg"; add_neutral = true },
      Sirius     = { "sirius1.ogg", "sirius2.ogg"; add_neutral = true },
      Dvaered    = { "dvaered1.ogg", "dvaered2.ogg"; add_neutral = true },
      ["Za'lek"] = { "zalek1.ogg", "zalek2.ogg", "approach.ogg"; add_neutral = true },
      Thurion    = { "motherload.ogg", "dark_city.ogg", "ambient1.ogg", "ambient3.ogg" },
      Proteron   = { "heartofmachine.ogg", "imminent_threat.ogg", "ambient4.ogg" },
   },
   combat_songs_func = function ()
      local sys = system.cur()
      local tags = sys:tags()
      local nebu = sys:nebula() > 0
      if tags.wildspace then
         return {
            "wild_space.ogg"
         }
      elseif nebu then
         return {
            "nebu_battle1.ogg",
            "nebu_battle2.ogg",
         }
      end
      return {
         "combat1.ogg",
         "combat2.ogg",
         "combat3.ogg",
         "vendetta.ogg",
         "run_under_the_sun.ogg",
      }
   end,
   -- Factional combat songs. Defaults to dominant factions.
   factional_combat_songs = {
      Collective = { "collective2.ogg", "galacticbattle.ogg", "battlesomething1.ogg", "combat3.ogg" },
      Pirate     = { "battlesomething2.ogg", "blackmoor_tides.ogg", add_neutral = true },
      Empire     = { "galacticbattle.ogg", "battlesomething2.ogg", add_neutral = true },
      Goddard    = { "flf_battle1.ogg", "battlesomething1.ogg", add_neutral = true },
      Dvaered    = { "flf_battle1.ogg", "battlesomething2.ogg", "for_grandeur.ogg", add_neutral = true },
      ["FLF"]    = { "flf_battle1.ogg", "battlesomething2.ogg", add_neutral = true },
      Frontier   = { "flf_battle1.ogg", add_neutral = true },
      Sirius     = { "galacticbattle.ogg", "battlesomething1.ogg", add_neutral = true },
      Soromid    = { "galacticbattle.ogg", "battlesomething2.ogg", add_neutral = true },
      ["Za'lek"] = { "collective2.ogg", "galacticbattle.ogg", "battlesomething1.ogg", add_neutral = true }
   },
   -- Spob-specific songs. Replace songs for certain spobs
   spob_songs = {
      ["Minerva Station"] = { "meeting_mtfox.ogg" },
      ["Strangelove Lab"] = { "landing_sinister.ogg" },
      ["One-Wing Goddard"] = { "/snd/sounds/songs/inca-spa.ogg" },
      ["Research Post Sigma-13"] = function ()
            if not diff.isApplied("sigma13_fixed1") and
               not diff.isApplied("sigma13_fixed2") then
               return "landing_sinister.ogg"
            end
         end,
   },
   -- Spob-specific songs.
   spob_songs_func = function( spb )
      local class = spb:class()
      local tags = spb:tags()
      local nebu_dens = system:cur():nebula()
      local services = spb:services()

      -- Special conditions that limit
      if tags.ruined then
         return {
            "ruined_suspense.ogg",
            "dark_orchestra.ogg",
            "space_emergency.ogg",
         }
      elseif tags.prison then
         return {
            "pitch_black_pit.ogg",
         }
      elseif nebu_dens > 0 and tags.station then
         return {
            "nordic_saxo.ogg",
         }
      end

      -- We'll add stuff here
      local lst = {}
      if tags.urban then
         tmergei( lst, { "meet_the_fish.ogg" } )
      end
      if tags.station then
         tmergei( lst, { "cosmostation.ogg", "snabba_labba.ogg" } )
      end
      if services.shipyard then
         tmergei( lst, { "gonna_be_gone.ogg" } )
      end

      -- Added based on class
      if class == "M" or class=="H" then
         tmergei( lst, { "agriculture.ogg", "peaceful_world.ogg", "boschs_garden.ogg" } )
      elseif class == "O" then
         tmergei( lst, { "ocean.ogg" } )
      elseif class == "P" then
         tmergei( lst, { "snow.ogg", "nordic_winter_25_for_25.ogg" } )
      elseif class=="I" or class=="J" or class=="S" or class=="T" or class=="Y" then
         tmergei( lst, { "methyl_swamp.ogg" } )
      end

      -- Nothing add, so try to add something
      if #lst <= 0 then
         -- More generic defaults
         if services.inhabited then
            tmergei( lst, { "upbeat.ogg" } )
         else
            tmergei( lst, { "end_of_time.ogg", "the_last_mystery.ogg", "winterstorm_1.ogg" } )
         end
      end

      return lst
   end,
   -- System-specific songs. Replace songs for certain systems
   system_ambient_songs = {
      ["Taiomi"] = { "/snd/sounds/songs/inca-spa.ogg" },
      ["Test of Enlightenment"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Alacrity"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Renewal"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Devotion"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
   },
}
