--[[
-- Dvaered Escape
-- This is the third mission of the Frontier War Dvaered campaign.
-- The player has to set up the invasion of a Goddard executive.
-- This executive will then help House Dvaered on the diplomatic point of view.

   Stages :
   0) Goto find Hamfresser

--]]

--TODO: Set the priority and conditions of this mission
-- TODO: add news comments about all this
-- TODO: scientific embezzlement

portrait_name = _("Captain Leblanc")
portrait_desc = _("Captain Leblanc is the top pilot of General Klank's task force. Her presence in this bar means that the High Command needs your help.")

hamfr_name = _("Captain Hamfresser")
hamfr_desc = _("Hamfresser and his team are together at a table. The captain drinks with his favorite pink straw while incessantly scanning the room.")
nikol_name = _("Sergeant Nikolov")
nikol_desc = _("The second in command of Hamfresser's squad seems to be as laid back as an hyperactive kid under caffeine. Clearly, open spaces like this bar with many people around are not suited to commando members, who are used to see any stranger as a potential hostile.")
tronk_name = _("Private Tronk")
tronk_desc = _("The young cyborg sits to the left of his captain, and looks suspiciously at his sparklong water glass.")
theru_name = _("Caporal Therus")
theru_desc = _("This soldier is the team's medic. As such, she seems to be slighlty less combat-suited than the others, but her large cybernetically-enhanced arms still make her look like she could crush a bull.")
straf_name = _("Lieutenant Strafer")
straf_desc = _("The pilot is the only one in the group who looks like the other people with whom you are used to work. His presence along with the others makes the group even more strange.")

ham2_name = _("Captain Hamfresser")
ham2_desc = _("Hamfresser is waiting for your signal to attack the spaceport's hospital and steal the machine that will save the VIP's life.")

propose_title = _("A difficult mission")
propose_text = _([[As you approach, Leblanc recognizes you. "Hello, citizen. The Major told me I would end up finding you by browsing all the shifty places in Dvaered space. He was right! We happend to need your services once more. But this time, it will be sort of a bit... More illegal." You then wonder how something could be more illegal than supporting a black ops commando in order to assassinate a pilot, steal a corvette and sabotage a warlord's cruiser, and you answer:]])

accept_title = _("The problem to solve")
accept_text1 = _([["Allright," Leblanc says, "Here is the situation: before starting to actually prepare our military operations, we need to protect our backs. And for that, Major Tam believes that the House Goddard is the key to ensure that the Empire will not thwart our plan. Tam would explain it much better than I, but as Goddard is right inbetween our space and imperial space, they really have much to loose in case of a direct conflict. What is more, as we are House Goddard's best customer, they tend to appreciate us (for example, we recently paid 6M credits to repair all the electronics of Klank and Battleaddict's Goddards).
   However, all the members of Goddard's executive board do not share the same view as regards our projects of invasion of the Frontier. Mr. Danftang, their Public Relation Manager, in particular, used to see it as an opportunuity to sell more cruisers to the High Command and was very favourable to the invasion. This man has been arrested recently by the Za'lek police for unclear reasons and many suspect this is linked due to a scheming settled by Goddard shareholders. As you have probably already guessed at this point, our mission is to make this man escape the Za'lek prisons."]])
accept_text2 = _([["The target is currently imprisoned on %s in %s, in Za'lek's VIP carceral center on this planet. He will be transferred to %s in %s for a preliminary interview with the judge. The Za'lek don't expect anybody to try to free him by violence, so they don't take too much precautions.
   "Major Tam has prepared the operation. First, you will pick up Hamfresser and his team on %s in %s. I'll be there too and then we will fly to %s and settle the ambush. There should be a few drones and a destroyer. We will destroy the drones and disable the destroyer, in order to let the commando enter the ship and recover the target. Tam has insisted that he doesn't want us to kill anybody as it could irritate the Za'lek a bit too much. After that, we will have to jump out of the system and to return to %s separately.
  "A fake transponder will be implemented on both our ships. This will ensure that provided we don't do anything stupid on our way back, we will not be recognized as hostile by the Za'lek patrols and ground control services. So we should be able to refuel without any problem on Za'lek planets."]])

refuse_title = _("Too bad")
refuse_text = _([["As you want, citizen. After all one can not obligate people to do their duty..."]])

ham_title = _("Captain Hamfresser")
ham_text = _([["Hello, %s! You remember us?" Asks the captain, apparently unaware of the fact that his appearance is not very common. "We are ready to embark whenever you want, pilot! You'll just have to make place for %d tons of cargo (we have a few equipment). Oh yes, and there is apparently a small change in the plan regarding the Captain Leblanc. She won't be able to join us. Talk to the Lieutenant, he will explain it better."]])

nikol_title = _("Sergeant Nikolov")
nikol_text = _([["Hello, citizen. I guess you would want to speak with the officers to have details about the plan. But tell me while you are here: there are many people in this room, but none of them does sit at the tables that are next to our. Do you know what is wrong with this part of the room?" You answer that the people do probably feel not safe whil sitting next to a group of dangerous-looking cyborgs like them, and she answers: "Meh, I don't think you're right. When I am in the Space Infantry refectory, there are dangerous-looking cyborgs all around and nobody feels unsafe. There should be something else..."]])

tronk_title = _("Private Tronk")
tronk_text = _([["I have ordered water and the waiter gave me that glass, with lots of bubbles. It does not look safe. Do you know what it is? I hope it's not alcohol..." As you tell him that it is simply sparkling water, and that it doesn't contain alcohol nor anything toxic, he gratefully answers: "Woah, you really look like an expert in drinks! You know, I was asking because we cyborgs of class gamma can't drink alcohol. We have to take every morning a special medication, the Spacemarine's Cocktail, to ensure that our organism supports our biological and cybernetic implants, and this coktail is incompatible with any alcohol.
   "My brother drank a beer once by mistake, and he had to spend two months at hospital, but now he is going better, he is back in his commando now." You ask him if everyone in his family is as strong as he is and he answers: "Oh no, by little sister is much stronger. She fights on Totoran in the 1v1 bare hand championship, and she won 6 matches in a raw, last season. But I am afraid she won't be able to take part to this cycle's championship as she did not fully recover from her decaptiation during her last fight."]])

therus_title = _("Caporal Therus")
therus_text = _([["Hi, citizen. Are you ready to transport us once more? Have you spoken to the Captain? And to the pilot? I don't really know the details of the operation, so you'll have to ask them." The caporal seem to hesitate, and then continues: "Today, the Lieutenant asked me a riddle: it's Major Tam who is running after a turtle. When Major Tam arrives at the point where the turtle was when he started to run, the turtle has moved forward a bit in the same timer, right? Then, he arrives at the point where the turtle was when he reached the previous point, but the turtle has again moved forward. And so on. Conclusion: Major Tam is quicker than the turtle, but he never catches up."
   Strafer then arrives: "Yep, I've been to the museum recently, and this riddle was written on a book from before the space age. The name of the author was: 'Senior High School Philosiphy Class' that's a strange name actually. I remembered that riddle while we were hiking on %s not so long ago, and we saw a turtle."]])

strafer_title = _("Lieutenant Strafer")
strafer_text = _([[You look at the lieutenant, surprised not to see the captain Leblanc, as expected. "Unplanned things did happend. The general has been attacked in Doranthex by mercenary pilots and our squadron had to rescure him. The second in command got killed, so Leblanc can not delagate her command anymore. We need her to lead the squadron, and she sent me instead. Do not worry, I might be slightly less rewarded as her, but I am still a dogfight ace. I have got 15 attested dogfight victories so far, you know, and that does not take into account the secret operations I have taken part to.
   "So on the way in, I will follow you with my civilian Vendetta, and you will just have to hail me if you want me to do anything special. During the interception, I'll focus on the drones so that you can take on the main ship. For the way back, as planned, we will take separate ways. We take off when you decide."]])

board_title = _("A new passenger")
board_text = _([[The commando gather near the airlock. This time, four combat androids are in the front. Hamfresser gives his orders and the first android smashes the enemy ship's airlock with its fist. After that, the team rushes into the ship and the explosions start to thunder. Before long, the team comes back. Nikolov enters first, carrying an immobile and blue man in a prisonner suit, then Hamfresser, followed by the medic Therus, buisy at applying compresses on a large bloody wound on the captain's side, and by Tronk, who seems to be trying to explain himself. After that come the androids, that seem to have recieved heavy damage. You jump at the cockpit and start the engines.]])

explain_title = _("We are in trouble")
explain_text = _([[While you finally jump out, Hamfresser reports: "We've got an unexpected situation in there. After we destroyed the androids, and got to the imprisonment room, we saw that there were three other prisonners along with the target, and much more human gards than expected. They exploded our first assault bot, and we had to take them down with the paralizers, but one of the prisonners took a weapon for some reason and started to shoot on us. Fortunately for me, he just reached my lung. That is a replaceable part.
   "Then, Tronk paralized all the prisonners and we identified and recovered the target. That's why the guy is blue actually. But in his hurry, Tronk used the armor-piercing dose. According to the medic, it is worse that we first thought. Apparently, she can maintain the guy alive for a few periods, but she needs a machine that is not abord to save him. So at next stop, I'm afraid we will have to steal the machine at the spaceport's hospital. It really annoys me as it's the kind of operation that can get ugly very quickly, especially since the killing interdiction still runs, but we have no choice. I'll just be waiting for your signal at the bar next time we land.
   "If I may, I'd like to recommend to land somewhere within 5 periods, otherwise the VIP is likely to die, and to choose a place with a shipyard and an outfitter so that you'll be able to prepare your ship at best in case we need to escape quickly."]])

hosp_title = _("At the hospital")
hosp_text = _([[You join the cockpit of your ship and start the motors in prevision of an escape. After a while, you see the commando running in your direction, pursued by Za'lek police androids. When the last member of the team enters the ship, you take off in a hurry, closely followed by a few drones. "We did a mess, out there!" Says Hamfresser "But at least we've got the machine!"]])

die_title = _("Journey to the other side")
die_text = _([[After having outrun your enemies, you start inquiring about how the operation went at the hospital. A quick look at your living quarters gives you an answer. You see the VIP, still unconscious, but not blue anymore, his body covered by electrodes connected to the machine in question. Next to him, the members of the commando look like they had better days. Hamfresser, his face as pale as death, sits on the ground, leaned on a pillar, buisy at changing a long and blood-dripping bandage on his left arm. The sergeant Nikolov looks at a huge hole in her foot with an empty eye and the medic Therus limps from the one to the other, spreading blood marks on the walls.
   In the center of the room is lying the private Tronk, covered with bandages. His battlearmor, pierced with multiple holes, has been thrown a few meters away in a blood puddle. The soldier looks at the two remaining fingers of his less damanged arm that are slowly walking on the ground, in a sad smile. "Leoplod, you remember when you were a kid? Did you play with your fingers like that?" Hamfresser answers: "Tronky-boy, don't use my first name, only dying people have used my first name before..." "Don't be afraid, this won't change, Leopold."
   Suddently, the soldier opens his eyes wide and calls the medic: "Therus! I know why Major Tam can catch the turtle! Each time, when he reaches the point where the turtle previously was, the time he needs for that is smaller. And at some point, it becomes so infinitely small that even if you have an infinity of steps to go, the total time is finite." The medic looks at the dying man: "Tronky, how did you? Tronky?" She then stops and takes the pulse at Tronk's neck "It's over, captain." Hamfresser answers: "Damn! It's the fifth kid who dies under my command and it still hurts as much. I just can't get used to that."]]) --"This soldier deserves funerals like the heros of the independance war. He will drift forever in Dvaered space, our flag around his coffin."]])

bloc_title = _("Troubles straight ahead!")
bloc_text = _([[When approaching the jump point, your sensors pick up a wing of Za'lek ships, that stationnate close to the jump point in a tight formation. No doubt those ships are here for you, and it looks more than chancy to try to force the blockade.]])

info_title = _("A friend in the dark")
info_text = _([[When you respond to the Koala's hail, you hear a familiar voice. "Strafer here. I was wondering why you were so long. It looks like you had troubles with the Za'lek after all, there are blockades in %s, %s, %s and %s. They scan all ships, you have no chance to cross these systems alive. What have you done to them to upset them like that? Anyway, I did not come empty-handed. I've got as much fuel as you want. Unfortunately, I can't board you as they would chase me as well, so I have jettison a few tanks at coordinates I will give to you. Just scoop them. Good luck!"]])


misn_desc = _("You will help a Goddard executive to evade his Za'lek prison.")
misn_reward = _("To contribute to the greatness of House Dvaered.")


osd_title = _("Dvaered Escape")
osd_msg1  = _("Meet the rest of the team in %s in %s")
osd_msg2  = _("Intercept the convoy in %s")
osd_msg3  = _("Report back on %s in %s")
osd_msg4  = _("Land anywhere to let Hamfresser steal a machine. Time left: %d")
osd_msg5  = _("Escape to on %s in %s. Do NOT destroy any Za'lek inhabited ship (only drones are allowed)")

commMass = 5
credits = 200000

-- Zlk Blocus:
-- Putatis -> Provectus Nova
--         -> Limbo
-- Stone Table -> Sollav
-- Xavier -> Sheffield
-- Straight Row -> Nunavut

-- Empire Blocus:
-- Overture -> Pas
--          -> Waterhole
-- Eneguoz  -> Hakoi (remove)
-- Mural -> Salvador
-- Arcturus -> Goddard
-- Delta Pavonis -> Goddard
-- Fortitude -> Pontus
-- Acheron -> Sikh

-- Pirate JP : Hakoi -> Trask, Firk -> Torg ?

function create()
   hampla, hamsys = planet.get("Vilati Vilata")
   reppla, repsys = planet.get("Dvaer Prime")
   pripla, prisys = planet.get("Wa'kan")
   zlkpla, zlksys = planet.get("House Za'lek Central Station")

   intsys = system.get("Gorman")

   if not misn.claim ( intsys ) then
      misn.finish(false)
   end

   misn.setNPC(portrait_name, "dvaered/dv_military_f8")
   misn.setDesc(portrait_desc)
end

function accept()
   if not tk.yesno( propose_title, propose_text ) then
      tk.msg(refuse_title, refuse_text)
      misn.finish(false)
   end
   tk.msg(accept_title, accept_text1)
   tk.msg(accept_title, accept_text2:format(pripla:name(), prisys:name(), zlkpla:name(), zlksys:name(), hampla:name(), hamsys:name(), reppla:name() ))

   misn.accept()
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)

   stage = 0
   hook.land("land")
   misn.osdCreate( osd_title, {osd_msg1:format(hampla:name(),hamsys:name()), osd_msg2:format(intsys:name()), osd_msg3:format(reppla:name(),repsys:name())} )
   mark = misn.markerAdd(hamsys, "low")
end
