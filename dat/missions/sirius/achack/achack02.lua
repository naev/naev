--[[
-- This is the second mission in the Academy Hack minor campaign.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    title1 = "An unexpected reunion"
    text1 = [[    When you approach her, the officer greets you with a smile. "What a surprise that we should run into each other again," she says. "I'm afraid to say I don't remember your name. What was it again? Ah yes, %s. I don't think I introduced myself last time, my name is Joanne. Well met. As you can see I'm still doing quite well, no poison in my wine or snakes in my bed or anything." Then her expression turns more serious. "Actually, about that. I think our friend Harja still has it in for me. You had the common sense to use your head, but I'm afraid not everyone is like that. I'm convinced Harja will try to hire more assassins to do what you didn't, so I'm in considerable danger."
    You sympathize with Joanne, but you wonder aloud why she hasn't asked the local authorities for protection. Joanne gives you a somewhat uncomfortable look.
    "Pride, I suppose. I'm a military officer. It wouldn't look too handsome if I asked for personal protection when the navy is already stretched thin out there, trying to protect our civilians from pirates and other criminals every day. Besides, my conflict with Harja is a personal matter. I feel I should resolve this with my own resources." She gives you a wan smile. "That being said, I wouldn't say no to a helping hand."]]
    
    text1r = [[    "Hello again, %s," Joanne greets you. "I'm afraid I still find myself under threat from mercenary assassins. Have you reconsidered my offer? Let me tell you again what I need."]]
    
    text2 = [[    "See, this is the situation," she continues. "My duties as a Sirian administration officer require me to pay visits to several military outposts throughout Sirius space. It's a long trek every time, but that just comes with the job. Now, the trouble is that Harja's assassins might have many opportunities to ambush me along the way. I've had combat training, of course, just like every other Serra soldier, but I'm afraid i have little actual fighting experience, and I'm not sure I could survive an attack unassisted. You, however, seem to have seen quite some action. If you were to escort me while I make my rounds, I would feel a lot more secure. I can reward you, of course. Let's see... Another 150,000 credits seems a fair sum. What do you think, are you willing to do it?"]]
    
    title2 = "Joanne's escort"
    text3 = [[    "That's wonderful! Thank you so much. We should get under way immediately."]] -- Wait, what? No. Finish this!
    
    title3 = "One damsel, safe and sound"
    text3 = [[    After you both land your ships, you meet Joanne in the spaceport bar.
    "Whew! That was definitely the most exciting round I've done to date! Thank you %s, I probably owe you my life. You more than deserved your payment, I've already arranged for the transfer." Joanne hesitates, but then apparently makes up her mind. "In fact, would you sit down for a while? I think you deserve to know what this whole business with Harja is all about. And to be honest, I kind of want to talk to someone about this, and seeing how you're involved already anyway..."
    You decide to sit down and listen to Joanne's story, not in the last place because you're rather curious yourself.
    "Thank you, %s," Joanne says, "I appreciate it. Well, I guess I should start at the beginning."]]
    
    title3 = "Joanne and Harja"
    text4 = [[    "Several SCU ago, me and Harja were both students at the High Academy on Sinass. It's a very prestigious place among us Sirii, as you may or may not know. It's only one jump away from Mutris itself and... Well, anyway, it's one of the best academies in all of Sirius space, and only the most capable students are even allowed to attend. Now, I don't mean to brag, you understand, but even in that environment I was among the top rated students. And, believe it or not, so was Harja. We were in the same study unit, actually.
    "Another thing you should know is that the High Academy offers the very best its students the chance to advance to the Serra echelon, an exceptional honor for those born into the Shaira or Fyrra echelons. As you might expect, the prospect of being rewarded like that is a very strong motivation for most of the students. It was no different for Harja and myself, since we were both Fyrra echelon. With our abilities, each of us had a good chance of earning the promotion. However, since we were in the same study unit, only one of us could be promoted, since only one promotion is awarded per study unit each curriculum. That meant that Harja and I were rivals, but we were rivals in good sport. We each had every intention of winning the promotion through fair competition... Or so I thought."]]
    
    text5 = [[    "After the final exams had been taken and we were only days away from receiving the results, there was an incident. There had been a security breach in the academy's main computer. Someone had hacked the system and altered the data for the final exams, mine to be exact. My grades had been altered to be straight one hundred per cents, in every subject. Can you believe that? Someone had actually tried to make it look like I was cheating, but how could anyone take scores like that seriously? The professors didn't, of course. Nobody would be stupid enough to alter their own scores that way, so the only reason my scores would have been altered is if someone else did it, to discredit me no doubt. And you guessed it, the prime suspect was Harja. After all, if I was disqualified, he would certainly have gotten the promotion. Instead, he got what he deserved, and was expelled for his low attempt to secure his own success."
    "That's basically the history between me and Harja. Up until you came to me, I just thought of him as an untrustworthy man whose own underhanded plan backfired on him. But here we are, cycles later, and now he's trying to kill me. Why, I wonder? Could he really be so bitter over what happened that he wants me dead? Even though he has nobody to blame but himself? I just don't understand it, %, I really don't."
    Joanne remains silent for a moment, then takes a deep breath. "Whew, I feel quite a bit better now for having told this to you. Thanks for listening, it means a lot to me. I shouldn't keep you here any longer though, I'm sure you have a lot of other problems to look after."
    Joanne leaves the spaceport bar. You can't help but reflect that even in the highest levels of society, you can find envy and vice.]]

    -- Mission info stuff
    joannename = "The Serra military officer"
    joannedesc = "You know this woman. She's the military officer from before, the one you were hired to assassinate."

    osd_msg   = {}
    osd_title = "Harja's Vengeance"
    osd_msg[1] = "Follow Joanne's ship"
    osd_msg[2] = "Defeat Joanne's attackers"
    osd_msg["__save"] = true

    misn_desc = [[Joanne needs you to escort her ship and fight off mercenaries sent to kill her.]]
    misn_reward = "Joanne will pay you another 150,000 credits."
end

function create()
    -- Note: this mission does not make any mission claims.

    misn.setNPC(joannename, "none") -- TODO: Joanne's portrait
    misn.setDesc(joannedesc)
end

function accept()
    if var.peek("achack02repeat") then
        tk.msg(title1, text1r:format(player.name()))
    else
        tk.msg(title1, text1:format(player.name()))
    end
    if not tk.yesno(title1, text2) then
        var.push("achack02repeat", true)
        abort()
    end
    tk.msg(title2, text3)
    
    misn.accept()
    misn.setDesc(misn_desc)
    misn.setReward(misn_reward)
    misn.osdCreate(osd_title, osd_msg)

    hook.land("land")
    hook.enter("enter")
end

function land()
    if planet.cur() == destplanet then
    end
end

function enter()
end

function abort()
    misn.finish(false)
end