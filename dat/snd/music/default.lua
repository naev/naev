return {
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
   -- System-specific songs. Replace songs for certain systems
   system_ambient_songs = {
      ["Taiomi"] = { "/snd/sounds/songs/inca-spa.ogg" },
      ["Test of Enlightenment"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Alacrity"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Renewal"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
      ["Test of Devotion"] = { "/snd/sounds/loops/kalimba_atmosphere.ogg" },
   },
}
