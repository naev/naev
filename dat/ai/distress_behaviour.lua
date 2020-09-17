-- Basic behaviour for non-combat ships when in distress

mem.shield_run = 100
mem.armour_run = 100
mem.defensive  = false
mem.enemyclose = 500
mem.distressmsgfunc = sos
mem.careful   = true

-- Send a distress signal which causes faction loss
function sos ()
   msg = {
      _("Local security: requesting assistance!"),
      _("Mayday! We are under attack!"),
      _("Requesting assistance. We are under attack!"),
      string.format(_("%s vessel under attack! Requesting help!"), ai.pilot():faction()),
      _("Help! Ship under fire!"),
      _("Taking hostile fire! Need assistance!"),
      _("We are under attack, require support!"),
      _("Mayday! Ship taking damage!"),
      string.format(_("Mayday! %s %s being assaulted!"), ai.pilot():faction(), string.lower( ai.pilot():ship():class() ))
   }
   ai.settarget( ai.target() )
   ai.distress( msg[ rnd.int(1,#msg) ])
end
