--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 01">
 <unique />
 <priority>0</priority>
 <chance>10</chance>
 <location>Computer</location>
 <cond>
   if spob.get("Gordon's Exchange"):system():jumpDist() < 4 then
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
local onion = require "common.onion"

local destpnt, destsys = spob.getS("Gordon's Exchange")

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
      vntk.msg(_([[You try to accept the mission, but a bunch of errors pop out, and the mission computer ends up crashing and rebooting. That was weird.]]))

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

      vntk.msg({
         _([[You try to accept the mission again, but the mission computer seems to lock up. After a while, it starts spamming errors, but it seems like you somehow managed to accept it.]]),
         fmt.f(_([[You return to your ship to find a puzzled dockworker scratching his head. They say it looks like the system is acting weirdly, but since it's not their job to figure it out, they end up handing you a small box. You guess this is the cargo you have to take to {pnt}?]]),
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
function glitch ()
   -- Want to allow inclusive claims
   if not naev.claimTest( system.cur(), true ) then
      return
   end

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
   shader.rmPPShader( noise_shader )
   noise_shader = pp_shaders.corruption( 1.0 )
   shader.addPPShader( noise_shader, "gui" )
   hook.timer( 5, "glitch_end" )
   glitch_isworse = true
end

function glitch_end ()
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
      table.insert( onions, {
         x = x,
         y = y,
         s = rnd.rnd(50,150) / (0.5*ow+0.5*oh),
         a = rnd.rnd(),
         t = rnd.rnd(2,5),
      } )
      nextonion = rnd.rnd(1,3)
      if glitch_isworse then
         nextonion = nextonion * 0.5
      end
   end
end
function welcome_to_onion ()
   for k,o in ipairs(onions) do
      local s = o.s
      lg.setColour( 1, 1, 1, o.a )
      onion_gfx:draw( o.x-s*0.5, o.y-s*0.5, 0, s, s )
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
   vn.na(_([[The moment the box is scanned, the lights slightly flicker, and you hear the inspector performing some improvized percussive maintenance on the scanning equipment. They give a puzzled look and tell you there's an issue with the system, and it might take a while to get it solved.]]))
   vn.na(_([[With nothing better to do, you walk around the station to kill time.]]))
   vn.func( function () music.stop() end )
   local oni = onion.vn_onion()
   vn.appear( oni )
   oni(_([[]]))
   vn.disappear( oni )
   vn.run()
end
