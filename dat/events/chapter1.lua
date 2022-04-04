--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Chapter 1">
 <flags>
  <unique />
 </flags>
 <trigger>enter</trigger>
 <chance>100</chance>
 <chapter>0</chapter>
</event>
--]]
--[[
   Event that triggrs a cutscene and the chapter 1 transitions.
--]]
local tut = require 'common.tutorial'
local vn  = require 'vn'
local fmt = require 'format'
local lg = require 'love.graphics'
local textoverlay = require "textoverlay"

local diff_progress1 = "hypergates_1"
local diff_progress2 = "hypergates_2"

-- luacheck: globals land fadein fadeout foreground update cutscene_start cutscene_emp1 cutscene_emp2 cutscene_emp3 cutscene_emp4 cutscene_emp5 cutscene_emp6 cutscene_emp3 cutscene_zlk cutscene_srm cutscene_srs cutscene_dvr cutscene_posttext cutscene_nebu cutscene_nebu_fade cutscene_cleanup cutscene_shipai (Hook functions passed by name)

function create ()
   evt.finish(false) -- disabled for now

   -- Set up some variables
   local has_license = diff.isApplied("heavy_combat_vessel_license") or (player.numOutfit("Heavy Combat Vessel License") > 0)
   local traded_total = var.peek("hypconst_traded_total") or 0

   -- Compute some sort of progress value
   local progress = traded_total * 100 / 2000 -- Would need 2000 to trigger the change by this itself
   for k,m in ipairs(player.misnDoneList()) do
      progress = progress + 100 / 25
   end
   if has_license then
      progress = progress + 50
   end

   -- Determine what to do
   if progress >= 100 then
      -- Make sure system isn't claimed, but we don't claim it
      if not evt.claim( system.cur(), true ) then evt.finish(false) end

      hook.safe( "cutscene_start" )
      return -- Don't finish

   elseif progress >= 50 then
      if not diff.isApplied( diff_progress1 ) then
         diff.apply( diff_progress1 )
      end

   end

   -- Finish the event, until next time :)
   evt.finish(false)
end

local function setHide( state )
   local pp = player.pilot()
   pp:setHide( state )
   pp:setInvincible( state )
   for k,p in ipairs(pp:followers()) do
      p:setHide( state )
      p:setInvincible( state )
   end
end

local fg, nw, nh
local function fg_setup( text )
   local fontsize = 32

   if not fg then
      fg = {}
      fg.font = lg.newFont( fontsize )
      fg.font:setOutline(3)
      fg.hook = hook.rendertop( "foreground" )
      fg.update = hook.update( "update" )

      fg.alpha = 1
      fg.alpha_dir = 1

      nw, nh = gfx.dim()
   end

   fg.text = text
   if fg.text then
      fg.w, fg.wrapped = fg.font:getWrap( fg.text, 0.6 * nw )
      fg.h = fg.font:getLineHeight() * #fg.wrapped

      fg.x = (nw-fg.w)/2
      fg.y = (nh-fg.h)/2
   end
end

-- Fades out to black
function fadeout ()
   fg.alpha_dir = 1
   fg.alpha = math.max( 0, fg.alpha )
end

-- Fades in from black
function fadein ()
   fg.alpha_dir = -1
   fg.alpha = math.min( 1, fg.alpha )
end

function foreground ()
   if fg.alpha > 0 then
      lg.setColor( 0, 0, 0, fg.alpha )
      lg.rectangle( "fill", 0, 0, nw, nh )

      if fg.text then
         lg.setColor( 1, 1, 1, fg.alpha )
         lg.printf( fg.text, fg.font, fg.x, fg.y, 0.6*nw, "center" )
      end
   end
end

function update( _dt, real_dt )
   fg.alpha = fg.alpha + fg.alpha_dir * 2 * real_dt
end

-- Set up the cutscene stuff
local origsys, empboss, emptester
function cutscene_start ()
   player.canDiscover( false )
   setHide( true )
   player.cinematics( true )
   local pp = player.pilot()
   pp:setNoJump(true)
   pp:setNoLand(true)

   pilot.clear()
   pilot.toggleSpawn(false)

   -- TODO music

   -- Get the Empire hypergate
   local hyp, hyps = spob.getS( "Hypergate Gamma Polaris" )
   origsys = system.cur()
   player.teleport( hyps, false, true )
   camera.set( hyp:pos(), true )

   -- Add some guys
   local pos = hyp:pos()
   local function addship( shipname )
      local p = pilot.add( shipname, "Empire", pos + vec2.newP( 200+100*rnd.rnd(), rnd.angle() ) )
      p:control()
      p:face( pos )
      return p
   end
   empboss = addship( "Empire Peacemaker" )
   addship( "Empire Hawking" )
   addship( "Empire Hawking" )
   emptester = addship( "Empire Pacifier" )
   addship( "Empire Pacifier" )

   fg_setup( _("And they built bridges across the stars…") )
   hook.timer( 5, "cutscene_emp1" )
   --hook.timer( 5, "cutscene_zlk" )
end

function cutscene_emp1 ()
   -- Show system
   fg_setup()
   hook.timer( 3, "cutscene_emp2" )
   fadein()
end

local countdown
function cutscene_emp2 ()
   empboss:broadcast( _("Beginning activation countdown!") )
   countdown = 5
   hook.timer( 7, "cutscene_emp3" )
end

-- Countdown
function cutscene_emp3 ()
   empboss:broadcast( fmt.f(_("{countdown}…"), {countdown=countdown}) )
   if countdown > 1 then
      countdown = countdown-1
      hook.timer( 1, "cutscene_emp3" )
   else
      hook.timer( 1, "cutscene_emp4" )
   end
end

function cutscene_emp4 ()
   -- Activate hypergate with big flash

   -- TODO post-process flash shader
   hook.timer( 5, "cutscene_emp5" )
end

function cutscene_emp5 ()
   hook.timer( 5, "cutscene_emp6" )
   -- TODO ship goes through jump
   emptester:taskClear()
   local hyp = spob.get( "Hypergate Gamma Polaris" )
   emptester:land( hyp )
end

function cutscene_emp6 ()
   -- Ship jumps
   hook.timer( 9.3, "fadeout" )
   hook.timer( 10, "cutscene_zlk" )
end

local function pangate( gatename )
   -- Go to the hypergate and pan camera
   local hyp, hyps = spob.getS( gatename )
   player.teleport( hyps, true, false, true )
   local dir = vec2.newP( 1, rnd.angle() )
   local pos = hyp:pos()
   camera.set( pos - 500*dir, true )
   camera.set( pos + 500*dir, false, 1000 / 5 )
end

function cutscene_zlk () -- Za'lek
   if diff.isApplied( diff_progress1 ) then
      diff.remove( diff_progress1 )
   end
   diff.apply( diff_progress2 )

   pangate( "Hypergate Ruadan" )
   fg_setup()
   fadein()

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene_srm" )
end

function cutscene_srm () -- Soromid
   pangate( "Hypergate Feye" )
   fadein()

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene_srs" )
end

function cutscene_srs () -- Sirius
   pangate( "Hypergate Kiwi" )
   fadein()

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene_dvr" )
end

function cutscene_dvr ()
   pangate( "Hypergate Dvaer" )
   fadein()

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene_posttext" )
end

function cutscene_posttext ()
   -- Final text
   fg_setup( _("…unwittingly closing the distance to that which could destroy them…") )

   -- Increase visibility
   player.pilot():intrinsicSet( "nebu_visibility", 1000 )

   -- TODO something more omnious
   local gatename = "Hypergate Polaris"
   pangate( gatename )
   local hyp = spob.get( gatename )
   camera.set( hyp:pos(), true )
   camera.setZoom( 1, true )
   camera.setZoom( 3, false )

   hook.timer( 4.3, "fadein" )
   hook.timer( 5, "cutscene_nebu" )
end

function cutscene_nebu ()
   fg_setup() -- Remove text
   camera.setZoom() -- Reset zoom

   -- TODO omnious music

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene_nebu_fade" )
end

function cutscene_nebu_fade ()
   -- Rsetore visibility
   player.pilot():intrinsicSet( "nebu_visibility", -1000 )

   -- Return to system and restore camera
   player.teleport( origsys, false, true )
   camera.set( nil, true )
   player.cinematics( false )
   fadein()
   hook.timer( 2, "cutscene_cleanup" )
end

-- Cleans up the cutscene stuf
function cutscene_cleanup ()
   setHide( false )

   -- Chapter 1 message
   textoverlay.init( _("CHAPTER 1"), _("The Hypergates Awaken") )

   -- Initialize fleet capacity
   player.setFleetCapacity( 100 )
   player.chapterSet( "1" )
   player.canDiscover( true )

   hook.timer( 10, "cutscene_shipai" )
end

function cutscene_shipai ()
   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(_([[Your ship AI suddenly materializes infront of you.]]))
   sai(_([[""]]))
   vn.done( tut.shipai.transition )
   vn.run()

   local pp = player.pilot()
   pp:setNoJump(false)
   pp:setNoLand(false)

   -- Have ship ai talk when landed
   hook.land("land")
end

function land ()
   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(_([[""]]))
   vn.done( tut.shipai.transition )
   vn.run()
end
