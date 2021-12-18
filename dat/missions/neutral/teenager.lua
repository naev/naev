--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The macho teenager">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>5</chance>
  <location>Bar</location>
  <faction>Dvaered</faction>
  <cond>player.numOutfit("Mercenary License") &gt; 0 and planet.cur():class() ~= "0" and planet.cur():class() ~= "1" and planet.cur():class() ~= "2" and planet.cur():class() ~= "3"</cond>
 </avail>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   MISSION: The macho teenager
   DESCRIPTION: A man tells you that his son has taken one of his yachts without permission and
   is joyriding it with his girlfriend to impress her. Disable the yacht and board it, then take
   the couple back to the planet (destroying the yacht incurs a penalty)

--]]
local fmt = require "format"
local neu = require 'common.neutral'

local reward = 300e3

local target -- Non-persistent state
-- luacheck: globals enter land targetBoard targetDeath targetExploded targetIdle (Hook functions passed by name)

function create ()
    mem.cursys = system.cur()
    mem.curplanet = planet.cur()
    misn.setNPC(_("A middle-aged man"), "neutral/unique/middleaged.webp", _("You see a middle-aged man, who appears to be one of the locals, looking around the bar, apparently in search of a suitable pilot."))
end


function accept ()
    if tk.yesno(_("Youngsters these days"), _([["Excuse me," the man says as you approach him. "I'm looking for a capable pilot to resolve a small matter for me. Perhaps you can help me? You see, it's my son. He's taken my yacht, along with his girlfriend, to space without my permission. That boy is such a handful. I'm sure he's trying to show off his piloting skills to impress her. I need you to get out there, disable the yacht, and take them both back here. Can you do this for me? I'll make it worth your while."]])) then
        misn.accept()
        misn.setDesc(fmt.f(_("A disgruntled parent has asked you to fetch his son and his son's girlfriend, who have taken a yacht and are joyriding it in the {sys} system."), {sys=mem.cursys}))
        misn.setReward(_("You will be compensated for your efforts."))
        misn.osdCreate(_("The macho teenager"), {
           _("Disable Gawain Credence"),
           fmt.f(_("Bring the teenagers back to planet {pnt}"), {pnt=mem.curplanet}),
        })
        tk.msg(_("It's a lousy job, but..."), _([["Thank you! The yacht doesn't have a working hyperdrive, so they won't have left the system. It's a Gawain named Credence. Just disable it, board it, and then transport my disobedient son and his girlfriend back here. Don't worry about the yacht, I'll have it recovered later. Oh, and one more thing, though it should go without saying: whatever you do, don't destroy the yacht! I don't want to lose my son over this. Well then, I hope to see you again soon."]]))
        hook.enter("enter")
        mem.targetlive = true
    else
        misn.finish()
    end
end

function enter()
    if system.cur() == mem.cursys and mem.targetlive then
        local location = vec2.newP(rnd.rnd() * system.cur():radius(), rnd.angle())
        target = pilot.add( "Gawain", "Independent", location, _("Credence") )
        target:control()
        target:setFaction("Dummy")
        target:memory().aggressive = true
        target:setHilight(true)
        target:setVisplayer(true)
        mem.hidle = hook.pilot(target, "idle", "targetIdle")
        hook.pilot(target, "exploded", "targetExploded")
        hook.pilot(target, "board", "targetBoard")
        targetIdle()
    end
end

function targetIdle()
    if not target:exists() then -- Tear down now-useless hooks.
        hook.rm(mem.hidle)
        return
    end
    local location = target:pos()
    local newlocation = vec2.newP(750, rnd.angle())
    target:taskClear()
    target:moveto(location + newlocation, false, false)
    hook.timer(5.0, "targetIdle")
end

function targetExploded()
   hook.timer( 2.0, "targetDeath" )
end

function targetDeath()
    local fine = math.max(-20e3, -player.credits())
    tk.msg(_("Whoops!"), fmt.f(_([[You have destroyed the Gawain! The family presses charges, and you are sentenced to a {credits} fine in absentia.]]), {credits=fmt.credits(-fine)}))
    player.pay(fine) -- I love this statement.
    misn.finish(true)
end

function targetBoard()
    player.unboard()
    tk.msg(_("End of the line, boyo"), _([[You board the Gawain and find an enraged teenage boy and a disillusioned teenage girl. The boy is furious that you attacked and disabled his ship, but when you mention that his father is quite upset and wants him to come home right now, he quickly pipes down. You march the young couple onto your ship and seal the airlock behind you.]]))
    target:setHilight(false)
    target:setVisplayer(false)
    local c = commodity.new( N_("Teenagers"), N_("Disillusioned teenagers.") )
    mem.cargoID = misn.cargoAdd(c,0)
    misn.osdActive(2)
    hook.land("land")
end

function land()
    if planet.cur() == mem.curplanet then
        tk.msg(_("You're grounded, young man"), _([[The boy's father awaits you at the spaceport. He gives his son and the young lady a stern look and curtly commands them to wait for him in the spaceport hall. The couple despondently walks off, and the father turns to face you.
    "You've done me a service, Captain," he says. "As promised, I have a reward for a job well done. You'll find it in your bank account. I'm going to give my son a reprimand he'll not soon forget, so hopefully he won't repeat this little stunt anytime soon. Well then, I must be going. Thank you again, and good luck on your travels."]]))
        player.pay( reward )
        misn.finish(true)

        neu.addMiscLog( _("Through force, you helped bring some young teenagers back from a joyride.") )
    end
end

function abort ()
   if target then
      target:setHilight(false)
      target:setVisplayer(false)
   end
   misn.finish(false)
end
