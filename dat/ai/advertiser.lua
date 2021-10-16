require 'ai.core.core'
require 'ai.core.idle.advertiser'
require 'ai.core.misc.distress'
local fmt = require "format"

mem.lanes_useneutral = true
mem.simplecombat = true

function create ()
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
   mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
         fmt.credits(mem.refuel))

   -- Selects an advertiser message
   local msg = {
      _("Fly safe, fly Milspec."),
      _("Reynir's Hot Dogs: enjoy the authentic taste of tradition."),
      _("Everyone is faster than light, but only Tricon engines are faster than thought!"),
      _("Dare excellence! Dare Teracom rockets!"),
      _("Most people are ordinary. For the others, Nexus designed the Shark fighter."),
      _("Never take off without your courage. Never take off without your Vendetta."),
      _("Unicorp: low price and high quality!"),
      _("Life is short, spend it at Minerva Station in the Limbo System!"),
      _("Insuperable Sleekness. Introducing the Krain Industries Starbridge."),
   }

   -- Faction specific messages
   local fpres = system.cur():presences()

   local fem = fpres["Empire"] or 0
   if fem > 1 then
      table.insert(msg, _("Do you love your Emperor as much as he loves you?"))
      table.insert(msg, _("You're quick and dependable? The Emperor needs you in the Armada!"))
      table.insert(msg, _("Made money hauling cargo? Remember to file your EW-59831 every cycle or face the consequences."))
      table.insert(msg, _("Need help with your EW-59831? Try Bob's Bureaucratic Bazaar at Semper!"))
      table.insert(msg, _("Not filing your EE-91726 for unauthorized pet iguanas is a crime. Report to your Empire Animal Bureau now."))
   end

   local fdv = fpres["Dvaered"] or 0
   if fdv > 1 then
      table.insert(msg, _("Only your hard work will save the Dvaered economy!"))
      table.insert(msg, _("Together, we will beat the economic crisis!"))
      table.insert(msg, _("Bet on Totoran and win incredible sums thanks to the Crimson Gauntlet!"))
      table.insert(msg, _("Mace rockets lacking shine? Try Lady Killington's premium rocket polish!"))
      table.insert(msg, _("Other warlords not letting you enjoy bloodshed? Join Lord Easytrigger's battalion today!"))
   end

   local fsr = fpres["Soromid"] or 0
   if fsr > 1 then
      table.insert(msg, _("Special offer on Crow: buy one IR-eye, and the second comes for free!"))
      table.insert(msg, _("Looking to modify an entire species? Visit Dr. Hu's Gene Clinic at Point Zero Station!"))
      table.insert(msg, _("Endogenous DNA damage hampering your Gene Drive? Drop by your local Chromosomal Rearrangement Laboratory for a check-up."))
   end

   local fzl = fpres["Za'lek"] or 0
   if fzl > 1 then
      -- Note that when running in the main menu background, player.name() might not exist (==nil), so
      -- we need to add a check for that.
      if player.name() then
         table.insert(msg, string.format(_("Dear Prof. %s, your recent work has left a deep impression on us. Due to the advance, novelty, and possible wide application of your innovation, we invite you to contribute other unpublished papers of relevant fields to the Interstellar Pay-to-win Journal for Mathematics and Applications."),player.name()))
      end
      table.insert(msg, _("Want to solve a large-dimensional stochastic PDE? The LMKSTD method is what you need!"))
      table.insert(msg, _("Love non-convex minimization? Join Ruadan's Computation Science Lab!"))
   end

   local fsi = fpres["Sirius"] or 0
   if fsi > 1 then
      table.insert(msg, _("Want a new look? Try Verrill's Ceremonial Robes at Burnan!"))
   end

   mem.ad = msg[rnd.rnd(1,#msg)]

   -- Custom greeting
   mem.comm_greet = mem.ad

   mem.loiter = rnd.rnd(5,20) -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end

