--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Escape">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <chance>100</chance>
   <done>Dvaered Sabotage</done>
   <location>Bar</location>
   <cond>var.peek("loyal2klank") == true</cond>
   <faction>Dvaered</faction>
  </avail>
 </mission>
 --]]
--[[
-- Dvaered Escape
-- This is the third mission of the Frontier War Dvaered campaign.
-- The player has to set up the evasion of a Goddard executive.
-- This executive will then help House Dvaered on the diplomatic point of view.

   Stages :
   0) Goto find Hamfresser
   1) Goto the interception
   2) Start to run away
   3) Goto an hospital
   4) Runaway
   5) Player has been hailed by Captain HewHew
   6) Player has met the Empire and Pirate agent, but did not accept any of their offers
   7) Accepted Empire solution: can cross blockade of Alteris->Goddard
   8) Accepted Pirate solution: can cross any blockade
   9) Accepted Pirate solution: can cross any blockade, but has paid cash
--]]

require "nextjump"
require "selectiveclear"
require "proximity"
require "portrait"
require "missions/dvaered/frontier_war/fw_common"
require "numstring"

-- TODO: Set the priority and conditions of this mission
-- TODO: add news comments about all this
-- TODO: check that no blockade has been forgotten

portrait_name = _("Captain Leblanc")
portrait_desc = _("Captain Leblanc is the top pilot of General Klank's task force. Her presence in this bar means that the High Command needs your help.")

hamfr_name = _("Captain Hamfresser")
hamfr_desc = _("Hamfresser and his team are together at a table. The captain drinks with his favorite pink straw while incessantly scanning the room.")
hamfr_des2 = _("The captain sits alone at a remote table. He nervously chews his pink straw, while waiting for your signal to infiltrate the hospital.")
nikol_name = _("Sergeant Nikolov")
nikol_desc = _("The second in command of Hamfresser's squad seems to be as laid back as a Totoran gladiator on cocaine. Clearly, open spaces like this bar with many people around are not suited to commando members, who are used to see any stranger as a potential hostile.")
tronk_name = _("Private Tronk")
tronk_desc = _("The young cyborg sits to the left of his captain, and looks suspiciously at his sparkling water glass.")
theru_name = _("Caporal Therus")
theru_desc = _("This soldier is the team's medic. As such, she seems to be slightly less combat-suited than the others, but her large cybernetically-enhanced arms still make her look like she could crush a bull.")
straf_name = _("Lieutenant Strafer")
straf_desc = _("The pilot is the only one in the group who looks like the other people with whom you are used to work. His presence along with the others makes the group even stranger.")

imp_name = _("Feather-hat agent")
imp_desc = _("The imperial agent looks like a nondescript trader, as there are so many in imperial space.")
pir_name = _("Fake transponder dealer")
pir_desc = _("This shifty person is for sure one of the pirates that want to sell their fake transponder to you.")

ham2_name = _("Captain Hamfresser")
ham2_desc = _("Hamfresser is waiting for your signal to attack the spaceport's hospital and steal the machine that will save the VIP's life.")


propose_title = _("A difficult mission")
propose_text = _([[As you approach, Leblanc recognizes you. "Hello, citizen. The Major told me I would end up finding you by browsing all the shifty places in Dvaered space. He was right! We happened to need your services once more. But this time, it will be sort of a bit... More illegal." You then wonder how something could be more illegal than supporting a black ops commando in order to assassinate a pilot, steal a corvette and sabotage a warlord's cruiser, and you answer:]])

accept_title = _("The problem to solve")
accept_text1 = _([["Alright," Leblanc says, "Here is the situation: before starting to actually prepare our military operations, we need to protect our backs. And for that, Major Tam believes that the House Goddard is the key to ensure that the Empire will not thwart our plan. Tam would explain it much better than I, but as Goddard is right in between our space and imperial space, they really have much to loose in case of a direct conflict. What is more, as we are House Goddard's best customer, they tend to appreciate us (for example, we recently paid 6M credits to repair all the electronics of Klank and Battleaddict's Goddards).
   However, all the members of Goddard's executive board do not share the same view as regards our projects of invasion of the Frontier. Mr. Danftang, their Public Relation Manager, in particular, used to see it as an opportunity to sell more cruisers to the High Command and was very favourable to the invasion. This man has been arrested recently by the Za'lek police for unclear reasons and many suspect this is linked to a scheming settled by Goddard shareholders. As you have probably already guessed at this point, our mission is to make this man escape the Za'lek prisons."]])
accept_text2 = _([["The target is currently imprisoned on %s in %s, in Za'lek's VIP carceral center on this planet. He will be transferred to %s in %s for a preliminary interview with the judge. The Za'lek don't expect anybody to try to free him by violence, so they don't take too much precautions.
   "Major Tam has prepared the operation. First, you will pick up Hamfresser and his team on %s in %s. I'll be there too and then we will fly in formation to %s and settle the ambush. It would be preferable if you would make sure that your ship doesn't outrun my Vendetta. There should be a few drones and a corvette. We will destroy the drones and disable the corvette, in order to let the commando enter the ship and recover the target. Tam has insisted that he doesn't want us to kill anybody as it could irritate the Za'lek a bit too much. After that, we will have to jump out of the system and to return to %s separately.
  "A fake transponder will be implemented on both our ships. This will ensure that provided we don't do anything stupid on our way back, we will not be recognized as hostile by the Za'lek patrols and ground control services. So we should be able to refuel without any problem on Za'lek planets."]])

refuse_title = _("Too bad")
refuse_text = _([["As you want, citizen. After all one can not obligate people to do their duty..."]])


ham_title = _("Captain Hamfresser")
ham_text = _([["Hello, %s! You remember us?" Asks the captain, apparently unaware of the fact that his appearance is hard to forget. "We are ready to embark whenever you want, pilot! You'll just have to make place for %d tons of cargo (we have a few equipment). Oh yes, and there is apparently a small change in the plan regarding the Captain Leblanc. She won't be able to join us. Talk to the Lieutenant, he will explain it better."]])

nikol_title = _("Sergeant Nikolov")
nikol_text = _([["Hello, citizen. I guess you would want to speak with the officers to have details about the plan. But tell me while you are here: there are many people in this room, but none of them does sit at the tables that are next to ours. Do you know what is wrong with this part of the room?" You answer that the people do probably feel not safe while sitting next to a group of dangerous-looking cyborgs like them, and she answers: "Meh, I don't think you're right. When I am in the Space Infantry refectory, there are dangerous-looking cyborgs all around and nobody feels unsafe. There should be something else..."]])

tronk_title = _("Private Tronk")
tronk_text = _([["I have ordered water and the waiter gave me that glass, with lots of bubbles. It does not look safe. Do you know what it is? I hope it's not alcohol..." As you tell him that it is simply sparkling water, and that it doesn't contain alcohol nor anything toxic, he gratefully answers: "Whoa, you really look like an expert in drinks! You know, I was asking because we cyborgs of class gamma can't drink alcohol. We have to take every morning a special medication, the Spacemarine's Cocktail, to ensure that our organism supports our biological and cybernetic implants, and this cocktail is incompatible with any alcohol.
   "My brother drank a beer once by mistake, and he had to spend two months at hospital. But he is going better now, and he is back in his unit." You ask him if everyone in his family is as strong as he is and he answers: "Oh no, by little sister is much stronger. She fights on Totoran in the 1v1 bare hand championship, and she won 6 matches in a raw, last season. But I am afraid she won't be able to take part to this cycle's championship as she did not fully recover from her decapitation during her last fight."]])

therus_title = _("Caporal Therus")
therus_text = _([["Hi, citizen. Are you ready to transport us once more? Have you spoken to the Captain? And to the pilot? I don't really know the details of the operation, so you'll have to ask them."
   The caporal seem to hesitate, and then continues: "Today, the Lieutenant asked me a riddle: let's say Major Tam is running after a turtle. When Major Tam arrives at the point where the turtle was when he started to run, the turtle has moved forward a bit in the same time, right? Then, he arrives at the point where the turtle was when he reached the previous point, but the turtle has again moved forward. And so on. Conclusion: Major Tam is quicker than the turtle, but he never catches up. How is that possible?"
   Strafer then arrives: "Yep, I've been to the museum of Theras one day, and this riddle was written on a book from before the space age. The name of the author was: 'Senior High School Philosophy Class' that's a strange name actually. I remembered that riddle while we were hiking on %s not so long ago, and we saw a turtle."]])

strafer_title = _("Lieutenant Strafer")
strafer_text = _([[You look at the lieutenant, surprised not to see the captain Leblanc, as expected. "Unplanned things did happen. The general has been attacked in Doranthex by mercenary pilots and our squadron had to rescue him. The second in command got killed, so Leblanc can not delegate her command anymore. We need her to lead the squadron, and she sent me instead. Do not worry, I might be slightly less rewarded as her, but I am still a dogfight ace. I have got 15 attested dogfight victories so far, you know, and that does not take into account the secret operations I have taken part to.
   "So on the way in, I will follow you with my civilian Vendetta, and you will just have to hail me if you want me to do anything special. During the interception, I'll focus on the drones so that you can take on the main ship. For the way back, as planned, we will take separate ways. We take off when you decide."]])


board_title = _("A new passenger")
board_text = _([[The commando gather near the airlock. This time, four combat androids are in the front. Hamfresser gives his orders and the first android smashes the enemy ship's airlock with its fist. After that, the team rushes into the ship and the explosions start to thunder. Before long, the team comes back. Nikolov enters first, carrying an immobile and blue man in a prisoner suit, then Hamfresser, followed by the medic Therus, busy at applying compresses on a large bloody wound on the captain's side, and by Tronk, who seems to be trying to explain himself. After that come the androids, that seem to have received heavy damage. You jump at the cockpit and start the engines.]])

explain_title = _("We are in trouble")
explain_text = _([[While you finally jump out, Hamfresser reports: "We've got an unexpected situation in there. After we destroyed the androids, and got to the imprisonment room, we saw that there were three other prisoners along with the target, and much more human guards than expected. They exploded our first assault bot, and we had to take them down with the paralyzers, but one of the prisoners took a weapon for some reason and started to shoot on us. Fortunately for me, he just botched my lung. That is a replaceable part.
   "Then, Tronk paralyzed all the prisoners and we identified and recovered the target. That's why the guy is blue actually. But in his hurry, Tronk used the armor-piercing dose. According to the medic, it is worse that we first thought. Apparently, she can maintain the guy alive for a few periods, but she needs a machine that is not on board to save him. So at next stop, I'm afraid we will have to steal the machine at the spaceport's hospital. It really annoys me as it's the kind of operation that can get ugly very quickly, especially since the killing interdiction still runs, but we have no choice. I'll just be waiting for your signal at the bar next time we land.
   "If I may, I'd like to recommend to land somewhere within 3 periods, otherwise the VIP is likely to die, and to choose a place with a shipyard and an outfitter so that you'll be able to prepare your ship at best in case we need to escape quickly."]])

vip_d_title = _("The mission failed")
vip_d_text = _([[Hamfresser rushes to the bridge. "Everything is lost, %s! The guy died. Our mission failed!"]])

sign_title = _("Ready for action?")
sign_text = _([[Are you ready to start the operation "drugstore thunder"? (Hamfresser did find the name)]])

hosp_title = _("At the hospital")
hosp_text = _([[After giving the signal to Hamfresser, you join the cockpit of your ship and start the motors in prevision of an escape. A heavy explosion coming from the distance shakes your ship, followed by detonations that seem to approach. After a while, you see the commando running in your direction, pursued by Za'lek police androids. When the last member of the team enters the ship, you take off in a hurry, closely followed by a few drones. "We did a mess, out there!" Says Hamfresser "But at least we've got the machine!"]])

die_title = _("Journey to the other side")
die_text = _([[After having outrun your enemies, you start inquiring about how the operation went at the hospital. A quick look at your living quarters gives you an answer. You see the VIP, still unconscious, but not blue anymore, his body covered by electrodes connected to the machine in question. Next to him, the members of the commando look like they had better days. Hamfresser, his face as pale as death, sits on the ground, leaned on a pillar, busy at changing a long and blood-dripping bandage on his left arm. The sergeant Nikolov looks at a huge hole in her foot with an empty eye and the medic Therus limps from the one to the other, spreading blood marks on the walls.
   In the center of the room is lying private Tronk, covered with bandages. His battlearmor, pierced with multiple holes, has been thrown a few meters away in a blood puddle. The soldier looks at the two remaining fingers of his less damaged arm that are slowly walking on the ground, in a sad smile. "Leopold, you remember when you were a kid? Did you play with your fingers like that?" Hamfresser answers: "Tronky-boy, don't use my first name, only dying people have used my first name before..." "Don't be afraid, this won't change, Leopold."
   Suddenly, the soldier opens his eyes wide and calls the medic: "Therus! I know why Major Tam can catch the turtle! Each time, when he reaches the point where the turtle previously was, the time he needs for that is smaller. And at some point, it becomes so infinitely small that even if you have an infinity of steps to go, the total time is finite." The medic looks at the dying man: "Tronky, how did you? Tronky?" She then stops and takes the pulse at Tronk's neck "It's over, captain." Hamfresser answers: "Damn! It's the fifth kid who dies under my command and it still hurts as much. I just can't get used to that."]])

bloc_title = _("Troubles straight ahead!")
bloc_text = _([[When approaching the jump point, your sensors pick up a squadron of military ships, that stationnate close to the jump point in a tight formation. No doubt those ships are here for you, and it looks more than chancy to try to force the blockade.]]) 

info_title = _("A friend in the dark")
info_text = _([[The Gawain's hails you. When you respond, you hear a familiar voice. "Strafer here. I was wondering why you were so long. It looks like you had troubles with the Za'lek after all. There are blockades in %s, %s, %s and %s. They scan all ships, you have no chance to cross these systems alive. What have you done to them to upset them like that? Anyway, I did not come empty-handed. I've got as much fuel as you want. Unfortunately, I can't board you as they would chase me as well, so I have jettison a few tanks at coordinates I will give to you. Just go there and scoop them. Good luck!"]])

kill_title = _("The mission failed")
kill_text = _([[The rule was not to kill anybody, did you remember?]])


imperial_title1 = _("Help offer")
imperial_text1 = _([[The pilot of the ship starts to talk with a strange and disturbing familiarity: "Doing good, folks? Ya just walked into those Za'lek's freak's space, wrecked a squadron, helped a prisoner escape and desolated an hospital. You're worse than the incident, mates!" You wonder how this pilot could know so much about your operation, but the spiel continues: "Hewhewhew! People usually think I'm one of those useless pirates scum. I know you thought about that! Neh, don't lie to me!"
   The pilot's voice suddenly becomes harsh: "In reality, I am a faithful subject of his Imperial Majesty, as you should be yourself, %s from Hakoi! But you denied your own nation, and for that you should be severely punished. Don't forget, %s: The Empire is watching you. Anywhere. Anytime. Anyhow.
   "Hewhewhew! And what was the other one already? Oh yeah: The Emperor sees all! So old-fashioned! Hey! But ya're all lucky, 'cause the Empire feels in a merciful mood today. So at your next stop, you will kindly go and talk to the agent with a feather hat, and both of you will agree on a way for us not to kill you!"]])

imperial_title2 = _("Deal with the Empire")
imperial_text2 = _([[As you approach, the agent seems to recognize you at first sight. "Hello, %s. I guess they told you that I may have the solution to your little... problem." You look suspiciously at the agent and ask: "What do you want in exchange?" The other one smiles: "Simple. I want to discuss with the Dvaered captain. Give me 10 hecotseconds on the spacedock alone with him, not more, and the commander of the fleet in Alteris will forget to scan your ship when you'll jump to Goddard."
   That is for sure an uncommon request. You think at all the state secrets a wounded Hamfresser is able to give to the Empire in 10 hectoseconds. You then remember that, after all, Hamfresser is a professional, trained not to reveal any valuable information. But probably the imperial agent is a professional as well, trained to recover valuable information, and on the other hand, the pirate proposition still holds. So you answer:]])
imperial_text3 = _("Do you accept the deal with the Empire?")

imp_yes_title = _("You made the only good choice")
imp_yes_text = _([[After accepting, you invite the agent to follow you to the dock. You then enter your ship, where the commando is waiting for you and inform Hamfresser on the situation. He anxiously looks at the two other remaining members of his team. Nikolov grimaces and Therus nervously hits the wall. "If you think we have no other choice..." Says the captain. After removing his uniform jacket (where his name and rank are written), Hamfresser takes a deep breath and joins the imperial agent outside, in front of the ship. From a window, you see them having what looks like a peaceful conversation.
   After a while, Hamfresser returns in the ship and the agent waves to indicate that you're allowed to take off. Nikolov ask her captain: "And?" "Who knows what these imperial weirdos wanted to know? I tried to elude all the questions, but well. You never know."]])

imp_no_title = _("That was the wrong answer")
imp_no_text = _([["Mwell." Says the agent. "I guess you want to check by yourself that this is the only solution. If you're still alive when you're done, come back, we will be waiting for you."]])


pirate_title = _("Other help offer")
pirate_text1 = _([[As you land, someone seems to be waiting for you on the spaceport. "Hello, colleague! Someone is in trouble with the authorities, out there. You seem to have had an argument with the Za'lek, and now the Imperials help them. I've seen blockades everywhere on the borders of Imperial space. Even the way to the secret jumps is impassable. It looks like the Empire wants to get you at all costs, but luckily enough, I have the solution. You probably already got a fake transponder, but they seem to have identified it, so what about receiving an other one? I can sell you an authentic fake transponder, coming straight outta Skulls and Bones factory."
   This person is for sure a pirate who wants to take the opportunity to get a few cash. The idea is not bad as the imperial ships would not look for a ship with a Skulls and Bones fake transponder. So you ask him how many credits he wants. "%s" is the answer. "That sounds a great many, doesn't it? But maybe it's a suitable amount of money for your life and the success of whatever unscrupulous mission you're trying to carry on. Of course, you may not have such an amount right here, so I'll accept if you give your word to pay me at some point. Your word and your DNA signature as well, so that I can find you if you try to trick me."
   You know that if you agree, you will have to pay whatever happens, otherwise you will be harassed by hit men until the end of your life. But actually, paying %s could allow you to skirt the messy and compromising deal you will otherwise have to do with the Imperial secret services. Meet the fake transponder dealer at the bar if interested.]])
pirate_text2 = _("Do you accept the deal with the pirates? It costs %s, and you'll be able to skirt any imperial blocus.")

pir_yes_answer = _("Accept, immediate payment")
pir_yes_title = _("Immediate payment")
pir_yes_text = _([[When you give the credit chip, the pirate looks surprised: "Whow, mate, I didn't know I was talking to a millionaire. Well then thanks, here is your transponder."]])
pir_notenough_title = _("Not enough money")
pir_notenough_text = _([["Don't try to trick me, crook! I can see from here that you don't have enough money!"]])

pir_debt_answer = _("Accept, deferred payment")
pir_debt_title = _("Pirate debt")
pir_debt_text = _([["Here is your transponder," the pirate says. "Don't forget to pay once you can, otherwise..."]])

pir_no_answer = _("Refuse")
pir_no_title = _("You're way too expensive")
pir_no_text = _([["As you wish," says the pirate. "Just come back when you've understood that I'm your only chance!"]])


vip_title = _("Shock of two worlds")
vip_text = _([[You hear an unusual and edgy voice coming from the living quarters. When you go there, you see that the VIP is back on his feet. "To sum up, a Dvaered general, whose name you don't want to tell, sent you to free me from the Za'lek, for a reason you don't want to tell, right?" Hamfresser answers: "Totally correct, sir."
   The executive moans and shakes his head "You Dvaered are silly! Do you think that violence is the solution to any problem?" Hamfresser seems surprised "Is there any other way to solve a problem?" The VIP then looks at you: "Hey, you seem a bit less fussy than the others... You're not a Dvaered, right? You know what? My grandma was always saying that the worse in the universe were the Dvaered and the Za'lek. And she was right. First the Za'lek imprisoned me under charge of 'scientific embezzlement' for alien motives, and now, you Dvaered shred your way to my cell and kidnap me."
   Hamfresser defends himself: "But sir, if the operation succeeds, you'll be back to business really soon. Isn't that wonderful?" "... and if the operation fails, we all die! No, strong-arm, learn that patience, negotiation, bribing and craftiness can achieve much more than violence and destruction. I have paid the best lawyers in Za'lek space and my assistants negotiate with the authorities, I was sure to get out in about half a cycle." Hamfresser simply raises his shoulders: "Dvaered warriors don't use deception, not patience, nor craftiness. Dvaered warriors use respectable methods instead, like violence and destruction."]])

back_title = _("Finally back")
back_text = _([[Upon landing, Hamfresser, the VIP and you go to the spaceport's military office, where the Major Tam is waiting for you along with a few other soldiers. He warmly greets the executive and addresses to Hamfresser: "Do you know that you scared us, people? We learned by the diplomatic canal that you destroyed an hospital's pharmacy quasi-entirely, along with two police tanks and half a dozen battle androids on %s. Apparently, you did not kill anyone at least, but the Za'lek were really upset".
   The captain explains: "Sir, we needed a machine to heal the VIP, that had been injured during the interception, but once in the hospital, we've been apparently spotted by a traffic cop. Things then got gradually worse and we had to escape through the pharmacy's wall. I've lost a soldier in this operation." Tam answers: "Well, you will make a detailed report later. And don't worry about the soldier, I will make sure he will be replaced immediately.
   "And you, %s, anything to report?"]])

back_debt_title = _("Everything is almost alright")
back_debt_text = _([[You explain to the major what problems you encountered. You talk about the strange deal the Empire has tried to make with you. "Yes, the Imperial intelligence services are formidable. It is very hard for us to hide them our intentions. It was right from you not to accept their offer. So you bought a pirate fake transponder, right? I hope it was not too expensive!"
  When you tell him the sum you had to promise to pay, Major Tam squeaks. "Whawhawhat? %s for a fake transponder! This is not trade, it is theft!" "Well, technically..." You answer "those folks are pirates, so it's their job to rob people." The major calms down "Alright. I'll take care of the payment, so that they don't kill you, but you'll have to refund us, don't forget that! Oh, and by the way, I made sure with the Za'lek that they don't blame you personally for what happened. They should accept you in their space now."
   The major starts to go away, but then comes back "Oh, I almost forgot to pay you. Hehe. Here are %s."]])

back_pay_title = _("No major problem to report")
back_pay_text = _([[You explain to the major what problems you encountered. You talk about the strange deal the Empire has tried to make with you. "Yes, the Imperial intelligence services are formidable. It is very hard for us to hide them our intentions. It was right from you not to accept their offer. So you bought a pirate fake transponder, right? I hope it was not too expensive!"
   You consider requesting to be refunded for your mission expenses, but then you remember that the Dvaered are tightfisted and violent, so you give up and simply answer: "Oh, no... not... really."
   Tam looks satisfied, and answers: "By the way, I made sure with the Za'lek that they don't blame you personally for what happened. They should accept you in their space now."
   The major starts to go away, but then comes back "Oh, I almost forgot to pay you. Hehe. Here are %s credits."]])

back_nodeal_title = _("No major problem to report")
-- FIXME The "Here are _ credits" language might require a full ngettext() call.
back_nodeal_text = _([[You explain to the major what problems you encountered. You talk about the strange deal the Empire has tried to make with you. "Yes, the Imperial intelligence services are formidable. It is very hard for us to hide them our intentions. It was right from you not to accept their offer. I guess it should have been very hard and risky to skirt the imperial blocus, congratulations! Oh, and by the way, I made sure with the Za'lek that they don't blame you personally for what happened. They should accept you in their space now."
  The major starts to go away, but then comes back "Oh, I almost forgot to pay you. Hehe. Here are %s."]])

back_empire_title = _("A problem with the Empire")
-- FIXME The "Here are _ credits" language might require a full ngettext() call.
back_empire_text = _([[You explain to the major what problems you encountered. You talk about the strange deal the Empire has forced you to make with them and the major's face turns red: "You did WHAT? The imperial intelligence service is the strongest in the world, they can deduce things you would not even imagine just by looking at someone, and you let them discuss with a black ops commando leader!"
   As you argue that you had no other choice, he seems to calm down a little bit "I will interrogate Hamfresser to see if one can understand what they were looking for. Damn! I'm afraid something awful may happen to us somehow because of that. Oh, and by the way, I made sure with the Za'lek that they don't blame you personally for what happened. They should accept you in their space now."
   The major starts to go away, but then comes back "Oh, I almost forgot to pay you. Hehe. Here are %s."]])


edie_title = _("Mission Failed: escort destroyed")
edie_text = _("Your escort died. You have to abort the mission")

tdie_title = _("Mission Failed: target destroyed")
tdie_text = _("You were supposed to disable that ship, not to destroy it. How are you supposed to free anyone now?")

tesc_title = _("Mission Failed: target escaped")
tesc_text = _("You were supposed to disable that ship, not to let it escape. How are you supposed to free anyone now?")


misn_desc = _("You will help a Goddard executive to evade his Za'lek prison.")
misn_reward = _("Hopefully something better than Gauss Guns...")

log_text_emp = _("You helped the Dvaered High Command to liberate Mr. Danftang, public relations executive at Goddard, who was imprisoned by the Za'lek for obscure reasons. This executive is likely to help House Dvaered on the diplomatic point of view. Many unexpected events happened during this operation, that forced you to make a deal with the Empire secret services.")
log_text_debt = _("You helped the Dvaered High Command to liberate Mr. Danftang, public relations executive at Goddard, who was imprisoned by the Za'lek for obscure reasons. This executive is likely to help House Dvaered on the diplomatic point of view. Many unexpected events happened during this operation, that forced you to get into debt with House Dvaered. It is very likely they won't entrust you with important missions until you repay them.")
log_text_pay = _("You helped the Dvaered High Command to liberate Mr. Danftang, public relations executive at Goddard, who was imprisoned by the Za'lek for obscure reasons. This executive is likely to help House Dvaered on the diplomatic point of view. Many unexpected events happened during this operation, that forced you to buy a fake transponder at an outrageous price.")
log_text_raw = _("You helped the Dvaered High Command to liberate Mr. Danftang, public relations executive at Goddard, who was imprisoned by the Za'lek for obscure reasons. This executive is likely to help House Dvaered on the diplomatic point of view. Many unexpected events happened during this operation, but you managed to survive somehow.")


osd_title = _("Dvaered Escape")
osd_msg1  = _("Meet the rest of the team in %s in %s")
osd_msg2  = _("Intercept the convoy in %s. Your Vendetta escort must survive")
osd_msg3  = _("Report back on %s in %s")
osd_msg4  = _("Land anywhere to let Hamfresser steal a machine. Time left: %s")
osd_msg5  = _("Escape to %s in %s. Do NOT destroy any Za'lek inhabited ship (only drones are allowed)")
osd_msg6  = _("Escape to %s in %s. Thanks to your deal with the Empire, the squadron in Alteris won't prevent you from jumping to Goddard")
osd_msg7  = _("Escape to %s in %s. Thanks to your new fake transponder, the squadrons should not stop you anymore")

sting_comm_fight = _("You made a very big mistake!")
sting_comm_flee = _("Just try to catch me, you pirate!")

commMass = 4

mark_name = _("FUEL")
fuelMsg = _("You filled your fuel tanks")


function create()
   hampla, hamsys = planet.get("Vilati Vilata")
   reppla, repsys = planet.get("Dvaer Prime")
   pripla, prisys = planet.get("Jorla")
   zlkpla, zlksys = planet.get("House Za'lek Central Station")

   intsys = system.get("Poltergeist")

   if not misn.claim ( intsys ) then
      misn.finish(false)
   end

   misn.setNPC(portrait_name, portrait_leblanc)
   misn.setDesc(portrait_desc)
end

function accept()
   if not tk.yesno( propose_title, propose_text ) then
      tk.msg(refuse_title, refuse_text)
      misn.finish(false)
   end
   tk.msg(accept_title, accept_text1)
   tk.msg(accept_title, accept_text2:format(_(pripla:name()), _(prisys:name()), _(zlkpla:name()), _(zlksys:name()), _(hampla:name()), _(hamsys:name()), _(intsys:name()), _(reppla:name()) ))

   misn.accept()
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)

   stage = 0
   hook.land("land")
   hook.enter("enter")
   misn.osdCreate( osd_title, {osd_msg1:format(_(hampla:name()), _(hamsys:name())), osd_msg2:format(_(intsys:name())), osd_msg3:format(_(reppla:name()), _(repsys:name()))} )
   mark = misn.markerAdd(hamsys, "low")
end

function land()
   lastPlanet = planet.cur()

   -- You land at the commando's planet
   if stage == 0 and planet.cur() == hampla then
      hamfresser = misn.npcAdd("discussHam", hamfr_name, portrait_hamfresser, hamfr_desc)
      nikolov = misn.npcAdd("discussNik", nikol_name, portrait_nikolov, nikol_desc)
      tronk = misn.npcAdd("discussTro", tronk_name, portrait_tronk, tronk_desc)
      therus = misn.npcAdd("discussThe", theru_name, portrait_therus, theru_desc)
      strafer = misn.npcAdd("discussStr", straf_name, portrait_strafer, straf_desc)

      commando = misn.cargoAdd("Commando", commMass) -- TODO: see if it gets auto-removed at the end of mission

      stage = 1
      misn.osdActive(2)
      misn.markerRm(mark)
      mark = misn.markerAdd(intsys, "high")

   -- You land to steal a medical machine
   elseif stage == 3 then
      hamfresser = misn.npcAdd("fireSteal", hamfr_name, portrait_hamfresser, hamfr_des2)

   -- Land at an Imperial planet and meet the agents
   elseif stage == 5 and planet.cur():faction() == faction.get("Empire") then
      tk.msg(pirate_title, pirate_text1:format(numstring(pirate_price), creditstring(pirate_price)))
      pirag = misn.npcAdd("pirateDealer", pir_name, getPortrait("Pirate"), pir_desc)
      impag = misn.npcAdd("imperialAgent", imp_name, getPortrait(), imp_desc)
      stage = 6

   -- Land to end the mission
   elseif stage >= 4 and planet.cur() == reppla then
      tk.msg(back_title, back_text:format(_(hospPlanet:name()), player.name()))
      var.push("dv_empire_deal", false)
      var.push("dv_pirate_debt", false)
      shiplog.createLog( "frontier_war", _("Frontier War"), _("Dvaered") )
      if stage == 7 then -- Empire solution
         tk.msg(back_empire_title, back_empire_text:format(creditstring(credits_02)))
         var.push("dv_empire_deal", true)
         shiplog.appendLog( "frontier_war", log_text_emp )
      elseif stage == 8 then -- Pirate debt
         tk.msg(back_debt_title, back_debt_text:format(creditstring(pirate_price), creditstring(credits_02)))
         var.push("dv_pirate_debt", true)
         shiplog.appendLog( "frontier_war", log_text_debt )
      elseif stage == 9 then -- Pirate cash
         tk.msg(back_pay_title, back_pay_text:format(numstring(credits_02)))
         shiplog.appendLog( "frontier_war", log_text_pay )
      else -- Normally, the player should not achieve that (maybe with a trick I did not foresee, but it should be Xtremely hard)
         tk.msg(back_nodeal_title, back_nodeal_text:format(creditstring(credits_02)))
         shiplog.appendLog( "frontier_war", log_text_raw )
      end
      player.pay(credits_02)

      -- Reset the zlk standing.
      stand1 = fzlk:playerStanding()
      fzlk:modPlayerRaw( stand0-stand1 )
      var.pop("loyal2klank") -- We don't need this one anymore
      misn.finish(true)
   end

   lastPla = planet.cur()
end

-- Put the npcs back at loading
function load()
   if stage == 1 and planet.cur() == hampla then
      hamfresser = misn.npcAdd("discussHam", hamfr_name, portrait_hamfresser, hamfr_desc)
      nikolov = misn.npcAdd("discussNik", nikol_name, portrait_nikolov, nikol_desc)
      tronk = misn.npcAdd("discussTro", tronk_name, portrait_tronk, tronk_desc)
      therus = misn.npcAdd("discussThe", theru_name, portrait_therus, theru_desc)
      strafer = misn.npcAdd("discussStr", straf_name, portrait_strafer, straf_desc)
   elseif stage == 3 then
      hamfresser = misn.npcAdd("fireSteal", hamfr_name, portrait_hamfresser, hamfr_des2)
   --elseif stage == 4 then -- TODO: decide if we do that
      --player.takeoff()
   end
end

-- Optional discussions with the team
function discussHam()
   tk.msg(ham_title, ham_text:format(player.name(), commMass))
end
function discussNik()
   tk.msg(nikol_title, nikol_text)
end
function discussTro()
   tk.msg(tronk_title, tronk_text)
end
function discussThe()
   tk.msg(therus_title, therus_text:format(wlrd_planet))
end
function discussStr()
   tk.msg(strafer_title, strafer_text)
end

function fireSteal()
   if tk.yesno(sign_title, sign_text) then
      tk.msg(hosp_title, hosp_text)
      stage = 4
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg5:format(_(reppla:name()), _(repsys:name()))} )
      hook.pilot(nil, "death", "killed_zlk")

      hospPlanet = planet.cur()
      hook.takeoff("takeoff")
      player.takeoff()
      firstBloc = true
      hook.rm(datehook)
   end
end

-- Test to see if the player killed a zlk inhabited ship
function killed_zlk(pilot,killer)
   if pilot:faction() == faction.get("Za'lek") and killer == player.pilot() then
      killed_ship = pilot:ship():name()
      if (elt_inlist( killed_ship, {"Za'lek Scout Drone", "Za'lek Light Drone", "Za'lek Heavy Drone", "Za'lek Bomber Drone"} ) == 0) then
         tk.msg(kill_title, kill_text)
         misn.finish(false)
      end
   end
end

function enter()
   -- Intercept the ship
   if stage == 1 and system.cur() == intsys then
      fzlk = faction.get("Za'lek")
      stand0 = fzlk:playerStanding() -- To reset it after the fight

      pilot.toggleSpawn(false)
      pilot.clear()
      prevsys = getNextSystem(system.cur(), prisys)
      nextsys = getNextSystem(system.cur(), zlksys)

      hook.timer(5000, "convoyEnter")

   -- At first jump, it gets announced that you've got to land
   elseif stage == 2 then
      hook.timer( 7000, "weNeed2land" )

   elseif stage == 4 and tronkDeath then
      tronkDeath = false
      tk.msg(die_title, die_text) -- The death of Tronk

   -- When entering Empire Space, contact with Captain HewHew
   elseif stage == 4 and system.cur():presences()["Empire"] and (not system.cur():presences()["Za'lek"]) then
      hook.timer(2000, "spawnHewHew", lastSys)
      hook.timer(10000, "backDialog")  -- And some dialog with the VIP
   end

   -- Spawn Strafer
   if stage == 1 then
      if lastSys == system.cur() then -- We're taking off
         origin = lastPla
      else
         origin = lastSys
      end

      strafer = pilot.addRaw("Vendetta", "baddie_norun", origin, "DHC")
      strafer:setHilight()
      strafer:setVisplayer()
      strafer:rename("Lieutenant Strafer")

      -- give him top equipment
      strafer:rmOutfit("all")
      strafer:rmOutfit("cores")
      strafer:addOutfit("S&K Light Combat Plating")
      strafer:addOutfit("Tricon Zephyr II Engine")
      strafer:addOutfit("Reactor Class I")
      strafer:addOutfit("Milspec Aegis 3601 Core System")
      strafer:addOutfit("Power Regulation Override")
      strafer:addOutfit("Shredder", 4)
      strafer:addOutfit("Vulcan Gun", 2)

      strafer:setHealth(100,100)
      strafer:setEnergy(100)
      strafer:setFuel(true)

      -- Behaviour
      strafer:control(true)
      strafer:follow( player.pilot(), true )
      hook.pilot(strafer, "death", "escort_died")
      hook.pilot(strafer, "hail", "escort_hailed")
   end

   if stage >= 4 then
      curname = system.cur():name()

      -- Zlk Blocus:
      -- Pultatis -> Provectus Nova
      --          -> Limbo
      -- Stone Table -> Sollav
      -- Xavier -> Sheffield
      -- Straight Row -> Nunavut
      zlk_list = { "Pultatis", "Stone Table", "Xavier", "Straight Row" }
      zlk_lisj = { {"Provectus Nova", "Limbo"}, {"Sollav"}, {"Sheffield"}, {"Nunavut"} }

      index = elt_inlist( curname, zlk_list )
      if index > 0 then -- /!\ We did not claim this system /!\
         pilot.toggleSpawn("Za'lek")
         pilot.clearSelect("Za'lek")
         pilot.toggleSpawn("Pirate")
         pilot.clearSelect("Pirate")

         if firstBloc then
            scanHooks = {}
            jpoutHook = hook.jumpout("rmScanHooks")
         end
         for i, j in ipairs(zlk_lisj[index]) do
            jp = jump.get( system.cur(), j )
            pos = jp:pos()
            spawnZlkSquadron( pos, (stage < 8) )
         end
      end

   -- Empire Blocus:
   -- Overture -> Pas
   --          -> Waterhole
   -- Eneguoz  -> Hakoi
   -- Mural -> Salvador
   -- Arcturus -> Goddard /!\ This one is passable if deal with the empire /!\
   -- (Delta Pavonis -> Goddard) ? TODO: this one is not necessary
   -- Fortitude -> Pontus
   --           -> Acheron
   -- Merisi -> Acheron
      emp_list = { "Overture", "Eneguoz", "Mural", "Arcturus", "Delta Pavonis", "Fortitude", "Merisi" }
      emp_lisj = { {"Pas", "Waterhole"}, {"Hakoi"}, {"Salvador"}, {"Goddard"}, {"Goddard"}, {"Pontus", "Acheron"}, {"Acheron"} }

      index = elt_inlist( curname, emp_list )
      if index > 0 then -- /!\ We did not claim this system /!\
         pilot.toggleSpawn("Za'lek")
         pilot.clearSelect("Za'lek")
         pilot.toggleSpawn("Pirate")
         pilot.clearSelect("Pirate")

         for i, j in ipairs(emp_lisj[index]) do
            jp = jump.get( system.cur(), j )
            pos = jp:pos()
            if system.cur() == system.get("Arcturus") and j == "Goddard" and stage == 7 then -- Special case: JP from Arcturus to Goddard
               spawnEmpSquadron( pos, false )
            else
               spawnEmpSquadron( pos, (stage < 8) )
            end
         end
      end

   end

   lastSys = system.cur()
end

-- Functions for the escort
function escort_died()
   tk.msg(edie_title, edie_text)
   misn.finish(false)
end

-- Makes the carceral convoy enter the system
function convoyEnter()
   target = pilot.add("Za'lek Sting", nil, prevsys)[1]
   target:memory().formation = "wedge"
   target:setHilight()
   target:setVisible()
   target:control(true)
   target:hyperspace(nextsys)

   escort = {}
   escort[1] = pilot.add("Za'lek Light Drone", nil, prevsys)[1]
   escort[2] = pilot.add("Za'lek Light Drone", nil, prevsys)[1]
   escort[3] = pilot.add("Za'lek Heavy Drone", nil, prevsys)[1]
   --escort[4] = pilot.add("Za'lek Heavy Drone", nil, prevsys)[1]

   athooks = {}
   for i, p in ipairs(escort) do
      p:setLeader(target)
      athooks[i] = hook.pilot(p, "attacked", "targetAttacked")
   end

   attackhook = hook.pilot(target, "attacked", "targetAttacked")
   boardhook  = hook.pilot(target, "board", "targetBoarded")
   hook.pilot( target, "death", "targetDied" )
   hook.pilot( target, "jump", "targetEscaped" )
   hook.pilot( target, "land", "targetEscaped" )
   -- TODO: not possible to jump nor to land
end

-- Hooks for the interception target
function targetAttacked()
   strafer:control(false)
   hook.rm(attackhook)
   for i, j in ipairs(athooks) do
      hook.rm(j)
      escort[i]:setFaction("Warlords") -- Hack so that Strafer attacks them and not the Sting
   end
   target:setHostile(true)

   -- Decide between fight or runaway
   if playerMoreThanCorvette() then
      target:taskClear()
      target:comm( sting_comm_flee )
      target:runaway(player.pilot())
   else
      target:control(false)
      target:comm( sting_comm_fight )
   end
end
function targetBoarded()
   tk.msg(board_title, board_text)
   player.unboard()
   hook.rm(boardhook)

   stage = 2
   misn.osdActive(3)
   misn.markerRm(mark)
   mark = misn.markerAdd(repsys, "low")

   -- Reset the zlk standing
   stand1 = fzlk:playerStanding()
   fzlk:modPlayerRaw( stand0-stand1 )
end
function targetDied()
   tk.msg(tdie_title, tdie_text)
   misn.finish(false)
end
function targetEscaped()
   tk.msg(tesc_title, tesc_text)
   misn.finish(false)
end

-- Hamfresser explains that we need to land at an hospital
function weNeed2land()
   stage = 3
   tk.msg(explain_title, explain_text)
   timelimit = time.get() + time.create(0,3,0)
   misn.osdCreate(osd_title, {osd_msg4:format((timelimit - time.get()):str())})
   datehook = hook.date(time.create(0, 0, 100), "tick")
end

function tick()
   if timelimit >= time.get() then
      misn.osdCreate(osd_title, {osd_msg4:format((timelimit - time.get()):str())})
   else
      tk.msg(vip_d_title, vip_d_text:format(player.name()))
      misn.finish(false)
   end
end

function takeoff( )
   -- Player takes off from planet after attacking the hospital
   if stage == 4 and lastPlanet:faction() == fzlk then
      fzlk:modPlayerRaw( -100 )
      hook.timer(1000, "spawnDrones")

      -- Clear all Zlk pilots in a given radius of the player to avoid being insta-killed at takeoff
      local dmin2 = 500^2
      zlkPilots = pilot.get("Za'lek")
      for i, p in ipairs(zlkPilots) do
         if vec2.dist2(player.pilot():pos()-p:pos()) < dmin2 then
            p:rm()
         end
      end
   end
end

-- Drones are after the player after the hospital attack
function spawnDrones()
   pilot.add("Za'lek Light Drone", nil, lastPlanet)
   pilot.add("Za'lek Light Drone", nil, lastPlanet)
   pilot.add("Za'lek Heavy Drone", nil, lastPlanet)
   tronkDeath = true -- This says that at next jump, Tronk will die
end

-- Spawn blockade ships
function spawnZlkSquadron( pos, bloc )
   squad = {}
   squad[1]  = pilot.add("Za'lek Mephisto", nil, pos)[1]
   squad[2]  = pilot.add("Za'lek Demon", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[3]  = pilot.add("Za'lek Demon", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[4]  = pilot.add("Za'lek Sting", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[5]  = pilot.add("Za'lek Sting", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]

   squad[6]  = pilot.add("Za'lek Light Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[7]  = pilot.add("Za'lek Light Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[8]  = pilot.add("Za'lek Heavy Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[9]  = pilot.add("Za'lek Heavy Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]

   squad[10] = pilot.add("Za'lek Bomber Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[11] = pilot.add("Za'lek Bomber Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[12] = pilot.add("Za'lek Bomber Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[13] = pilot.add("Za'lek Bomber Drone", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]

   for i, j in ipairs(squad) do
      j:setSpeedLimit( .0001 ) -- 0 disables the stuff so it's unusable
      j:setHostile(bloc)
   end

   if firstBloc then
      scanHooks[#scanHooks+1] = hook.timer(500, "proximityScan", {focus = squad[2], funcname = "scanBloc"})
   end
end
function spawnEmpSquadron( pos, bloc )
   squad = {}
   squad[1]  = pilot.add("Empire Hawking", nil, pos)[1]
   squad[2]  = pilot.add("Empire Pacifier", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[3]  = pilot.add("Empire Pacifier", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[4]  = pilot.add("Empire Admonisher", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[5]  = pilot.add("Empire Admonisher", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]

   squad[6]  = pilot.add("Empire Shark", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[7]  = pilot.add("Empire Shark", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[8]  = pilot.add("Empire Lancelot", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[9]  = pilot.add("Empire Lancelot", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]

   squad[10] = pilot.add("Empire Lancelot", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[11] = pilot.add("Empire Lancelot", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[12] = pilot.add("Empire Lancelot", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]
   squad[13] = pilot.add("Empire Lancelot", nil, pos + vec2.new(rnd.sigma()*300, rnd.sigma()*300))[1]

   for i, j in ipairs(squad) do
      j:setSpeedLimit( .0001 ) -- 0 disables the stuff so it's unusable
      j:setHostile(bloc)
   end
end

-- The player sees the blocus fleet
function scanBloc()
   if firstBloc then -- avoid having that happening twice in systems where there are 2 blocus
      tk.msg(bloc_title, bloc_text)

      player.pilot():control()
      player.pilot():brake() -- Normally, nobody should want to kill the player
      player.cinematics( true )
      camera.set( squad[1]:pos() ) -- TODO if possible: choose the right squad

      rmScanHooksRaw()
      firstBloc = false
      hook.timer(4000, "spawnStrafer")
   end
end

-- Strafer enters the system
function spawnStrafer()
   strafer = pilot.add("Trader Gawain")[1]
   strafer:setHilight(true)
   strafer:setVisible(true)
   strafer:control(true)
   strafer:follow( player.pilot() )
   camera.set( strafer )
   prox = hook.timer(500, "proximity", {anchor = strafer, radius = 2000, funcname = "straferDiscuss", focus = player.pilot()})
end

-- The player discuss with Strafer
function straferDiscuss()
   hook.rm(prox)
   camera.set()
   player.cinematics(false)
   player.pilot():control(false)

   tk.msg(info_title, info_text:format( zlk_list[1], zlk_list[2], zlk_list[3], zlk_list[4] ) )
   strafer:control(false)  -- Strafer stops following the player

   -- Add some fuel, far away so that no npc gathers it
   pos = vec2.new( -1.2*system.cur():radius(), 0 )
   system.addGatherable( "Fuel", 1, pos, vec2.new(0,0), 3600 )
   Imark = system.mrkAdd( mark_name, pos )
   gathHook = hook.gather("gather")
end

-- Player gathers fuel
function gather( comm, qtt )
   hook.rm(gathHook)
   pilot.cargoRm( player.pilot(), comm, qtt )
   player.pilot():setFuel(true)
   player.msg( fuelMsg )
   system.mrkRm(Imark)
end

-- Remove scan hooks
function rmScanHooks()
   rmScanHooksRaw()
   hook.rm(jpoutHook)
end
function rmScanHooksRaw()
   if scanHooks ~= nil then
      for i, j in ipairs(scanHooks) do
         hook.rm(j)
      end
      scanHooks = nil
   end
end

-- Spawns the odd imperial pilot
function spawnHewHew( origin )
   hewhew = pilot.addRaw("Hyena", "civilian", origin, "Civilian")
   hewhew:rename("Strange Pilot")
   hewhew:setInvincible()  -- Don't wreck my Captain HewHew
   hewhew:hailPlayer()
   hailie = hook.pilot(hewhew, "hail", "hailMe")
end
function hailMe()
   hook.rm(hailie)
   player.commClose()
   tk.msg( imperial_title1, imperial_text1:format(player.name(),player.name()) )
   stage = 5
end

-- Discuss with the Pirate or Imperial agent
function pirateDealer()
   local c = tk.choice(pirate_title, pirate_text2:format(creditstring(pirate_price)), pir_yes_answer, pir_debt_answer, pir_no_answer)
   if c == 1 then
      if player.credits() >= pirate_price then
         player.pay(-pirate_price)
         tk.msg(pir_yes_title,pir_yes_text)
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_msg7:format(_(reppla:name()), _(repsys:name()))} )
         misn.npcRm(pirag)
         stage = 9
      else
         tk.msg(pir_notenough_title, pir_notenough_text)
      end
   elseif c == 2 then
      tk.msg(pir_debt_title,pir_debt_text)
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg7:format(_(reppla:name()), _(repsys:name()))} )
      misn.npcRm(pirag)
      stage = 8
   else
      tk.msg(pir_no_title,pir_no_text)
   end
end
function imperialAgent()
   if tk.yesno(imperial_title2, imperial_text2:format(player.name())) then
      tk.msg(imp_yes_title,imp_yes_text)
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg6:format(_(reppla:name()), _(repsys:name()))} )
      misn.markerAdd( system.get("Alteris"), "plot" )
      misn.npcRm(impag)
      stage = 7
   else
      tk.msg(imp_no_title,imp_no_text)
   end
end

function backDialog()
   tk.msg(vip_title, vip_text)
end

-- Aborting if stage >= 4: reset zlk reputation
function abort()
   if stage >= 4 then
      stand1 = fzlk:playerStanding()
      fzlk:modPlayerRaw( stand0-stand1 )
   end
end
