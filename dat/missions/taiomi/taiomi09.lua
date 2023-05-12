--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 9">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 8</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 09

   Player has to make a deal with a smuggler to bring goods to bastion.
]]--
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"
local taiomi = require "common.taiomi"
local escort = require "escort"
local fleet = require "fleet"
local pilotai = require "pilotai"
local lmisn = require "lmisn"

local reward = taiomi.rewards.taiomi09
local title = _("Smuggler's Deal")
local base, basesys = spob.getS("One-Wing Goddard")
local smugden, smugsys = spob.getS("Darkshed")
local startspob, startsys = spob.getS("Arrakis")
local fightsys = system.get("Gamel")
local handoffsys = system.get("Bastion")
local handoffpos = vec2.new( 8e3, 3e3 )

--[[
   0: mission started
   1: made deal with smuggler
   2: escort from xxx to bastion
   3: fight done
   4: cutscene done
--]]
mem.state = 0

function create ()
   if not misn.claim{ startsys, fightsys, handoffsys } then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   -- Store if the player knows the station
   mem.smugden_known = smugden:known()

   misn.accept()

   -- Mission details
   misn.setTitle( title )

   misn.setDesc(fmt.f(_("You have been tasked to contact smugglers at {smugden} ({smugsys}) to obtain new materials for the citizens of {basesys}."),
      {smugden=smugden, smugsys=smugsys, basesys=basesys}))
   misn.setReward(reward)

   mem.marker = misn.markerAdd( smugden )

   misn.osdCreate( title, {
      fmt.f(_("Find the smuggler in {spob} ({sys})"),{spob=smugden, sys=smugsys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

local land_smuggler, land_escorts, land_final
function land ()
   local scur = spob.cur()
   if mem.state==0 and scur==smugden then
      land_smuggler()
   elseif mem.state==1 and scur==startspob then
      land_escorts()
   elseif mem.state==4 and scur==base then
      land_final()
   end
end

function land_smuggler ()
   vn.clear()
   vn.scene()
   local s = vn.Character.new( _("Smuggler"), { image=vni.generic() } )
   local scav = vni.soundonly( 1, {pos="farleft", color=taiomi.scavenger.colour} )
   vn.transition()

   if mem.smugden_known then
      vn.na(fmt.f(_([[You land on {spob} and follow the directions Scavenger gave you to locate the smuggler. Although you are somewhat familiar with the layout of {spob}, Scavenger's instructions take you to an area you have never visited before.]]),
         {spob=smugden}))
   else
      vn.na(fmt.f(_([[You land on {spob} and follow the directions Scavenger gave you to locate the smuggler. Following Scavenger's you head into the depths of the station.]]),
         {spob=smugden}))
   end
   vn.na(_([[Making your way through the labyrinth of dimly lit corridors and rooms, you finally make it to the airlock to your destination.]]))
   vn.menu{
      {_([[Knock.]]), "cont01_knock"},
      {_([[Barge in.]]), "cont01_barge"},
   }

   vn.label("cont01_knock")
   vn.appear( s )
   vn.na(_([[As soon as you knock, you are told to come in just to find yourself looking down the barrel of a plasma shotgun.]]))
   vn.jump("cont01")

   vn.label("cont01_barge")
   vn.appear( s )
   vn.na(_([[You barge in unannounced, and quickly find yourself looking down the barrel of a plasma shotgun.]]))
   vn.jump("cont01")

   vn.label("cont01")
   s(_([["Giv'me a reason not to blow ye brains to make some abstract art on them wall behind'ye."]]))
   vn.menu{
      {_([[Put your hands up.]]), "cont02_hands"},
      {_([[Try to tackle them.]]), "cont02_tackle"},
   }

   local tackled = false
   vn.label("cont02_hands")
   vn.na(_([[You put your hands up non-aggressively and mention that you bring a deal to the table.]]))
   s(_([["Let's see about that. Keep 'em hands up."]]))
   s(_([[The pat you down and find the holodrive you have containing Scavenger's deal. They take it despite your objections, and begin playback.]]))
   vn.jump("cont02")

   vn.label("cont02_tackle")
   vn.func( function () tackled = true end )
   vn.na(_([[You begin to move and immediately are hit on the shoulder flinging you helplessly against the wall. Then suddenly an intense burning sensation begins to spread from your shoulder.]]))
   s(_([["Shouldn't of done that. Be glad I set them settin' to non-lethal."]]))
   s(_([[Barely able to make what is going on due to the pain, you can make out them whistling to themself as they approach you. Still stunned and unable to react, they pat you down and take a holodrive you have containing Scavenger's deal.]]))
   s(_([[With you still writhing in pain, they begin playback.]]))
   vn.jump("cont02")

   vn.label("cont02")
   vn.appear( scav )
   vn.na(_([[The holodrive begins playing a sound-only file. You recognize the voice as Scavenger's.]]))
   scav(_([["Hello. Who I am is not important, but it has come to my attention that you are in possession of large amounts of contraband hypergate components."]]))
   scav(_([["Such components are heavily marked and it is unlikely that anybody would be able to use them without arising suspicions from the Empire. While you could try to take it apart to reuse some components, that is a waste of the full potential of your contraband."]]))
   scav(_([["I believe we could reach a mutual agreement to take the contraband off your hands. Take a look at the attached sample. There is more where this comes from."]]))
   scav(fmt.f(_([["I need the cargo delivered to the location marked in the {sys} system. The individual there will provide escort."]]),
      {sys=handoffsys}))
   vn.na(_([[The audio playback ends.]]))
   vn.disappear( scav )

   s(_([["Let us see what we have here…"]]))
   vn.na(_([[They fiddle with the console to access the rest of the contents of the holodrive.]]))
   s(_([[Suddenly they break out into a chuckle.
"Ho-ho-ho, that is mighty interesting, ain't it? Your client must be really interested in getting their hands on them hardware."
Their put away their weapon.]]))

   vn.func( function ()
      if not tackled then
         vn.jump("cont03")
      end
   end )
   s(_([[They gesture towards your burning shoulder.
"Let me fix that in a jiffy. Stay still hun."]]))
   s(_([[They approach you and spray your injury with what appears to be some wound sealing spray.
"That'll let you get back to your ship. You might want to get it checked up though, or the skin grows back funny. Now back to business!"]]))

   vn.label("cont03")
   s(_([["So I've decided to, umm, carefully think it over, and I thinks we got a deal!"]]))
   s(fmt.f(_([["Them convoy with them parts be at {spob} in the {sys} system. I'll send word about them deal and have them meet you at the spaceport."]]),
      {spob=startspob, sys=startsys}))
   s(fmt.f(_([["We'll do our part with them convoy, but it'll be your job to keep it in one piece to {destsys}. Not that I think people gonna be waitin' fer a convoy to that empty place."]]),
      {destsys=handoffsys}))
   vn.na(_([[You feel like you've overstayed your due and head out back to the station to go on with the job.]]))

   vn.func( function ()
      if not tackled then
         vn.jump("cont04")
      end
   end )
   vn.na(_([[You don't feel any pain in your shoulder, but it doesn't look too good, but you should recover. At least they didn't aim at your head. Time to get back to your ship before the painkillers wear out.]]))

   vn.label("cont04")
   vn.run()

   misn.osdCreate( title, {
      fmt.f(_("Rendezvous with smugglers at {spob} ({sys})"),{spob=startspob, sys=startsys}),
   } )
   misn.markerMove( mem.marker, startspob )
   mem.state = 1
end

function land_escorts ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_([[You land and are quickly greeted by smugglers. They briefly introduce themselves, and say they'll be delivering the promised cargo to the specified location at {sys} before leaving on shuttles. After the formality head back to their ships to await your departure.]]),
      {sys=handoffsys}))
   vn.run()

   local ships = { "Mule", "Mule", "Mule" }
   escort.init( ships, {
      func_pilot_create = "escort_spawn",
   } )
   escort.setDest( handoffsys, "escort_success", "escort_failure" )

   misn.osdCreate( title, {
      fmt.f(_("Escort the smugglers to {sys}"),{sys=handoffsys}),
   } )
   misn.markerMove( mem.marker, handoffsys )
   mem.state = 2
end

local function escort_faction ()
   -- If we use "Pirate" it defaults to hostile to the player which can be problematic
   return faction.dynAdd( "Independent", "taiomi_convoy", _("Smugglers"), {clear_enemies=true, clear_allies=true} )
end

-- luacheck: globals escort_spawn
function escort_spawn( p )
   p:rename(_("Smuggler"))
   p:setFaction( escort_faction() )
   local m = p:memory()
   m.vulnerability = 100 -- less preferred as targets compared to player
end

-- luacheck: globals escort_success
function escort_success ()
   -- Not actually done yet
   for k,e in ipairs(escort.pilots()) do
      e:control(true)
      local pos = handoffpos + vec2.newP( 200*rnd.rnd(), rnd.angle() )
      e:moveto( pos )
   end
   escort_inpos()
end

function escort_inpos ()
   local notstopped = false
   for k,p in ipairs(escort.pilots()) do
      if not p:isStopped() then
         notstopped = true
         break
      end
   end
   if notstopped then
      hook.timer( 1, "escort_inpos" )
   else
      hook.timer( 3, "cutscene00" )
   end
end

function cutscene00()
   local ep = escort.pilots()
   for k,v in ipairs(ep) do
      v:setInvisible(true)
      v:setInvincible(true)
      v:brake()
   end
   mem.survived = #ep
   hook.timer( 6, "cutscene01" )
end

function cutscene01()
   local ep = escort.pilots()
   local fconvoy = escort_faction()
   local leavers = {}
   local j = jump.get( system.cur(), fightsys )
   for k,v in ipairs(ep) do
      local p = pilot.add("Schroedinger", fconvoy, v:pos())
      p:setDir( v:dir() )
      p:setInvisible( true )
      p:setInvincible( true )
      p:control()
      p:hyperspace( j )
      if #leavers==0 then
         p:broadcast(_("Deal is done, time to scram!"), true )
      end
      table.insert( leavers, p )
   end

   mem.state = 4
   misn.osdCreate( title, {
      fmt.f(_("Return to {spob} ({sys})"),{spob=base, sys=basesys}),
   } )
   misn.markerMove( mem.marker, base )
end

function enter ()
   local scur = system.cur()
   if scur==fightsys then
      local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
      local flt
      if fct== "Soromid" then
         flt = {
            "Soromid Nyx",
            "Soromid Odium",
            "Soromid Odium",
            "Soromid Reaver",
            "Soromid Reaver",
         }
      elseif fct=="Dvaered" then
         flt = {
            "Dvaered Vigilance",
            "Dvaered Phalanx",
            "Dvaered Phalanx",
            "Dvaered Vendetta",
            "Dvaered Vendetta",
         }
      else
         flt = {
            "Empire Pacifier",
            "Empire Admonisher",
            "Empire Admonisher",
            "Empire Lancelot",
            "Empire Lancelot",
         }
      end
      fct = faction.get( fct )

      pilot.toggleSpawn(false)
      pilot.clear()

      local function add_fleet( pos, local_fct )
         pos = pos or vec2.newP( scur:radius()*rnd.rnd(), rnd.angle() )
         local_fct = local_fct or fct
         local p = fleet.add( 1, flt, local_fct, pos )
         for k,v in ipairs(p) do
            local m = v:memory()
            m.loiter = math.huge
            m.doscans = true
         end
         return p
      end

      if mem.state < 2 then
         for i=1,4 do
            add_fleet()
         end
      else
         local fescort = escort_faction()
         local fbaddies = faction.dynAdd( fct, "taiomi_baddies", fct:name() )
         fbaddies:dynEnemy( fescort ) -- dynamic faction gives no faction hits, good I guess?
         -- positions chosen so that hopefully the player doesn't fight them all at once
         local f1 = add_fleet( vec2.new( 3e3, -8e3 ), fbaddies )
         local f2 = add_fleet( vec2.new( 3e3, 13e3 ), fbaddies )
         for k,v in ipairs(f1) do
            v:hostile()
         end
         for k,v in ipairs(f2) do
            v:hostile()
         end
         local wp = {
            vec2.new( 6e3, 6e3 ),
            vec2.new( 13e3, -7e3 ),
         }
         pilotai.patrol( f1[1], wp )
         pilotai.patrol( f2[1], wp )
         hook.timer( 8, "incoming_bogies" )
      end
   elseif scur==handoffsys and mem.state==2 then
      hook.timer( 5, "almost_there" )
   end
end

function incoming_bogies ()
   local pe = escort.pilots()
   pe[1]:broadcast(_("Incoming bogies!"))
end

function almost_there ()
   local pe = escort.pilots()
   pe[1]:broadcast(_("Almost there! Head to the rendezvous point!"))
end

-- luacheck: globals escort_failure
function escort_failure ()
   lmisn.fail(_("The smuggler ships were all destroyed!"))
end

function land_final ()
   var.push("taiomi09_done", time.get() )

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and find eagerly Scavenger waiting for you.]]))
   if mem.survived==3 then
      s(_([["Thank you for bringing the convoy over without a loss. I estimate the probability of that occurring being 3%. All the resources are going to be very useful!"]]))
   else
      s(_([["You managed to bring the convoy over! Do not worry about the convoy losses, they are well within design tolerances."]]))
   end
   s(_([["We have already started to bring in the materials. It should not take much longer before they are all here."]]))
   s(_([["Now all that is left is to finish the construction and run some preliminary tests. Given our synchronization, I estimate that the construction should be over in a few periods at most."]]))
   vn.menu{
      {_([[Ask about what was given to the smuggler.]]), "cont01_smuggler"},
      {_([["Is this enough for construction?"]]), "cont01_enough"},
      {_([[…]]), "cont01"},
   }

   vn.label("cont01_smuggler")
   s(_([[Scavenger seems to do what you can only describe as a chuckle.
"Every human has their weakness, and the smuggler was no exception. Some want credits, some want fame, but others want more… special things."]]))
   s(_([["They had a weakness for historical documents, many of which we have recovered throughout our existence. We do not have much need for them anymore. It is a small price to pay for our freedom."]]))
   vn.jump("cont01")

   vn.label("cont01_enough")
   s(_([["The materials should be more than sufficient to finish the hypergate functionality. As it only needs to work once, we can avoid much of the complexities added by reusability."]]))
   vn.jump("cont01")

   vn.label("cont01")
   s(_([["The construction is already very advanced, I estimate it will take a mean 2.981 periods. We are so close that my processor core is almost skipping cycles!"]]))
   vn.menu{
      {_([["Construction is almost done?"]]),"cont02_advanced"},
      {_([["Slower than I expected!"]]),"cont02_slower"},
      {_([[…]]),"cont02"},
   }

   vn.label("cont02_advanced")
   s(fmt.f(_([["You have not noticed? The {spob} is the construction. The derelict is being fitted with rudimentary hypergate capabilities. Making usage of the existing infrastructure is much more efficient than building a new structure from scratch."]]),
      {spob=base}))
   vn.func( function () var.push( "taiomi_construction_known", true ) end )
   vn.jump("cont02")

   vn.label("cont02_slower")
   s(_([["I would suggest you adjust your priors. Given our numbers, I do not think it is possible to reduce the construction time without increasing the synchronization ratio over 100%."]]))
   vn.jump("cont02")

   vn.label("cont02")
   s(_([["You can leave the construction to us. However, it will take some time until it is completed. You are free to wait aboard your ship, or travel around. We will wait for you to begin our one-way trip."]]))
   s(_([["First, let me give you a reward for your troubles in bringing the supplies over."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )

   vn.na(_([[What shall you do?]]))
   vn.menu{
      {_([[Wait until the construction is finished.]]), "wait"},
      {_([[Leave immediately.]]), "nowait"},
   }

   vn.label("wait")
   local wait_time = 3
   vn.na(fmt.f(_([[You take a well-deserved break and relax on your ship until the Scavenger's construction is complete.

{amount} periods have passed. It is now {date}.]]),
      {amount=wait_time, date=time.get()}))
   vn.func( function ()
      time.inc( time.new(0,wait_time,0) )
   end )
   vn.jump("done")

   vn.label("nowait")
   vn.na(_([[You decide to not wait for the construction to complete, and are ready to go back to space.]]))

   vn.label("done")
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_([[You managed to convince a smuggler to deliver contraband hypergate components to the inhabitants of Taiomi. The cargo was successfully delivered after you were able to successfully defend it from patrols.]]))
   misn.finish(true)
end
