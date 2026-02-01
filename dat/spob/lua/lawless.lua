local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land  = -50,
   std_bribe = -80,
   std_dangerous = -100,
   msg_granted = {
      _([["Just land or whatever."]]),
      _([["Who cares? Land already."]]),
      _([["It's a free port."]]),
   },
}
