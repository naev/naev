local fmt = require "format"

-- Basic behaviour for non-combat ships when in distress

mem.shield_run = 100
mem.armour_run = 100
mem.defensive  = false
mem.enemyclose = 500
mem.careful    = true

-- Send a distress signal which causes faction loss
local function sos ()
   local plt = ai.pilot()
   local fct = plt:faction()
   local msg = {
      _("Local security: requesting assistance!"),
      _("Mayday! We are under attack!"),
      _("Requesting assistance. We are under attack!"),
      fmt.f(_("{fct} vessel under attack! Requesting help!"), {fct=fct}),
      _("Help! Ship under fire!"),
      _("Taking hostile fire! Need assistance!"),
      _("We are under attack, require support!"),
      _("Mayday! Ship taking damage!"),
      fmt.f(_("Mayday! {fct} {cls} being assaulted!"), {fct=fct, cls=_(plt:ship():class())})
   }
   ai.settarget( ai.taskdata() )
   ai.distress( msg[ rnd.rnd(1,#msg) ])
end

-- Must be defined after sos
mem.distressmsgfunc = sos
