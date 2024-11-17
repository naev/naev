local dv = {}

dv.prefix = "#o".._("DVAERED: ").."#0" -- Repeatable mission prefix

function dv.addAntiFLFLog( text )
   shiplog.create( "dv_antiflf", _("Anti-FLF Campaign"), _("Dvaered") )
   shiplog.append( "dv_antiflf", text )
end

function dv.addStandardLog( text )
   shiplog.create("dvaered_standard", _("Dvaered Standard"), _("Dvaered"))
   shiplog.append("dvaered_standard", text)
end

-- Gets all alive warlords
function dv.warlords ()
   return {
      _("Lord Jim"),
      _("Lady Bitterfly"),
      _("Lady Pointblank"),
      _("Lord Chainsaw"),
      _("Lord Painbishop"),
      _("Lord Kriegsreich Hundertfeuer"),
      _("Lady Blackswan"),
      _("Lady Killington"),
      _("Lord Richthofen"),
      _("Lady Dewinter"),
      _("Lord Easytrigger"),
      _("Lady Sainte-Beuverie"),
      _("Lord Louverture"),
      _("Lord Abdelkiller"),
      _("Lady Proserpina")
   }
end

return dv
