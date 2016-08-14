--[[

   Raelid Outpost Restoration
   Copyright (C) 2014-2016 Julie Marchant <onpon4@riseup.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

-- localization stuff
lang = naev.lang()
if lang == "notreal" then
else -- default English
   title = {}
   text = {}

   title[1] = "The Deal"
   text[1] = [["Ah, there you are, %s!" She motions for you to sit down and orders you a drink. "We have looked over the conditions set forth by the Empire. I have to be perfectly honest with you: not everyone is taking it well. I like the deal, personally. I think it's the way to go to actually make progress, rather than just blowing things up as we have been doing. Most of the other higher-ups agree with me, but when word got out of the deal, a lot of lower-ranking officials were outraged.
    "I'm sorry, I should explain why this is controversial. See, one of the main conditions the Empire set forth was that we have to gradually put an end to all aggression against House Dvaered, and to prove that we are willing to do this, we have to agree to remove explosives that were planted within the Raelid Outpost recently."]]

   title[2] = "What explosives?"
   text[2] = [[Cheryl sees the confusion on your face. "Ah, yes, I had forgotten. You assisted in that mission, but you were not provided specific details. See, back when you were diverting attention away from Raelid, the operation at hand was to plant hidden explosives in Raelid Outpost. It was a very complex plan that took months of planning and quite a few pay-offs and was going to end in the destruction of that outpost.
    "Well, the Empire apparently noticed our actions; they don't know where we planted the explosives or what exactly we were planning on doing with them, but they detected the explosives aboard one of our ships as well as it landed on the outpost. I don't know for sure why they didn't warn the Dvaereds, but my guess is it was a result of some sort of tactical decision."]]

   title[3] = "The Mission"
   text[3] = [["So, anyway, that's your mission: to disarm and remove all of the explosives planted in Raelid Outpost. I know this is outside of your normal job responsibilities, but you are the only one of us who has personally earned the Empire's trust and this will also help restore trust between you and the Dvaereds, if the mission is successful."
    Cheryl takes a deep breath. "Now, there is a chance that this could be a trap. I don't think it is, but a couple people insisted on this: you will be escorted by several FLF pilots. Your escorts will meet with you when you get to Raelid. However, they have been instructed NOT to engage the Dvaereds except as an absolute last resort. You, especially, must follow this rule. If you act aggressively against Dvaered ships, the Empire will surely not only cancel the deal, but begin earnestly assisting the Dvaereds against us. We cannot survive if that happens."]]

   text[4] = [[Cheryl hands you a chip. "This data chip contains the authorization code you will need to land on Raelid Outpost, a map showing you the locations of all of the explosives, and instructions for how to disarm them," she explains. "You will need to contact Raelid Outpost first and transmit the authorization code. Once you are given clearance, land and complete your mission. Report back here when you are finished."
    She stands, as do you. "Good luck, %s. I hope you are successful. We will await your return with great anticipation." As she leaves, you notice her walking past an old acquaintance: Corporal Benito! How odd; she is drinking alone, and she seems troubled. You wonder what could possibly be on her mind.]]

   misn_title = "Raelid Outpost Restoration"
   misn_desc = "Go to Raelid Outpost and remove the hidden explosives planted there."
   misn_reward = "Sealing the deal with the Empire"

   npc_name = "Cheryl"
   npc_desc = "Cheryl is walking around the room, seemingly looking for someone."

   def_name = "Benito"
   def_desc = "You see Corporal Benito in the bar. She seems troubled."

   osd_title   = "Raelid Outpost Restoration"
   osd_desc    = {}
   osd_desc[1] = "Go to the %s system"
   osd_desc[2] = "Contact %s for authorization to land"
   osd_desc[3] = "Land on %s and remove all explosives"
   osd_desc[4] = "Return to FLF base and report back to Cheryl"
   osd_desc["__save"] = true
end
