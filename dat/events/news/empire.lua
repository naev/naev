local header = {
   _("Welcome to the Empire News Centre."),
}
local greeting = {
   _("Fresh news from around the Empire."),
   _("Remembering the Incident."),
   _("Keeping you informed."),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      tag = N_([[Terraforming Emperor's Fist]]),
      desc = _([[New bleeding-edge terraforming techniques to be tried on Emperor's Fist. Studies show that these techniques could speed up the terraforming process by as much as 40%.]])
   },
   {
      tag = N_([[Bees Introduced to Emperor's Fist]]),
      desc = _([[As they prepare the gardens of the future Imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5.]])
   },
   --[[
      Business
   --]]
   {
      tag = N_([[Empire Keeping Traders Safe]]),
      desc = _([[Recent studies show that reports of piracy on Trader vessels have gone down by up to 40% in some sectors. This is a demonstration of the Empire's commitment to eradicating piracy.]])
   },
   {
      tag = N_([[Nexus Contract Finalised]]),
      desc = _([[The Empire agreed to terms with shipbuilder Nexus for a new generation of military craft. The deal extends the partnership with the government for another 10 cycles.]])
   },
   --[[
      Politics
   --]]
   {
      tag = N_([[New Empire Recruits]]),
      desc = _([[Emperor's recruiting strategy a success. Many new soldiers joining the Empire Armada. "We haven't had such a successful campaign in ages!" - Raid Steele, spokesman for recruiting campaign.]])
   },
   {
      tag = N_([[Governor Helmer Jailed]]),
      desc = _([[Imperial Auditors arrested governor Rex Helmer of Anecu on charges of corruption. He has been removed from office and transported to a holding facility awaiting trial.]])
   },
   {
      tag = N_([[Imperial Council Opens Doors]]),
      desc = _([[The supreme advisory body invited undergraduates from six top schools to sit in on a day's deliberations. Topics required biodiversity strategy.]])
   },
   --[[
      Human interest.
   --]]
   {
      tag = N_([[New Cat in the Imperial Family]]),
      desc = _([[The Emperor's daughter was recently gifted a cat. Cat could be named "Snuggles" and seems to be all white.]])
   },
   {
      tag = N_([[Emperor's Aid Gets Hitched]]),
      desc = _([[Imperial secretary Karil Lorenze married long time fiancee Rachid Baouda in the future palace gardens on Emperor's Fist. His Eminence the Bishop of Bao performed the ceremony.]])
   },
   {
      tag = N_([[Remembering the Past]]),
      desc = _([[The Emperor has scheduled a new monument to be constructed on Emperor's Fist in honour of all those dead in the Incident.]])
   },
}

return function ()
   return "Empire", header, greeting, articles
end
