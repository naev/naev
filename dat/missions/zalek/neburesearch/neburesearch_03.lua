--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="The Substitute Speaker">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <done>Emergency of Immediate Inspiration</done>
   <chance>100</chance>
   <location>Bar</location>
   <planet>Jorla</planet>
  </avail>
 </mission>
--]]
--[[
   
   Mission: The Substitute Speaker
   
   Description: The player has to impersonate a scientist and give a scientific talk.
   
   Difficulty: Easy

--]]

require "dat/scripts/cargo_common.lua"
require "dat/scripts/numstring.lua"
require "dat/missions/zalek/common.lua"


bar_desc = _("The student is already awaiting you.")
mtitle = _("The Substitute Speaker")
mdesc = _("Fly to %s in the %s system before %s and give a scientific talk.")
misn_reward = _("a reputation as scientist (?)")
landing_title = _("Arriving on %s")
arriving_title = _("On your way to the institute")
cancel_title = _("Tactical Retreat")
talk_title = _("Scientific Talk")
lab_cout_title = _("Lab Cout Shop")
timeup_title = _("Time to visit the institute")
bar_text = _([[You join the student and explain him that Dr. Mensing actually hasn't told you what you are supposed to do.
    "Ah, right. There is a conference on %s in the %s system soon. Actually she is supposed to be the substitution for professor Voges. He is absent for weeks by now. No one knows what he is doing on Ruadan. Anyway, back to topic. You have to bring me there as ersatz-substitution for Dr. Mensing as she got seriously ill. But you already know that as you brought her here."
    You tell him that she looked perfectly fine and said she wants to conduct her research instead of going to the conference.
    "WHAT?? It was MY idea, initially! She just wants to kick me out! I bet she thinks she could just pull it off entirely without me to be the only author! No way she could ever achive that!"
    "%s, you have to go to the conference instead! What do you mean you don't know anything about science? How about this, I just give you my presentation and while you're on the way I type the exact text I'd recite during my talk and send it to you. Just recite it on my behalf and it's going to be fine!"]])
leave_text = _([["No time for discussions, sorry. While we're talking Dr. Mensing is trying to get an advantage! I have no time to lose!"
    He stands up and searches his pockets for something. "Here, the presentation," he says and hands you a small data chip. He further adds, "And don't be too late!" before rushing out of the bar in a hurry. Wait a minute, you haven't agreed to accept the mission!]])
arrival_text = _([[You arrived on %s on time. There is even some time left to prepare your talk. During the flight you threw a glance at the presentation on the data chip but you haven't managed to figure out the meaning of it. How is this supposed to work out??
    Your thoughts are interputed by an incoming message from the student. "As promised I'm sending you the text you have to recite. I hope you appreciate the time I wasted on typing that down. I haven't considered that you'll have to answer questions after the talk, though. Do what every good scientist does and just talk about some random stuff that sounds like it may be related to the question. I'm sure you'll do it just fine. Good luck!" This message is followed by what you assume is the text you have to recite.]])
cancel_text = _([[This is too much! You better leave before doing anything embarrassing or humiliating.]])
learn_text = _([[You decide to learn your text rather than simply reciting the text. It turned out to be the right decision because you haven't ever seen most of those words.]])
spaceport_text = _([[You decide to explore the spaceport. There are a couple of shops here which may sell something useful for your mission.]])
lab_cout_text = _([[You enter a shop that sells only lab couts. The assortment of lab couts is impressive: lab couts of all colors, different materials, and cuts. There are distinct sections for casual and working lab couts. Whatever the difference is, it is too subtle for you to grasp. You settle on a "business" lab cout because something formal is probably suitable for your talk. The price tag reads %s. %s]])
electronics_text = _([[While walking through a store selling electronics you notice a pair of glasses with integrated displays. You could use them to display the text you are supposed to recite. They are rather expensive though, %s. %s]])
return_text = _([[You return to your ship.]])
timeup_text = _([[It's time to go to the institute.]])
institute_text = _([[You make your way to the institute where you are supposed to give the talk, following the signs. On arriving you ask around where you should head to for your talk. Apparently you were already awaited. You are lead to a rather small seminar room. Finally you are ready to give your talk and a few scientist take seat. Actually you expected more than 17 listener. Maybe this talk is not such a big deal as you expected. They wouldn't just send you to an important presentation, right?]])
no_lab_cout_text = _("It is just now that you realize that you're the only person now wearing a lab cout. That's a nightmare! Well, maybe it would be for a scientist?")
has_lab_cout_text = _("Wearing a lab cout you blend in nicely with your audience.")
talk_unprepared_text = _([[%s
    You start reciting the text you got from the student. You realize that you should have learned the text as some of the occuring terms are difficult to pronounce correctly.]])
talk_prepared_text = _([[%s
    You start reciting the text you got from the student. It turned out learning your text was the right choice. Some of the occuring terms are difficult to pronounce correctly.]])
talk_glasses_text = _([[%s
    You start reciting the text you got from the student. With the text being displayed on your glasses you don't have to look down on a paper to read the text, but some of the occuring terms are difficult to pronounce correctly.]])
talk_finished_text = _([[Finally you are finished with your text and sigh in relief. After a brief applause someone raises his hand. Apparently he wants to ask a question. What should you do?]])
question_text = _([[You haven't understood the question, not even a single word.]])
run_text = _([[You decide to run towards the closest door and leave the building as fast as possible. You continue running until you reach the spaceport and enter your ship. You can't be the first one running from a talk, right?]])
avoid_question_text = _([[You try to avoid the question. The scientist seems to be upset. There are no further questions. Apparently there is another talk that starts very soon so people hurry off. You manage to leave without gaining much attention and return to your ship. Enough science for today!]])

choice_learn = _("Start learning your text")
choice_leave = _("Take off immediately")
choice_return = _("Return to your ship")
choice_run = _("Run!")
choice_institute = _("Go to the institute")
choice_spaceport = _("Explore the space port")
choice_lab_cout = _("Buy a lab cout")
choice_electronics = _("Visit the electronics store")
choice_open_question = _("It's an open question.")
choice_good_question = _("This is a good question.")

msg_timeup = _("You were too late. You're never going to be a great scientist!")

-- Mission info stuff
osd_msg   = {}
osd_title = _("The Substitute Speaker")
osd_msg1 = _("Fly to %s in the %s system before %s\n(%s remaining)")

log_text = _([[You gave a scientific talk in %s. Did anyone noticed you're not a scientist?]])


function create()
    -- mission variables
    homeworld, homeworld_sys = planet.get("Jorla")
    dest_planet, dest_sys = planet.get("Neo Pomerania")
    lab_cout_price = 25000 -- 25k
    glasses_price = 40000 -- 40k
    local numjumps = homeworld_sys:jumpDist(dest_sys, false)
    local traveldist = cargo_calculateDistance(homeworld_sys, homeworld:pos(), dest_sys, dest_planet)
    local stuperpx   = 0.225
    local stuperjump = 10000
    local stupertakeoff = 10500
    local allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 250 * numjumps
    timelimit  = time.get() + time.create(0, 0, allowance)
    
    -- Spaceport bar stuff
    misn.setNPC(_("Robert"), "zalek/unique/student")
    misn.setDesc(bar_desc)
end

function accept()
    tk.msg("", bar_text:format(dest_planet:name(), dest_sys:name(), player:name()))
    tk.msg("", leave_text)
    learned_text = false
    has_lab_cout = false
    has_glasses = false
    time_left = 4
    
    -- Set up mission information
    misn.setTitle(mtitle)
    misn.setReward(misn_reward)
    misn.setDesc(mdesc:format(dest_planet:name(), dest_sys:name(), timelimit:str()))
    misn_marker = misn.markerAdd(dest_sys, "high")
    
    misn.accept()
    osd_msg[1] = osd_msg1:format(dest_planet:name(), dest_sys:name(), timelimit:str(), (timelimit - time.get()):str())
    misn.osdCreate(osd_title, osd_msg)
    
    hook.land("land")
    hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
end


function land()
    landed = planet.cur()
    if landed == dest_planet then
        tk.msg(landing_title:format(dest_planet:name()), arrival_text:format(dest_planet:name()))
        enter_ship()
    end
end

function enter_ship()
    local c = 0
    time_left = time_left - 1
    if time_left < 0 then
        tk.msg(timeup_title, timeup_text)
        start_talk()
        return
    end
    if learned_text then
        c = tk.choice("", _("You wonder what to do..."), choice_spaceport, choice_leave)
    else
        c = tk.choice("", _("You wonder what to do..."), choice_spaceport, choice_leave, choice_learn)
    end
    if c == 1 then
        tk.msg("", spaceport_text)
        enter_spaceport()
        return
    elseif c == 2 then
        tk.msg(cancel_title, cancel_text)
        misn.finish(false)
        return
    elseif c == 3 then
        learned_text = true
        tk.msg("", learn_text)
        time_left = time_left - 1
    end
    enter_ship()
end

function enter_spaceport()
    local c = 0
    time_left = time_left - 1
    if time_left < 0 then
        tk.msg(timeup_title, timeup_text)
        start_talk()
        return
    end
    if has_lab_cout then
        if has_glasses then
            c = tk.choice("", _("You wonder what to do..."), choice_return, choice_institute)
        else
            c = tk.choice("", _("You wonder what to do..."), choice_return, choice_institute, choice_electronics)
        end
    else
        if has_glasses then
            c = tk.choice("", _("You wonder what to do..."), choice_return, choice_institute, choice_lab_cout)
        else
            c = tk.choice("", _("You wonder what to do..."), choice_return, choice_institute, choice_lab_cout, choice_electronics)
        end
    end
    if c == 1 then
        tk.msg("", return_text)
        enter_ship()
        return
    elseif c == 2 then
        start_talk()
        return
    elseif not has_lab_cout and c == 3 then
        if player.credits() < lab_cout_price then
            tk.msg(lab_cout_title, lab_cout_text:format(creditstring(lab_cout_price), _("Apparently this is too expensive for you. Looks like you have to give your talk without a lab cout.")))
        else
            if tk.choice(lab_cout_title, lab_cout_text:format(creditstring(lab_cout_price), _("Will you buy the lab cout?")), _("Yes"), _("No")) == 1 then
                player.pay(-lab_cout_price)
                has_lab_cout = true
            end
        end
    elseif (has_lab_cout and c == 3) or (not has_lab_cout and not has_glasses and c == 4) then
        if player.credits() < glasses_price then
            tk.msg("", glasses_text:format(creditstring(glasses_price), _("Apparently this is too expensive for you.")))
        else
            if tk.choice("", glasses_text:format(creditstring(glasses_price), _("Will you buy the glasses?")), _("Yes"), _("No")) == 1 then
                player.pay(-lab_cout_price)
                has_glasses = true
            end
        end
    end
    enter_spaceport()
end

function start_talk()
    local c = 0
    local text1
    if has_lab_cout then
        text1 = has_lab_cout_text
        faction.modPlayerSingle("Za'lek", 1)
    else
        text1 = no_lab_cout_text
    end
    tk.msg(talk_title, institute_text)
    if learned_text then
        tk.msg(talk_title, talk_prepared_text:format(text1))
        faction.modPlayerSingle("Za'lek", 1)
    elseif has_glasses then
        tk.msg(talk_title, talk_glasses_text:format(text1))
        faction.modPlayerSingle("Za'lek", 1)
    else
        tk.msg(talk_title, talk_unprepared_text:format(text1))
    end
    tk.msg(talk_title, talk_finished_text)
    c = tk.choice(talk_title, question_text, choice_good_question, choice_open_question, choice_run)
    
    if c == 3 then
        tk.msg(cancel_title, run_text)
        zlk_addNebuResearchLog(log_text:format(dest_planet:name()))
        misn.finish(true)
        return
    end
    
    tk.msg(talk_title, avoid_question_text)
    zlk_addNebuResearchLog(log_text:format(dest_planet:name()))
    faction.modPlayerSingle("Za'lek", 1)
    misn.finish(true)
end

-- Date hook
function tick()
    local osd_msg = {}
    if timelimit <= time.get() then
        -- Case missed second deadline
        player.msg(msg_timeup)
        misn.finish(false)
    else
        osd_msg[1] = osd_msg1:format(dest_planet:name(), dest_sys:name(), timelimit:str(), (timelimit - time.get()):str())
        misn.osdCreate(osd_title, osd_msg)
    end
end

