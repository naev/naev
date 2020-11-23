--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Meeting">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <chance>100</chance>
   <done>Dvaered Diplomacy</done>
   <location>Bar</location>
   <faction>Dvaered</faction>
  </avail>
 </mission>
 --]]
--[[
-- Dvaered Meeting
-- This is the 5th mission of the Frontier War Dvaered campaign.
-- The player has to securize a Dvaered Warlord Meeting.
-- Hamelsen lures out the player, while Strafer gets killed by a spy who runs away.

   Stages :
   0) 
--]]

require "dat/missions/dvaered/frontier_war/fw_common.lua"

--TODO: Set the priority and conditions of this mission

npc_name = _("Lieutenant Strafer")
npc_desc = _("Judging by how he looks at you, Strafer needs you for an other mission along with the Dvaered Space Forces.")
npc_desc = _("Harsh voice, frank gaze and easy trigger. The lieutenant Strafer is a Dvaered pilot.")

propose_title = _("We need you once more")
propose_text = _([[As you sit at his table, the clearly anxious Dvaered pilot stops biting his nails and explains why he is here.
   "The High Command summoned, under request of general Klank, an extraordinary meeting of the high council of Warlords, and all of them have accepted to come..." You frown, and before you have a chance to ask where is the problem, he continues: "... but we received an intelligence report according to which the ex-colonel Hamelsen (who has already tried to murder Major Tam several times) has for to take advantage of this meeting to take action against us."
   "Do you want to help us against that threat?"]])

accept_title = _("Here is the situation")
accept_text = _([["When a meeting of the high council of warlords occurs, a short truce takes place and they all come on the DHC station with their Goddards. This fact alone is already enough to put the station's security service under pressure, as the warlords sometimes tend to provoke each other and to make brawls. But this time, we believe that Hamelsen will try either to assassinate warlords; or to record our invasion plan in order to sell it to foreign hostile powers.
   "This is why Major Tam wants our squadron from the Special Operations Forces to support the regular units of the station. Fly to Dvaer Prime and meet me in the bar there."]])

refuse_title = _("Too bad")
refuse_text = _([[Mm. I see. you probably have many much more interesting things to do that being loyal to the Dvaered Nation...]])


lore_title = _("Lieutenant Strafer")
lore_text0 = _("What do you want to ask to the lieutenant before taking off?")
lore_already_told = _("")
quitstraf = _("Take off when you're ready for action!")
question = {}
lore_text = {}

question[1] = _("Ask for a briefing")
lore_text[1] = _([["Both squadrons of the DHC station's space security force will be deployed with full range ships from Vendettas to Goddards. Those squadrons are 'Beta-Storks' and 'Beta-Hammer' and their missions will be to control medium and heavy ships and to provide anti-heavy firepower in case of need. Our squadron, named 'Alpha-NightClaws', is in charge of fast ships (Yachts and Fighters). We will be flying Hyenas.
   "The procedure is the following: any ship approaching the station will be assigned to a squad by the fleet leader, and then to a pilot by the squad leader (Captain Leblanc). When a ship is attributed to you, it will appear green on your screen, and you'll have to hail the pilot and request the security clearance code. The code will be automatically processed by the system we install right now on your core unit. Afterwards, the ship will be allowed to land, or ordered to fly away. The same thing happens for ships that leave the station.
   "Finally, in case something happens, you will of course have to obey to orders. Watch your messages closely. A few pilots will be kept in reserve close to the station.
   "Oh, and there is an other point I must warn you about: it's the warlord's humour. When they see a small ship close to their Goddard, they may get the idea to shoot a small railgun-volley in your direction. Some of them tend to enjoy seeing pilots dodge for their life. Dvaered laws authorize warlords to do so provided they can assure the High Command that there was no hostile intention. That can be a bit annoying, sometimes."]])

question[2] = _("Ask about Colonel Hamelsen")
lore_text[2] = _([[When you mention the colonel Hamelsen, Strafer cuts you: "Ex-colonel Hamelsen! She is a traitor and has lost all her commendations. Now, she is nothing for the Dvaered, and things are better like that." You ask him if things may have turned differently for her and he answers:
   "Watch out, citizen %s: this kind of question leads to compassion. Compassion leads to weakness and weakness leads to death. Death for yourself and for those who trusted you. Death for your leaders and for your subordinates. Death for people you love and people you respect. Remember: if you want to be strong, don't listen to compassion. Don't even let compassion any chance to reach your heart."]])

question[3] = _("Ask why you were hired for this mission")
lore_text[3] = _([[You ask Strafer why Major Tam requested you to be part of the mission. "Actually, we did not need a private pilot. I just managed to convince Captain Leblanc to hire you." As you wonder why he did that, Strafer thinks a bit and smiles: "Well, I get the feeling we are doing good job, together. Aren't we?"]])

question[4] = _("Ask how one becomes a warlord")
lore_text[4] = _([["You wish to become one of them?" Before you have a chance to deny, he continues: "Anybody can become a warlord. One just has to have received the '9th grade commendation', and to conquer a planet (or a station). In the army, every rank gives you a commendation grade, for example, I have the 3rd grade. Civilian also obtain commendation for their high deeds, you obtained the first grade commendation for your involvement in the FLF destruction, if I am right. The 9th grade commendation, that is associated to the rank of first class General in the army, gives the right to own a military base, and by extension, to be granted the regal power over a region.
   "In the Dvaered army, everybody starts as a raw soldier, no matter if you're an infantryman, a pilot, a medic or even a General's child. And only Valor decides how quick you rise in the hierarchy. Warlord is the ultimate rank for the military (and combat private pilots, like yourself)"]])


hamtitle = _("Hi there!")
hamtext = _([[The fleeing ship suddenly hails you. You answer and the face of the Colonel Hamelsen emerges from your holoscreen. "No, you won't best me, %s. Not this time. Not anymore." Aware that she is now too far away for you to hope to catch her, you ask her what interest she finds in constantly harassing Major Tam. "This is all that remains from me" she answers.
   "My hate for Tam and Klank is all that remains from me now that my Lord is dead. I dedicated my entire life to the greatness of House Dvaered, I practiced and sharpened all my skills to serve the Army at best. When I got recruited by Lord Battleaddict, I became faithful to him because he was giving me the opportunity to serve House Dvaered through him. And then...
   "Since the day when Klank assassinated my Lord, I have been rejected by the High Command. Rejected by the Warlords. Rejected by that nation that claims to reward Valor and Righteousness. Tell me when I have given up Valor! Tell me when I have given up Righteousness! Never! The Dvaered social contract is broken as far as I am concerned.
   "All that remains from me is a vassal without suzerain, a colonel without regiment, a corpse without grave. I will haunt your team until your demise. I will be on the way of any of your wishes, big ones as well as small ones. There will be no forgiveness, no remission, no relief, neither for you nor for me."
   After this very encouraging speech, Hamelsen cuts off the communication channel and jumps away.]])

lords_chatter = [ _("Ahoy, suckers! Here comes your master!"),
                  _("Look down, you weaklings."),
                  _("Only submission will save you from my anger!"), ]

