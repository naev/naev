--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 9">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Antlejos V</spob>
 <cond>require('common.antlejos').unidiffLevel() &gt;= 9</cond>
 <done>Terraforming Antlejos 8</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Deal the final major blow get rid of the PUAAA mothership harassing Antlejos
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"
local lg = require "love.graphics"
local pilotai = require "pilotai"

local reward = ant.rewards.ant09

local retpnt, retsys = spob.getS("Antlejos V")
local mainsys = system.get("Klintus")

local mothership_name = _("Planet Saviour")


function create ()
   if not misn.claim(mainsys) then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, ant.verner.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_([[You find a dust-covered Verner once again enjoying a horrible smelling drink at the bar.]]))
   v(fmt.f(_([["Hello! The core drilling is going slow but great. The dust is a pain to get off though, it seems to permeate into clothing. This was my favourite set too. Oh well."
"Speaking of which, some of my scouts have recently been able to track down some PUAAA activity to be originating from {sys}. It seems like there is a mothership or something like that over there. I know it's quite a challenge, but I would need you to take care of it. Would you be willing to help {pnt} again?"]]),
      {sys=mainsys, pnt=retpnt}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(fmt.f(_([["Great! I know you're thinking about doing the same you did with the supply ship you took out in the Knave system, but this time it won't be that easy. This seems to be the source of almost all of the PUAAA attacks. In particular, I believe it's the notorious '{shipname}', which is well-known for having stopped the terraforming of Em 2. Normally, we wouldn't have much of a chance against the {shipname}, but in a stroke of luck, it seems like they've had some trouble and they're now stuck offline in {sys}, meaning it's the perfect time to take it down once and for all."]]),
      {shipname=mothership_name, sys=mainsys}))
   v(fmt.f(_([["The plan is pretty straight forward: I've got some extra explosives we've been using to terraform {pnt}, but I think they could be put to better use "terraforming" the {shipname}. All you have to do is fly to the {sys} system, get to the {shipname}, plant the explosives, and get back here. Now, that's probably easier said than done, and the system is going to be probably crawling with those asteroid huggers, but I've seen your flying skills and have the faith you can pull it off."]]),
      {shipname=mothership_name, sys=mainsys, pnt=retpnt}))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end
   local title = _("Eliminate the PUAAA Mothership")

   misn.accept()
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("Eliminate the PUAAA mothership named {shipname} that islocated in the {sys} system. You have been provided explosives to detonate the mothership if you are able to get close enough to it."),
      {sys=mainsys,shipname=mothership_name}))
   misn.setReward(reward)
   misn.osdCreate( title , {
      fmt.f(_("Go to {sys}"),{sys=mainsys}),
      fmt.f(_("Plant explosives on the {shipname}"),{shipname=mothership_name}),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=retpnt, sys=retsys}),
   })
   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 0

   local c = commodity.new( N_("Terraforming Explosives"), N_("Terraforming explosives meant to perform drastic changes to planet topology.") )
   misn.cargoAdd(c, 0)

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==2 and spob.cur() == retpnt then
      vn.clear()
      vn.scene()
      local v = ant.vn_verner()
      vn.newCharacter( v )
      vn.transition()
      vn.na(_([[You land and are promptly greeted by the always enthusiastic Verner.]]))
      v(_([["How did it go? … I see, better than expected! This calls for a celebration, now where did I put it? Ah, there it is."
Verner picks up what seems to be an industrial barrel labelled 'Ammonia' and sloppily dumps it into two steel mugs. He notices you giving him a following look.
"Ah, don't worry about that, I cleaned the barrel before filling it. This is the bartender's specialty "Antlejos Brew'! It's made with all natural ingredients found exclusively here!"]]))
      vn.na(_([[Quickly a really strong stench fills the docks, almost like some sort of bleach. He offers you a drink, what do you do?]]))
      vn.menu{
         {_([[Carefully try the drink.]]), "drink_little"},
         {_([[Chug the drink.]]), "drink_alot"},
         {_([[Refuse the drink.]]), "drink_refuse"},
      }

      vn.label("drink_little")
      vn.func( function () var.push( "antlejos_drank", true ) end )
      vn.na(_([[You carefully bring the drink up to your mouth and first take a whiff. It's awful. It's horrible and much worse up close. Giving Verner the benefit of the doubt, you bring it up to your lips and take a small sip, it could taste better than what it smells like, right?]]))
      vn.na(_([[Immediately the indescribable taste invades your mouth, overloading your sensory system. Your body reflexively curls up and you almost struggle to keep consciousness. You spend a while trying to regain your senses and catch your breath, all the while under Verner's sly grin.]]))
      v(_([["I've never seen that reaction on first taste! Maybe it's still a bit too soon for you to be able to savour Antlejos delicacies."
He takes a long swig and finishes his mug.
"Uuuaaaaaaaahh! Now that's the stuff. It gets better the more you try it."]]))
      vn.jump("cont01")

      vn.label("drink_alot")
      vn.func( function () var.push( "antlejos_drank", true ) end )
      vn.na(_([[You grab the mug, and in an extreme act of determination, ignore all the warning signs your body screams at you and you begin to chug the drink. With the first sip, your body convulses briefly, but you continue to drink until nothing is left. Or at least that's what you think happened before you passed out.]]))
      vn.scene()
      vn.func( function ()
         local lw, lh = lg.getDimensions()
         vn.setBackground( function ()
            lg.setColour( 0, 0, 0, 1 )
            lg.rectangle( "fill", 0, 0, lw, lh )
         end )
         music.stop()
      end )
      vn.transition( "circleopen" )
      vn.na(_([[Time stops. You try to move your body, but nothing responds, you are lost in darkness.]]))
      vn.na(_([[You struggle in vain until you realize you aren't feeling anything. You feel no sensory input, it's like your body doesn't exist.]]))
      vn.na(_([[You try to focus and see what's going on, but realize something is really amiss. You have a really subtle feeling of movement, like many little things vibrating. Could this be the vibrations of the cells of your body? The vibration of your atoms?]]))
      vn.scene()
      vn.newCharacter( v )
      vn.func( function ()
         vn.setBackground()
         music.play()
      end )
      vn.transition( "circleopen" )
      vn.na(_([[You are awoken to a splash of water and a strong burning sensation throughout your entire body. As if your internal organs had caught on fire and were relentlessly burning.]]))
      v(_([["Glad to have you back with us. I'm pretty sure your heart stopped beating for a few seconds. Maybe I should have warned you about the Antlejos delicacies."
He takes a long swig and finishes his mug.
"Uuuaaaaaaaahh! Now that's the stuff. It gets better the more you try it."]]))
      vn.na(_([[You spend a few minutes on the ground, with Verner patiently waiting, before you finally feel like you can sit up and follow a conversation.]]))
      vn.jump("cont01")

      vn.label("drink_refuse")
      vn.func( function () var.pop( "antlejos_drank" ) end )
      vn.na(_([[You refuse the drink and Verner chuckles.]]))
      v(_([["I guess it may be a bit much if you aren't used to it. It's especially good at clearing terraforming dust that gets caught in your throat! Maybe next time you'll give it a shot."]]))
      vn.jump("cont01")

      vn.label("cont01")
      v(fmt.f(_([["You've been such a help with the PUAAA, now with the {shipname} out of commission, and the end of the terraforming, we'll finally be able to get some peace and quiet here. It's almost like a dream come true. Honestly, I never expected to get this far, and if it weren't for you, I probably never would have. Cheers!"]]),
         {shipname=mothership_name}))
      v(fmt.f(_([["I have to take care of some other things now. You make sure to take some rest, OK? I think you've more than deserved it. Here, even though it's much less than you deserve, take this credit stick for all you've done for us here at {pnt}. You sure you don't want to take a barrel of Antlejos brew with you? No? Alright."]]),
         {pnt=retpnt}))
      v(_([[He gives you a strong handshake and is off, busy as ever. Alone, you now spend a while roaming around the base, more like a small city now, and make note of all the incredible progress that has gone on this small moon in such a small amount of time.]]))

      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      player.pay( reward )
      ant.log(fmt.f(_("You eliminated the PUAAA mothership bringing more security to {pnt}."),{pnt=retpnt}))
      misn.finish(true)
   end
end

local mothership
function enter ()
   if mem.state==2 then
      -- Nothing to do here
      return
   end
   -- Wrong system
   if mem.state==1 and system.cur() ~= mainsys then
      lmisn.fail(fmt.f(_("You were not supposed to leave {sys}!"),{sys=mainsys}))
      return
   end
   if system.cur() ~= mainsys then
      return
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   mem.state = 1
   misn.osdActive(2)

   -- Main positions
   local pos_mothership = vec2.new( -4e3, 0 )

   -- Define the routes
   local route0 = {
      jump.get( mainsys, "Senara" ):pos(),
      jump.get( mainsys, "NGC-3219" ):pos(),
      jump.get( mainsys, "Antlejos" ):pos(),
      jump.get( mainsys, "Kobopos" ):pos(),
   }
   local route1 = treverse( route0 )
   local route2 = {
      jump.get( mainsys, "Senara" ):pos(),
      jump.get( mainsys, "NGC-3219" ):pos(),
      vec2.new( -14e3, 0 ),
   }
   local route3 = {
      vec2.new( -14e3,  6e3 ),
      vec2.new(  10e3,  6e3 ),
      vec2.new(  10e3, -6e3 ),
      vec2.new( -14e3, -6e3 ),
   }
   local route4 = {
      vec2.new( -10e3,    0 ),
      vec2.new(  -4e3,  4e3 ),
      vec2.new(   4e3,  4e3 ),
      vec2.new(   4e3, -4e3 ),
      vec2.new(  -4e3, -4e3 ),
   }
   local route5 = {
      vec2.new(  -14e3, 4e3 ),
      pos_mothership,
      vec2.new(  -14e3, -4e3 ),
   }
   local route6 = {
      vec2.new(  6e3, -14e3 ),
      vec2.new(  6e3,  10e3 ),
      vec2.new( -6e3,  10e3 ),
      vec2.new( -6e3, -14e3 ),
   }
   local route7 = {
      vec2.new( -6e3, -5e3 ),
      vec2.new( -4e3, 4e3 ),
   }

   -- Initialize ship stuff
   local puaaa = ant.puaaa()

   mothership = pilot.add( "Zebra", puaaa, pos_mothership, mothership_name, {ai="baddiepos"} )
   mothership:setVisplayer(true)
   mothership:setHilight(true)
   mothership:control()
   mothership:brake()
   mothership:setHostile(true)
   -- Can't detect anything
   mothership:intrinsicSet( "ew_hide", 300 )
   mothership:intrinsicSet( "ew_detect", -1000 )
   -- Much more bulky than normal
   mothership:intrinsicSet( "shield", 500 )
   mothership:intrinsicSet( "armour", 1500 )
   mothership:intrinsicSet( "absorb", 30 )
   mothership:setActiveBoard(true) -- Can board them to blow them up
   hook.pilot( mothership, "death", "mothershipdeath" )
   hook.pilot( mothership, "board", "mothershipboard" )

   local function spawn_ship( s, pos )
      local p = pilot.add( s, puaaa, pos, _("PUAAA Figher") )
      p:setHostile(true)
      return p
   end

   local function add_patrol_group( route, ships, start )
      start = start or rnd.rnd(1,#route)
      local pos = route[ start ]
      local l
      for k, s in ipairs( ships ) do
         local p = spawn_ship( s, pos )
         p:changeAI( "baddiepatrol" )
         if k==1 then
            l = p
         else
            p:setLeader( l )
         end
         pilotai.patrol( p, route )
      end
   end

   local small_group = {
      "Lancelot",
      "Shark",
      "Shark",
   }
   local medium_group = {
      "Admonisher",
      "Ancestor",
      "Ancestor",
   }
   local large_group = {
      "Pacifier",
      "Vendetta",
      "Vendetta",
      "Ancestor",
   }

   -- route0 and route1 can't be random or it might spawn directly on the player
   add_patrol_group( route0, small_group, 1 )
   add_patrol_group( route1, small_group, 1 ) -- route0 in reverse
   add_patrol_group( route2, large_group )
   add_patrol_group( route3, medium_group )
   add_patrol_group( route4, small_group )
   add_patrol_group( route5, medium_group )
   add_patrol_group( route6, large_group )
   add_patrol_group( route7, small_group )
end

function mothershipdeath ()
   if mem.state == 2 then return end
   player.msg("#g".._("You eliminated the mothership!").."#0")
   mem.state = 2
   misn.osdActive(3)
   misn.markerMove( mem.mrk, retpnt )
end

function mothershipboard ()
   vntk.msg(fmt.f(_("Terraforming {shipname}"),{shipname=mothership_name}), _([[Without boarding the ship you quickly clamp the terraforming explosives on the hull, set the timer, and detach. Time to get away from there before it blows!]]))
   player.unboard()

   hook.timer( 3, "mothershipexplosives" )
end

function mothershipexplosives ()
   if mothership:exists() then
      mothership:setHealth( -1, -1 )
   end
end
