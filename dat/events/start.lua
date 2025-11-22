--[[
<?xml version='1.0' encoding='utf8'?>
<event name="start_event">
 <location>none</location>
</event>
--]]
--[[
   Event run when creating a new pilot.
--]]
local intro = require "intro"
local lg = require "love.graphics"
local cinema = require "cinema"

local names = {
   -- Because we might as well allude to an existing parody. Proper spelling would be "Aluminium", by the way.
   _("Aluminum Mallard"), -- codespell:ignore
   _("Armchair Traveller"),
   _("Attitude Adjuster"),
   _("Commuter"),
   _("Death Trap"),
   _("Eclipse"),
   _("Exitprise"),
   _("Fire Hazard"),
   _("Gunboat Diplomat"),
   _("Heart of Lead"),
   _("Icarus"),
   _("Little Rascal"),
   _("Myrmidon"),
   _("Opportunity"),
   _("Outward Bound"),
   _("Pathfinder"),
   _("Planet Jumper"),
   _("Rustbucket"),
   _("Serendipity"),
   _("Shove Off"),
   _("Sky Cutter"),
   _("Terminal Velocity"),
   _("Titanic MLXVII"),
   p_("ship name", "Vagabond"),
   _("Vindicator"),
   _("Windjammer"),
   _("Tuna Can"),
   _("Vermilion Dwarf"), -- Red Dwarf
}
local function name()
   return names[rnd.rnd(1,#names)]
end

function timer_tutorial()
   naev.missionStart( "Tutorial" )
   evt.finish( true )
end

function create()
   local pp = player.pilot()
   pp:rename( name() ) -- Assign a random name to the player's ship.
   pp:outfitAdd( "Laser Cannon MK1", 1 ) -- Tutorials tell the player to buy and equip ion cannon later, need one weapon slot empty
   jump.setKnown( "Delta Polaris", "Jade" )

   -- Set player-specific settings
   var.push( "autonav_reset_shield", 1 )
   var.push( "autonav_reset_dist", 3e3 )
   var.push( "autonav_compr_speed", 5e3 )
   var.push( "autonav_compr_max", 50 )

   -- Give all GUIs
   -- TODO: Would be better to remove these outfits and the association,
   -- but they're so tightly integrated atm (with no other way to define
   -- GUIs as usable) that I'm implementing it this way for now.
   player.outfitAdd( "GUI - Brushed" )
   player.outfitAdd( "GUI - Slim" )
   player.outfitAdd( "GUI - Slimv2" )
   player.outfitAdd( "GUI - Legacy" )

   -- Apply starting unidiff
   diff.apply( "Chapter 0" )

   -- Intro animation
   cinema.on()
   music.choose("intro")
   local imgpath = "gfx/intro/"
   intro.init()
   intro.text(_("The night sky always draws our eyes. People look up with a question, or in wonder, and sometimes with a dream: to venture forth among the stars."))
   intro.text()
   intro.text(_("From Earth, the first spacefarers spied tiny jewels shining with the light of distant suns, far away but so clear. To reach out and grasp one would only cost a lifetime. Cheap at the price."))
   intro.text()
   intro.image( lg.newImage(imgpath.."first_steps_into_space") )
   intro.text(_("So men and women locked their sleeping bodies into hulking metal boxes. They hurled themselves out into the void. On arrival, they decanted and woke to find new stars staring coldly down from stranger skies."))
   intro.text()
   intro.image( lg.newImage(imgpath.."faster_than_light") )
   intro.text(_("The first wave of explorers crept from star to star at sub-light speeds, until the invention of the hyperspace drive spurred a second growth. Then civilization spread, as it always has, with exponential enthusiasm -- ripples in the dark sea of space."))
   intro.text()
   intro.text(_("However, the heroic age of discovery lasted only as long as human curiosity outpaced greed. Too soon, conflict erupted on the colonies. Land, fuel, food, and distrust sowed the seeds of discord. Once begun, the fever of war spread from world to world and became a holocaust."))
   intro.text()
   intro.image( lg.newImage(imgpath.."reign_of_daedris") )
   intro.text(_("After a hundred years of interstellar conflict, on the ashes that remained, at last a new order built its foundations."))
   intro.text()
   intro.text(_("A man named Daedris asserted dominion over the exhausted worlds. He founded a Galactic Empire and named himself its ruler. Under the iron fist of his military, relations between planets resumed; abandoned trade routes saw traffic for the first time in living memory."))
   intro.text()
   intro.text(_("Scholars call it the zenith: technology leapt forward, culture flourished, irradiated rock was cleansed and dead planets brought back to life. Peace reigned in an imperial golden age."))
   intro.text()
   intro.text(_("After Daedris, under his successors, the light of the empire waned. Civil strife and religious division weakened social bonds. The great projects of the imperium went awry. In some regions, administration fell into the hands of other powers. The Great Houses now policed the space lanes."))
   intro.text()
   intro.text(_("Rise and fall, the rhythm of history."))
   intro.text()
   intro.text(_("But the march of history stumbled over the unknown, over the Incident."))
   intro.text()
   intro.text(_("It tore the heart out of the Empire."))
   intro.text()
   intro.text(_("From Earth, a cataclysmic explosion ripped through space, annihilating stars, vaporizing worlds. Billions died. Dozens of systems disappeared into the mysterious nebula left behind. Now the great, galaxy-spanning civilization survives as a shadow of its former glory."))
   intro.text()
   intro.image( lg.newImage(imgpath.."questions") )
   intro.text(_("What else remains? Questions."))
   intro.text()
   intro.text(_("Ten Standard Cycles have passed in a universe ruled by the shards of a decaying empire. After the Incident, corruption and piracy run rampant, but where there is chaos, there is also opportunity."))
   intro.text()
   intro.text(_("Welcome to the universe of Naev..."))
   intro.text()
   intro.run()
   music.choose("ambient")
   cinema.off()

   hook.timer(3, "timer_tutorial")
end
