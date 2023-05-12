--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Gauntlet">
 <unique />
 <priority>3</priority>
 <cond>faction.playerStanding("Nasin") &gt;= 0</cond>
 <chance>50</chance>
 <location>Bar</location>
 <faction>Sirius</faction>
 <notes>
   <campaign>Heretic</campaign>
 </notes>
</mission>
 --]]
--[[misn name - the gauntlet]]--
--[[cargo smuggle into sirius territory to assist a
    sabotage mission being carried out by the Nasin.
    Credits to KAHR-Alpha for the word "lackadaisically",
    and to BTAxis for the word "discombobulate"]]

local fmt = require "format"
local srs = require "common.sirius"


function create()
   --this mission makes no mission claims
   --set the variables
   mem.reward = 200e3 --mem.reward algorithm after this mission = 100e3 + (rnd.rnd(5,8)*2e3 * (nasin_rep^1.51). flat rate for first mission.
   mem.targetasset = spob.get("Margot")
   mem.targetsystem = system.get("Brendon")
   if system.cur() == mem.targetsystem then --I didn't want the player to start the mission in the same system as it was supposed to end
      misn.finish(false)
   end
   --set the mission stuff
   misn.setReward(mem.reward)
   misn.setTitle(_("The Gauntlet"))
   misn.setNPC(_("A Scrappy Man"), "sirius/unique/strangeman.webp", _("You see a rougher looking man sitting at the bar and guzzling a brownish ale."))
end

function accept()
   --the obligatory opening messages
   if not tk.yesno( _("The Gauntlet"), fmt.f( _([[You walk up to a scrappy little man leaning against the bar. You sit next to him, and he eyes you up and down. You return the stare coolly and he half-heartedly tries to strikes up a conversation. "Nice drinks they have here." You feign interest so as not to be impolite.
    He continues impatiently. "You look like you're in need of a couple spare credits," he finally says. "I have, uh, a shipment that needs getting to {pnt}. Are you interested? Just has to be kept under wraps if you know what I mean. Pay is good though. {credits}. That's all you need to know." He pauses for a moment. "How about it?"]]), {pnt=mem.targetasset, credits=fmt.credits(mem.reward)} ) ) then
      tk.msg(_("The Gauntlet"),_([["Well, that's your choice. Be on your way now. I'm busy."]]))
      return
   end
   tk.msg(_("The Gauntlet"), fmt.f(_([[You feel a very large hand slap you on the back. "I knew you would do it! A great choice!" he says. "I'll have my boys load up the cargo. Remember, all you gotta do is fly to {pnt}, and avoid the Sirius military. You know, don't let them scan you. I'll let my contacts know to expect you. They'll pay you when you land."
    You shake his sticky hand and walk off, content that you've made an easy buck.]]), {pnt=mem.targetasset}))
   misn.setDesc(fmt.f(_("You are to deliver a shipment to {pnt} in the {sys} system for a strange man you met at a bar, avoiding Sirius ships."), {pnt=mem.targetasset, sys=mem.targetsystem}))
   misn.accept()
   misn.markerAdd(mem.targetasset, "high")
   misn.osdCreate(_("The Gauntlet"), {
      fmt.f(_("Deliver the shipment to {pnt} in the {sys} system"), {pnt=mem.targetasset, sys=mem.targetsystem}),
   })
   misn.osdActive(1)
   local freecargo = player.pilot():cargoFree() --checks to make sure the player has 5 tons available
   if freecargo < 5 then
      tk.msg(_("The Gauntlet"),_([["You say you want this job, but you don't have enough cargo space! Stop wasting my time!"]])) --and if they don't, the mission finishes.
      return
   end
   local c = commodity.new( N_("Small Arms"), N_("An assortment of weapons that are not legal in Sirius space.") )
   c:illegalto( {"Sirius"} )
   mem.small_arms = misn.cargoAdd(c,5) --I'd like this to be contraband when this actually gets implemented in-game.
   hook.land("land")
end

function land ()
   if spob.cur() == mem.targetasset then
      tk.msg( _("The Gauntlet"), fmt.f(_([[As you descend onto the {pnt} spaceport, you notice how deserted the place seems to be. Finally, after a search that seems to take cycles, you see a small group of gruff and wary men waiting for you. Once you find them, they quickly unload the goods and disappear before you can even react.
    You then notice that one person, a large, unshaven man, remains from the group. You ask him for your payment. "Yes, yes, of course," he says as he hands you a credit chip. "Actually... if you're interested, we may have another mission for you. A message, as it were. The commander will be in the bar if you'd like to learn more about this opportunity." With that, he retreats along with the rest of the group. You wonder if you should pursue the offer or ignore it.]]), {pnt=mem.targetasset} ) )
      player.pay(mem.reward)
      misn.cargoRm(mem.small_arms) --this mission was an act against Sirius, and we want Sirius to not like us a little bit.
      faction.modPlayer("Nasin",3) --Nasin reputation is used in mission rewards, and I am trying to avoid having the pay skyrocket.
      var.push("heretic_misn_tracker",1) --using "misn_tracker", as later on in-game, i plan on having multiple arcs to the ending.
      srs.addHereticLog( fmt.f(_([[You helped a rough-looking man deliver an illegal shipment. After you completed the delivery, another man told you that there may be another mission opportunity and that you should meet some commander in the bar on {pnt} ({sys} system) if you're interested.]]), {pnt=mem.targetasset, sys=mem.targetsystem} ) )
      misn.finish( true )
   end
end
