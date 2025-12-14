return {
   -- Priority of the songs here. Lower priority will precede. Defaults to 5 if unspecified.
   priority = 10,
   loading_songs = {
      "machina",
   },
   intro_songs = {
      "intro",
   },
   credits_songs = {
      "empire1",
   },
   -- Songs chosen randomly when taking off
   takeoff_songs = {
      "liftoff",
      "launch2",
      "launch3chatstart",
   },
   -- Neutral ambient songs
   ambient_songs_func = function ()
      local sys = system.cur()
      local tags = sys:tags()
      local nebu = sys:nebula() > 0
      if tags.wildspace then
         return {
            "wild_space",
         }
      elseif nebu then
         return {
            "ambient1",
            "ambient3",
            "dreamy_homage",
            "mellow_suspension",
         }
      end
      return {
         "ambient2",
         "ambient2_5",
         "ambient4",
         "peace1",
         "peace2",
         "peace4",
         "peace6",
         "mission",
         "void_sensor",
         "ambiphonic",
         "terminal",
         "eureka",
         "78pulse",
         "therewillbestars",
      }
   end,
   -- Factional songs. Systems default to songs of dominant factions.
   factional_songs = {
      -- TODO disco_melody for dreamer clan
      Collective = { "collective1", "automat" },
      Pirate     = { "pirate1_theme1", "pirates_orchestra", "ambient4",
                     "terminal" },
      Empire     = { "empire1", "empire2"; add_neutral = true },
      Sirius     = { "sirius1", "sirius2"; add_neutral = true },
      Dvaered    = { "dvaered1", "dvaered2"; add_neutral = true },
      ["Za'lek"] = { "zalek1", "zalek2", "approach"; add_neutral = true },
      Thurion    = { "motherload", "dark_city", "ambient1", "ambient3" },
      Proteron   = { "heartofmachine", "imminent_threat", "ambient4" },
   },
   combat_songs_func = function ()
      local sys = system.cur()
      local tags = sys:tags()
      local nebu = sys:nebula() > 0
      if tags.wildspace then
         return {
            "wild_space"
         }
      elseif nebu then
         return {
            "nebu_battle1",
            "nebu_battle2",
         }
      end
      return {
         "combat1",
         "combat2",
         "combat3",
         "vendetta",
         "run_under_the_sun",
      }
   end,
   -- Factional combat songs. Defaults to dominant factions.
   factional_combat_songs = {
      Collective = { "collective2", "galacticbattle", "battlesomething1", "combat3" },
      Pirate     = { "battlesomething2", "blackmoor_tides", add_neutral = true },
      Empire     = { "galacticbattle", "battlesomething2", add_neutral = true },
      Goddard    = { "flf_battle1", "battlesomething1", add_neutral = true },
      Dvaered    = { "flf_battle1", "battlesomething2", "for_grandeur", add_neutral = true },
      ["FLF"]    = { "flf_battle1", "battlesomething2", add_neutral = true },
      Frontier   = { "flf_battle1", add_neutral = true },
      Sirius     = { "galacticbattle", "battlesomething1", add_neutral = true },
      Soromid    = { "galacticbattle", "battlesomething2", add_neutral = true },
      ["Za'lek"] = { "collective2", "galacticbattle", "battlesomething1", add_neutral = true }
   },
   -- Spob-specific songs. Replace songs for certain spobs
   spob_songs = {
      ["Minerva Station"] = { "meeting_mtfox" },
      ["Strangelove Lab"] = { "landing_sinister" },
      ["One-Wing Goddard"] = { "/snd/sounds/songs/inca-spa" },
      ["Research Post Sigma-13"] = function ()
            if not diff.isApplied("sigma13_fixed1") and
               not diff.isApplied("sigma13_fixed2") then
               return "landing_sinister"
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
            "ruined_suspense",
            "dark_orchestra",
            "space_emergency",
         }
      elseif tags.prison then
         return {
            "pitch_black_pit",
         }
      elseif nebu_dens > 0 and tags.station then
         return {
            "nordic_saxo",
         }
      end

      -- We'll add stuff here
      local lst = {}
      if tags.urban then
         tmergei( lst, { "meet_the_fish" } )
      end
      if tags.station then
         tmergei( lst, { "cosmostation", "snabba_labba" } )
      end
      if services.shipyard then
         tmergei( lst, { "gonna_be_gone" } )
      end

      -- Added based on class
      if class == "M" or class=="H" then
         tmergei( lst, { "agriculture", "peaceful_world", "boschs_garden" } )
      elseif class == "O" then
         tmergei( lst, { "ocean" } )
      elseif class == "P" then
         tmergei( lst, { "snow", "nordic_winter_25_for_25" } )
      elseif class=="I" or class=="J" or class=="S" or class=="T" or class=="Y" then
         if not services.inhabited then
            tmergei( lst, { "methyl_swamp" } )
         end
      end

      -- Nothing add, so try to add something
      if #lst <= 0 then
         -- More generic defaults
         if services.inhabited then
            tmergei( lst, { "upbeat" } )
         else
            tmergei( lst, { "end_of_time", "the_last_mystery", "winterstorm_1" } )
         end
      end

      return lst
   end,
   -- System-specific songs. Replace songs for certain systems
   system_ambient_songs = {
      ["Taiomi"] = { "/snd/sounds/songs/inca-spa" },
      ["Test of Enlightenment"] = { "/snd/sounds/loops/kalimba_atmosphere" },
      ["Test of Alacrity"] = { "/snd/sounds/loops/kalimba_atmosphere" },
      ["Test of Renewal"] = { "/snd/sounds/loops/kalimba_atmosphere" },
      ["Test of Devotion"] = { "/snd/sounds/loops/kalimba_atmosphere" },
   },
}
