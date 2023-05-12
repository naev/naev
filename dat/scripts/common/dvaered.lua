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

return dv
