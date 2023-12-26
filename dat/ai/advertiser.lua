require 'ai.core.core'
require 'ai.core.idle.advertiser'
require 'ai.core.misc.distress'
local fmt = require "format"
local dv = require "common.dvaered"

mem.lanes_useneutral = true
mem.simplecombat = true

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
   _("Endogenous DNA damage hampering your Gene Drive? Drop by your local Chromosomal Rearrangement Laboratory for a check-up."),
   _("Visit Bohr Laboratory for all your epistatic mutation woes. 10 locations and counting!"),
   _("Worried about your bio-ship adenosine triphosphate output? Leave it to ATP Specialists!"),
   _("Preemptively treat your bio-ship for space fleas with Dr. Nastya's Ointment!"),
}
local ads_zalek = {
   _("Want to solve a large-dimensional stochastic PDE? The LMKSTD method is what you need!"),
   _("Love non-convex minimization? Join Ruadan's Computation Science Lab!"),
   _("Keeping your drones in top shape. Prof. Imarisha's Robotic Laboratory."),
   _("Interested in Genetic Lifeforms research? Apply to Interstice Science!"), -- Reference to Aperture Science (synonyms) from Portal
   _("Want to learn about Anti-Mass Spectometry? Join Ebony Plateau today!"), -- Reference to Black Mesa (synonyms) from Half-Life
}
local ads_sirius = {
   _("Want a new look? Try Verrill's Ceremonial Robes at Burnan!"),
}

-- Special ads that can be multi-faction
local ads_cyber = {
   _("Nexus Augmentation: trust the galactic leader in cyber-organs rental!"),
   _("Love your children? Get them the new Nexus Augmentation NCB-567K cyber-brain and they will never fail an exam!"),
   _("Rent arrears for your cyber-organs? Take out a credit at Nexus Bank and save your vital organs from being removed!"),
}

function create ()
   create_pre()

   -- Credits.
   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/180, price/40) )

   -- No bribe
   local bribe_msg = {
      _([["Just leave me alone!"]]),
      _([["What do you want from me!?"]]),
      _([["Get away from me!"]])
   }
   mem.bribe_no = bribe_msg[ rnd.rnd(1,#bribe_msg) ]

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   mem.refuel_msg = fmt.f(_([["I'll supply your ship with fuel for {credits}."]]),
         {credits=fmt.credits(mem.refuel)})

   -- Set up potential advertiser messages in msg variable
   local msg = tmergei( {}, ads_generic )

   -- Faction specific messages
   local fpres = system.cur():presences()

   -- Empire messages
   local fem = fpres["Empire"] or 0
   if fem > 1 then
      msg = tmergei( msg, ads_empire )
      msg = tmergei( msg, ads_cyber )
   end

   -- Dvaered messages
   local fdv = fpres["Dvaered"] or 0
   if fdv > 1 then
      msg = tmergei( msg, ads_dvaered )
      local badwords = {
         _("Butthead"),
         _("Nincompoop"),
         _("Dunderhead"),
         _("Ass"),
         _("Fool"),
         _("Coward"),
      }
      local lords = dv.warlords () -- Gets all warlorlds
      local r = rnd.rnd(1,#lords)
      local butthead = lords[r]
      table.remove( lords, r )
      local sponsor = lords[ rnd.rnd(1,#lords) ]
      local params = {butthead=butthead, badword=badwords[rnd.rnd(1,#badwords)], sponsor=sponsor, article="a"}
      if params.badword == "Ass" then
         params.article = "an"
      end
      table.insert(msg, fmt.f(_("I hereby declare {butthead} is {article} {badword}. -{sponsor}"), params))
      table.insert(msg, fmt.f(_("Let it be known that {butthead} is {article} {badword}. -{sponsor}"), params))
   end

   -- Soromid messages
   local fsr = fpres["Soromid"] or 0
   if fsr > 1 then
      msg = tmergei( msg, ads_soromid )
   end

   -- Soromid+Empire messages
   if fsr > 1 and fem > 1 then
      table.insert(msg, _("Remember to fill in your EX-29528-B form if returning to the Empire from Soromid territory."))
   end

   -- Za'lek messages
   local fzl = fpres["Za'lek"] or 0
   if fzl > 1 then
      msg = tmergei( msg, ads_zalek )
      msg = tmergei( msg, ads_cyber )
      -- Note that when running in the main menu background, player.name() might not exist (==nil), so
      -- we need to add a check for that.
      local pn = player.name()
      if pn then
         table.insert(msg, fmt.f(_("Dear Prof. {player}, your recent work has left a deep impression on us. Due to the advance, novelty, and possible wide application of your innovation, we invite you to contribute other unpublished papers of relevant fields to the Interstellar Pay-to-win Journal for Mathematics and Applications."), {player=pn}))
      end
   end

   -- Sirius messages
   local fsi = fpres["Sirius"] or 0
   if fsi > 1 then
      msg = tmergei( msg, ads_sirius )
   end

   mem.ad = msg[rnd.rnd(1,#msg)]

   -- Custom greeting
   mem.comm_greet = mem.ad

   mem.loiter = rnd.rnd(5,20) -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end
