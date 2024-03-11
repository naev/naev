--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Black Cat">
 <unique />
 <priority>4</priority>
 <chance>0</chance>
 <location>None</location>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

   Black cat found on a derelict ship. Have to return it.

]]--
local fmt = require "format"
local vn  = require 'vn'
local vni = require 'vnimage'
local vntk= require 'vntk'
local tut = require "common.tutorial"
local der = require 'common.derelict'
local audio = require 'love.audio'
local luaspfx = require "luaspfx"
local love_shaders = require "love_shaders"

local cat_image = "blackcat.webp"
local cat_colour = nil

local owner_image = vni.generic()
local owner_colour = nil
local owner -- Non-persistent state

local meow = audio.newSource( "snd/sounds/meow.ogg" )

local credit_reward = 200e3

function create ()
   local tookcat = false

   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   local cat = vn.Character.new( _("Black Cat"), {image=cat_image, colour=cat_colour} )
   vn.transition()
   vn.na(_([[You make your way through the derelict, each step you take resonating throughout the vacuous vessel. As your traverse a hallway you notice a peculiar texture on one of the walls. As your light illuminates the wall, you can make out a hastily written graffiti. Although it is hard to read, you can make out the following text "#rBEW-RE OF C-T#0". What could it mean?]]))
   vn.na(_([[You eventually reach the command room when your ship suddenly informs you that there is a life form present on the ship. Not only that, it's very close! You frantically ready for a fight and prepare for the worst…

It's right on top of you!]]))
   vn.sfx( meow )
   vn.appear( cat )
   cat(_([[You make out two glowing eyes glinting in the darkness. As you shine your light on them, a dark shape emerges from the shadows.
"Meow."]]))
   cat(fmt.f(_([[In front of you is what {shipai}, your ship's AI, confirms to be a Felis catus or domesticated house cat. It looks at you with expressionless eyes with a gaze that seems to pierce your soul.
After what seems an eternity with you holding your breath, the cat stands up and walks past you.]]),{shipai=tut.ainame()}))
   cat(_([[You follow the cat throughout the ship as it leads you to… the airlock you came in from!
It seems like it wants to come back with you. What do you do?]]))
   vn.menu{
      {_("Take the cat with you"), "takecat"},
      {_("Let the cat be"), "leavecat"},
   }
   vn.label("takecat")
   vn.sfx( meow )
   vn.func( function () tookcat = true end )
   cat(_([[You open the airlock and the cat enters your ship, only to immediately turn around and start scratching the airlock again. You open the airlock again and it goes back into the derelict. Soon after, you hear scratching on the other side of the door. You let out a big sigh and the cat walks into your ship again. Not wanting to get stuck in an infinite loop, you gently prod the cat so it goes into your ship.]]))
   cat(_([[The cat struts around and behaves like it owns the place. You're going to have to figure out what to do with it. Your ship is no place for a cat to live.]]))
   vn.sfx( der.sfx.unboard )
   vn.done()

   vn.label("leavecat")
   vn.na(_("Are you sure you want to abandon the cat to its fate aboard the sinister, derelict ship?"))
   vn.menu{
      {_("Take the cute cat with you"), "takecat"},
      {_("Definitely let the beast be"), "leavecatdef"},
   }

   vn.label("leavecatdef")
   vn.na(_("You leave the cat behind and unceremoniously depart from the derelict."))
   vn.sfx( der.sfx.unboard )
   vn.run()

   if not tookcat then
      der.addMiscLog(fmt.f(_([[You left a sinister black cat on an abandoned derelict ship in {sys}.]]), {sys=system.cur()}))

      misn.finish(false)
      return
   end

   misn.accept()

   misn.setTitle(_("Black Cat"))
   misn.setDesc(_("You found a black cat on a derelict ship. It seems to want to go back somewhere, but you aren't sure where. Maybe if you could find its owner?"))
   misn.setReward(_("Unknown"))

   local c = commodity.new( N_("Black Cat"), N_([[A cute four-legged mammal "Felis catus" that seems to enjoy chasing random things around the ship.]]) )
   misn.cargoAdd( c, 0 )

   misn.osdCreate( _("Black Cat"), {
      _("Find the Black Cat's owner"),
   } )

   -- Important variables
   mem.times_jumped = 0

   hook.enter("enter")
   hook.jumpin("jumpin")
end

local function islucky ()
   local pp = player.pilot()
   if pp:ship():tags().lucky then
      return true
   end
   for k,o in ipairs(pp:outfitsList("all")) do
      if o:tags().lucky then
         return true
      end
   end
   return false
end

local event_list = {
   function () -- Overheat
      local pp = player.pilot()
      luaspfx.sfx( false, nil, meow )
      if islucky() then
         player.msg(_("Black cat hair has clogged the radiators but burns up before overheating the ship."), true)
         return
      end
      local t = pp:temp()
      pp:setTemp( math.max(400, t+50) )
      player.msg(_("Black cat hair has clogged the radiators and overheated the ship!"), true)
      player.autonavReset()
   end,
   function () -- Temporarily disable
      local pp = player.pilot()
      local _a, _s, _st, dis = pp:health()
      if dis then return end -- Already disabled
      luaspfx.sfx( false, nil, meow )
      if islucky() then
         player.msg(_("The black cat accidentally hit the ship restart button, but nothing happens."), true)
         return
      end
      pp:disable( true )
      hook.timer( 5, "disable_restart" )
      player.msg(_("The black cat accidentally hit the ship restart button!"), true)
      player.autonavReset()
   end,
   function () -- Energy discharge
      local pp = player.pilot()
      luaspfx.sfx( false, nil, meow )
      if islucky() then
         player.msg(_("The black cat managed to accidentally disconnect the energy capacitors, but the back up system takes over while you fix it."), true)
         return
      end
      pp:setEnergy( 0 )
      player.msg(_("The black cat managed to accidentally disconnect the energy capacitors!"), true)
      player.autonavReset()
   end,
}
local function event ()
   -- Larger chance of just random messages
   if rnd.rnd() < 2/3 then
      local msg_list = {
         _("The black cat stares at you ominously."),
         _("A waft of black cat hair flies around."),
         _("The black cat's tail fluffs up and it sprints away."),
         _("The black cat scratches the airlock. It wants out?"),
         _("The black cat unceremoniously barfs up a hairball."),
         _("You hear weird noises from the black cat freaking out over nothing."),
         _("The black cat suddenly sprints through the ship."),
         _("The black cat curls up and falls asleep on top of the control panel."),
         _("The black cat shows you its belly, but bites you when you try to pet it."),
         _("The black cat uses the commander chair as a scratching post."),
         _("The black cat bumps into your ship's self-destruct button, but you manage to abort it in time."),
      }
      luaspfx.sfx( false, nil, meow )
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
      mem.event_check_hook = hook.timer( 20+10*rnd.rnd(), "event_check" )
      return
   end
   mem.event_check_hook = hook.timer( 10+5*rnd.rnd(), "event_check" )
end

function enter ()
   if mem.event_check_hook then
      hook.rm( mem.event_check_hook )
   end
   if not mem.event_finish then
      mem.event_check_hook = hook.timer( 20+10*rnd.rnd(), "event_check" )
   end
end

function jumpin ()
   -- This hook is run _BEFORE_ the enter hook.
   mem.times_jumped = mem.times_jumped+1
   mem.event_finish = false
   local chance = math.max( (mem.times_jumped-10)*0.05, 0 )
   if rnd.rnd() > chance or system.cur():presence("Wild Ones") < 50 then
      return
   end

   -- Set up event ending
   mem.event_finish = true

   local fwo = faction.get("Wild Ones")
   local fpir = faction.dynAdd( fwo, "blackcat_owner", fwo:name(), {clear_enemies=true, clear_allies=true, player=0} )
   fpir:setPlayerStanding(0)

   local pos = vec2.newP( 0.8*system.cur():radius()*rnd.rnd(), rnd.angle() )
   owner = pilot.add( "Pirate Shark", fpir, pos )
   owner:control(true)
   owner:follow( player.pilot() )
   hook.timer( 1, "owner_hail_check" )
   hook.pilot( owner, "hail", "owner_hail" )
   hook.pilot( owner, "board", "owner_board" )
   mem.owner_was_hailed = false
end

function owner_hail_check ()
   if not owner or not owner:exists() then return end

   local pp = player.pilot()
   local det, scan = pp:inrange( owner )

   if det and scan then
      owner:hailPlayer( true )
      return
   end

   hook.timer( 1, "owner_hail_check" )
end

function owner_hail ()
   if mem.owner_was_hailed then
      owner:comm( _("P-p-please bring them o-over.") )
      player.commClose()
      return
   end

   vn.clear()
   vn.scene()
   local o = vn.newCharacter( _("Nervous Individual"), {image=owner_image, colour=owner_colour, shader=love_shaders.hologram()} )
   vn.transition("electric")
   vn.na(fmt.f(_("You open a communication channel with the {plt} and a hologram of a nervous-looking individual materializes in front of you."),{plt=owner}))
   o(_([["H-h-hello there. You w-w-wouldn't happen to have a c-cat onboard?"]]))
   vn.menu{
      {p_("blackcat", [["Yes."]]), "catyes"},
      {p_("blackcat", [["No (lie)."]]), "catno"},
   }

   vn.label("catno")
   vn.sfx( meow )
   vn.na(_([[Just as you utter the word "No", the black cat drowns out your reply with a resonating "Meow", that is clearly heard on the other side of the communication channel.]]))
   vn.jump("catyes")

   vn.label("catyes")
   o(_([[They let out a sigh of relief.
"You f-f-found it! I thought I was a g-g-goner! I'll b-brake my ship so you c-can bring it over."]]))
   vn.na(fmt.f(_("Given the streak of bad luck the feline has brought you, you figure it is in your best interest to bring the cat over to the {plt}."),{plt=owner}))

   vn.done("electric")
   vn.run()

   owner:control()
   owner:taskClear()
   owner:brake()
   owner:setActiveBoard(true)
   owner:setHilight()

   mem.owner_was_hailed = true
   player.commClose()
end

function owner_board ()
   vn.clear()
   vn.scene()
   local _cat = vn.Character.new(_("Black Cat"), {image=cat_image, colour=cat_colour})
   vn.sfx( der.sfx.board )
   vn.transition()
   vn.na(fmt.f(_("Your ship locks its boarding clamps on the {plt}, and the airlock opens up revealing some strangely musty air and pitch black darkness. How odd."),{plt=owner}))
   vn.na(_("You realize the cat is nowhere to be seen and start to search for it to bring it over. Funny how it always seems to be where you don't want it and when you need it you can't find it."))
   vn.sfx( meow )
   vn.na(_("You scour the ship and end up going back to the commander chair. As you are about to look behind it, you hear a sonorous meow and a black shadow flies past you towards the airlock."))
   vn.sfx( der.sfx.unboard )
   vn.na(_("You run to try to catch it, but hear the sound of the airlock closing and detaching of the locking clamps. You run back to your command chair to see what the other ship is doing, but you can not find it anywhere. They seem to have a knack for fleeing."))
   vn.na(fmt.f(_("You sit resigned and outwitted at your command chair when you notice a credit chip with {credits} on the floor. It looks like it has cat bite marks too."),{credits=fmt.credits(credit_reward)}))
   vn.sfxVictory()
   vn.na(_("You then get around to cleaning up the copious amounts of cat hair invading every last corner of your ship. With the amount collected you make a cute black cat doll. It's like a tiny version of the real thing without the assholiness."))
   vn.run()

   player.pay( credit_reward )
   player.outfitAdd( "Black Cat Doll" )

   der.addMiscLog(_([[You rescued a black cat from a derelict ship and safely delivered it to its owner, who was flying a Wild Ones pirate ship.]]))
   faction.get("Wild Ones"):modPlayerSingle(3)

   player.unboard()
   hook.safe("owner_gone")
end

function owner_gone ()
   owner:rm()
   misn.finish(true)
end

function abort ()
   vntk.msg(_("No cat in sight…"), _("You go to get rid of the black cat, but can not find it anywhere. After a long search you reach the only logical conclusion; that it vanished into thin air. Guess you can forget about it for now."))

   der.addMiscLog(fmt.f(_([[You rescued a black cat from a derelict ship only to have it disappear into thin air in {sys}.]]), {sys=system.cur()}))

   misn.finish(false)
end
