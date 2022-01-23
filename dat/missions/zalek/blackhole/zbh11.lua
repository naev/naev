--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 11">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <spob>Research Post Sigma-13</spob>
  <location>Bar</location>
  <done>Za'lek Black Hole 10</done>
 </avail>
 <tags>
  <tag>zlk_cap_ch01_lrg</tag>
 </tags>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 11 (Epilogue)

   Perform non-standard jump with Zach to the anubis black hole and recover a drone.
]]--
local vn = require "vn"
local fmt = require "format"
local sokoban = require "minigames.sokoban"
local zbh = require "common.zalek_blackhole"

-- luacheck: globals land enter zach_say board_drone heartbeat heartbeat_bh (Hook functions passed by name)

local retpnt, retsys = spob.getS("Research Post Sigma-13")
local targetsys = system.get("Anubis Black Hole")
local jret, jtarget = jump.get( retsys, targetsys )

local title = _("Anubis Black Hole") -- For OSD and stuff
local mass_limit = 500 -- Maximum mass of the ship the player can fly

function create ()
   misn.finish()
   if not misn.claim( {retsys, targetsys} ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You find Zach scrunched over his cyberdeck, endlessly pouring over the data he obtained from the drone that Icarus brought over.]]))
   z(_([["This is fascinating. My colleagues, foretelling their demise, were able to load their research notes into drones, and launch them to try to save them. Apparently there were several, although some seem to have been destroyed by Dr. Slorn and his team. It looks like there should be one nearby, would you be willing to help me go retrieve it?"]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   z(_([["The drone that Icarus brought over supplements greatly the notes I was able to piece together and greatly surpasses them. Apparently, they were really onto something given the depth of the documents. However, another drone that is mentioned in the documents is even more interesting, and we should spare no efforts to try to recover it as soon as possible, given the expected location."]]))
   z(_([["So, it seems like one of the drones they sent out, not only contains many additional notes, but it was also performing one last experiment. However, from some of the preliminary flight data, it seems like they were in too much of a hurry and the drone ended up malfunctioning. I'm not certain it was able to perform the task it was set to do, but hopefully it failed afterwards. The tricky thing is where the drone is located: in an orbit of the Anubis Black Hole."]]))
   vn.menu{
      {_([["You want to fly into the black hole?"]]), "01bh" },
      {_([["That's suicidal!"]]), "01bh" },
      {_([["How do we get there?"]]), "01bh" },
      {_([[Stay silent.]]), "01bh" },
   }

   vn.label("01bh")
   z(_([["It's not as bad as it sounds. We won't be travelling into the black hole, just pretty close to it, but at a distance that the ship should be able to easily avoid the gravitational pull. Nothing to worry about. However, we will have to disable all the ship's built-in security jump systems to allow it to make the jump."]]))

   vn.menu{
      {_([["Isn't that dangerous?"]]), "02danger" },
      {_([["Who needs security anyway?"]]), "02security" },
      {_([[Stay silent.]]), "02cont" },
   }

   vn.label("02danger")
   z(_([["It should be in the acceptable parameters, and my simulations only give a 7% chance of disaster. Much better than the average of what we've been doing up until now. There is no need to worry, the math is solid!…　Wait, did I forget to carry the one again?"
He furrows his brows a bit as he tries to remember.]]))
   vn.jump("02cont")

   vn.label("02security")
   z(_([["According to some studies, pilots have 97% chance of meeting a horrible fate without the security jump system. However, for responsible captains who run all the simulations and properly compute the trajectories, there is only a 13% chance of the ship hitting a large gravitational body. Wait, what point was I making again?"]]))
   vn.jump("02cont")

   vn.label("02cont")
   z(_([["All you have to do is get to the optimal jump point, and a custom AI I've written should handle all the jump intricacies for us. I'll also be joining you in case anything goes wrong, but it should all be well within parameters. Once we get there, I'll provide you with a rough estimate of the drone location so we can go recover it. All we need is the data, the drone itself is not necessary to recover."]]))
   z(fmt.f(_([["One important thing I forgot to mention, my calculations only work for ships up to {mass} of mass. Anything larger and not getting sucked into the black hole and crushed to the size of a hairpin becomes a rather complicated affair."]])
      {mass=fmt.tonnes(mass_limit)}))
   z(_([["The drone is in a decaying orbit around the black hole, so we do not have much time to spare before it gets sucked in. Let us make haste to recover it."]]))

   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( title )
   misn.setReward( _("Unknown") )
   misn.setDesc(_("Perform a non-stand jump towards the Anubis Black hole to recover a damaged scout drone."))

   mem.mrk = misn.markerAdd( retsys )
   mem.state = 1

   local c = commodity.new( N_("Zach"), N_("A Za'lek scientist.") )
   misn.cargoAdd(c, 0)

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==3 and spob.cur() == retpnt then

      vn.clear()
      vn.scene()
      local z = vn.newCharacter( zbh.vn_zach() )
      vn.transition( zbh.zach.transition )
      vn.na(_([[TODO]]))
      z(_([["TODO"]]))
      vn.sfxVictory()
      --vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      faction.modPlayer("Za'lek", zbh.fctmod.zbh11)
      --player.pay( reward )
      zbh.log(_([[TODO]]))
      misn.finish(true)
   end
end

local pexp
function enter ()
   if mem.state==1 and system.cur() == retsys then
      local pp = player.pilot()
      if pp:mass() > mass_limit then
         misn.osdCreate( title, {
            fmt.f(_("Get into a ship with less than {mass} of mass"),{mass=fmt.tonnes(mass_limit)})
         } )
         hook.timer(  5, "zach_say", fmt.f(_("The ship we are in is too heavy. We'll need a ship under {mass} to make the jump."), {mass=fmt.tonnes(mass_limit)} ))

      else
         misn.osdCreate( title, {
            fmt.f(_("Perform a non-standard jump to the black hole from {sys}"),{sys=retsys}),
            fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=retpnt,sys=retsys}),
         } )

         hook.timer(  5, "zach_say", _("I've marked the safest point to try the non-standard jump on your map.") )
         hook.timer( 12, "zach_say", _("Non-standard doesn't mean non-safe though.") )
         hook.timer( 17, "zach_say", _("Actually, in this case it does though…") )
         hook.timer( 22, "zach_say", _("Statistically, we're more likely to survive.") )
         hook.timer( 27, "zach_say", _("I'll shut up now.") )
         system.mrkAdd( jret:pos() )
         hook.timer( 30, "heartbeat" )
      end

   elseif mem.state==2 and system.cur() == retsys then
      local pp = player.pilot()
      pp:control(false)
      mem.state = 3

   elseif mem.state==1 and system.cur() == targetsys then

      music.stop(true) -- No music to make more omnious
      camera.shake()
      local pp = player.pilot()
      pp:control(false)

      local pos = vec2.newP( 5000, 2000 )
      local p = pilot.add( "Za'lek Scout Drone", "Za'lek", pos, _("Damaged Drone") )
      p:disable()
      p:setVisplayer(true)
      p:setHilight(true)
      p:setActiveBoard(true) -- To allow reboarding
      pexp = p
      pilot.hook( p, "board", "board_drone" )

      hook.timer(  5, "zach_say", _("Damn! That was closer than I wanted.") )
      hook.timer( 12, "zach_say", _("Stabilizers… check. Engine… check. Gravitational pull… acceptable.") )
      hook.timer( 18, "zach_say", _("Looks like we're in the clear!") )
      hook.timer( 23, "zach_say", _("Wow, such a beauty.") )
      hook.timer( 28, "zach_say", _("Let's see, I've marked the expected drone position on the map.") )
      hook.timer( 45, "zach_say", _("I wonder how close we can get…") )
   end
end

function board_drone ()
   if mem.state==2 then return end

   local hacked = false

   vn.clear()
   vn.scene()
   vn.transition( zbh.zach.transition )

   local z = zbh.vn_zach()
   local d = vn.newCharacter( pexp:name(), {
      image = "gfx/ship/zalek/zalek_drone_light_comm.webp",
   } )
   vn.na("You hook up to the damage drone and are able to access the control panel. Since Zach seems to be distracted, it seems like you have to access it yourself.")

   sokoban.vn{ levels={8,9}, header="Drone Control Panel"}
   vn.func( function ()
      if sokoban.completed() then
         mem.state = 2
         vn.jump("sokoban_done")
         pexp:setActiveBoard(false)
         pexp:setHilight(false)
         hacked = true
         return
      end
      vn.jump("sokoban_fail")
   end )

   vn.label([[sokoban_done]])
   vn.disappear( d )
   vn.appear( z, zbh.zach.transition )
   vn.na(_([[As you are finally able to access the data, you see Zach was peering over your shoulder most of the time.]]))
   z(_([["Your form could use some work, but not bad for not having formal training. Here, let me look at the data."]]))
   z(_([[He jacks his cyberdeck into the drone interface and begins inputting commands at a dizzying speed. He begins to hum to himself as he blazes across the filesystem, scouring every nook and cranny in every directory in a semi-automated fashion. Finally, he seems happy with what he found.]]))
   z(_([["This is most peculiar, although there was a mistake in the calculations which caused the drone to malfunction, the theoretical formulations seem to be all correct. I'll have to think of the implications, but this does seem to indicate that there is a lot of untapped potentially."]]))
   z(fmt.f(_([["If you can take the ship to the position I've marked on the map, the AI should be able to take us back to {sys}."]]),
      {sys=retsys}))
   z(_([[Without saying anything else, he plops down on a chair and starts analyzing the data on his cyberdeck while spinning around slowly in the chair, once again humming to himself.]]))
   vn.done( zbh.zach.transition )
   vn.done()

   vn.label("sokoban_fail")
   vn.na(_([[You failed to access the drone data.]]))

   vn.done( zbh.zach.transition )
   vn.run()

   -- Didn't hack through
   if not hacked then
      return
   end

   misn.osdActive(2)
   system.mrkAdd( jtarget:pos() )
   misn.markerMove( mem.mrk, retpnt )
   hook.timer( 1, "heartbeat_bh" )

   mem.state = 2
   player.unboard()
   pexp:setNoboard() -- Don't allow boarding again
end

function zach_say( msg )
   player.autonavReset( 3 )
   player.pilot():comm(fmt.f(_([[Zach: "{msg}"]]),{msg=msg}))
end

local hstate = 0
function heartbeat ()
   local nexttime = 1
   local pp = player.pilot()
   if hstate==0 and jret:pos():dist( pp:pos() ) < 700 then
      zach_say(_("The AI will take it from here."))
      nexttime = 5
      pp:control(true)
      pp:brake()
      hstate = 1

   elseif hstate==1 then
      zach_say(_("Don't worry about the warning lights, they're normal… I think."))
      pp:taskClear()
      pp:hyperspace( jret )
      hstate = 2

   elseif hstate==2 then
      player.omsgAdd("#r".._("WARNING: Hyperdrive security system disabled!").."#0")
      return

   end

   hook.timer( nexttime / player.dt_mod(), "heartbeat" )
end

function heartbeat_bh ()
   local pp = player.pilot()
   if jtarget:pos():dist( pp:pos() ) < 700 then
      pp:comm(_("The AI takes control of your ship to perform the jump."))
      player.omsgAdd("#r".._("WARNING: Hyperdrive security system disabled!").."#0")
      pp:control(true)
      pp:hyperspace( jtarget )
      return
   end
   hook.timer( 1, "heartbeat_bh" )
end
