local drill = require "outfits.lib.mining_drill"

function init( p, po )
   -- Since this outfit is usually off, we use shipstats to forcibly set the
   -- base stats
   po:set( "asteroid_scan", 350 )
   po:set( "mining_bonus", 80 )
   mem.isp = (p == player.pilot())
   drill.setup( p, po, {
      speed = 0.8*math.pi,
      shots_max = 4,
   })
end

ontoggle = drill.ontoggle
