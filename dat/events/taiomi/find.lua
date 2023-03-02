--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Finding Taiomi">
 <location>enter</location>
 <unique />
 <chance>100</chance>
 <cond>system.cur() == system.get("Bastion")</cond>
 <chapter>[^0]</chapter>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</event>
--]]
--[[

   Finding Taiomi Event

--]]
local vn = require 'vn'
local vne = require "vnextras"

local board_flashback -- Forward-declared functions
local derelict_mule, derelicts, drone, evt_state, fidget_hook, numboarded -- Event state, never saved.

-- Threshold distances to detect the drone
local dist_detect_mule = 3e3 -- first encounter at mule
local dist_detect_jump = 3e3 -- second encounter at jump

--[[
-- Event states:
-- 0: not found by the player
-- 1: CUTSCENE player near derelict, zoom in and show the drone which runs away
-- 2: player chases drone until near jump
-- 3: CUTSCENE drone jumps off
-- 4: player has to search for jump in derelicts
--]]
evt_state = 0

function create ()
   -- Extra derelicts
   numboarded = 0
   derelicts = {}
   local function addDerelicts( number, shiptype, boardfunc )
      local pos = vec2.new( 15000, 6000 )
      for i = 1,number do
         local p = pos + vec2.new( 3000*rnd.rnd(), rnd.rnd()*359 )
         local s = shiptype[ rnd.rnd(1,#shiptype) ]
         local d = pilot.add( s, "Derelict", p, p_("ship", "Derelict") )
         d:setInvincible(true)
         d:disable()
         hook.pilot( d, "board", boardfunc )
         table.insert( derelicts, d )
      end
   end
   addDerelicts( 9, { "Llama", "Koala", "Quicksilver", "Hyena" }, "boardnothing" )
   addDerelicts( 5, { "Mule", "Rhino" }, "boardothers" )

   -- Main derelict
   local pos = vec2.new( 12000, 4250 )
   derelict_mule = pilot.add( "Mule", "Derelict", pos, p_("ship", "Derelict") )
   derelict_mule:setInvincible(true)
   derelict_mule:disable()

   -- Drone
   pos = derelict_mule:pos() + vec2.newP( 30, rnd.angle() )
   drone = pilot.add( "Drone (Hyena)", "Independent", pos )
   drone:setInvisible(true)
   drone:setInvincible(true)
   drone:control()
   drone:brake()

   hook.pilot( derelict_mule, "board", "boardnothing" )

   fidget_hook = hook.timer( 3.0, "fidget" )
   hook.timer( 0.5, "heartbeat" )
   hook.jumpout("leave")
   hook.land("leave")
end

function leave () --event ends on player leaving the system or landing
   -- only mark done if the player found the jump
   local j = jump.get( system.cur(), "Taiomi" )
   evt.finish( j:known() )
end

function fidget ()
   local pos = derelict_mule:pos() + vec2.newP( 30, rnd.angle() )
   drone:moveto( pos )
   if evt_state==0 then
      fidget_hook = hook.timer( 5.0, "fidget" )
   end
end

function heartbeat ()
   local pp = player.pilot()

   local function player_setup ()
      player.autonavAbort(_("You have noticed something strange…"))
      pp:setInvincible(true)
      pp:control()
      pp:brake()
   end

   if evt_state==0 then
      local dist = pp:pos():dist( drone:pos() )
      if dist < dist_detect_mule then
         evt_state = 1
         player_setup()
         camera.set( derelict_mule:pos() )
         camera.setZoom( math.max(1.5,camera.getZoom()) )
         vn._sfx.eerie:play()
         hook.timer( 6.0, "drone_runaway" )
         hook.timer( 10.0, "returncontrol", 2 )
      end
   elseif evt_state==2 then
      local dist = pp:pos():dist( drone:pos() )
      if dist < dist_detect_jump then
         evt_state = 3
         player_setup()
         camera.set( drone:pos() )
         camera.setZoom( math.max(1.5,camera.getZoom()) )
         drone:taskClear()
         drone:hyperspace( "Taiomi" )
         vn._sfx.eerie:play()
         hook.timer( 11.0, "returncontrol", 4 )
         hook.timer( 14.0, "whatwasthat" )
      end
   end
   hook.timer( 0.5, "heartbeat" )
end

function drone_runaway ()
   hook.rm( fidget_hook )
   drone:taskClear()
   drone:moveto( jump.get( system.cur(), "Taiomi" ):pos() )
end
function returncontrol( state )
   camera.set()
   camera.setZoom()
   local pp = player.pilot()
   pp:setInvincible(false)
   pp:control(false)
   evt_state = state
end
function whatwasthat ()
   player.msg(_("What was that?"))
end

function boardnothing ()
   local messages = {
      _("There is nothing of worth left on the ship."),
      _("The ship consists of nothing more than a bare hull."),
      _("The ship has already been scavenged."),
      _("There is not much left besides the hull of the ship."),
      _("The ship looks like it has been picked clean."),
   }
   player.msg( messages[ rnd.rnd(1,#messages) ] )
   player.unboard()
end

-- luacheck: globals boardothers
function boardothers( _p )
   numboarded = numboarded + 1
   if numboarded == 2 then
      board_flashback()
      jump.get( system.cur(), "Taiomi" ):setKnown(true)
      player.unboard()
   else
      boardnothing()
   end
end

function board_flashback()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_("You board the ship and don't see much of value. As you are about to give up and go back to your ship, something in the corner of your eye catches your attention."))
   vn.na(_("You find a paper notebook stuck in a part of damaged hull. Most people use the ship computers to store notes and information and rarely rely on physical storage."))
   vn.na(_("You begin to read the passages."))

   local log = vne.notebookStart()
   log(_([[UST 602:1914

Although my peers will likely make fun of me if they find out, I have decided to start a paper log of my upcoming travels aboard the Beagle. It was hard to find a place selling paper notebooks, as everyone uses holopads, but I was able to find a small place at an antique shop on Antica. For such primitive technology, it was very expensive.

I look forward to starting my travels next decaperiod! It will be my first time leaving Arcturus in ages!]]))
   log(_([[UST 602:1928

I have met the crew. They seem fairly friendly except for the big silent guy. I wonder if it will all work out… I just have to stop messing things up this time.

My room is a bit small, but as expected. My roommate seems like a quiet person. I'm not sure if I'll be able to get used to all this floating around in zero-gravity though.]]))
   log(_([[UST 602:1971

We are on route to the Delta Pavonis system.

Things haven't been working too well. There was a leak in one of the radiators. We almost got fried, but my roommate was able to fix it in time.]]))
   log(_([[UST 602:1980

The Captain got in a fight with the provider that was supposed to give us the cargo to take to the Qex system… I don't know what we'll do if this trip gets cancelled… It will look horrible on my Curriculum Vitae…]]))
   log(_([[UST 602:1983

It seems like we got another commission, but we have to go to some place called Hatter in Soromid space… The navigator claims he knows a shortcut, but I have a bad feeling about this.

I want to run back home, but I can't quit so early. They would never stop making fun of me. Maybe Soromid space will be interesting?

We leave to Dune next period.]]))
   log(_([[UST 602:1995

We have made it to Arrakis in the Dune system.

The sand is awful here. It gets in everything and everywhere, so I decided to cut short my visit of the local area. It's also nice to be in the ship alone. Much quieter and more peaceful.]]))
   log(_([[UST (the date is blank)

(This page consists of scribbled drawings of the ship and people. A person captioned "Captain" is drawn with horns coming out of his head.)]]))
   log(_([[UST 602:1999

We have finished loading the cargo, which consists of some sort of bottles of something like sand? The Captain said it is very expensive and was yelling at us all the time while we were loading the cargo.

I have decided that once this is over I will try to find a new job. I don't think I will ever get used to the Captain's temper.

Since it seems like the navigator has a bit of a hangover, we leave in 5 periods.]]))
   log(_([[UST 602:2008

It was very hard to wake the navigator and we are behind schedule again. I hate getting yelled at.

We are on on our way to Soromid space, but I don't think the navigator is in much condition to fly.]]))
   log(_([[UST 602:2011

(the writing is much more frantic)
We are being shot at! There's pirates all over! This is not what I signed up for!

WE'RE GOING TO DIE! WE'RE GOING TO DIE! WE'RE GOING TO DIE!
(The writing gets abruptly cut off.)
]]))
   log(_([[UST 602.2020

This is awful! We are hiding out in an asteroid field. Our cockpit was blown out and the navigator and captain are dead.

I have no idea what to do. It is very hard to write in my space suit, but I don't think the atmosphere unit is going to last long.]]))
   log(_([[UST (the date is blank)

Something is scratching at the hull. It sounds like a weird animal. I haven't seen any living person in I don't know how long.

There is no atmosphere and I can't take off my suit anymore. I don't see any way out.]]))
   log(_([[UST (the date is blank)

I have lost all notion of time.

They are all around. I don't really understand what they are. They look like small ships, but they are slowly and mechanically tearing apart the ship.

I'm going to die aren't I?]]))
   log(_([[UST (the date is blank)

I'm getting cold. I think the heating unit is malfunctioning.

I may have finally lost my mind. There are weird ships prying and removing the ship components. They seem to behave like some sort of pack of wild animals.]]))
   log(_([[UST (the date is blank)

I don't feel hunger anymore. The numbness sensation is spreading.

The ships don't seem to pay me much attention as they come and go. They remind me of ants back home.]]))
   log(_([[UST (the date is blank)

(There are lots of weird scribbles on the page, however, you can make some sort of map with an area indicated by a big X. It seems like you could investigate this.)
]]))
   vn.sfxBingo()
   log(_([[UST (the date is blank)

A---s--hm---t---

(It seems like it is the end of the written part of the notebook.)]]))

   vne.notebookEnd()
   vn.sfxEerie()
   vn.na(_([[You tune your ship sensors to pick up the most miniscule of disturbances and focus on the area indicated by the notebook. You are about to give up when you detect an anomaly. It looks like you can use this to jump, but where could it lead?]]))
   vn.done()
   vn.run()
end
