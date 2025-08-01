require 'ai.core.core'
require 'ai.core.idle.advertiser'
require 'ai.core.misc.distress'
local fmt = require "format"
local dv = require "common.dvaered"
local ads = require "scripts.common.ads"

mem.lanes_useneutral = true
mem.atk_skill = 0

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
   mem.refuel_msg = _([["I'll supply your ship with fuel for {credits}."]])

   -- Set up potential advertiser messages in msg variable
   local msg = tmergei( {}, ads.ads_for_faction("generic") )

   -- Faction specific messages
   local fpres = system.cur():presences()

   -- Empire messages
   local fem = fpres["Empire"] or 0
   if fem > 1 then
      msg = tmergei( msg, ads.ads_for_faction("empire") )
      msg = tmergei( msg, ads.ads_for_faction("cyber") )
   end

   -- Dvaered messages
   local fdv = fpres["Dvaered"] or 0
   if fdv > 1 then
      msg = tmergei( msg, ads.ads_for_faction("dvaered") )
      local badwords = {
         _("a Butthead"),
         _("a Nincompoop"),
         _("a Dunderhead"),
         _("an Ass"),
         _("a Fool"),
         _("a Coward"),
      }
      local lords = dv.warlords () -- Gets all warlorlds
      local r = rnd.rnd(1,#lords)
      local butthead = lords[r]
      table.remove( lords, r )
      local sponsor = lords[ rnd.rnd(1,#lords) ]
      local params = {butthead=butthead, badword=badwords[rnd.rnd(1,#badwords)], sponsor=sponsor}
      table.insert(msg, fmt.f(_("I hereby declare {butthead} is {badword}. -{sponsor}"), params))
      table.insert(msg, fmt.f(_("Let it be known that {butthead} is {badword}. -{sponsor}"), params))
   end

   -- Soromid messages
   local fsr = fpres["Soromid"] or 0
   if fsr > 1 then
      msg = tmergei( msg, ads.ads_for_faction("soromid") )
   end

   -- Soromid+Empire messages
   if fsr > 1 and fem > 1 then
      table.insert(msg, _("Remember to fill in your EX-29528-B form if returning to the Empire from Soromid territory."))
   end

   -- Za'lek messages
   local fzl = fpres["Za'lek"] or 0
   if fzl > 1 then
      msg = tmergei( msg, ads.ads_for_faction("zalek") )
      msg = tmergei( msg, ads.ads_for_faction("cyber") )
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
      msg = tmergei( msg, ads.ads_for_faction("sirius") )
   end

   mem.ad = msg[rnd.rnd(1,#msg)]

   -- Custom greeting
   mem.comm_greet = fmt.f(_([["{msg}"]]), {msg=mem.ad})

   mem.loiter = rnd.rnd(5,20) -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end
