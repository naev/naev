local drill = require "outfits.lib.mining_drill"

function init( p, po )
   -- Since this outfit is usually off, we use shipstats to forcibly set the
   -- base stats
   po:set( "asteroid_scan", 200 )
   po:set( "mining_bonus", 50 )
   mem.isp = (p == player.pilot())
   drill.setup( p, po, {
      speed = math.pi,
      shots_max = 3,
   })
end

ontoggle = drill.ontoggle
