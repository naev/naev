--[[misn name - the gauntlet]]--
--[[cargo smuggle into sirius territory to assist a
    sabatoge mission being carried out by the nasin.
    Credits to KAHR-Alpha for the work "lackadaisically",
    and to BTAxis for the word "discombobulate"]]

include "numstring.lua"
    
lang = naev.lang()

--the intro messages
bmsg = {}
bmsg[1] = [[You walk up to a scrappy little man leaning against the bar. You sit next to him, and he eyes you up and down. You return the stare cooly. Lackadaisically, he strikes up a converrsation.
		"Good weather on %s these days, I hear." 
		"I hear nothing but good things about %s." you reply, badly feigning interest. You order a drink, as he takes a gulp of his.
		He looks at you, sizing you up. He probably has been waiting for someone to set next to him all day. "You look like a man in need of a couple spare credits. I have a, uhh, shipment that needs getting to %s. Are you interested? Thing is, its not exactly... legal. Pay is good though. All you need to know," he continues, "is that someone will be at the starport in %s waiting for this shipment. And they got credits. %s credits. Just for you, kiddo, if you want."
		You get your drink, a greenish slimy-looking thing called a "tartar coobadu", and decide you just have to ask. "What exactly is this shipment?"]]
bmsg[2] = [[He almost looks suprised at the question. He motions you to a corner table as if to tell you to go there, but forcifully leads you. People seem to be avoiding this table, probably because of a peculiar vomit-odor that is wafting from somewhere underneath. He sits you down rather roughly, and then sits down himself.
		"Look, I really can't tell you the exact contents of the box. Lets just say its "small" weaponry. My... employer... isn't fond of the government of %s. As such, he needs... " he takes a drink "a shipment delivered there ASAP. Thats all I can tell you, I'm afraid"
		You sip your slime, and answer him...]]
bmsg[3] = [[You feel a very large hand slap you on the back. "Thats a lad!" he cries exuberantly. "I'll have my boys load up the cargo, as quickly as you please. Remember, all you gotta do is fly to %s, and avoid the military, police, and civvies that like to stick their noses where they don't belong. I'll let my contacts know to expect you, and to pay you when you land."
			You shake his sticky hand, and walk off, content that you've made an easy buck.]]

--ending messages
emsg = {}
emsg[1] = [[As you descend to the %s spaceport, you notice a severe lack of... well... anybody. The place seems deserted. You received an impersonal message saying *LAND IN BAY 71A4* on your way into the atmosphere, but that is the only communication you received. You don't even see any vessels flying around. As you steer your ship into the aforementioned bay, you finally see a small group of gruff and wary men waiting for you. Almost as soon as you land, they have the goods out, and as you walk out to say hello, you already see it disappearing on the far end of the bay.]]
emsg[2] = [[You approach the man who appears to be the leader of the group. "One box of... stuff, as requested." You say, motioning to the now-gone box. 
			The large, unshaven man looks you right in the eyes."Yes, thanks. I suppose you will be wanting payment now." He hands you a credit chip. "You know, we need to get a message back to our... employers. Interested in taking one more trip? If you are, I'll be in the bar after I've verified the contents of our shipment. Meet me there."]]

--misn osd stuffs
osd = {}
osd[1] = "Deliver the shipment to %s in the %s system."
--random odds and ends
notenoughcargo = [["You say you want this job, but you don't have enough cargo room for this assignment, little man!" Ragnarok turns away, displeased at wasting a whole 5 STU on you.]]
rejected = [[He looks at you, almost appearing confused.
			"Well, thats your choice boy. Be on your way now. I'm busy."
			You walk away, wondering if you really missed out on an oppourtunity.]]
npc_name = "Ragnarok"
bar_desc = "You see a rougher looking gent sitting at the bar, guzzling a brownish ale."
misn_desc = "Deliver the shipment to %s in %s for the Nasin."
misn_title = "The Gauntlet"

function create()
   --this mission makes no mission claims
   --set the variables
   reward = 20000 --reward algorithm after this mission = 10000 + (rnd.rnd(5,8)*200 * (nasin_rep^1.51). flat rate for first mission.
   startworld = planet.cur()
   targetasset = planet.get("Margot")
   targetsystem = system.get("Brendon")
   if system.cur() == targetsystem then --I didn't want the player to start the mission in the same system as it was supposed to end
      misn.finish(false)
   end
   --set the mission stuff
   misn.setReward(reward)
   misn.setTitle(misn_title)
   misn.setNPC(npc_name,"neutral/thief1") --using a generic picture for now.
   misn.setDesc(bar_desc)
   --format all the messages
   bmsg[1] = bmsg[1]:format(targetasset:name(),targetasset:name(),targetsystem:name(),targetasset:name(),numstring(reward))
   bmsg[2] = bmsg[2]:format(targetasset:name())
   bmsg[3] = bmsg[3]:format(targetasset:name())
   emsg[1] = emsg[1]:format(targetasset:name())
   osd[1] = osd[1]:format(targetasset:name(),targetsystem:name())
   misn_desc = misn_desc:format(targetasset:name(),targetsystem:name())
end

function accept()
   --the obligatory opening messages
   tk.msg(misn_title,bmsg[1])
   if not tk.yesno(misn_title,bmsg[2]) then
      tk.msg(misn_title,rejected)
      misn.finish(false)
   end
   tk.msg(misn_title,bmsg[3])
   misn.setDesc(misn_desc)
   misn.accept()
   misn.markerAdd(targetsystem,"plot")
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)
   freecargo = pilot.cargoFree(pilot.player()) --checks to make sure the player has 5 tons available
   if freecargo < 5 then
      tk.msg(misn_title,notenoughcargo) --and if he doesnt, the mission finishes.
      misn.finish(false)
   end
   small_arms = misn.cargoAdd("Small Arms",5) --i'd like this to be contraband when this actually gets implemented in-game.
   hook.land("land")
end

function land ()
   if planet.cur() == targetasset then
      tk.msg(misn_title,emsg[1])
      tk.msg(misn_title,emsg[2])
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
