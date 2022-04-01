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
--local fmt = require 'format'
local lg = require 'love.graphics'

local diff_progress1 = "hypergates_1"
local diff_progress2 = "hypergates_2"

-- luacheck: globals land fadein fadeout foreground update cutscene00 cutscene01 cutscene02 cutscene03 cutscene_zlk cutscene_srm cutscene_srs cutscene_dvr cutscene08 cutscene09 cutscene10 cutscene11 cutscene_cleanup (Hook functions passed by name)

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

      hook.safe( "cutscene00" )
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
      fg.w = fg.font:getWidth( fg.text )
      fg.h = fontsize

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
         lg.print( fg.text, fg.font, fg.x, fg.y )
      end
   end
end

function update( _dt, real_dt )
   fg.alpha = fg.alpha + fg.alpha_dir * 2 * real_dt
end

-- Set up the cutscene stuff
local origsys
function cutscene00 ()
   setHide( true )
   local pp = player.pilot()
   pp:setNoJump(true)
   pp:setNoLand(true)

   pilot.clear()
   pilot.toggleSpawn(false)

   -- TODO music

   -- Get the Empire hypergate
   local hyp, hyps = spob.getS( "Hypergate Gamma Polaris" )
   origsys = system.cur()
   player.teleport( hyps )
   camera.set( hyp:pos(), true )

   fg_setup( _("And they built bridges across the stars…") )
   --hook.timer( 5, "cutscene01" )
   hook.timer( 5, "cutscene_zlk" )
end

function cutscene01 ()
   -- Show system
   fg_setup()
   hook.timer( 10, "chapter02" )
   fadein()
end

function cutscene02 ()
   -- Activate hypergate
   hook.timer( 5, "chapter03" )
end

function cutscene03 ()
   -- Ship jumps
   hook.timer( 9.3, "fadeout" )
   hook.timer( 10, "chapter_zlk" )
end

local function pangate( gatename )
   -- Go to the hypergate and pan camera
   local hyp, hyps = spob.getS( gatename )
   player.teleport( hyps, true )
   local dir = vec2.newP( 1, rnd.angle() )
   local pos = hyp:pos()
   camera.set( pos - 500*dir, true )
   camera.set( pos + 500*dir, false, 1000 / 5 )
end

function cutscene_zlk () -- Za'lek
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
   hook.timer( 5, "cutscene08" )
end

function cutscene08 ()
   -- Final text
   fg_setup( _("…unwittingly closing the distance to that which could destroy them…") )

   -- TODO something more omnious
   pangate( "Hypergate Polaris" )

   hook.timer( 4.3, "fadein" )
   hook.timer( 5, "cutscene09" )
end

function cutscene09 ()
   fadein()

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene10" )
end

function cutscene10 ()
   -- Return to system and restore camera
   player.teleport( origsys )
   camera.set( nil, true )
   fadein()
   hook.timer( 2, "cutscene_cleanup" )
end

function cutscene11 ()
   -- Chapter 1 message
--[[
_("CHAPTER 1")
_("The Hypergates Awaken")
--]]
end

-- Cleans up the cutscene stuf
function cutscene_cleanup ()
   local pp = player.pilot()
   pp:setNoJump(false)
   pp:setNoLand(false)
   setHide( false )

   -- Initialize fleet capacity
   player.setFleetCapacity( 100 )

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
