--[[
-- This is the first mission in the Academy Hack minor campaign.
--]]


title1 = _("A Sirian with a grudge")
text1 = _([[You find a young Fyrra man sitting uncomfortably amidst the uncouth characters that frequent the Wringer. He seems to be here with a purpose, but nobody seems to be giving him the time of day. Curious, you decide to talk to the man and find out why he is here.
    "Well met, stranger," the young Sirian greets you. "My name is Harja. I'm looking for someone who can help me with this... problem I have, it requires some violence. But nobody I've talked to so far seems interested! I thought this place was supposed to be filled with mercenaries and killers for hire. I can't believe how difficult this is!"
    Harja clearly seems frustrated. And it seems he's here to hire someone to do some dirty work for him. Maybe it was not such a good idea to talk to him after all?
    "Listen," he continues. "I don't intend to bore you with my personal sob story, so let's just say there's someone I want dead, a dangerous criminal. This woman did something to me cycles ago that just can't go unpunished. I've got money, I'm willing to pay. All you need to do is locate her, and discreetly take her out. I don't care how you do it. I don't even care if you enjoy it. Just come back when she's dead, and I'll pay you 400,000 credits. Do we have a deal?"]])

text2 = _([["Great! I was about to give up hope that I would find anyone with enough guts to do this for me. Okay, so, let me tell you about your target. She's a member of the Serra echelon, and she's got long, brown hair and blue eyes. She can usually be found in Sirius space, I believe she'll be on %s in the %s system right now. Come back to me when she's dead, and I'll give you your reward!"
    Harja leaves the spacedock bar, satisfied that he's finally found someone to take his request. You can't help but wonder why he would try to hire a mercenary in a place like the Wringer, though. If the target is such a dangerous criminal, then wouldn't he be better off posting a bounty mission on the public board? Oh well, it's none of your business. You accepted the job, now all that's left is to complete it.]])

title2 = _("A Sirian military officer")
text3 = _([[You approach the young officer, determined to find out what you've gotten yourself involved with. You hope this was just a big mistake.
    "Good day, pilot," the officer greets you as you approach her table. She seems quite polite, and nothing indicates that she is anything less than completely respectable. "Is there something I can help you with?"
    You introduce yourself and explain that in your travels you've come across a man who tried to hire you to murder her. When you mention that his name is Harja, the officer's eyes go wide.
    "Are you sure? Unbelievable. Just unbelievable. I know the man you speak of, and I certainly don't count him as one of my friends. But I assure you, what he told you about me is a complete lie. Yes, there is bad blood between us, it's rather personal. But as you can see my record is clean, or I wouldn't have been accepted into the military.
    "I appreciate that you used your better judgment instead of recklessly trying to attack me, which would have ended badly for at least one of us. You said Harja offered you 400,000 credits for my death, yes? I will arrange for half that again to be deposited into your account. Consider it a token of gratitude. Now, it seems I may have a situation to take care of..."
    The officer excuses herself from the table and heads to the local military station. You are satisfied for now that you got paid without having to get your hands dirty. You just hope this won't come back to bite you in the butt one day.]])

-- Mission info stuff
harjaname = _("A Fyrra civilian")
harjadesc = _("You see a young Fyrra man with a determined expression on his face.")
joannename = _("A Serra military officer")
joannedesc = _("This woman matches the description Harja gave you... But she's a military officer! This can't be right. You'd better talk to her and find out what's going on.")

osd_msg   = {}
osd_title = _("Sirian Bounty")
osd_msg[1] = _("Fly to %s")
osd_msg[2] = _("Find your target on %s and kill her")
osd_msg["__save"] = true

misn_desc = _([[A Sirian man named Harja has hired you to dispatch a "dangerous criminal" who supposedly committed some kind of crime against him.]])
misn_reward = _("Harja promised 400,000 credits.")

function create()
    -- This mission ONLY spawns if the system it's in is not claimed by another mission. Special hack to mutex with Dark Shadow.
    if not misn.claim ( {system.cur()} ) then
        abort()
    end

    misn.setNPC(harjaname, "sirius/unique/harja")
    misn.setDesc(harjadesc)
end

function accept()
    if not tk.yesno(title1, text1) then
        abort()
    end
    destplanet, destsys = planet.get("Racheka")
    tk.msg(title1, text2:format(destplanet:name(), destsys:name()))
    osd_msg[1] = osd_msg[1]:format(destsys:name())
    osd_msg[2] = osd_msg[2]:format(destplanet:name())
    misn.accept()
    misn.osdCreate(osd_title, osd_msg)
    misn.setDesc(misn_desc)
    misn.setReward(misn_reward)
    misn.markerAdd(destsys, "high")

    hook.land("land")
    hook.enter("enter")
end

function land()
    if planet.cur() == destplanet then
        misn.npcAdd("talkJoanne", joannename, "sirius/unique/joanne", joannedesc, 4)
    end
end

function enter()
    if system.cur() == destsys then
        misn.osdActive(2)
    else
        misn.osdActive(1)
    end
end

function talkJoanne()
    tk.msg(title2, text3)
    player.pay(500000) -- 500K
    misn.finish(true)
end

function abort()
    misn.finish(false)
end
