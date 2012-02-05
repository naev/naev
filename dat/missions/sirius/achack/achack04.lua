--[[
-- This is the fourth mission in the Academy Hack minor campaign.
-- This mission is started from a helper event.
--]]

include "scripts/fleethelper.lua"
include "scripts/proximity.lua"
include "scripts/enum.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title1 = "You have mail"
   text1 = [[   Your computer console flashes you a notice. It seems you received a message through the Sirian information exchange network. You play it.
   The message is from Joanne, the woman you've had dealings with in the past. Her recorded image looks at you from the screen. "Dear %s," she begins. "You have helped me on several occasions in regard with my personal problem. I've given it some thought since then, and I've come to the conclusion that I want to get to the bottom of this. To do so, I will need your help yet again. I'm currently on assignment on %s in the %s system. Please meet me there at the earliest opportunity."
   The message ends. You save it for later reference. Maybe you should swing by %s to see what Joanne wants.]]
   
   title2 = "Joanne"
   text2 = [[   Joanne greets you warmly. She is clearly glad to see you again. "Thank you for coming, %s," she says. "As I already mentioned in my message, I've decided that I want to clear up this whole mess with Harja and the academy incident. Too much has happened for me to just forget about it, and whatever my opinion of Harja may be, I cannot ignore his oath as a follower of Sirichana. This matter has evolved from an old grudge to a mystery."]]
   text2r = [[   "Hello again, %s," Joanne says. "I still require your help to solve the mystery of the academy computer hack. Let me tell you again what I need from you."]]
   
   text3 = [[   Joanne reaches into her briefcase and takes out a data storage unit, which she then puts on the table. "This data unit contains an invitation from me to Harja. I'm asking him to meet me here. I would send it to him directly, but unfortunately I have no way of reaching him other than through you. You've found him twice before, I'm sure you can do it again. Undoubtedly, he will be in Sirius space, frequenting the spaceport bars. All I ask is that you keep an eye out for him in your travels, and when you see him, give him my message." She hesitates, but then continues. "You've met him, so you know he's a bit temperamental these days. Please convince him to accept my invitation. Without violence, if you can. Could you do this for me?"]]

   title3 = "Once more, with feeling"
   text4 = [[   You pocket the data unit and tell Joanne you will see what you can do. "Thank you %s," she says. "I'm still pretty busy with my job, so I won't be here all the time, but just ping me on the information exchange when you've found Harja, and I'll make sure to be here when you arrive."
   When Joanne is gone, you take a moment to reflect that you're going to have to deal with Harja again. Joanne wanted no violence, but will Harja leave room for that? You'll find out when you catch him.]]
   
   title4 = "Remember me?"
   text5 = [[   ]]

   -- Mission info stuff
   joannename = "Joanne"
   joannedesc = "Joanne the Serra military officer is here, enjoying a drink by herself."
   harjaname = "Harja"
   harjadesc = "You've found Harja. He's sourly watching the galactic news, and hasn't noticed you yet."

   osd_msg   = {}
   osd_title = "Sirian Truce"
   osd_msg[1] = "Look for Harja in Sirian bars"
   osd_msg2org = "Convince Harja to come with you"
   osd_msg2alt = "Go to %s and deal with Harja's associates"
   osd_msg[2] = osd_msg2org
   osd_msg[3] = "Return to %s (%s)"
   osd_msg["__save"] = true

   misn_desc = "Joanne has contacted you. She wants to meet you on %s (%s)."
   misn_desc2 = "Joanne wants you to find Harja and convince him to meet her in person."
   misn_reward = "Not specified."
end

function create()
   -- Note: this mission does not make any system claims.
   startplanet, startsys = planet.get("Eenerim")
   tk.msg(title1, text1:format(player.name(), startplanet:name(), startsys:name(), startplanet:name()))

   stages = enum("start", "findHarja", "killAssociates", "end")
   stage = 1
   
   -- This mission auto-accepts, but a choice will be offered to the player later. No OSD yet.
   misn.accept()
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc:format(startplanet:name(), startsys:name()))
   hook.land("land")
end

-- Land hook.
function land()
   if planet.cur() == startplanet and stage == stages.start then
      misn.npcAdd("talkJoanne", joannename, "sirius/unique/joanne", joannedesc, 4)
   elseif planet.cur() ~= startplanet and stage == stages.findHarja then
      -- Harja appears randomly in the spaceport bar.
      -- TODO: Add checks for planet metadata. Harja must not appear on military installations and such.
      if rnd.rnd() < 0.25 then
         misn.npcAdd("talkHarja", harjaname, "sirius/unique/harja", harjadesc, 4)
      end
   end
end

-- Talking to Joanne.
function talkJoanne()
   if var.peek("achack04repeat") then
      tk.msg(title2, text2r:format(player.name()))
   else
      tk.msg(title2, text2:format(player.name()))
   end
   if not tk.yesno(tk.msg(title2, text3)) then
      -- rejected
      abort()
   else
      -- accepted
      stage = stage + 1
      tk.msg(title3, text4)
      misn.osdCreate(osd_title, osd_msg)
      misn.setDesc(misn_desc2)
   end
end

-- Talking to Harja.
function talkHarja()
   tk.msg(title4, text5)
   stage = stage + 1
end

function abort()
   misn.finish(false)
   var.push("achack04repeat", time.get():toNumber()) -- This is to ensure the mission won't repeat for a while.
end