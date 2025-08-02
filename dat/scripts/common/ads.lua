-- Shared resource for advertisement messages

local ads_generic = {
   _("Fly safe, fly Milspec."),
   _("Reynir's Hot Dogs: enjoy the authentic taste of tradition."),
   _("Everyone is faster than light, but only Tricon engines are faster than thought!"),
   _("Dare excellence! Dare Teracom rockets!"),
   _("Most people are ordinary. For the others, Nexus designed the Shark fighter."),
   _("Never take off without your courage. Never take off without your Vendetta."),
   _("Unicorp: low price and high quality!"),
   _("Life is short, spend it at Minerva Station in the Limbo System!"),
   _("Insuperable Sleekness. Introducing the Krain Industries Starbridge."),
   _("Take care of the ones you do love. Let your Enygma System Turreted Launchers deal with the ones you don't!"),
}

local ads_empire = {
   _("Do you love your Emperor as much as he loves you?"),
   _("You're quick and dependable? The Emperor needs you in the Armada!"),
   _("Made money hauling cargo? Remember to file your EW-59831 every cycle or face the consequences."),
   _("Need help with your EW-59831 form? Try Bob's Bureaucratic Bazaar at Semper!"),
   _("Not filing your EE-91726 for unauthorized pet iguanas is a crime. Report to your Empire Animal Bureau now."),
   _("Keep your documents properly filed. Unicorp Filing Cabinets."),
   _("Want to test your skills in racing? Come to the Melendez Dome in the Qex system!"),
}

local ads_dvaered = {
   _("Only your hard work will save the Dvaered economy!"),
   _("Together, we will beat the economic crisis!"),
   _("Bet on Totoran and win incredible sums thanks to the Crimson Gauntlet!"),
   _("Mace rockets lacking shine? Try Lady Killington's premium rocket polish!"),
   _("Other warlords not letting you enjoy bloodshed? Join Lord Easytrigger's battalion today!"),
   _("A Dvaered Success Story: Buy the outstanding autobiography by Lady Bitterfly. "),
}

local ads_soromid = {
   _("Remember Sorom."),
   _("Special offer on Crow: buy one IR-eye, and the second comes for free!"),
   _("Looking to modify an entire species? Visit Dr. Hu's Gene Clinic at Point Zero Nidus!"),
   _(
      "Endogenous DNA damage hampering your Gene Drive? Drop by your local Chromosomal Rearrangement Laboratory for a check-up."
   ),
   _("Visit Bohr Laboratory for all your epistatic mutation woes. 10 locations and counting!"),
   _("Worried about your bio-ship adenosine triphosphate output? Leave it to ATP Specialists!"),
   _("Preemptively treat your bio-ship for space fleas with Dr. Nastya's Ointment!"),
}

local ads_zalek = {
   _("Want to solve a large-dimensional stochastic PDE? The LMKSTD method is what you need!"),
   _("Love non-convex minimization? Join Ruadan's Computation Science Lab!"),
   _("Keeping your drones in top shape. Prof. Imarisha's Robotic Laboratory."),
   _("Interested in Genetic Lifeforms research? Apply to Interstice Science!"), -- Reference to Aperture Science (synonyms) from Portal
   _("Want to learn about Anti-Mass Spectrometry? Join Ebony Plateau today!"), -- Reference to Black Mesa (synonyms) from Half-Life
}

local ads_sirius = {
   _("Want a new look? Try Verrill's Ceremonial Robes at Burnan!"),
}

-- Special ads that can be multi-faction
local ads_cyber = {
   _("Nexus Augmentation: trust the galactic leader in cyber-organs rental!"),
   _("Love your children? Get them the new Nexus Augmentation NCB-567K cyber-brain and they will never fail an exam!"),
   _(
      "Rent arrears for your cyber-organs? Take out a credit at Nexus Bank and save your vital organs from being removed!"
   ),
}

-- Returns the table of text based on the faction, in string form
-- May be worth doing as an enum instead
local function ads_for_faction(faction)
   if faction == "generic" then
      return ads_generic
   elseif faction == "empire" then
      return ads_empire
   elseif faction == "dvaered" then
      return ads_dvaered
   elseif faction == "soromid" then
      return ads_soromid
   elseif faction == "zalek" then
      return ads_zalek
   elseif faction == "sirius" then
      return ads_sirius
   elseif faction == "cyber" then
      return ads_cyber
   else
      error("Unknown faction name used: ads.lua")
   end
end

-- Returns a random ad from the specified faction
local function random_ad_for_faction(faction)
   local tbl = ads_for_faction(faction)
   return tbl[rnd.rnd(1, #tbl)]
end

return {
   ads_for_faction = ads_for_faction,
   random_ad_for_faction = random_ad_for_faction,
}
