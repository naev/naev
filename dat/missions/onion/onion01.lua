--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 01">
 <unique />
 <priority>0</priority>
 <chance>10</chance>
 <location>Computer</location>
 <cond>
   if spob.get("Gordon's Exchange"):system():jumpDist() &lt; 4 then
      return false
   end
   return require("misn_test").cargo()
 </cond>
 <chapter>1</chapter>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Stub mission to trigger onion01
--]]
local fmt = require "format"
local vntk = require "vntk"
local strmess = require "strmess"
local pp_shaders = require 'pp_shaders'
local lg = require "love.graphics"
local car = require "common.cargo"
local vn = require "vn"
local vne = require "vnextras"
local onion = require "common.onion"

local destpnt, destsys = spob.getS("Gordon's Exchange")
local money_reward = onion.rewards.misn01

-- Create the mission
function create()
   if not var.peek("testing") then return false end

   -- This will mess up strings quite badly
   local messup = function ( str )
      return strmess.messup( str, 0.2 )
   end

   local null = "#r".._("NULL").."#0"

   misn.markerAdd( system.get("Sol"), "computer" )
   local title = fmt.f(_("Shipment to {pnt} in {sys} ({tonnes}) #oWARNING: Invalid Formatting Detected#0"),
         {pnt=null, sys=null, tonnes=_("∞ t")} )
   misn.setTitle( messup( title ) )

   local desc = fmt.f( _("Small shipment of {amount} of {cargo} to {pnt} in the {sys} system."),
         {cargo=null, amount=_("Sol system"), pnt="#y".._("INVALID RECORD").."#0", sys=null} )
   desc = desc.."\n\n"..fmt.f(_("#nCargo:#0 {amount}"),{amount=null})
   desc = desc.."\n#r".._("ERROR: NoncurrentModificationExceptionException: Exception modified while excepting.").."#0"

   misn.setDesc( messup(desc) )
   misn.setReward(_("-∞ ¤"))
end

-- Messes up the text a bit every time the player jumps
local function osd_update ()
   misn.osdCreate( strmess.messup(_("Strange Shipment")), {
      strmess.messup(fmt.f(_("Deliver a small package to {pnt} ({sys} system)"),
      {pnt=destpnt, sys=destsys})),
   })
end

local accepted_tries = 0
function accept ()
   accepted_tries = accepted_tries + 1
   if accepted_tries == 1 then
      vntk.msg(_([[System Error]]),_([[You try to accept the mission, but a bunch of errors pop out, and the mission computer ends up crashing and rebooting. That was weird.]]))

      -- Generate next iteration of the mission
      misn.markerRm()
      misn.markerAdd( destpnt, "computer" )
      local title = _("Shipment to Shipment to Shipment to Shipment to")
      misn.setTitle( strmess.messup( title, 0.1 ) )
      local numjumps   = system.cur():jumpDist( destsys, false )
      local dist = car.calculateDistance( system.cur(), spob.cur():pos(), destsys, destpnt, false )
      local desc = fmt.f(_([[ERROR: BufferOverrua80ho0ajqnc
hq;8eoa 8q0 h
08qj h
2

5 250arcqj0a8eSmall shipment of of of ofof of
to {pnt} in the {sys} system]]),
         {pnt=destpnt, sys=destsys} )
      desc = desc.."\n\n"..fmt.f(_("#nCargo:#0 {amount}"),{amount=_("Small Box")})
      desc = desc.."\n"..fmt.f( n_( "#nJumps:#0 {jumps}", "#nJumps:#0 {jumps}", numjumps ), {jumps=numjumps} )
      desc = desc.."\n".. fmt.f( n_("#nTravel distance:#0 {dist}", "#nTravel distance:#0 {dist}", dist), {dist=fmt.number(dist)} )
      misn.setDesc( strmess.messup( desc, 0.1 ) )
      misn.computerRefresh()
   else

      vntk.msg(_([[Mission Accepted?]]), {
         _([[You try to accept the mission again, but the mission computer seems to lock up. After a while, it starts spamming errors, but it seems like you somehow managed to accept it.]]),
         fmt.f(_([[You return to your ship to find a puzzled dockworker scratching their head. They say it looks like the system is acting weirdly, but since it's not their job to figure it out, they end up handing you a small box. You guess this is the cargo you have to take to {pnt}?]]),
            {pnt=destpnt}),
      })

      misn.accept()
      osd_update()
      hook.enter("enter")
      hook.land("land")

      local c = commodity.new( N_("Small Box"), N_("The box is sealed tight. You think you can hear a faint beeping sound occasionally.") )
      mem.carg_id = misn.cargoAdd( c, 0 )
   end
end

local glitched = false
function enter ()
   osd_update()

   if not glitched then
      hook.timer( 5, "glitch" )
   end
end

local noise_shader, onion_hook, update_hook, onion_gfx, glitch_isworse, nextonion, onions
--local snd_onion
function glitch ()
   -- Want to allow inclusive claims
   if not naev.claimTest( system.cur(), true ) then
      return
   end

   --[[
   snd_onion = audio.new( onion.loops.circus, "stream" )
   snd_onion:setLooping(true)
   snd_onion:play()
   snd_onion:setVolume( 0.2 )
   --]]

   player.autonavReset( 10 )
   noise_shader = pp_shaders.corruption( 0.5 )
   shader.addPPShader( noise_shader, "gui" )
   onion_hook = hook.renderfg( "welcome_to_onion" )
   update_hook = hook.update( "update" )
   onion_gfx = lg.newImage( "onion_society.png" ) -- TODO path
   hook.timer( 5, "glitch_worsen" )
   glitch_isworse = false
   nextonion = 0
   onions = {}
end

function glitch_worsen ()
   --snd_onion:setVolume( 0.2 )

   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 1.0 )
   shader.addPPShader( noise_shader, "gui" )
   hook.timer( 5, "glitch_end" )
   glitch_isworse = true
end

function glitch_end ()
   --snd_onion:stop()

   shader.rmPPShader( noise_shader )
   hook.rm( onion_hook )
   hook.rm( update_hook )
   glitched = true
end

function update( dt )
   -- Update onions
   local newonions = {}
   for k,o in ipairs(onions) do
      o.t = o.t - dt
      if o.t > 0 then
         table.insert( newonions, o )
      end
   end
   onions = newonions

   -- See if we add a new one
   nextonion = nextonion - dt
   if nextonion < 0 then
      nextonion = rnd.rnd()
      local w, h = gfx.dim()
      local x = rnd.rnd(1,w)
      local y = rnd.rnd(1,h)
      local ow, oh = onion_gfx:getDimensions()
      local s = rnd.rnd(50,150) / (0.5*ow+0.5*oh)
      local flip = 1
      if rnd.rnd() > 0.5 then
         flip = -1
      end
      table.insert( onions, {
         x = x,
         y = y,
         s = s,
         flip = flip,
         a = rnd.rnd(),
         t = rnd.rnd(2,5),
      } )
      nextonion = 0.5 + 0.5*rnd.rnd()
      if glitch_isworse then
         nextonion = nextonion * 0.5
      end
   end
end
function welcome_to_onion ()
   for k,o in ipairs(onions) do
      local s = o.s
      lg.setColour( 1, 1, 1, o.a )
      onion_gfx:draw( o.x-s*0.5, o.y-s*0.5, 0, s*o.flip, s )
   end
end

function land ()
   if spob.cur() ~= destpnt then
      return
   end

   -- TODO final mission + cutscene
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[You get off your ship with the small box in hand, and go to deliver it to the spacedock cargo office. However, when you go to pull up the delivery information, it seems to be missing from your computer logs. Puzzled, you hand over the small box anyway, which they proceed to do the routine scan.]]))
   vn.na(_([[The moment the box is scanned, the lights slightly flicker, and you hear the inspector performing some improvised percussive maintenance on the scanning equipment. They give a puzzled look and tell you there's an issue with the system, and it might take a while to get it solved.]]))
   vn.na(_([[With nothing better to do, you walk around the station to kill time.]]))
   -- TODO silly 8-bit music
   --vn.music( onion.loops.circus )
   local oni = onion.vn_onion()
   vn.appear( oni )
   oni(_([[Suddenly, all the lights go off and all the holoscreens throughout the station flash into activity.
"How are you gentlemen?"]]))
   oni(_([["Tsk tsk tsk. Such sloppy security. You should be ashamed of yourselves. "]]))
   oni(_([["Oh, look at all these finance records. Shame if something were to happen to them..."]]))
   oni(_([["Oops, I accidentally crashed the database. They don't make them how they used to. Quite a mess I'm making here. Can't be helped, I'm Ogre."]]))
   oni:rename(_("Ogre"))
   --[=[oni(_([["Anyway, it was fun crashing this place. Lots of interesting things to see. Got to get going. By the way, you guys should do something about the life control systems, it seems like they're going critical. Toodaloo〜♪."]]))--]=]
   -- Probably better to use the Toodaloo with another less script-kiddy one.
   oni(_([["Anyway, it was fun crashing this place. Lots of interesting things to see. Got to get going. By the way, you guys should do something about the life control systems, it seems like they're going critical."]]))
   vn.disappear( oni )

   vne.alarmStart()
   vn.na(_([[The hologram fades and the alarms start blaring, probably indicating that life control systems are critical. People begin to scramble like wild trying to save themselves, and it looks like you're about to get dragged into it.

What do you do?]]))
   vn.menu{
      {_([[Run to your ship]]), "01_docks"},
      {_([[Try to get to the station control room]]), "01_control"},
   }

   vn.label("01_docks")
   vn.na(_([[You follow the flow of people streaming towards the docks, however, the sheer amount of people makes progress slow. Eventually you get stuck in the hallway, with people pressing on all sides. The stench of pure fear is almost unbearable, and you do your best to not suffocate.]]))
   vn.na(_([[Time seems to freeze as the yelling gets louder. You think you feel the oxygen thinning, is this the end? Not going out in a bang, but in a whimper, crushed by humans as the life support system fails?]]))
   vn.jump("01_cont")

   vn.label("01_control")
   vn.na(_([[You try to work yourself to the centre of the station to see if you can somehow solve the issue. You see people all around you trying to run towards the spacedocks, bumping into you and slowing your progress. ]]))
   vn.na(_([[After what seems like an eternity, you manage to make it to the station terminal room. The door is wide open, and you see a frenzy of commotion inside. You try to figure out what is going on, but there is only chaos. ]]))
   vn.na(_([[As you struggle to make sense of the controls and what can be done, you start to have a sinking feeling that it may have not been the best idea to come to the control room.]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   vne.alarmEnd()
   vn.appear( oni )
   vn.na(_([[Suddenly, the alarm goes quiet, lights are restored, and the holograms flash back into life.]]))
   oni(_([["Psyche! But seriously though, your database is toast."]]))
   vn.disappear( oni )
   vn.na(_([[Despite the apparently reassuring message, the chaos continues for quite a while. Eventually, you manage to make it back to your ship, extremely worn out and tired, but at least in one piece. That so-called "Ogre" seems like quite a pain in the ass.]]))
   vn.na(_([[As you let out a big sigh, you suddenly notice you have an unnoticed message. It seems like you have an anonymous transfer to your account.]]))
   vn.sfxMoney()
   vn.func( function ()
      player.pay( money_reward )
   end )
   vn.na(fmt.reward(money_reward))
   vn.run()

   news.add{
      {
         faction = "Generic",
         head = fmt.f(_("Chaos on {spb}"),{spb=destpnt}),
         body = fmt.f(_("{spb} in the {sys} system devolved into chaos on {date} when a hacker who identifies as 'Ogre' broke into the station control system. Local authorities estimate the damages in the order of billions of credits, and have issued a reward for any information that will lead to the capture of the saboteur."), {
            spb = destpnt,
            sys = destsys,
            date = time.get(),
         } ),
         date_to_rm = time.get()+time.new(0,50,0)
      },
   }
   onion.log(fmt.f(_([[You accepted a mysterious and buggy mission from the local mission computer, and delivered a package to {spb} in the {sys} system. To your dismay, this triggered a hack on the station, a lot of chaos, and more stress than is healthy. In the end, you got rewarded for your troubles, but you are not sure it was worth it.]]), {
      spb=destpnt,
      sys=destsys,
   }))

   misn.finish(true)
end
