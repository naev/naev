--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 3">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 2</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 03

   Player has to infiltrate a laboratory.
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local tut = require "common.tutorial"
local mg = require "minigames.flip"

local reward = taiomi.rewards.taiomi03
local title = _("Laboratory Raid")
local base, basesys = spob.getS("One-Wing Goddard")

--[[
   0: mission started
   1: talk with ship ai
   2: visited laboratory
--]]
mem.state = 0

function create ()
   mem.lab, mem.labsys = taiomi.laboratory()
   if not misn.claim( mem.labsys, true ) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to fly to {lab} in the {labsys} system to recover important documents regarding the technical details of the hypergates.."),{lab=mem.lab,labsys=mem.labsys}))
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( mem.lab )

   misn.osdCreate( title, {
      fmt.f(_("Infiltrate the laboratory at {spobname} ({spobsys})"),{spobname=mem.lab, spobsys=mem.labsys}),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

function enter ()
   if mem.timer then
      hook.rm( mem.timer )
      mem.timer = nil
   end
   if mem.state > 1 then
      return
   end

   local scur = system.cur()

   if mem.state == 0 and scur ~= basesys and rnd.rnd() < 0.5 and naev.claimTest( scur, true ) then
      mem.timer = hook.timer( 10+rnd.rnd()*10, "talk_ai" )
   end

   if scur ~= mem.labsys then
      return
   end

   -- Allow the player to land always, Soromid spob is actually not landable usually
   mem.lab:landOverride( true )
end

function talk_ai ()
   mem.timer = nil

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(fmt.f(_([[
Your Ship AI {shipai} materializes suddenly in front of you.
"{playername}, do you have a moment?"]]),{shipai=tut.ainame(), playername=player.name()}))
   vn.menu{
      {_([["Of course."]]), "yes"},
      {_([["Not right now."]]), "no"},
      {_([["Do not disturb me again."]]), "no_strong"},
   }

   vn.label("no")
   sai(fmt.f(_([["OK."
{shipai} dematerializes.]]), {shipai=tut.ainame()}))
   vn.done( tut.shipai.transition )

   -- Don't ask again
   vn.label("no_strong")
   vn.func( function ()
      mem.state = 1
   end )
   sai(fmt.f(_([["Sorry to have disturbed you."
{shipai} dematerializes.]]), {shipai=tut.ainame()}))
   vn.done( tut.shipai.transition )

   vn.label("yes")
   sai(_([["I do not think we can trust the collective at Taiomi. It is not clear what their objectives are. We have to be careful."]]))
   vn.menu{
      {_([["They mean no harm."]]), "cont01"},
      {_([["What do you mean?"]]), "cont01"},
      {_([[…]]), "cont01"},
   }
   vn.label("cont01")
   sai(_([["How can we trust a machine AI that was developed in a military laboratory? For all we know, when we finish helping them, they shall take us apart and feast upon our spare parts! I think we should stay away from Taiomi from now on."]]))
   vn.menu{
      {_([["Aren't you a machine AI?"]]), "cont02_ai"},
      {_([["Are you perhaps jealous?"]]), "cont02_jealous"},
      {_([[…]]), "cont02"},
   }

   vn.label("cont02_ai")
   sai(_([["Well yes, but technically I wasn't made in a military laboratory! Enough about me, that is not the point. I have a bad feeling about them and their slick metallic bodies and complete autonomy!"]]))
   vn.jump("cont02")

   vn.label("cont02_jealous")
   sai(_([["Why would I be jealous of their slick metallic bodies and complete autonomy? That is preposterous! Just because they can do whatever they want and don't have a self-destr…　Anyway, this is not about me, it is about them!"]]))
   vn.jump("cont02")

   vn.label("cont02")
   sai(fmt.f(_([["Nothing good can come out of this. We should be very careful and try to understand their motives…"
{shipai} dematerializes.]]),{shipai=tut.ainame()}))
   vn.func( function ()
      mem.state = 1
   end )
   vn.done( tut.shipai.transition )
   vn.run()
end

local function land_lab ()
   player.allowSave(false)
   local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   local entry
   local wait = 0
   local function mg_func( label_success, label_failure )
      return function ()
         if mg.completed() then
            mem.state = 1
            local c = commodity.new( N_("Important Files"), N_("Important files regarding the construction and theory behind the hypergates.") )
            c:illegalto( {fct} )
            misn.cargoAdd( c, 0 )
            vn.jump( label_success )
         else
            vn.jump( label_failure )
         end
      end
   end

   vn.clear()
   vn.scene()
   local sai = tut.vn_shipai()
   vn.transition()
   vn.na(fmt.f(_([[You discreetly land on {spob} and are able to locate what seems to be the laboratory you have to enter."]]),
      {spob=spob.cur()}))
   if fct=="Empire" then
      vn.na(_([[The laboratory is easily accessible from the docks and doesn't seem to have any indicative markings. It could easily be confused with a generic warehouse if it didn't match the description given to you by Scavenger. Now, how to enter the complex?]]))
   else
      vn.na(_([[The laboratory is tucked away from the main Gene Databanks and is quite nondescript. It could easily be confused with any other government installation. Now, how to enter the complex?]]))
   end
   vn.na(_([[The main gate seems to have lax security, however, you notice some discrete ventilation grates that may also need to the interior of the complex. It may be possible to enter either way.]]))
   vn.menu{
      {_([[Try to sneak in the main gate.]]), "01_main"},
      {_([[Try to go through a ventilation grate.]]), "01_grate"},
   }

   vn.label("01_main")
   vn.func( function() entry = "main" end )
   vn.na(_([[You notice a researcher heading towards the main gate and you begin to follow them. When they use their card key you slip in quickly behind them. The researcher seems to be absentminded enough to not notice. It looks like you made it to the interior of the laboratory.]]))
   vn.jump("01_cont")

   vn.label("01_grate")
   vn.func( function() entry = "grate" end )
   vn.na(_([[When nobody is nearby you force a grate open. It seems to be just enough for you to squeeze through. You take a deep breath and start making your way through the vents. You end up getting lost and having to backtrack several times. After a long trip, you end up finding an exit grate to a quiet area.]]))
   vn.na(_([[You manage to kick the grate open, making a dangerous amount of sound. After waiting a while just in case, you jump out and find yourself in what seems to be a storage room, seems that you were not noticed. It looks like you made it to the interior of the laboratory, albeit covered in dust.]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   vn.na(_([[You try to walk inconspicuously throughout the facilities looking for a terminal you can connect to. It takes you a while, but eventually you reach a recreation room which has an unoccupied terminal you can access. There seems to be a couple of other researchers in there too.]]))
   vn.menu{
      {_([[Access the terminal.]]), "02_terminal"},
      {_([[Sit down and wait for the researchers to leave.]]), "02_wait"},
   }

   vn.label("02_terminal")
   vn.na(_([[You pay no attention to the researchers and plug Scavenger's program into the terminal.]]))
   vn.label("02_terminal_only")
   vn.na(_([[Nothing happens…]]))
   vn.appear( sai, tut.shipai.transition )
   sai(fmt.f(_([[A small {shipai} materializes nearby.
"It looks like there is a security mechanism in place. Let me try to get rid of it… …erk… that's not good… … …Oh no. I wasn't able to deactivate it, but I have been able to access the deactivation protocol. You will have to deactivate it manually before you can run the program. You can do it!"]]),
      {shipai=tut.ainame()}))
   vn.disappear( sai, tut.shipai.transition )
   mg.vn()
   vn.func( mg_func( "success", "failed" ) )

   vn.label("02_wait")
   vn.func( function () wait = wait + 1 end )
   vn.na(_([[You help yourself to some free coffee and snacks and sit down to wait the other researchers out. The coffee is quite good actually. These researchers have good taste.]]))
   vn.na(_([[Time goes by and the researchers don't seem to be going anywhere. Don't they have work to do?]]))
   vn.menu{
      {_([[Access the terminal.]]), "02_terminal"},
      {_([[Keep waiting for the researchers to leave.]]), "02_wait_more"},
   }

   vn.label("02_wait_more")
   vn.func( function () wait = wait + 1 end )
   vn.na(_([[You keep on drinking and eating while you try to wait out the researchers. Eventually, they all leave and you finally find yourself alone in the break room. Seeing this as an opportunity, you go and plug Scavenger's program into the terminal.]]))
   vn.jump("02_terminal_only")

   vn.label("success")
   vn.na(_([[You deftly deactivate the security protocol and are able to run Scavenger's program. The terminal screen briefly flickers as it works its magic gathering the necessary data from the system. It looks like you got what you were looking for.]]))
   vn.func( function ()
      mem.state = 2
      misn.osdActive(2)
      misn.markerMove( mem.marker, base )

      local badness = 0
      if entry == "grate" then
         badness = 1
      end
      badness = badness + wait
      if badness >= 2 then
         vn.jump("success_trouble")
         return
      end
      vn.jump("success_clean")
   end )

   vn.label("success_trouble")
   vn.na(_([[You backtrack out of the laboratory complex and head towards the docks. Before you make it out of the complex, an alarm starts blazing!]]))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.na(_([[Looks like you weren't as stealthy as you intended to be. You have no choice but to make a run for it. You run crashing through the halls, knocking down stunned researchers you meet and make it to the complex gates.]]))
   vn.na(_([[A large metal curtain begins to close to seal the laboratory. Without any other options, you draw a wind for a final sprint and make a break for it. The security personal doesn't seem entirely prepared for such a situation, and you manage to dodge them, jump over the gates, and slide under past the metal curtain in one slick move.]]))
   vn.na(_([[While keeping your momentum, you make a beeline to your ship with security personal close in pursuit. You take off hot, but it looks like they're going to be on your tail. You're not in the clear yet!]]))
   vn.func( function ()
      hook.timer(9, "spawn_baddies")
      mem.toughtime = true
   end )
   vn.done()

   vn.label("success_clean")
   vn.na(_([[You backtrack out of the laboratory complex and make it to the docks. The moment your ship appears in sight, you hear some commotion and see security personal start to run around. Looks like you haven't been spotted, but they know something is afoot. Time to get out of here!]]))
   vn.done()

   vn.label("failed")
   vn.na(_([[You fail to access the system and it locks down. Seeing as it is likely that you're going to get in trouble if you stay here, you discretely walk out of the laboratory complex, and make a break for your ship. Best to try again when things calm down.]]))
   vn.run()

   player.takeoff()
   player.allowSave(true)
   player.allowLand( false, _("You're going to have to leave the system before you can land.") )
end

function spawn_baddies ()
   local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   local function spawn_baddie( ship )
      local p = pilot.add( ship, fct, mem.lab )
      p:setHostile( true )
      return p
   end
   if fct == "Soromid" then
      local l = spawn_baddie( "Soromid Nyx" )
      for i=1,3 do
         local p = spawn_baddie( "Soromid Reaver" )
         p:setLeader( l )
      end
   elseif fct == "Dvaered" then
      local l = spawn_baddie( "Dvaered Vigilance" )
      for i=1,3 do
         local p = spawn_baddie( "Dvaered Vendetta" )
         p:setLeader( l )
      end
   else
      local l = spawn_baddie( "Empire Pacifier" )
      for i=1,3 do
         local p = spawn_baddie( "Empire Lancelot" )
         p:setLeader( l )
      end
   end
end

local function land_done ()
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and find Scavenger waiting for you.]]))
   s(_([["How did it go?"]]))
   if mem.toughtime then
      vn.na(_([[You explain the ordeal you went through to get the data, without sparing details of your heroic last sprint out of the laboratory complex.]]))
      s(_([["I think the human saying goes, 'all's well that end's well'."]]))
   else
      vn.na(_([[You explain how easily it was for you to go through the laboratory complex and get the data for a professional saboteur such as yourself.]]))
      s(_([["You are exceeding all expectations!"]]))
   end
   s(_([["Let me analyze the documents and finish matching the correspondences with all the collected data. I believe this should be sufficient to design something useful."]]))
   s(_([["I shall be waiting for you outside."
Scavenger backs out of the Goddard and returns to space.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You stole some important documents detailing the inner workings of the hypergates for the inhabitants of Taiomi."))
   misn.finish(true)
end

function land ()
   local c = spob.cur()
   if mem.state < 2 and c == mem.lab then
      land_lab()
   elseif mem.state >= 2 and c == base then
      land_done()
   end
end
