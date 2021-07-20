-- Basic behaviour for non-combat ships when in distress

mem.shield_run = 100
mem.armour_run = 100
mem.defensive  = false
mem.enemyclose = 500
mem.careful    = true

-- Send a distress signal which causes faction loss
function sos ()
   local plt = ai.pilot()
   local fname = plt:faction():name()
   local msg = {
      _("Local security: requesting assistance!"),
      _("Mayday! We are under attack!"),
      _("Requesting assistance. We are under attack!"),
      string.format(_("%s vessel under attack! Requesting help!"), fname),
      _("Help! Ship under fire!"),
      _("Taking hostile fire! Need assistance!"),
      _("We are under attack, require support!"),
      _("Mayday! Ship taking damage!"),
      string.format(_("Mayday! %s %s being assaulted!"), fname, string.lower( plt:ship():class() ))
   }
   ai.settarget( ai.taskdata() )
   ai.distress( msg[ rnd.int(1,#msg) ])
end

-- Must be defined after sos
mem.distressmsgfunc = sos
