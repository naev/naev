--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Substitute Speaker">
 <unique />
 <priority>4</priority>
 <done>Emergency of Immediate Inspiration</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Jorla</spob>
 <notes>
  <campaign>Nebula Research</campaign>
 </notes>
</mission>
--]]
--[[

   Mission: The Substitute Speaker

   Description: The player has to impersonate a scientist and give a scientific talk.

   Difficulty: Easy

--]]

local car = require "common.cargo"
local fmt = require "format"
local nebu_research = require "common.nebu_research"
local vn = require 'vn'

local student_portrait = nebu_research.student.portrait

-- Mission constants
local homeworld, homeworld_sys = spob.getS("Jorla")
local dest_planet, dest_sys = spob.getS("Neo Pomerania")
local lab_coat_price = 25e3
local glasses_price = 40e3

local enter_ship -- Forward-declared functions

function create()
    -- mission variables
    local numjumps = homeworld_sys:jumpDist(dest_sys, false)
    local traveldist = car.calculateDistance(homeworld_sys, homeworld:pos(), dest_sys, dest_planet)
    local stuperpx   = 0.225
    local stuperjump = 10000
    local stupertakeoff = 10500
    local allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 250 * numjumps
    mem.timelimit  = time.get() + time.new(0, 0, allowance)

    -- Spaceport bar stuff
    misn.setNPC(_("Robert"), student_portrait, _("The student is already awaiting you."))
end

function accept()
    vn.clear()
    vn.scene()
    local student = vn.newCharacter( nebu_research.vn_student() )
    vn.transition("fade")
    vn.na(_("You join the student and explain to him that Dr. Mensing actually hasn't told you what you are supposed to do."))
    student(fmt.f(_([["Ah, right. There is a conference on {pnt} in the {sys} system soon. Actually she is supposed to be the substitute for professor Voges. He's been absent for weeks by now. No one knows what he is doing on Ruadan. Anyway, back to topic. You have to bring me there as ersatz-substitution for Dr. Mensing as she got seriously ill. But you already know that as you brought her here."]]), {pnt=dest_planet, sys=dest_sys}))
    vn.na(_("You tell him that she looked perfectly fine and said she wants to conduct her research instead of going to the conference."))
    student(_([["WHAT?? It was MY idea initially! She just wants to kick me out! I bet she thinks she could just pull it off entirely without me and be the only author! No way she could ever achieve that!"]]))
    student(fmt.f(_([["{player}, you have to go to the conference instead! What do you mean you don't know anything about science? How about this, I just give you my presentation and while you're on the way I type the exact speech I'd recite during my talk and send it to you. Just recite it on my behalf and it'll go fine!"]]), {player=player.name()}))
    student(_([["No time for discussions, sorry. While we're talking Dr. Mensing is trying to get an advantage! I have no time to lose!"]]))
    student(_([[He stands up and searches his pockets for something.
"Here, the presentation," he says and hands you a small data chip. He further adds, "And don't be too late!" before rushing out of the bar in a hurry.]]))
    vn.na(_("Wait a minute, you haven't agreed to accept the mission!"))
    vn.done()
    vn.run()

    mem.learned_text = false
    mem.has_lab_coat = false
    mem.has_glasses = false
    mem.time_left = 5

    -- Set up mission information
    misn.accept()
    misn.setTitle(_("The Substitute Speaker"))
    misn.setReward(_("a reputation as scientist (?)"))
    misn.setDesc(fmt.f(_("Fly to {pnt} in the {sys} system before {time} and give a scientific talk."), {pnt=dest_planet, sys=dest_sys, time=mem.timelimit}))
    mem.misn_marker = misn.markerAdd(dest_planet, "high")

    hook.land("land")
    hook.date(time.new(0, 0, 100), "tick") -- 100STU per tick
    tick() -- set OSD
end


function land()
    mem.landed = spob.cur()
    if mem.landed == dest_planet then
        vn.clear()
        vn.scene()
        vn.transition("fade")
        vn.na(fmt.f(_("You arrived at {pnt} on time. There is even some time left to prepare your talk. During the flight, you glanced at the presentation on the data chip, but you haven't managed to figure out the meaning of it. How is this supposed to work out??"), {pnt=dest_planet}))
        local student = vn.newCharacter( nebu_research.vn_student() )
        student(_([[Your thoughts are interrupted by an incoming message from the student.
"As promised I'm sending you the speech you have to recite. I hope you appreciate the time I wasted on typing that out. I hadn't considered that you'll have to answer questions after the talk, though. Do what every good scientist does and just talk about some random stuff that sounds like it may be related to the question. I'm sure you'll do it just fine. Good luck!"]]))
        vn.na(_("This message is followed by what you assume is the speech you have to recite."))
        vn.done()
        vn.run()
        enter_ship()
    end
end

function enter_ship()
    mem.location = "ship"
    vn.clear()
    vn.scene()
    vn.transition()
    vn.label( "next_turn" )
    vn.func( function ()
        mem.time_left = mem.time_left - 1
        if mem.time_left < 0 then
            vn.jump( "time_over" )
        elseif mem.location == "ship" then
            if mem.learned_text then
                vn.jump("at_ship_learned")
            else
                vn.jump("at_ship")
            end
        elseif mem.location == "spaceport" then
            if mem.has_lab_coat then
                if mem.has_glasses then
                    vn.jump("at_spaceport_lab_coat_glasses")
                else
                    vn.jump("at_spaceport_lab_coat")
                end
            else
                if mem.has_glasses then
                    vn.jump("at_spaceport_glasses")
                else
                    vn.jump("at_spaceport")
                end
            end
        end
    end )

    vn.label( "at_ship" )
    vn.na(_("You wonder what to do."))
    vn.menu( {
        { _("Explore the spaceport"), "spaceport" },
        { _("Take off immediately"), "abbort" },
        { _("Start learning your speech"), "learn" },
    } )
    vn.label( "at_ship_learned" )
    vn.menu( {
        { _("Explore the spaceport"), "spaceport" },
        { _("Take off immediately"), "abbort" },
    } )

    vn.label( "at_spaceport" )
    vn.menu( {
        { _("Buy a lab coat"), "lab_coat_store" },
        { _("Visit the electronics store"), "electronics_store" },
        { _("Return to your ship"), "ship" },
        { _("Go to the institute"), "time_over" },
    } )
    vn.label( "at_spaceport_lab_coat" )
    vn.menu( {
        { _("Visit the electronics store"), "electronics_store" },
        { _("Return to your ship"), "ship" },
        { _("Go to the institute"), "time_over" },
    } )
    vn.label( "at_spaceport_glasses" )
    vn.menu( {
        { _("Buy a lab coat"), "lab_coat_store" },
        { _("Return to your ship"), "ship" },
        { _("Go to the institute"), "time_over" },
    } )
    vn.label( "at_spaceport_lab_coat_glasses" )
    vn.menu( {
        { _("Return to your ship"), "ship" },
        { _("Go to the institute"), "time_over" },
    } )


    vn.label( "abbort" )
    vn.na(_("This is too much! You better leave before doing anything embarrassing or humiliating."))
    vn.func( function ()
        misn.finish(false)
    end )
    vn.done()

    vn.label( "learn" )
    vn.func( function ()
        mem.learned_text = true
    end )
    vn.na(_("You decide to learn your speech rather than simply reciting the text. It turned out to be the right decision because you haven't ever seen most of those words."))
    vn.jump( "next_turn" )

    vn.label( "ship" )
    vn.na(_("You return to your ship."))
    vn.func( function ()
        mem.location = "ship"
    end)
    vn.jump( "next_turn" )

    vn.label( "spaceport" )
    vn.na(_("You decide to explore the spaceport. There are a couple of shops here which may sell something useful for your mission."))
    vn.func( function ()
        mem.location = "spaceport"
    end)
    vn.jump( "next_turn" )

    vn.label( "lab_coat_store" )
    vn.na(fmt.f(_([[You enter a shop that sells only lab coats. The assortment of lab coats is impressive: lab coats of all colours, different materials, and cuts. There are distinct sections for casual and working lab coats. Whatever the difference is, it is too subtle for you to grasp. You settle on a "business" lab coat because something formal is probably suitable for your talk. The price tag reads {credits}.]]), {credits=fmt.credits(lab_coat_price)}))
    vn.func( function ()
        if player.credits() < lab_coat_price then
            vn.jump("lab_coat_too_expensive")
        else
            vn.jump("lab_coat_for_sale")
        end
    end )
    vn.label( "lab_coat_too_expensive" )
    vn.na(_("Apparently this is too expensive for you. Looks like you have to give your talk without a lab coat. Is there anything else you want to do?"))
    vn.jump( "next_turn" )
    vn.label( "lab_coat_for_sale" )
    vn.menu( {
        { _("Buy the lab coat"), "buy_lab_coat" },
        { _("Don't buy the lab coat"), "next_turn" },
    } )

    vn.label( "buy_lab_coat" )
    vn.na(_("You buy the lab coat and put it on. You almost feel like a real scientist. Is there anything else you want to do?"))
    vn.func( function ()
        player.pay(-lab_coat_price)
        mem.has_lab_coat = true
        vn.jump( "next_turn" )
    end )

    vn.label( "electronics_store" )
    vn.na(fmt.f(_([[While walking through a store selling electronics, you notice a pair of glasses with integrated displays. You could use them to display the text you are supposed to recite. They are rather expensive though, {credits}.]]), {credits=fmt.credits(glasses_price)}))
    vn.func( function ()
        if player.credits() < glasses_price then
            vn.jump("glasses_too_expensive")
        else
            vn.jump("glasses_for_sale")
        end
    end )
    vn.label( "glasses_too_expensive" )
    if mem.learned_text then
        vn.na(_("Apparently this is too expensive for you. Is there anything else you want to do?"))
    else
        vn.na(_("Apparently this is too expensive for you. Maybe you should start learning your text?"))
    end
    vn.jump( "next_turn" )
    vn.label( "glasses_for_sale" )
    vn.menu( {
        { _("Buy the glasses"), "buy_glasses" },
        { _("Don't buy the glasses"), "next_turn" },
    } )

    vn.label( "buy_glasses" )
    vn.na(_("They will surely come in handy. Is there anything else you want to do?"))
    vn.func( function ()
        player.pay(-glasses_price)
        mem.has_glasses = true
        vn.jump( "next_turn" )
    end )

    vn.label( "time_over" )
    vn.na(_("It's time to go to the institute."))
    vn.func( function ()
        if mem.has_lab_coat then
            mem.text1 = _("Wearing a lab coat, you blend in nicely with your audience.")
            faction.modPlayer("Za'lek", nebu_research.fctmod.znr03_lab_coat)
        else
            mem.text1 = _("It is just now that you realize that you're the only person not wearing a lab coat. This is a nightmare! Well, maybe it would be for a scientist?")
        end
        if mem.learned_text then
            mem.text2 = _([[You start reciting the speech you got from the student. It turned out learning your text was the right choice. Some of the occurring terms are difficult to pronounce correctly.]])
            faction.modPlayer("Za'lek", nebu_research.fctmod.znr03_learned_speech)
        elseif mem.has_glasses then
            mem.text2 = _([[You start reciting the speech you got from the student. With the text being displayed on your glasses you don't have to look down on a paper to read the text, but some of the occurring terms are difficult to pronounce correctly.]])
            faction.modPlayer("Za'lek", nebu_research.fctmod.znr03_glasses)
        else
            mem.text2 = _([[You start reciting the speech you got from the student. You realize that you should have learned the text as some of the occurring terms are difficult to pronounce correctly.]])
        end
    end )
    vn.na(_([[You make your way to the institute where you are supposed to give the talk, following the signs. On arriving you ask around where you should head to for your talk. Apparently you were already awaited. You are led to a rather small seminar room. Finally, you are ready to give your talk and a few scientists take seats. Actually you expected more than 17 listeners. Maybe this talk is not such a big deal as you expected. They wouldn't just send you to an important presentation, right?]]))
    vn.na(function() return mem.text1 end)
    vn.na(function() return mem.text2 end)
    vn.na(_([[You finish with your text and sigh in relief. After a brief applause, someone raises his hand. Apparently, he wants to ask a question. What should you do?
You haven't understood the question, not even a single word.]]))
    vn.menu( {
        { _("This is a good question."), "avoid_question" },
        { _("It's an open question."), "avoid_question" },
        { _("Run!"), "run" },
    } )

    vn.label( "avoid_question" )
    vn.na(_([[You try to avoid the question. The scientist seems to be upset. There are no further questions. Apparently, there is another talk that starts very soon so people hurry off. You manage to leave without gaining much attention and return to your ship. Enough science for today!]]))
    vn.func( function ()
        faction.modPlayer("Za'lek", nebu_research.fctmod.znr03_questions)
    end )
    vn.done()

    vn.label( "run" )
    vn.na(_([[You decide to run towards the closest door and leave the building as fast as possible. You continue running until you reach the spaceport and enter your ship. You can't be the first one running from a talk, right?]]))
    vn.done()
    vn.run()

    nebu_research.log(fmt.f(_([[You gave a scientific talk on {pnt}. Did anyone notice you're not a scientist?]]), {pnt=dest_planet}))
    misn.finish(true)
end

-- Date hook
function tick()
    local osd_msg = {}
    if mem.timelimit <= time.get() then
        -- Case missed second deadline
        player.msg(_("You were too late. You're never going to be a great scientist!"))
        misn.finish(false)
    else
        osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system before {time_limit}\n({time} remaining)"),
	   {pnt=dest_planet, sys=dest_sys, time_limit=mem.timelimit, time=(mem.timelimit - time.get())})
        misn.osdCreate(_("The Substitute Speaker"), osd_msg)
    end
end
