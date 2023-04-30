--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Racing Skills 2">
 <priority>3</priority>
 <cond>player.pilot():ship():class() == "Yacht" and spob.cur():class() ~= "1" and spob.cur():class() ~= "2" and spob.cur():class() ~= "3" and system.cur():presences()["Independent"] ~= nil and system.cur():presences()["Independent"] &gt; 0</cond>
 <done>Racing Skills 1</done>
 <!--
 <chance>20</chance>
 <location>Bar</location>
 -->
 <chance>0</chance>
 <location>None</location>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   --
   -- MISSION: Racing Skills 2
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
   misn.setNPC(_("A laid back person"), "neutral/unique/laidback.webp", _("You see a laid back person, who appears to be one of the locals, looking around the bar, apparently in search of a suitable pilot."))
   mem.credits_easy = rnd.rnd(20e3, 100e3)
   mem.credits_hard = rnd.rnd(200e3, 300e3)
end


function accept ()
   if tk.yesno(_("Another race"), _([["Hey there, great to see you back! You want to have another race?"]])) then
      misn.accept()
      misn.setDesc(_("You're participating in another race!"))
      misn.osdCreate(_("Racing Skills 2"), {
         _("Board checkpoint 1"),
         _("Board checkpoint 2"),
         _("Board checkpoint 3"),
         fmt.f(_("Land at {pnt}"), {pnt=mem.curplanet}),
      })
      local s = fmt.f(_([["There are two races you can participate in: an easy one, which is like the first race we had, or a hard one, with smaller checkpoints and no afterburners allowed. The easy one has a prize of {credits1}, and the hard one has a prize of {credits2}. Which one do you want to do?"]]), {credits1=fmt.credits(mem.credits_easy), credits2=fmt.credits(mem.credits_hard)})
      mem.choice, mem.choicetext = tk.choice(_("Choose difficulty"), s, _("Easy"), _("Hard"))
      if mem.choice == 1 then
         mem.credits = mem.credits_easy
         tk.msg(_("Easy Mode "), _([["Let's go have some fun then!"]]))
      else
         mem.credits = mem.credits_hard
         tk.msg(_("Hard Mode"), _([["You want a challenge huh? Remember, no afterburners on your ship or you will not be allowed to race. Let's go have some fun!"]]))
      end
      misn.setReward(mem.credits)
      hook.takeoff("takeoff")
      else
      tk.msg(_("Refusal"), _([["I guess we'll need to find another pilot."]]))
   end
end

function takeoff()
   if player.pilot():ship():class() ~= "Yacht" then
      tk.msg(_("Illegal ship!"), _([["You have switched to a ship that's not allowed in this race. Mission failed."]]))
      misn.finish(false)
   end
   if mem.choice ~= 1 then
      for k,v in ipairs(player.pilot():outfitsList()) do
         if v:type() == "Afterburner" then
            tk.msg(_("Illegal ship!"), _([["You have outfits on your ship which is not allowed in this race in hard mode. Mission failed."]]))
            misn.finish(false)
         end
      end
   end
   misn.osdActive(1)
   checkpoint = {}
   racers = {}
   pilot.toggleSpawn(false)
   pilot.clear()
   local srad = system.cur():radius()
   local location1 = vec2.newP( srad * rnd.rnd(), rnd.angle() )
   local location2 = vec2.newP( srad * rnd.rnd(), rnd.angle() )
   local location3 = vec2.newP( srad * rnd.rnd(), rnd.angle() )
   if mem.choice == 1 then
      mem.shiptype = "Goddard"
   else
      mem.shiptype = "Koala"
   end
   checkpoint[1] = pilot.add(mem.shiptype, "Trader", location1, nil, {ai="stationary"})
   checkpoint[2] = pilot.add(mem.shiptype, "Trader", location2, nil, {ai="stationary"})
   checkpoint[3] = pilot.add(mem.shiptype, "Trader", location3, nil, {ai="stationary"})
   for i, j in ipairs(checkpoint) do
      j:rename(fmt.f(_("Checkpoint {n}"), {n=i}))
      j:setHilight(true)
      j:setInvincible(true)
      j:setActiveBoard(true)
      j:setVisible(true)
   end
   racers[1] = pilot.add("Llama", "Independent", mem.curplanet)
   racers[2] = pilot.add("Gawain", "Independent", mem.curplanet)
   racers[3] = pilot.add("Llama", "Independent", mem.curplanet)
   if mem.choice == 1 then
      racers[1]:outfitAdd("Engine Reroute")
      racers[2]:outfitAdd("Engine Reroute")
      racers[3]:outfitAdd("Improved Stabilizer")
   else
      for i in pairs(racers) do
         racers[i]:outfitRm("all")
         racers[i]:outfitRm("cores")

         racers[i]:outfitAdd("Unicorp PT-16 Core System")
         racers[i]:outfitAdd("Unicorp D-2 Light Plating")
         local en_choices = {
            "Nexus Dart 150 Engine", "Tricon Zephyr Engine" }
         racers[i]:outfitAdd(en_choices[rnd.rnd(1, #en_choices)])
      end
   end
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
      player.msg(_("This race is sponsored by Melendez Corporation. Problem-free ships for problem-free voyages!"))
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
            tk.msg(fmt.f(_("Checkpoint {n} reached"), {n=i}), fmt.f(_("Proceed to land at {pnt}"), {pnt=mem.curplanet}))
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
   misn.finish(false)
end

function racerland(p)
   player.msg(fmt.f(_("{plt} just landed at {pnt} and finished the race"), {plt=p, pnt=mem.curplanet}))
end

function land()
   if target[4] == 4 then
      if racers[1]:exists() and racers[2]:exists() and racers[3]:exists() then
         if mem.choice==2 and player.outfitNum("Racing Trophy") <= 0 then
            tk.msg(_("You Won!"), fmt.f(_([[A man in a suit and tie takes you up onto a stage. A large name tag on his jacket says 'Melendez Corporation'. "Congratulations on your win," he says, shaking your hand, "that was a great race. On behalf of Melendez Corporation, I would like to present to you your trophy and prize money of {credits}!" He hands you one of those fake oversized cheques for the audience, and then a credit chip with the actual prize money on it. At least the trophy looks cool.]]), {credits=fmt.credits(mem.credits)}))
            player.outfitAdd("Racing Trophy")
         else
            tk.msg(_("You Won!"), fmt.f(_([[A man in a suit and tie takes you up onto a stage. A large name tag on his jacket says 'Melendez Corporation'. "Congratulations on your win," he says, shaking your hand, "that was a great race. On behalf of Melendez Corporation, I would like to present to you your prize money of {credits}!" He hands you one of those fake oversized cheques for the audience, and then a credit chip with the actual prize money on it.]]), {credits=fmt.credits(mem.credits)}))
         end
         player.pay(mem.credits)
         misn.finish(true)
      else
         tk.msg(_("You failed to win the race."), _([[As you congratulate the winner on a great race, the laid back person comes up to you.
   "That was a lot of fun! If you ever have time, let's race again. Maybe you'll win next time!"]]))
         misn.finish(false)
      end
   else
      tk.msg(_("You left the race!"), _([["Because you left the race, you have been disqualified."]]))
      misn.finish(false)
   end
end
