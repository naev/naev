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

local car = require "common.cargo"
local fmt = require "format"
local zlk = require "common.zalek"

-- Mission constants
local homeworld, homeworld_sys = planet.getS("Jorla")
local dest_planet, dest_sys = planet.getS("Neo Pomerania")
local lab_coat_price = 25e3
local glasses_price = 40e3

function create()
    -- mission variables
    local numjumps = homeworld_sys:jumpDist(dest_sys, false)
    local traveldist = car.calculateDistance(homeworld_sys, homeworld:pos(), dest_sys, dest_planet)
    local stuperpx   = 0.225
    local stuperjump = 10000
    local stupertakeoff = 10500
    local allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 250 * numjumps
    timelimit  = time.get() + time.create(0, 0, allowance)

    -- Spaceport bar stuff
    misn.setNPC(_("Robert"), "zalek/unique/student.webp", _("The student is already awaiting you."))
end

function accept()
    tk.msg("", fmt.f(_([[You join the student and explain him that Dr. Mensing actually hasn't told you what you are supposed to do.
    "Ah, right. There is a conference on {pnt} in the {sys} system soon. Actually she is supposed to be the substitution for professor Voges. He is absent for weeks by now. No one knows what he is doing on Ruadan. Anyway, back to topic. You have to bring me there as ersatz-substitution for Dr. Mensing as she got seriously ill. But you already know that as you brought her here."
    You tell him that she looked perfectly fine and said she wants to conduct her research instead of going to the conference.
    "WHAT?? It was MY idea, initially! She just wants to kick me out! I bet she thinks she could just pull it off entirely without me to be the only author! No way she could ever achieve that!"
    "{player}, you have to go to the conference instead! What do you mean you don't know anything about science? How about this, I just give you my presentation and while you're on the way I type the exact text I'd recite during my talk and send it to you. Just recite it on my behalf and it's going to be fine!"]]), {pnt=dest_planet, sys=dest_sys, player=player:name()}))
    tk.msg("", _([["No time for discussions, sorry. While we're talking Dr. Mensing is trying to get an advantage! I have no time to lose!"
    He stands up and searches his pockets for something. "Here, the presentation," he says and hands you a small data chip. He further adds, "And don't be too late!" before rushing out of the bar in a hurry. Wait a minute, you haven't agreed to accept the mission!]]))
    learned_text = false
    has_lab_coat = false
    has_glasses = false
    time_left = 4

    -- Set up mission information
    misn.setTitle(_("The Substitute Speaker"))
    misn.setReward(_("a reputation as scientist (?)"))
    misn.setDesc(fmt.f(_("Fly to {pnt} in the {sys} system before {time} and give a scientific talk."), {pnt=dest_planet, sys=dest_sys, time=timelimit}))
    misn_marker = misn.markerAdd(dest_sys, "high")

    misn.accept()
    hook.land("land")
    hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
    tick() -- set OSD
end


function land()
    landed = planet.cur()
    if landed == dest_planet then
        tk.msg(
            fmt.f(_("Arriving on {pnt}"), {pnt=dest_planet}),
            fmt.f(_([[You arrived on {pnt} on time. There is even some time left to prepare your talk. During the flight you threw a glance at the presentation on the data chip but you haven't managed to figure out the meaning of it. How is this supposed to work out??
    Your thoughts are interrupted by an incoming message from the student. "As promised I'm sending you the text you have to recite. I hope you appreciate the time I wasted on typing that down. I hadn't considered that you'll have to answer questions after the talk, though. Do what every good scientist does and just talk about some random stuff that sounds like it may be related to the question. I'm sure you'll do it just fine. Good luck!" This message is followed by what you assume is the text you have to recite.]]), {pnt=dest_planet})
        )
        enter_ship()
    end
end

function enter_ship()
    local c
    time_left = time_left - 1
    if time_left < 0 then
        tk.msg(_("Time to visit the institute"), _([[It's time to go to the institute.]]))
        start_talk()
        return
    end
    if learned_text then
        c = tk.choice("", _("You wonder what to do..."), _("Explore the space port"), _("Take off immediately"))
    else
        c = tk.choice("", _("You wonder what to do..."), _("Explore the space port"), _("Take off immediately"), _("Start learning your text"))
    end
    if c == 1 then
        tk.msg("", _([[You decide to explore the spaceport. There are a couple of shops here which may sell something useful for your mission.]]))
        enter_spaceport()
        return
    elseif c == 2 then
        tk.msg(_("Tactical Retreat"), _([[This is too much! You better leave before doing anything embarrassing or humiliating.]]))
        misn.finish(false)
        return
    elseif c == 3 then
        learned_text = true
        tk.msg("", _([[You decide to learn your text rather than simply reciting the text. It turned out to be the right decision because you haven't ever seen most of those words.]]))
        time_left = time_left - 1
    end
    enter_ship()
end

function enter_spaceport()
    local c
    time_left = time_left - 1
    if time_left < 0 then
        tk.msg(_("Time to visit the institute"), _([[It's time to go to the institute.]]))
        start_talk()
        return
    end
    if has_lab_coat then
        if has_glasses then
            c = tk.choice("", _("You wonder what to do..."), _("Return to your ship"), _("Go to the institute"))
        else
            c = tk.choice("", _("You wonder what to do..."), _("Return to your ship"), _("Go to the institute"), _("Visit the electronics store"))
        end
    else
        if has_glasses then
            c = tk.choice("", _("You wonder what to do..."), _("Return to your ship"), _("Go to the institute"), _("Buy a lab coat"))
        else
            c = tk.choice("", _("You wonder what to do..."), _("Return to your ship"), _("Go to the institute"), _("Buy a lab coat"), _("Visit the electronics store"))
        end
    end
    if c == 1 then
        tk.msg("", _([[You return to your ship.]]))
        enter_ship()
        return
    elseif c == 2 then
        start_talk()
        return
    elseif not has_lab_coat and c == 3 then
        local lab_coat_text = fmt.f(_([[You enter a shop that sells only lab coats. The assortment of lab coats is impressive: lab coats of all colors, different materials, and cuts. There are distinct sections for casual and working lab coats. Whatever the difference is, it is too subtle for you to grasp. You settle on a "business" lab coat because something formal is probably suitable for your talk. The price tag reads {credits}.]]), {credits=fmt.credits(lab_coat_price)})
        if player.credits() < lab_coat_price then
            tk.msg(_("Lab Coat Shop"), lab_coat_text .. "\n\n" .. _("Apparently this is too expensive for you. Looks like you have to give your talk without a lab coat."))
        else
            if tk.choice(_("Lab Coat Shop"), lab_coat_text .. "\n\n" .. _("Will you buy the lab coat?"), _("Yes"), _("No")) == 1 then
                player.pay(-lab_coat_price)
                has_lab_coat = true
            end
        end
    elseif (has_lab_coat and c == 3) or (not has_lab_coat and c == 4) then
        local electronics_text = fmt.f(_([[While walking through a store selling electronics you notice a pair of glasses with integrated displays. You could use them to display the text you are supposed to recite. They are rather expensive though, {credits}.]]), {credits=fmt.credits(glasses_price)})
        if player.credits() < glasses_price then
            tk.msg("", electronics_text .. "\n\n" .. _("Apparently this is too expensive for you."))
        else
            if tk.choice("", electronics_text .. "\n\n" .. _("Will you buy the glasses?"), _("Yes"), _("No")) == 1 then
                player.pay(-lab_coat_price)
                has_glasses = true
            end
        end
    end
    enter_spaceport()
end

function start_talk()
    local text1
    if has_lab_coat then
        text1 = _("Wearing a lab coat you blend in nicely with your audience.")
        faction.modPlayerSingle("Za'lek", 1)
    else
        text1 = _("It is just now that you realize that you're the only person not wearing a lab coat. That's a nightmare! Well, maybe it would be for a scientist?")
    end
    tk.msg(_("Scientific Talk"), _([[You make your way to the institute where you are supposed to give the talk, following the signs. On arriving you ask around where you should head to for your talk. Apparently you were already awaited. You are lead to a rather small seminar room. Finally you are ready to give your talk and a few scientist take seat. Actually you expected more than 17 listener. Maybe this talk is not such a big deal as you expected. They wouldn't just send you to an important presentation, right?]]))
    if learned_text then
        tk.msg(_("Scientific Talk"), text1 .. "\n\n" .. _([[You start reciting the text you got from the student. It turned out learning your text was the right choice. Some of the occurring terms are difficult to pronounce correctly.]]))
        faction.modPlayerSingle("Za'lek", 1)
    elseif has_glasses then
        tk.msg(_("Scientific Talk"), text1 .. "\n\n" .. _([[You start reciting the text you got from the student. With the text being displayed on your glasses you don't have to look down on a paper to read the text, but some of the occurring terms are difficult to pronounce correctly.]]))
        faction.modPlayerSingle("Za'lek", 1)
    else
        tk.msg(_("Scientific Talk"), text1 .. "\n\n" .. _([[You start reciting the text you got from the student. You realize that you should have learned the text as some of the occurring terms are difficult to pronounce correctly.]]))
    end
    tk.msg(_("Scientific Talk"), _([[Finally you are finished with your text and sigh in relief. After a brief applause someone raises his hand. Apparently he wants to ask a question. What should you do?]]))
    local c = tk.choice(_("Scientific Talk"), _([[You haven't understood the question, not even a single word.]]), _("This is a good question."), _("It's an open question."), _("Run!"))

    if c == 3 then
        tk.msg(_("Tactical Retreat"), _([[You decide to run towards the closest door and leave the building as fast as possible. You continue running until you reach the spaceport and enter your ship. You can't be the first one running from a talk, right?]]))
    else
        tk.msg(_("Scientific Talk"), _([[You try to avoid the question. The scientist seems to be upset. There are no further questions. Apparently there is another talk that starts very soon so people hurry off. You manage to leave without gaining much attention and return to your ship. Enough science for today!]]))
        faction.modPlayerSingle("Za'lek", 1)
    end
    zlk.addNebuResearchLog(fmt.f(_([[You gave a scientific talk on {pnt}. Did anyone notice you're not a scientist?]]), {pnt=dest_planet}))
    misn.finish(true)
end

-- Date hook
function tick()
    local osd_msg = {}
    if timelimit <= time.get() then
        -- Case missed second deadline
        player.msg(_("You were too late. You're never going to be a great scientist!"))
        misn.finish(false)
    else
        osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system before {time_limit}\n({time} remaining)"),
	   {pnt=dest_planet, sys=dest_sys, time_limit=timelimit, time=(timelimit - time.get())})
        misn.osdCreate(_("The Substitute Speaker"), osd_msg)
    end
end

