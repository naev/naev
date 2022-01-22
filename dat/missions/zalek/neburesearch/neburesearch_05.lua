--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Expert Nebula Research">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>Shielding Prototype Funding</done>
  <chance>100</chance>
  <location>Bar</location>
  <spob>Jorla</spob>
 </avail>
 <notes>
  <campaign>Nebula Research</campaign>
 </notes>
</mission>
--]]
--[[
   
   Mission: Expert Nebula Research
   
   Description: The nebula shielding prototype is ready and the player has to test it. The first stages go well, but in the final stage the prototype fails and the player is about to die in the Sol nebula. He gets rescued by a Thurion carrier instead and is bought into Thurion space.
   
   Difficulty: Easy (requires at least a corvette class ship)

--]]

local fmt = require "format"
local nebu_research = require "common.nebu_research"

local mensing_portrait = nebu_research.mensing.portrait

-- luacheck: globals (Hook functions passed by name)

-- Mission constants
local credits = nebu_research.rewards.credits05
local homeworld, homeworld_sys = spob.getS("Jorla")
local dest_planet, dest_sys = spob.getS("Cerberus")
local testing_sys = system.get("Nasona")
local sol_sys = system.get("Sol")

function create()
    -- Spaceport bar stuff
    misn.setNPC(_("Dr. Mensing"), mensing_portrait, _("She looks happy. Maybe the work on the shielding prototype has finished?"))
end

function accept()
    local accepted = false
    vn.clear()
    vn.scene()
    local mensing = vn.newCharacter( nebu_research.vn_mensing() )
    vn.transition("fade")
    mensing(fmt.f(_([["{player}? Great timing! I need someone insane.. eh.. I meant brave enough to test the nebula resistant shielding prototype in a realistic testing environment. And there is no better place for this tests than the Sol nebula itself!"]], {player=player:name()})))
    mensing(_([["No no, it's absolutely safe! We ran some basic tests on the device in a simulation chamber and tested on a ship in space. It works absolutely fine so far. The next step is to actually test it within the Sol nebula itself. This is your chance to make history! Are you ready?"]]))
    vn.menu( {
        { _("Accept the job"), "accept" },
        { _("Decline to help"), "decline" },
    } )
    vn.label("decline")
    mensing(_([["Too bad. I hope you will change your mind. Otherwise some other pilot will take the chance to make history."]]))
    vn.done()
    vn.label("accept")
    mensing(_([["I'm glad to hear that you're in. First of all - since our motto is "safety first".. Wait, why are you rolling your eyes? Anyway, you'll require at least a corvette class ship or something larger and please equip some additional shield boosters and shield capacitors. Just in case that there will be a problem with the prototype.."]]))
    vn.func( function ()
        if not (shipclass == "Corvette" or shipclass == "Destroyer" or shipclass == "Cruiser" or shipclass == "Battleship" or shipclass == "Carrier" or shipclass == "Armoured Transporter" or shipclass == "Bulk Freighter") then
            mensing(_([["However, since your ship is very small it's not a good idea to send you into the Sol nebula. Please return with a larger ship. I'd say at least a Corvette class ship. It will increase your chances to survive. Just in case, you know? I don't actually expect that something goes wrong."]]))
            vn.done()
        else
            accepted = true
        end
    end )
    mensing(fmt.f(_([["My team will bring the shielding prototype onboard your ship. Don't forget to install it before departing! Your first destination will be {pnt} in the {sys} system. Robert will be waiting there since he demanded to be involved in this project."]]), {pnt=dest_planet, sys=dest_sys}))
    mensing(fmt.f(_([["Please pick him up and head towards the {sys} system, but don't jump into the {sol} system itself! We have no idea what could possibly happen. Also, I wouldn't let any ship go that far without extensive testing of the prototype."]]), {sol=sol_sys}))
    vn.done()
    vn.run()

    if not accepted then
        misn.finish()
        return
    end

    stage = 0
    player.addOutfit("Nebular Shielding Prototype")

    -- Set up mission information
    misn.setTitle(_("Expert Nebula Research"))
    misn.setReward(creditstring(credits))
    misn.setDesc(_("Test the shielding prototype by flying into the Sol nebula."))
    misn_marker = misn.markerAdd(dest_sys, "low")

    misn.accept()
    local osd_title = _("Expert Nebula Research")
    local osd_msg   = {}
    osd_msg[1] = fmt.f(_("Land on {pnt} in the {sys} system."), {pnt=dest_planet:name(), sys=dest_sys:name()})
    osd_msg[2] = fmt.f(_("Fly to the {sys} system and carry out the testing procedure."), {sys=testing_sys:name()})
    osd_msg[3] = fmt.f(_("Return to {pnt} in the {sys} system."), {pnt=homeworld:name(), sys=homeworld_sys:name()})
    misn.osdCreate(osd_title, osd_msg)

    hook.land("land")
    hook.jumpin("jumpin")
    hook.takeoff("takeoff")
end

function land()
    if stage == 0 and spob.cur() == dest_planet then
        vn.clear()
        vn.scene()
        local student = vn.newCharacter( nebu_research.vn_student() )
        vn.transition("fade")
        student(_([[Shortly after landing you are greeted by the student.
"That took long enough! Being stuck in a place like this.. It's always us students who have to do all the hard work!]]))
        student(_([["Anyway, I'm done here with my measurements. Since the shielding prototype requires fine tuning of some parameters these measurements are essential. I just need a few minutes to configure the device. We can start anytime soon."]]))
        if not hasShieldingPrototypeEquiped() then
            student(_([["You did remember to install the shielding prototype, right?"]]))
        end
        vn.done()
        vn.run()
        misn.osdActive(2)
        stage = 1
    elseif stage == 5 and spob.cur() == homeworld then
        vn.clear()
        vn.scene()
        local student = vn.newCharacter( nebu_research.vn_student() )
        vn.transition("fade")
        student(_([["I'm glad we're back and still in one piece. We were lucky to be saved like this and I don't like depending on luck. I still have no idea what went wrong but I'll continue investigating. Even though the data I collected makes just no sense to me."]]))
        local mensing = vn.newCharacter( nebu_research.vn_mensing() )
        mensing(_([[Dr. Mensing rushes towards you.
"Are you fine? I heard there were complications. What happened?"]]))
        student(_([["We're fine but it was too close. The prototype suddenly stopped working for no apparent reason as far as I can tell. Everything appeared to be fine!"]]))
        mensing(_([["In that case the principles that our design works based on may be flawed. I'm sorry, it's all my fault. I should have been more careful when checking Robert's calculations."]]))
        student(_([["What, now it's my fault?! After you tried to steal all my ideas?"]]))
        vn.menu( {
            { _("Clear your throat."), "continue" },
            { _("Sigh."), "continue" },
            { _("Ask about the payment."), "continue" },
        } )
        vn.label("continue")
        mensing(fmt.f(_([["Ah right, let's postpone the scientific issues for a moment. You have earned a reward for all the trouble."
With this she hands you a credit chip worth {credits}.]]), {credits=fmt.credits(credits)}))
        vn.na(_("The both of them continue arguing shortly afterwards. You decide it is better to leave now."))
        vn.done()
        vn.run()

        player.outfitAdd("Broken Nebular Shielding Prototype")
        player.pay(credits)
        misn.markerRm(mem.misn_marker)
        nebu_research.log( _("You helped Dr. Mensing to test the nebula resistant shielding prototype by flying deep into the Sol nebula. However, the prototype failed and you almost died due to the volatility of the nebula. It is a major setback for the project.") )
        misn.finish(true)
    end
end

function takeoff()
    if stage == 1 then
        vn.clear()
        vn.scene()
        local student = vn.newCharacter( nebu_research.vn_student() )
        vn.transition("fade")
        vn.na(_("The last time you were out here with the student you almost died due to his tinkering with your ship's electronics and now you're doing something even more dangerous. Was it really a good idea to accept this mission?"))
        student(fmt.f(_([["Alright, let's go to the {sys} system. I'm going to monitor the prototype to make sure everything is right."]]), {sys=testing_sys:name()}))
        vn.done()
        vn.run()
        stage = 2
    end
end

function jumpin()
    if stage == 2 and system.cur() == testing_sys then
        vn.clear()
        vn.scene()
        local student = vn.newCharacter( nebu_research.vn_student() )
        vn.transition("fade")
        stage = 3
        point_pos = vec2.new(0,0)
        mark = system.mrkAdd( _("Checkpoint"), point_pos )
        vn.na(fmt.f(_("You arrived in the {sys} system and the student signals you that everything works as expected."), {sys=testing_sys:name()}))
        student(fmt.f(_([["To proceed, please fly towards the checkpoint that I just marked on your map. Afterwards we'll return to the {sys} system."]]), {sys=dest_sys:name()}))
        vn.done()
        vn.run()
        osd_msg[2] = fmt.f(_("Fly to the checkpoint in the {sys} system."), {sys=testing_sys:name()})
    end
end

function hasShieldingPrototypeEquiped()
    local o = player.pilot():outfits("utility")
    for i=1,#o do
        if o[i]:nameRaw()=="Nebular Shielding Prototype" then
            return true
        end
    end
    return false
end

function timer()
    if stage == 3 and player_pos:dist( point_pos ) < 500 then
        stage = 4
        misn.markerRm( marker )
        osd_msg[2] = _("Wait...")
        beginScan()
    end
end

function beginScan()
    ghost = pilot.add("Thurion Ingenuity", "Thurion", point_pos, _("Ghost Ship"))[1]
    ghost:setInvincible()
    ghost:control()
--    ghost:follow(player.pilot())
    moveto()
    hook.timer(5000, "moveto")
    hook.timer(30000, "endScan")
end

function moveto()
    ghost:goto(ps:pos(), false, false)
    hook.timer(5000, "moveto")
end

function endScan()
    local c = 0
    local first_text = ""
    tk.msg(shadows_title, shadows_text)
    ghost:hyperspace()
    player.cinematics(true)
    player.cinematics(false)
    ps = player.pilot()
    ps:rmOutfit("Nebular Shielding Prototype")
    vn.clear()
    vn.scene()
    vn.transition("fade")
    vn.menu( {
        { fmt.f(_("Retreat to {sys}"), {sys=system.get("Arandon"):name()}), "retreat" },
        { _("Talk to Robert"), "talk" },
    } )
    vn.label("retreat")
    vn.na(_("You set course back to the jump point even though you know that you won't make it back. Your shields will be depleted before arriving."))
    vn.na(_("The rate at which your shields are drained is nearly constant so that you can easily determine how much time is left until your death. Apparently you won't be able to reach the jump point. It's not even close."))
    if ps:nameRaw():sub(1,6) == "Za'lek" then
        vn.na(_("Even with the strong shields your Za'lek capital ship offers the decay of the shield energy is too fast to make it. It must be a side effect of the broken shielding prototype."))
    else
        vn.na(_("Maybe you could make it with one of those Za'lek capital ships. Too bad you don't have one!"))
    end
    vn.na(_([[You ran out of ideas. Meanwhile Robert's only reply was "I'm still working on it." Maybe there is in fact no way to survive this?]]))
    vn.done()
    vn.label("talk")
    local student = vn.newCharacter( nebu_research.vn_student() )
    vn.transition("fade")
    vn.na(_("You walk to the section in the cargo hold in a hurry where a control terminal for the shielding prototype has been installed. On arriving you find Robert hastily typing on a keyboard. He mutters some unfamiliar words to himself that you identify as Za'lek curses even without prior knowledge."))
    vn.menu( {
        { _("Clear your throat"), "clear_throat" },
        { _("Ask what the problem is"), "ask" },
        { _("Lose your nerves"), "lose_nerves" },
    } )
    vn.label("clear_throat")
    vn.label("ask")
    student(_([["Oh, captain we have a few problems.. It's the.. Well, I'm not sure. To be honest, I have no idea what happened. I guess we won't make it. There is no reason to give up though, at least for me, but you may want to start praying or whatever you non-scientists do."]]))
    vn.jump("talk_end")
    vn.label("lose_nerves")
    student(_([["Ah.. Well, the problem is.. uh.. Well, I guess you're right. Please calm down. Panic won't help us right now. At least I will continue to working on the problem. You may want to start praying or whatever you non-scientists do."]]))
    vn.label("talk_end")
    vn.clear()
    vn.transition("fade")
    vn.na(_("You return to the cockpit and set course back to the jump point even though you know that you won't make it back. Your shields will be depleted before arriving."))
    vn.na(_([[Staring out of the window you see the shadows swirling around your ship. They became numerous by now but you do not know how many there are. They just vanish as fast as they appear out of nowhere. "It's no surprise that so many pilots are afraid of those ghost stories" you think, but you have no reason to fear them. You know it is the nebula that will destroy your ship.]]))
    vn.na(_("Eventually all of the shadows disappeared one by one without reappearing elsewhere again. You're afraid that they left you alone to die here. Your shields are almost depleted when suddenly a large dark spot appears in front of your ship, slowly growing in size. You realize that your ship's sensors have recognized the ominous object."))
    vn.done()
    vn.run()

    peacemaker = pilot.add("Empire Peacemaker", nil, ps:pos()+vec2.new(-100,25))[1]
    peacemaker:setDir(180.)
    peacemaker:setFriendly(true)
    peacemaker:follow(ps)
    peacemaker:setActiveBoard(true)
    peacemaker:setVisplayer(true)
    peacemaker:setHilight(true)
    peacemaker:setInvincible(true)
--    peacemaker:hailPlayer()
    hook.pilot(peacemaker, "board", "rescue")
    vn.clear()
    vn.scene()
    vn.na(fmt.f(_("An empire Peacemaker emerged! The ship approaches slowly towards your direction as if they knew exactly your current position. Finally, you receive a message from the Peacemaker. They command you to dock with their ship so that their shields can protect your ship on the way back to the {sys} system."), {sys=dest_sys:name()}))
    vn.done()
    vn.run()
end

function rescue()
    ps = player.pilot()
    player.unboard()
    player.teleport(dest_sys:name())
    peacemaker = pilot.add("Empire Peacemaker", nil, ps:pos())[1]
    peacemaker:hyperspace()
    vn.clear()
    vn.scene()
    local captain = vn.newCharacter( nebu_research.vn_empire_captain() )
    vn.na(_("After docking to the Peacemaker its captain invited you on his ship. It turned out that she is as curious about your rescue as you are. Unfortunately, you are not able to answer any of her questions satisfactorily."))
    vn.na(_("She tells you that her crew received a distress signal and followed it to your position. With the heavy interference of the nebula this should be impossible or at least extremely unlikely."))
    captain(_([["It is nearly impossible that a signal passes through the nebula out here. So either there's something you're hiding or you had incredible luck. And just what were you doing out there? I saw those Za'lek doing even more ridiculous stuff though."]]))
    vn.menu( {
        { _("Ask for the logs related to the distress signal"), "signal" },
        { _("Apologize"), "apologize" },
    } )
    vn.label("signal")
    vn.na(fmt.f(_("You ask the captain for the sensor logs regarding the distress signal. At first she refuses your request. While she ultimately declines to hand you out the logs she allows you to at least take a look on them. You don't see anything remarkable except the time stamp. The distress signal was received shortly after you jumped into the {sys} system!"), {sys=testing_sys:name()}))
    vn.na(_("While that explains how the Empire ship reached you in time it raises several new questions."))
    captain(_([["Seeing anything interesting?"
You decide that it's better not to tell her about your finding.]]))
    vn.jump("end_talk")
    vn.label("apologize")
    captain(_([["Well, it's part of my job to help out civilians. But please don't do anything suicidal during my shift."]]))
    vn.na(_("While she was just joking you can't help but realise how much look you had."))
    vn.label("end_talk")
    captain(_([["You may leave now, civilian. But be aware that we'll keep an eye on you."]]))
    vn.na(fmt.f(_("An officer leads you back onto your ship. Your journey to {sys} is uneventful and you are ordered to depart after arriving."), {sys=dest_sys:name()}))
    vn.done()
    vn.run()

    stage = 5
    misn.markerMove(misn_marker, homeworld_sys)
    misn.osdActive(3)
end


