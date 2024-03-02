local fmt = require "format"

local function flt_standard( p )
   local s = p:services()
   if not s.land or not s.inhabitable then
      return false
   end
   local t = p:tags()
   if t.restricted or t.military then
      return false
   end
   return true
end

local function emp_spob( filter )
   filter = filter or flt_standard
   local lst = spob.get( faction.get("Empire") )
   local newlst = {}
   for k,p in ipairs(lst) do
      if filter( p ) then
         newlst[ #newlst+1 ] = p
      end
   end
   return newlst[ rnd.rnd(1,#newlst) ]
end

local head = {
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
      head = N_([[Terraforming Emperor's Fist]]),
      body = _([[New bleeding-edge terraforming techniques to be tried on Emperor's Fist. Studies show that these techniques could speed up the terraforming process by as much as 40%.]])
   },
   {
      head = N_([[Bees Introduced to Emperor's Fist]]),
      body = _([[As they prepare the gardens of the future Imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5.]])
   },
   {
      head = N_([[Onion Society Causes Havoc]]),
      body = function ()
         local spb = emp_spob()
         return fmt.f(_([[A cyberattack orchestrated by the Onion Society took down the local network of {spb} for 3 periods. Imperial security analysts believe a zero-day exploit in new tax forms was used in the attack.]]), {
            spb = spb,
         } )
      end,
   },
   --[[
      Business
   --]]
   {
      head = N_([[Empire Keeping Traders Safe]]),
      body = _([[Recent studies show that reports of piracy on Trader vessels have gone down by up to 40% in some sectors. This is a demonstration of the Empire's commitment to eradicating piracy.]])
   },
   {
      head = N_([[Nexus Contract Finalised]]),
      body = _([[The Empire agreed to terms with shipbuilder Nexus for a new generation of military craft. The deal extends the partnership with the government for another 10 cycles.]])
   },
   --[[
      Politics
   --]]
   {
      head = N_([[New Empire Recruits]]),
      body = _([[Emperor's recruiting strategy a success. Many new soldiers joining the Empire Armada. "We haven't had such a successful campaign in ages!" - Raid Steele, spokesman for recruiting campaign.]])
   },
   {
      head = N_([[Governor Helmer Jailed]]),
      body = _([[Imperial Auditors arrested governor Rex Helmer of Dolmen on charges of corruption. He has been removed from office and transported to a holding facility awaiting trial.]])
   },
   {
      head = N_([[Imperial Council Opens Doors]]),
      body = _([[The supreme advisory body invited undergraduates from six top schools to sit in on a day's deliberations. Topics required biodiversity strategy.]])
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[New Cat in the Imperial Family]]),
      body = _([[The Emperor's daughter was recently gifted a cat. Cat could be named "Snuggles" and seems to be all white.]])
   },
   {
      head = N_([[Emperor's Aid Gets Hitched]]),
      body = _([[Imperial secretary Karil Lorenze married long time fiancée Rachid Baouda in the future palace gardens on Emperor's Fist. His Eminence the Bishop of Bao performed the ceremony.]])
   },
   {
      head = N_([[Remembering the Past]]),
      body = _([[The Emperor has scheduled a new monument to be constructed on Emperor's Fist in honour of all those dead in the Incident.]])
   },
}

return function ()
   return "Empire", head, greeting, articles
end
