require 'ai.core.core'
require 'ai.core.idle.advertiser'
require 'ai.core.misc.distress'
local fmt = require "format"
local ads = require "scripts.common.ads"

mem.lanes_useneutral = true
mem.atk_skill = 0

function create ()
   create_pre()

   -- Advertiser can give you the location for popular brands
   -- This is how you make custom menu items!
   mem.comm_custom = mem.comm_custom or {}
   table.insert(mem.comm_custom, {
      menu = _("Locate a brand"),
      setup = function(vn, _p, _plt)
         vn.na(fmt.f(_('"I\'d be happy to help you out! Which company are you interested in finding?"')))
         vn.label("companies")
         vn.menu({
            { _([[Krain Industries]]), "starbridge" },
            { _([[Unicorp]]), "unicorp" },
            { _([[Milspec]]), "milspec" },
            { _([[Tricon]]), "tricon" },
            { _([[Teracom]]), "teracom" },
            { _([[Nexus]]), "nexus" },
            { _([[Enygma]]), "enygma" },
            { _([[Back]]), "back" },
         })
         vn.label("back")
         vn.jump("menu")

         vn.label("starbridge")
         vn.na(_('"Head to New Xantusia in Lynx to get the incredible Krain Industries Starbridge."'))
         vn.jump("companies")

         vn.label("unicorp")
         vn.na(_('"You can find everything Unicorp at the Unicorp Orbital in Botarn."'))
         vn.jump("companies")

         vn.label("milspec")
         vn.na(_('"Check out the smart and safe Milspec ships at Mining Vrata Guildhouse in Holly."'))
         vn.jump("companies")

         vn.label("tricon")
         vn.na(_('"Tricon\'s market-leading engines can be conveniently found on Bolero in Delta Polaris."'))
         vn.jump("companies")

         vn.label("teracom")
         vn.na(_('"Look for the whole selection of Teracom rockets on Dundrop Monastery in Botarn."'))
         vn.jump("companies")

         vn.label("nexus")
         vn.na(_('"Nexus\' elite line of ships can be found at their very own Nexus Shipyards HQ in Valerie."'))
         vn.jump("companies")

         vn.label("enygma")
         vn.na(
            _(
               '"Um, one second, let me check my notes. *ruffling of papers* You can find the Enygma System Turreted Launchers on Sharak in Opal."'
            )
         )
         vn.jump("companies")
      end,
   })

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

   -- Generate a random ad
   mem.ad = ads.generate_ad()

   -- Custom greeting
   mem.comm_greet = fmt.f(_([["{msg}"]]), {msg=mem.ad})

   mem.loiter = rnd.rnd(5,20) -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end
