--[[misn name - the gauntlet]]--
--[[cargo smuggle into sirius territory to assist a
    sabatoge mission being carried out by the nasin.
    Credits to KAHR-Alpha for the work "lackadaisically",
    and to BTAxis for the word "discombobulate"]]

include "dat/scripts/numstring.lua"
    
--the intro messages
bmsg = {}
bmsg[1] = _([[You walk up to a scrappy little man leaning against the bar. You sit next to him, and he eyes you up and down. You return the stare cooly and he half-heartedly tries to strikes up a conversation. "Nice drinks they have here." You feign interest so as not to be impolite.
    He continues impatiently. "You look like you're in need of a couple spare credits," he finally says. "I have, uh, a shipment that needs getting to %s. Are you interested? Just has to be kept under wraps if you know what I mean. Pay is good though. %s credits. That's all you need to know." He pauses for a moment. "How about it?"]])
bmsg[2] = _([[You feel a very large hand slap you on the back. "I knew you would do it! A great choice!" he says. "I'll have my boys load up the cargo. Remember, all you gotta do is fly to %s, and avoid the military and police. I'll let my contacts know to expect you. They'll pay you when you land."
    You shake his sticky hand and walk off, content that you've made an easy buck.]])

--ending messages
emsg = {}
emsg[1] = _([[As you descend onto the %s spaceport, you notice how deserted the place seems to be. Finally, after a search that seem to take cycles, you see a small group of gruff and wary men waiting for you. Once you find them, they quickly unload the goods and disappear before you can even react.
    You then notice that one person, a large, unshaven man, remains from the group. You ask him for your payment. "Yes, yes, of course," he says as he hands you a credit chip. "Actually... if you're interested, we may have another mission for you. A message, as it were. The commander will be in the bar if you'd like to learn more about this opportunity." With that, he retreats along with the rest of the group. You wonder if you should pursue the offer or ignore it.]])

--misn osd stuffs
osd = {}
osd[1] = _("Deliver the shipment to %s in the %s system")
--random odds and ends
notenoughcargo = _([["You say you want this job, but you don't have enough cargo space! Stop wasting my time!"]])
rejected = _([["Well, that's your choice. Be on your way now. I'm busy."]])
npc_name = _("A Scrappy Man")
bar_desc = _("You see a rougher looking man sitting at the bar and guzzling a brownish ale.")
misn_desc = _("You are to deliver a shipment to %s in the %s system for a strange man you met at a bar, avoiding police.")
misn_title = _("The Gauntlet")
misn_reward = _("%s credits")

function create()
   --this mission makes no mission claims
   --set the variables
   reward = 200000 --reward algorithm after this mission = 100000 + (rnd.rnd(5,8)*2000 * (nasin_rep^1.51). flat rate for first mission.
   startworld = planet.cur()
   targetasset = planet.get("Margot")
   targetsystem = system.get("Brendon")
   if system.cur() == targetsystem then --I didn't want the player to start the mission in the same system as it was supposed to end
      misn.finish(false)
   end
   --set the mission stuff
   misn.setReward(misn_reward:format(numstring(reward)))
   misn.setTitle(misn_title)
   misn.setNPC(npc_name, "sirius/unique/strangeman")
   misn.setDesc(bar_desc)
   osd[1] = osd[1]:format(targetasset:name(),targetsystem:name())
   misn_desc = misn_desc:format(targetasset:name(),targetsystem:name())
end

function accept()
   --the obligatory opening messages
   local aname = targetasset:name()

   if not tk.yesno( misn_title, bmsg[1]:format( aname, numstring(reward) ) ) then
      tk.msg(misn_title,rejected)
      misn.finish(false)
   end
   tk.msg(misn_title,bmsg[2]:format(aname))
   misn.setDesc(misn_desc)
   misn.accept()
   misn.markerAdd(targetsystem,"high")
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)
   freecargo = player.pilot():cargoFree() --checks to make sure the player has 5 tons available
   if freecargo < 5 then
      tk.msg(misn_title,notenoughcargo) --and if they don't, the mission finishes.
      misn.finish(false)
   end
   small_arms = misn.cargoAdd("Small Arms",5) --i'd like this to be contraband when this actually gets implemented in-game.
   hook.land("land")
end

function land ()
   if planet.cur() == targetasset then
      tk.msg( misn_title, emsg[1]:format( targetasset:name() ) )
      player.pay(reward)
      misn.cargoRm(small_arms) --this mission was an act against sirius, and we want sirius to not like us a little bit.
      faction.modPlayer("Nasin",3) --nasin rep is used in mission rewards, and I am trying to avoid having the pay skyrocket.
      var.push("heretic_misn_tracker",1) --using "misn_tracker", as later on in-game, i plan on having multiple arcs to the ending.
      misn.osdDestroy()
      misn.finish( true )
   end
end

function abort ()
   misn.finish(false)
end
