--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Black Cat">
 <avail>
  <priority>4</priority>
  <chance>0</chance>
  <location>None</location>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

   Black cat found on a derelict ship. Have to return it.

]]--
local pir = require "common.pirate"
local fmt = require "format"
local vn  = require 'vn'
local vntk= require 'vntk'
local tut = require "common.tutorial"
local der = require 'common.derelict'
local audio = require 'love.audio'

local cat_image = "blackcat.webp"
local cat_colour = nil

local meow = audio.newSource( "snd/sounds/meow.ogg" )

function create ()
   local tookcat = false

   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   local cat = vn.Character.new( _("Black Cat"), {image=cat_image, color=cat_colour} )
   vn.transition()
   vn.na(_([[You make your way through the derelict, each step you take resonating throughout the vacuous vessel. As your traverse a hallway you notice a peculiar texture on one of the walls. As your light illuminates the wall, you can make out a hastily written graffiti. Although it is hard to read, you can make out the following text "#rBEW-RE OF C-T#0". What could it mean?]]))
   vn.na(_([[You eventually reach the command room when your ship suddenly informs you that there is a life form present on the ship. Not only that, it's very close! You frantically ready your weapons and prepare for the worst…

It's right on top of you!]]))
   vn.sfx( meow )
   vn.appear( cat )
   cat(_([[You make out two glowing eyes glinting in the darkness. As you shine your light on them, a dark shape emerges from the shadows.
"Meow."]]))
   cat(fmt.f(_([[In front of you is what {shipai} confirms to be a Felis catus or domesticated house cat. It looks at you with expressionless eyes with a gaze that seems to pierce your soul.
After what seems an eternity with you holding your breath, the cat stands up, and walks past you.]]),{shipai=tut.ainame()}))
   cat(_([[You follow the cat throughout the ship as it leads you to… the airlock you came in from.
It seems like it wants to come back with you. What do you do?]]))
   vn.menu{
      {_("Take the cat with you"), "takecat"},
      {_("Let the cat be"), "leavecat"},
   }
   vn.label("takecat")
   vn.sfx( meow )
   vn.func( function () tookcat = true end )
   cat(_([[You open the airlock and the cat enters your ship, only to immediately turn around and start scratching the airlock again. You open the airlock again and it goes back into the derelict. Soon after, you hear scratching on the other side of the door. You let out a big sigh and the cat walks into your ship again. Not wanting to get stuck in an infinite loop, you gently prod the cat so it goes into your ship.]]))
   cat(_([[The cat struts around and behaves like it owns the place. You're going to have to figure out what to do with it. Your ship is no place for a cat to live in.]]))
   vn.sfx( der.sfx.unboard )
   vn.done()

   vn.label("leavecat")
   vn.na(_("Are you sure you want to abandon the cat to its fate aboard the sinister derelict ship?"))
   vn.menu{
      {_("Take the cute cat with you"), "takecat"},
      {_("Definitely let the beast be"), "leavecatdef"},
   }

   vn.label("leavecatdef")
   vn.na(_("You leave the cat behind and unceremoniously depart from the derelict."))
   vn.sfx( der.sfx.unboard )
   vn.run()

   if not tookcat then
      misn.finish(false)
      return
   end

   misn.accept()

   local c = misn.cargoNew( N_("Black Cat"), N_([[A cute four-legged mammal "Felis catus" that seems to enjoy chasing random things around the ship.]]) )
   misn.cargoAdd( c, 0 )

   misn.osdCreate( _("Black Cat"), {
      _("Find the Black Cat's owner"),
   } )

   -- Important variables
   times_jumped = 0

   hook.enter("enter")
   hook.jumpin("jumpin")
end

local event_list = {
   function () -- Overheat
      local pp = player.pilot()
      local t = pp:temp()
      pp:setTemp( math.max(400, t+50) )
      meow:play()
      player.msg(_("Black cat hair has clogged your radiator and overheated your ship!"), true)
   end,
   function () -- Temporary disable
      local pp = player.pilot()
      local a, s, st, dis = pp:health()
      if dis then return end -- Already disabled
      pp:disable( true )
      hook.timer( 5, "disable_restart" )
      meow:play()
      player.msg(_("The black cat accidentally hit the ship restart button!"), true)
   end,
   function () -- Energy discharge
      local pp = player.pilot()
      pp:setEnergy( 0 )
      meow:play()
      player.msg(_("The black cat managed to accidentally disconnect the energy capacitors!"), true)
   end,
}
function event ()
   -- Larger chance of just random messages
   if rnd.rnd() < 0.5 then
      local msg_list = {
         _("The black cat stares at you ominously."),
         _("A waft of black cat hair flies around."),
         _("The black cat's tail fluffs up ad it sprints away."),
         _("The black cat scratches he airlock. It wants out?"),
         _("The black cat unceremoniously barfs up a hairball."),
         _("You hear weird noises from the black cat freaking out over nothing."),
         _("The black cat suddenly sprints through the ship."),
         _("The black cat curls up and falls asleep on top of the control panel."),
         _("The black cat shows you its belly, but bites you when you pet it."),
         _("The black cat uses the commander chair as a scratching post."),
      }
      meow:play()
      player.msg( msg_list[rnd.rnd(1,#msg_list)], true )
      return
   end
   -- Proper (bad) events
   event_list[ rnd.rnd(1,#event_list) ]()
end

function disable_restart ()
   local pp = player.pilot()
   local a, s = pp:health()
   pp:setHealth( a, s, 0 )
end

function event_check ()
   if rnd.rnd() < 0.05 then
      event()
      event_check_hook = hook.timer( 20+10*rnd.rnd(), "event_check" )
      return
   end
   event_check_hook = hook.timer( 10+5*rnd.rnd(), "event_check" )
end

function enter ()
   if event_check_hook then
      hook.rm( event_check_hook )
   end
   event_check_hook = hook.timer( 20+10*rnd.rnd(), "event_check" )
end

function jumpin ()
   times_jumped = times_jumped+1
   local chance = math.max( (times_jumped-20), 0 )
   if rnd.rnd() < chance or system.cur():presence("Wild Ones") < 50 then
      return
   end

   -- Set up event ending
end

function abort ()
   vntk.msg(_("No cat in sight…"), _("You go to get rid of the black cat, but can not find it in sight. After a long search you reach the only logical conclusion that it vanished. Guess you can forget about it for now."))
   misn.finish(false)
end
