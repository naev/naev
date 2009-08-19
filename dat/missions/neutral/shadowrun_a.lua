--[[
-- This is a helper script for the Shadowrun mission.
-- "shadowrun" stack variable:
-- 1 = player has met Rebina, but hasn't accepted the mission
-- 2 = player has accepted Rebina's mission, but has not talked to SHITMAN
-- 3 = player has talked to SHITMAN
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}

   title[1] = "An unpleasant man"
   title[2] = "Jorek's scorn"
   title[3] = "Dismissal"
   text[1] = [[You join the man at his table. He doesn't particularly seem to welcome your company, though, because he gives you a look most people would reserve for particularly unwelcome guests. Determined not to let that get to you, you ask him if his name is indeed Jorek.
   "Yeah, that's me," he replies. "What'd ya want, kid?"
   You explain to him that you've come to see to his special needs. This earns you a sneer from Jorek. "Ha! So you're running errands for the little lady, are you? Oh don't tell me, I've got a pretty good idea what it is you want from me." He leans onto the table, bringing his face closer to yours. "Listen, buddy. I don't know if you noticed, but people are watchin' me. And you too, now that you're talkin' to me. Those goons over there? Yeah, they're here for me. Used to be fancy undercover agents, but I've been sittin' on my ass here for a long time and they figured out I was on to them, so they replaced 'em with a bunch of grunts. Cheaper, see."
   "And it's not just them," Jorek continues. "On your way here, did you see the flotilla of 'patrol ships' hangin' around? You guessed it, they're waitin' for me to split this joint. I'm HOT, kid. If I step onto your ship, you'll be hot too. And you have absolutely no problem with that, is that what you're tellin' me?]]
   text[2] = [[Jorek roars with laughter. "Hah! Yeah, I'm sure you don't! I know what you're thinkin', I do. You'll take me outta here, pull a heroic bust past them Empire ships, save me, and the day while you're at it, then earn your stripes with the lady, am I right? Syeah, I bet you'd take on the world for a pretty face and a coy smile." He doesn't so much as make an attempt to keep the mocking tone out of his voice.
   "Well, good for you. You're a real hero, right enough. But you know what? I'm stayin' put. I don't care if you have the vixen's approval. I'm not gettin' on some random Joe's boat just so he can get us both blasted to smithereens."
   Your patience with Jorek's abuse is finally at an end, and you heatedly make it clear to him that your abilities as a pilot aren't deserving of this treatment. Jorek, however, seems unimpressed. He tells you to stick it where the sun doesn't shine, gets up from his chair and squarely deposits himself at another table. Unwilling to stoop to his level, you choose not to follow him.]]
   text[3] = [[Jorek exhales derisively. "No, I thought not. Probably thought this was going to be a walk in the park, didn't you? But when the chips are down, you back out. Wouldn't want to mess with be big scary Empire, would we?" He raises his voice for this, taunting the military personnel in the bar. They don't show any sign of having even heard Jorek speak. Jorek snorts, then focuses his attention back on you.
   "I've got no use for wusses like yourself. Go on, get out of here. Go back to your ship and beat it off this rock. Maybe you should consider gettin' yourself a desk job, eh?"
   With that, Jorek leaves your table and sits down at a nearby empty one. Clearly this conversation is over, and you're not going to get anything more out of him.]]
   text[4] = [[Jorek pointedly ignores you. It doesn't seem like he's willing to give you the time of day any longer. You decide not to push your luck.]]
   
   -- Mission details
   bar_desc = "A middle-aged, cranky looking man is sitting at a table by himself. You are fairly certain that this is the fellow you're looking for."
   
end

function create ()
    misn.setNPC( "An unpleasant man", "none" )
    misn.setDesc( bar_desc ) 
end

function accept()
    if var.peek("shadowrun") == 2 then
        if tk.yesno(title[1], text[1]) then 
            tk.msg(title[2], text[2])
        else
            tk.msg(title[2], text[3])
        end
        var.push("shadowrun", 3)
        misn.finish()
    else
        tk.msg(title[3], text[4])
        misn.finish()
    end
end