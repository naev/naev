-- Shared resource for advertisement messages
local fmt = require "format"
local dv = require "common.dvaered"

local DV_BADWORDS = {
   _("a Butthead"),
   _("a Nincompoop"),
   _("a Dunderhead"),
   _("an Ass"),
   _("a Fool"),
   _("a Coward"),
   _("a vacuous toffee-nosed malodorous Pervert"), -- Monty Python and the Holy Grail
}

-- Fun generator.
local function dv_generate_insult( msg )
   local lords = rnd.permutation( dv.warlords() ) -- Gets all warlorlds in random order
   return fmt.f( msg, {
      butthead = lords[1], -- First random warlord
      badword  = DV_BADWORDS[rnd.rnd(1,#DV_BADWORDS)],
      sponsor  = lords[2], -- Second random should be != first
   } )
end

local ads = {
   ads_generic = {
      _("Fly safe, fly Milspec."),
      _("Reynir's Hot Dogs: enjoy the authentic taste of tradition."),
      _("Everyone is faster than light, but only Tricon engines are faster than thought!"),
      _("Dare excellence! Dare Teracom rockets!"),
      _("Most people are ordinary. For the others, Nexus designed the Shark fighter."),
      _("Never take off without your courage. Never take off without your Vendetta."),
      _("Unicorp: low price and high quality!"),
      _("Life is short, spend it at Minerva Station in the Limbo System!"),
      _("Insuperable Sleekness. Introducing the Krain Industries Starbridge."),
      _("Take care of the ones you do love. Let your Enygma Systems Turreted Launchers deal with the ones you don't!"),
   },
   ads_empire = {
      _("Do you love your Emperor as much as he loves you?"),
      _("You're quick and dependable? The Emperor needs you in the Armada!"),
      _("Made money hauling cargo? Remember to file your EW-59831 every cycle or face the consequences."),
      _("Need help with your EW-59831 form? Try Bob's Bureaucratic Bazaar at Semper!"),
      _("Not filing your EE-91726 for unauthorized pet iguanas is a crime. Report to your Empire Animal Bureau now."),
      _("Keep your documents properly filed. Unicorp Filing Cabinets."),
      _("Want to test your skills in racing? Come to the Melendez Dome in the Qex system!"),
   },
   ads_dvaered = {
      _("Only your hard work will save the Dvaered economy!"),
      _("Together, we will beat the economic crisis!"),
      _("Bet on Totoran and win incredible sums thanks to the Crimson Gauntlet!"),
      _("Mace rockets lacking shine? Try Lady Killington's premium rocket polish!"),
      _("Other warlords not letting you enjoy bloodshed? Join Lord Easytrigger's battalion today!"),
      _("A Dvaered Success Story: Buy the outstanding autobiography by Lady Bitterfly. "),
      _("Kids show poor discipline? Lord Bigbonk's Military Academy can help!"),
      function () return dv_generate_insult(_("I hereby declare {butthead} is {badword}. -{sponsor}")) end,
      function () return dv_generate_insult (_("Let it be known that {butthead} is {badword}. -{sponsor}")) end,
   },
   ads_soromid = {
      _("Remember Sorom."),
      _("Special offer on Crow: buy one IR-eye, and the second comes for free!"),
      _("Looking to modify an entire species? Visit Dr. Hu's Gene Clinic at Point Zero Nidus!"),
      _("Endogenous DNA damage hampering your Gene Drive? Drop by your local Chromosomal Rearrangement Laboratory for a check-up."),
      _("Visit Bohr Laboratory for all your epistatic mutation woes. 10 locations and counting!"),
      _("Worried about your bio-ship adenosine triphosphate output? Leave it to ATP Specialists!"),
      _("Preemptively treat your bio-ship for space fleas with Dr. Nastya's Ointment!"),
      _("Love life lacking? Ask your genetic specialist about Dr. Zoidberg's Sensorial Tentacles and Pheromones enhancements!"), -- Futurama
   },
   ads_zalek = {
      _("Want to solve a large-dimensional stochastic PDE? The LMKSTD method is what you need!"),
      _("Love non-convex minimization? Join Ruadan's Computation Science Lab!"),
      _("Keeping your drones in top shape. Prof. Imarisha's Robotic Laboratory."),
      _("Interested in Genetic Lifeforms research? Apply to Interstice Science!"), -- Reference to Aperture Science (synonyms) from Portal
      _("Want to learn about Anti-Mass Spectrometry? Join Ebony Plateau today!"), -- Reference to Black Mesa (synonyms) from Half-Life
      function ()
         -- Player may not exist, so add fallback
         local pn = player.name() or _("$LASTNAME $FIRSTNAME")
         return fmt.f(_("Dear Prof. {player}, your recent work has left a deep impression on us. Due to the advance, novelty, and possible wide application of your innovation, we invite you to contribute other unpublished papers of relevant fields to the Interstellar Pay-to-win Journal for Mathematics and Applications."), {player=pn})
      end,
   },
   ads_sirius = {
      _("Want a new look? Try Verrill's Ceremonial Robes at Burnan!"),
      _("New flow focus courses at Firriot's Academy!"),
      _("Pilgrim ferry operators wanted! Inquire at your local mission BBS."),
      _("New call for Flow Resonator Calibrator Specialists. Inquire at your nearest Monastery now!"),
      _("All Praise Sirichana! Paid for by the Sirichana Preacher Association."),
   },

   -- Special ads that can be multi-faction
   ads_empire_zalek = {
      _("Nexus Augmentation: trust the galactic leader in cyber-organs rental!"),
      _("Love your children? Get them the new Nexus Augmentation NCB-567K cyber-brain and they will never fail an exam!"),
      _("Rent arrears for your cyber-organs? Take out a credit at Nexus Bank and save your vital organs from being removed!"),
   },
   ads_empire_soromid = {
      _("Remember to fill in your EX-29528-B form if returning to the Empire from Soromid territory."),
   }
}

--[[--
Obtains all the available advertisements in a system.

   @tparam Boolean flatten Whether or not to convert all messages to strings as necessary.
   @treturn Table{String,...} A table of strings (and functions if not flatten) containing the different potential advertisements.
--]]
function ads.system_ads( flatten )
   local msg = tmergei( {}, ads.ads_generic )

   -- Faction specific messages
   local fpres = system.cur():presences()

   -- Empire messages
   local fem = fpres["Empire"] or 0
   if fem > 1 then
      msg = tmergei( msg, ads.ads_empire )
   end

   -- Dvaered messages
   local fdv = fpres["Dvaered"] or 0
   if fdv > 1 then
      msg = tmergei( msg, ads.ads_dvaered )
   end

   -- Soromid messages
   local fsr = fpres["Soromid"] or 0
   if fsr > 1 then
      msg = tmergei( msg, ads.ads_soromid )
   end

   -- Za'lek messages
   local fzl = fpres["Za'lek"] or 0
   if fzl > 1 then
      msg = tmergei( msg, ads.ads_zalek )
   end

   -- Sirius messages
   local fsi = fpres["Sirius"] or 0
   if fsi > 1 then
      msg = tmergei( msg, ads.ads_sirius )
   end

   -- Soromid+Empire messages
   if fsr > 1 and fem > 1 then
      msg = tmergei( msg, ads.ads_empire_soromid )
   end

   -- Empire+Za'lek messages
   if fem > 1 or fzl > 1 then
      msg = tmergei( msg, ads.ads_empire_zalek )
   end

   -- Flatten the functions
   if flatten then
      for i,m in ipairs(msg) do
         msg[i] = ads.ad_to_text(m)
      end
   end

   return msg
end

function ads.generate_ad ()
   local a = ads.system_ads()
   return ads.ad_to_text( a[rnd.rnd(1,#a)] )
end

function ads.ad_to_text( ad )
   if type(ad)=="function" then
      return ad()
   end
   return ad
end

return ads
