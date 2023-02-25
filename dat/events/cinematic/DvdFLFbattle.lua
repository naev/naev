--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Cinematic Dvaered/FLF battle">
 <location>enter</location>
 <chance>10</chance>
 <cond>system.cur() == system.get("Tuoladis")</cond>
 <unique />
 <notes>
  <campaign>Join the FLF</campaign>
  <tier>1</tier>
 </notes>
</event>
--]]
--[[

    This is the first of many planned eye candy cinematics.
    In this one, there will be a battle between the Dvaered and the FLF in the Tuoladis system.

]]--

local fleet = require "fleet"

local flfwave, dvaeredwave -- Non-persistent state

local articles = {
{
   faction = "Empire",
   head = _("Dvaered forces engaged in combat with small terrorist group"),
   body = _("In what the Dvaered call a 'Mighty victory', a Dvaered force was attacked and damaged by a small group of poorly-equipped and disorganized FLF, in the Tuoladis system. The Dvaered lost many tens of millions of credits worth of ships and crew before driving off or destroying the FLF squadron. The FLF's relative victory will only embolden them, leading to more internal strife and violence."),
   date_to_rm = time.get()+time.new(0,30,0),
}, {
   faction = "Dvaered",
   head = _("FLF terrorists blasted by joint Dvaered military forces"),
   body = _("A Dvaered patrol in Tuoladis was ambushed by a large armada of FLF terrorists. The Dvaered patrol not only held firm under the assault, but, like a hammer, brought the full might of the Dvaered down upon the FLF terrorists, crushing them absolutely. This incident shows the willingness of the terrorists to attack us and kill whomever they please. The Dvaered military is always ready to protect its citizens and kill and conquer those who would harm them."),
   date_to_rm = time.get()+time.new(0,30,0),
}, {
   faction = "Frontier",
   head = _("Dvaered forces engage FLF freedom fighters in open combat"),
   body = _("A small group of FLF freedom fighters was beset by a Dvaered patrol in Tuoladis, and immediately called backup. The situation escalated, and a large scale battle occurred, where the Dvaered forces lost tens of millions of credits worth of ships. This marks the first time FLF freedom fighters have had the ships and weapons to stand toe to toe against the Dvaered, and shall serve as an example for all who dare to stand for freedom, in life or death."),
   date_to_rm = time.get()+time.new(0,30,0),
}, {
   faction = "Independent",
   head = _("Dvaered, FLF clash in Tuoladis"),
   body = _("In a chance encounter, a Dvaered patrol encountered a group of FLF. The small skirmish quickly escalated to a large scale battle with many dozens of large ships engaging in combat.  The Dvaered have claimed victory, though sources hint that it came at great cost. The Dvaered High Command used the event as an excuse to call for military action against the frontier worlds."),
   date_to_rm = time.get()+time.new(0,30,0),
}
}


function create ()
    -- Messes spawns so exclusive claim
    if not evt.claim( system.cur() ) then
        evt.finish( false )
    end

    pilot.clear()
    pilot.toggleSpawn(false)

    flfwave = 1
    dvaeredwave = 1
    hook.timer(3.0, "FLFSpawn")
    hook.timer(12.0, "DvaeredSpawn")

    news.add( articles )

    hook.jumpout("leave")
    hook.land("leave")
end

function FLFSpawn ()
    local source_system = system.get("Zacron")
    local ships = { "Vendetta", "Vendetta", "Vendetta", "Vendetta", "Pacifier", "Lancelot", "Lancelot" }

    fleet.add( 1, ships, "FLF", source_system )

    flfwave = flfwave + 1
    if flfwave <=5 then
        hook.timer(1.0, "FLFSpawn")
    end
end

function DvaeredSpawn ()
    local source_system = system.get("Doranthex")
    local ships = { "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Ancestor",
                    "Dvaered Vigilance", "Dvaered Vigilance", "Dvaered Goddard" }

    fleet.add( 1, ships, "Dvaered", source_system )

    dvaeredwave = dvaeredwave + 1
    if dvaeredwave <= 5 then
        hook.timer(3.0, "DvaeredSpawn")
    end
end

function leave () --event ends on player leaving the system or landing
    evt.finish()
end
