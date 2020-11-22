--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Animal transport">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>10</chance>
   <location>Bar</location>
   <faction>Sirius</faction>
   <cond>planet.cur():class() ~= "0" and planet.cur():class() ~= "1" and planet.cur():class() ~= "2" and planet.cur():class() ~= "3"</cond>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[
--
-- MISSION: Animal transport
-- DESCRIPTION: A man asks you to transport a crate of specially bred creatures for
-- his in-law's exotic pet store on another planet. It's a standard fare A-to-B mission,
-- but doing this mission infests the player's current ship with the creatures.
--
--]]

require "jumpdist.lua"
require "missions/neutral/common.lua"


text = {}
title = {}

title[1] = _("Animal transport")
text[1] = _([["Good day to you, captain. I'm looking for someone with a ship who can take this crate here to planet %s in the %s system. The crate contains a colony of rodents I've bred myself, and my in-law has a pet shop on %s where I hope to sell them. Upon delivery, you will be paid 200,000 credits. Are you interested in the job?"]])

text[2] = _([["Excellent! My in-law will send someone to meet you at the spaceport to take the crate off your hands, and you'll be paid immediately on delivery. Thanks again!"]])

text[3] = _([[As promised, there's someone at the spaceport who accepts the crate. In return, you receive a number of credit chips worth 200,000 credits, as per the arrangement. You go back into your ship to put the chips away before heading off to check in with the local authorities. But did you just hear something squeak...?]])

NPCname = _("A Fyrra civilian")
NPCdesc = _("There's a civilian here, from the Fyrra echelon by the looks of him. He's got some kind of crate with him.")

misndesc = _("You've been hired to transport a crate of specially engineered rodents to %s (%s system).")
misnreward = _("You will be paid 200,000 credits on arrival.")

OSDtitle = _("Animal transport")
OSD = {}
OSD[1] = _("Fly to the %s system and land on planet %s")

log_text = _([[You successfully transported a crate of rodents for a Fyrra civilian. You could have swore you heard something squeak.]])


function create ()
    -- Get an M-class Sirius planet at least 2 and at most 4 jumps away. If not found, don't spawn the mission.
    local planets = {}
    getsysatdistance( system.cur(), 2, 4,
        function(s)
            for i, v in ipairs(s:planets()) do
                if v:faction() == faction.get("Sirius") and v:class() == "M" and v:canLand() then
                    planets[#planets + 1] = {v, s}
                end
           end
           return false
        end )

    if #planets == 0 then
        abort()
    end

    index = rnd.rnd(1, #planets)
    destplanet = planets[index][1]
    destsys = planets[index][2]

    misndesc = misndesc:format(_(destplanet:name()), _(destsys:name()))
    OSD[1] = OSD[1]:format(_(destsys:name()), _(destplanet:name()))

    misn.setNPC(NPCname, "sirius/unique/rodentman")
    misn.setDesc(NPCdesc)
end


function accept ()
    if tk.yesno(title[1], text[1]:format(_(destplanet:name()), _(destsys:name()), _(destplanet:name()))) then
        misn.accept()
        misn.setDesc(misndesc)
        misn.setReward(misnreward)
        misn.osdCreate(OSDtitle, OSD)
        tk.msg(title[1], text[2])
        misn.markerAdd(destsys, "high")
        hook.land("land")
    else
        misn.finish()
    end
end

function land()
    if planet.cur() == destplanet then
        tk.msg(title[1], text[3])
        player.pay(200000) -- 200K
        var.push("shipinfested", true)
        addMiscLog( log_text )
        misn.finish(true)
    end
end

function abort ()
   misn.finish(false)
end
