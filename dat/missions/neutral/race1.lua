--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Racing Skills 1">
 <unique />
 <priority>3</priority>
 <!--
 <cond>player.pilot():ship():class() == "Yacht" and spob.cur():class() ~= "1" and spob.cur():class() ~= "2" and spob.cur():class() ~= "3" and system.cur():presences()["Independent"] ~= nil and system.cur():presences()["Independent"] &gt; 0</cond>
 <chance>10</chance>
 <location>Bar</location>
 -->
 <chance>0</chance>
 <location>None</location>
</mission>
--]]
--[[
--
-- MISSION: Racing Skills 1
-- DESCRIPTION: A person asks you to join a race, where you fly to various checkpoints and board them before landing back at the starting planet
--
--]]

local fmt = require "format"

local checkpoint, racers, target -- Non-persistent state

local chatter = {}
chatter[1] = _("Let's do this!")
chatter[2] = _("Wooo!")
chatter[3] = _("Time to Shake 'n Bake")
target = {1,1,1,1}

function create ()
   mem.this_planet, mem.this_system = spob.cur()
   local missys = {mem.this_system}
   if not misn.claim(missys) then
      misn.finish(false)
   end
   mem.curplanet = spob.cur()
   misn.setNPC(_("A laid back person"), "neutral/unique/laidback.webp", _("You see a laid back person, who appears to be one of the locals, looking around the bar."))
   mem.credits = rnd.rnd(20e3, 100e3)
end


function accept ()
   if tk.yesno(_("Looking for a 4th"), fmt.f(_([["Hiya there! We're having a race around this system soon and need a 4th person to participate. You have to bring a Yacht class ship, and there's a prize of {credits} if you win. Interested?"]]), {credits=fmt.credits(mem.credits)})) then
      misn.accept()
      misn.setDesc(_("You're participating in a race!"))
      misn.setReward(mem.credits)
      misn.osdCreate(_("Racing Skills 1"), {
         _("Board checkpoint 1"),
         _("Board checkpoint 2"),
         _("Board checkpoint 3"),
         fmt.f(_("Land at {pnt}"), {pnt=mem.curplanet}),
      })
      tk.msg(_("Awesome"), fmt.f(_([["That's great! Here's how it works: We will all be in Yacht class ships. Once we take off from {pnt}, there will be a countdown, and then we will proceed to the various checkpoints in order, boarding them before going to the next checkpoint. After the last checkpoint has been boarded, head back to {pnt} and land. Let's have some fun!"]]), {pnt=mem.curplanet}))
      hook.takeoff("takeoff")
   else
      tk.msg(_("Refusal"), _([["I guess we'll need to find another pilot."]]))
   end
end


function takeoff()
   if player.pilot():ship():class() ~= "Yacht" then
      tk.msg(_("Illegal ship!"), _([["You have switched to a ship that's not allowed in this race. Mission failed."]]))
      abort()
   end
   misn.osdActive(1)
   checkpoint = {}
   racers = {}
   pilot.toggleSpawn(false)
   pilot.clear()
   local location1 = vec2.newP(rnd.rnd() * system.cur():radius(), rnd.angle())
   local location2 = vec2.newP(rnd.rnd() * system.cur():radius(), rnd.angle())
   local location3 = vec2.newP(rnd.rnd() * system.cur():radius(), rnd.angle())
   checkpoint[1] = pilot.add("Goddard", "Trader", location1, nil, {ai="stationary"})
   checkpoint[2] = pilot.add("Goddard", "Trader", location2, nil, {ai="stationary"})
   checkpoint[3] = pilot.add("Goddard", "Trader", location3, nil, {ai="stationary"})
   for i, j in ipairs(checkpoint) do
      j:rename(fmt.f(_("Checkpoint {n}"), {n=i}))
      j:setHilight(true)
      j:setInvincible(true)
      j:setActiveBoard(true)
      j:setVisible(true)
   end
   racers[1] = pilot.add("Llama", "Independent", mem.curplanet)
   racers[1]:outfitAdd("Engine Reroute")
   racers[2] = pilot.add("Llama", "Independent", mem.curplanet)
   racers[2]:outfitAdd("Engine Reroute")
   racers[3] = pilot.add("Llama", "Independent", mem.curplanet)
   racers[3]:outfitAdd("Improved Stabilizer")
   for i, j in ipairs(racers) do
      j:rename(fmt.f(_("Racer {n}"), {n=i}))
      j:setHilight(true)
      j:setInvincible(true)
      j:setVisible(true)
      j:control()
      j:face(checkpoint[1]:pos(), true)
      j:broadcast(chatter[i])
   end
   player.pilot():control()
   player.pilot():face(checkpoint[1]:pos(), true)
   mem.countdown = 5 -- seconds
   mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)
   mem.counting = true
   mem.counterhook = hook.timer(1.0, "counter")
   hook.board("board")
   hook.jumpin("jumpin")
   hook.land("land")
end


function counter()
   mem.countdown = mem.countdown - 1
   if mem.countdown == 0 then
      player.omsgChange(mem.omsg, _("Go!"), 1000)
      hook.timer(1.0, "stopcount")
      player.pilot():control(false)
      mem.counting = false
      hook.rm(mem.counterhook)
      for i, j in ipairs(racers) do
         j:control()
         j:moveto(checkpoint[target[i]]:pos())
         hook.pilot(j, "land", "racerland")
      end
      mem.hp1 = hook.pilot(racers[1], "idle", "racer1idle")
      mem.hp2 = hook.pilot(racers[2], "idle", "racer2idle")
      mem.hp3 = hook.pilot(racers[3], "idle", "racer3idle")
   else
      player.omsgChange(mem.omsg, tostring(mem.countdown), 0)
      mem.counterhook = hook.timer(1.0, "counter")
   end
end


function racer1idle(p)
   player.msg( fmt.f( _("{plt} just reached checkpoint {n}"), {plt=p, n=target[1]}) )
   p:broadcast( fmt.f(_("Checkpoint {n} baby!"), {n=target[1]}) )
   target[1] = target[1] + 1
   hook.timer(2.0, "nexttarget1")
end


function nexttarget1()
   if target[1] == 4 then
      racers[1]:land(mem.curplanet)
      hook.rm(mem.hp1)
   else
      racers[1]:moveto(checkpoint[target[1]]:pos())
   end
end


function racer2idle(p)
   player.msg( fmt.f( _("{plt} just reached checkpoint {n}"), {plt=p, n=target[2]}) )
   p:broadcast(_("Hooyah"))
   target[2] = target[2] + 1
   hook.timer(2.0, "nexttarget2")
end


function nexttarget2()
   if target[2] == 4 then
      racers[2]:land(mem.curplanet)
      hook.rm(mem.hp2)
   else
      racers[2]:moveto(checkpoint[target[2]]:pos())
   end
end


function racer3idle(p)
   player.msg( fmt.f( _("{plt} just reached checkpoint {n}"), {plt=p, n=target[3]}) )
   p:broadcast(_("Next!"))
   target[3] = target[3] + 1
   hook.timer(2.0, "nexttarget3")
end


function nexttarget3()
   if target[3] == 4 then
      racers[3]:land(mem.curplanet)
      hook.rm(mem.hp3)
   else
      racers[3]:moveto(checkpoint[target[3]]:pos())
   end
end


function stopcount()
   player.omsgRm(mem.omsg)
end


function board( ship )
   for i,j in ipairs(checkpoint) do
      if ship == j and target[4] == i then
         player.msg( fmt.f( _("{plt} just reached checkpoint {n}"), {plt=player.name(), n=target[4]}) )
         misn.osdActive(i+1)
         target[4] = target[4] + 1
         if target[4] == 4 then
            tk.msg(fmt.f(_("Checkpoint {n} reached"), {n=i}), fmt.f(_("Land on {pnt}"), {pnt=mem.curplanet}))
         else
            tk.msg(fmt.f(_("Checkpoint {n} reached"), {n=i}), fmt.f(_("Proceed to Checkpoint {n}"), {n=i+1}))
         end
         break
      end
   end
   player.unboard()
end


function jumpin()
   tk.msg(_("You left the race!"), _([["Because you left the race, you have been disqualified."]]))
   abort()
end


function racerland(p)
   player.msg(fmt.f(_("{plt} just landed at {pnt} and finished the race"), {plt=p, pnt=mem.curplanet}))
end


function land()
   if target[4] == 4 then
      if racers[1]:exists() and racers[2]:exists() and racers[3]:exists() then
         tk.msg(_("You Won!"), _([[The laid back person comes up to you and hands you a credit chip.
   "Nice racing! Here's your prize money. Let's race again sometime soon!"]]))
         player.pay(mem.credits)
         misn.finish(true)
      else
         tk.msg(_("You failed to win the race."), _([[As you congratulate the winner on a great race, the laid back person comes up to you.
   "That was a lot of fun! If you ever have time, let's race again. Maybe you'll win next time!"]]))
         abort()

      end
   else
      tk.msg(_("You left the race!"), _([["Because you left the race, you have been disqualified."]]))
      abort()
   end
end


function abort ()
   misn.finish(false)
end
