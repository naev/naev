return {
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
   ambient_songs = {
      "ambient2.ogg", "mission.ogg",
      "peace1.ogg", "peace2.ogg", "peace4.ogg", "peace6.ogg",
      "void_sensor.ogg", "ambiphonic.ogg",
      "ambient4.ogg", "terminal.ogg", "eureka.ogg",
      "ambient2_5.ogg", "78pulse.ogg", "therewillbestars.ogg",
   },
   -- Factional songs. Systems default to songs of dominant factions.
   factional_songs = {
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
   nebula_combat_songs = {
      "nebu_battle1.ogg",
      "nebu_battle2.ogg",
      "combat1.ogg",
      "combat2.ogg",
   },
   combat_songs = {
      "combat3.ogg",
      "combat1.ogg",
      "combat2.ogg",
      "vendetta.ogg",
   },
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
      ["Hypergate Protera"] = { "landing_sinister.ogg" },
      ["Protera Husk"] = { "landing_sinister.ogg" },
      ["One-Wing Goddard"] = { "/snd/sounds/songs/inca-spa.ogg" },
      ["Research Post Sigma-13"] = function ()
            if not diff.isApplied("sigma13_fixed1") and
               not diff.isApplied("sigma13_fixed2") then
               return "landing_sinister.ogg"
            end
         end,
   },
   -- System-specific songs. Replace songs for certain systems
   system_ambient_songs = {
      ["Taiomi"] = { "/snd/sounds/songs/inca-spa.ogg" },
      ["Test of Enlightenment"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Alacrity"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Renewal"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Devotion"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
   },
}
