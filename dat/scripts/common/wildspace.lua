local wildspace = {}

function wildspace.log( msg )
   shiplog.create( "wildspace", _("Wild Space"), _("Wild Space") )
   shiplog.append( "wildspace", msg )
end

return wildspace
