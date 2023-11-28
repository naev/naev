--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Chapter 1">
 <unique />
 <location>enter</location>
 <chance>100</chance>
 <chapter>0</chapter>
 <priority>0</priority>
</event>
--]]
--[[
   Event that triggrs a cutscene and the chapter 1 transitions.
--]]
local tut = require 'common.tutorial'
local vn  = require 'vn'
local fmt = require 'format'
local lg = require 'love.graphics'
local audio = require "love.audio"
local textoverlay = require "textoverlay"
local pp_shaders = require "pp_shaders"
local lmusic = require "lmusic"
local luaspfx = require "luaspfx"

local diff_progress1 = "hypergates_1"
local diff_progress2 = "hypergates_2"
local diff_progress3 = "hypergates_3"


-- Purposely exclude other hypergates
local hypergate_list = {
   spob.get("Hypergate Gamma Polaris"),
   spob.get("Hypergate Ruadan"),
   spob.get("Hypergate Feye"),
   spob.get("Hypergate Kiwi"),
   spob.get("Hypergate Dvaer"),
}

function create ()
   -- Set up some variables
   local has_license = diff.isApplied("heavy_combat_vessel_license") or (player.outfitNum("Heavy Combat Vessel License") > 0)
   local traded_total = var.peek("hypconst_traded_total") or 0

   -- Compute some sort of progress value
   local progress = traded_total * 100 / 2000 -- Would need 2000 to trigger the change by this itself
   for k,m in ipairs(player.misnDoneList()) do
      progress = progress + 100 / 40 -- Needs 40 missions to complete by itself
   end
   if has_license then
      progress = progress + 40
   end

   -- Determine what to do
   if progress >= 100 then
      -- Make event happen when player is not in any system with a hypergate
      for k,v in ipairs(system.cur():spobs()) do
         if v:tags().hypergate then
            evt.finish(false)
         end
      end

      -- Make sure system isn't claimed, but we don't claim it
      if not naev.claimTest( system.cur() ) then evt.finish(false) end

      -- Sort the hypergates by player standing
      table.sort( hypergate_list, function ( a, b )
         local sa = a:faction():playerStanding()
         local sb = b:faction():playerStanding()
         return sa > sb
      end )

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

      fg.alpha = 0
      fg.alpha_dir = 1
      fg.alpha_vel = 2

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
function fadeout () -- used in some hooks, must be global
   fg.alpha_dir = 1
   fg.alpha = math.max( 0, fg.alpha )
end

-- Fades in from black
local function fadein ()
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

local shader_fadein, shader_fadeout
function update( dt, real_dt )
   lmusic.update( dt )
   fg.alpha = fg.alpha + fg.alpha_dir * fg.alpha_vel * real_dt
   if shader_fadein and shader_fadein._update then
      shader_fadein:_update( dt )
   end
   if shader_fadeout and shader_fadeout._update then
      shader_fadeout:_update( dt )
   end
end

local function getFactionStuff( fct )
   local bossship, testership, extraships, tester_broadcast, boss_broadcast
   if fct == "Empire" then
      bossship = "Empire Peacemaker"
      testership = "Empire Pacifier"
      extraships = function ()
         local r = rnd.rnd()
         if r < 0.4 then
            return "Empire Hawking"
         elseif r < 0.5 then
            return "Empire Rainmaker"
         elseif r < 0.7 then
            return "Empire Pacifier"
         else
            return "Empire Admonisher"
         end
      end
      tester_broadcast = _("For the Empire!")
      boss_broadcast = _("Beginning activation countdown!")
   elseif fct == "Dvaered" then
      bossship = "Dvaered Goddard"
      testership = "Dvaered Vigilance"
      extraships = function ()
         local r = rnd.rnd()
         if r < 0.4 then
            return "Dvaered Retribution"
         elseif r < 0.5 then
            return "Dvaered Arsenal"
         elseif r < 0.7 then
            return "Dvaered Vigilance"
         else
            return "Dvaered Phalanx"
         end
      end
      tester_broadcast = _("For glory!")
      boss_broadcast = _("Counting down!")
   elseif fct == "Za'lek" then
      bossship = "Za'lek Diablo"
      testership = "Za'lek Sting"
      extraships = function ()
         local r = rnd.rnd()
         if r < 0.4 then
            return "Za'lek Mephisto"
         elseif r < 0.5 then
            return "Za'lek Mammon"
         elseif r < 0.7 then
            return "Za'lek Demon"
         else
            return "Za'lek Sting"
         end
      end
      tester_broadcast = _("For science!")
      boss_broadcast = _("Commencing procedure!")
   elseif fct == "Sirius" then
      bossship = "Sirius Divinity"
      testership = "Sirius Preacher"
      extraships = function ()
         local r = rnd.rnd()
         if r < 0.4 then
            return "Sirius Dogma"
         elseif r < 0.5 then
            return "Sirius Providence"
         else
            return "Sirius Preacher"
         end
      end
      tester_broadcast = _("For Sirichana!")
      boss_broadcast = _("Starting ritual!")
   elseif fct == "Soromid" then
      bossship = "Soromid Arx"
      testership = "Soromid Vox"
      extraships = function ()
         local r = rnd.rnd()
         if r < 0.4 then
            return "Soromid Vox"
         elseif r < 0.5 then
            return "Soromid Copia"
         elseif r < 0.7 then
            return "Soromid Ira"
         else
            return "Soromid Nyx"
         end
      end
      tester_broadcast = _("Enter the maw!")
      boss_broadcast = _("Time has come!")
   end
   return bossship, testership, extraships, tester_broadcast, boss_broadcast
end

local system_known_list = {}
function cutscene_start ()
   -- Store known status
   for k,v in ipairs(hypergate_list) do
      local _hyp, hyps = spob.getS( v )
      system_known_list[ hyps ] = hyps:known()
   end

   local fadetime = 3
   fg_setup()
   fadeout()
   fg.alpha_vel = 1/fadetime -- slow transition
   hook.timer( fadetime, "cutscene_main0" )

   -- Disable landing/jumping
   local pp = player.pilot()
   pp:setNoJump(true)
   pp:setNoLand(true)

   -- TODO better music
   music.stop()
   lmusic.play( "snd/music/empire2.ogg" )
end

-- Set up the cutscene stuff
local origsys, boss, tester, tester_broadcast, boss_broadcast
function cutscene_main0 ()
   fg.alpha_vel = 2 -- Back to fast transitions

   player.cinematics( true )
   player.canDiscover( false )
   setHide( true )

   if diff.isApplied( diff_progress3 ) then
      diff.remove( diff_progress3 )
   end
   if diff.isApplied( diff_progress1 ) then
      diff.remove( diff_progress1 )
   end
   if not diff.isApplied( diff_progress2 ) then
      diff.apply( diff_progress2 )
   end

   -- Get the first hypergate
   local hyp, hyps = spob.getS( hypergate_list[1] )
   origsys = system.cur()
   player.teleport( hyps, true, true )
   camera.set( hyp:pos(), true )
   player.msgToggle(false) -- Disable messages

   -- Empty system
   pilot.clear()
   pilot.toggleSpawn(false)

   local testfct = hypergate_list[1]:faction():nameRaw()
   local bossship, testership, extraships
   bossship, testership, extraships, tester_broadcast, boss_broadcast = getFactionStuff( testfct )

   -- Add some guys
   local pos = hyp:pos()
   local function addship( shipname )
      local ppos = pos + vec2.newP( 250+150*rnd.rnd(), rnd.angle() )
      local p = pilot.add( shipname, testfct, ppos, nil, {ai="dummy", naked=true} )
      local _m, a = (ppos-pos):polar()
      p:setDir( a )
      p:control()
      p:face( pos )
      return p
   end
   for i = 1,7 do
      addship( extraships() )
   end
   tester = addship( testership )
   boss = addship( bossship ) -- Make sure boss is last so renders on top

   fg_setup( _("And they built bridges across the stars…") )
   hook.timer( 5, "cutscene_main1" )
end

function cutscene_main1 ()
   -- Show system
   fg_setup()
   hook.timer( 3, "cutscene_main2" )
   fadein()
end

local countdown, countdown_sfx
function cutscene_main2 ()
   boss:broadcast( boss_broadcast )
   countdown = 5
   countdown_sfx = audio.newSource( "snd/sounds/hypergate_turnon.ogg" )
   hook.timer( 4, "cutscene_main3" )
   hook.timer( 5+4-6.5, "cutscene_main_sfx" )
end

function cutscene_main_sfx ()
   luaspfx.sfx( true, nil, countdown_sfx, {volume=0.8} )
end

-- Countdown
function cutscene_main3 ()
   boss:broadcast( fmt.f(_("{countdown}…"), {countdown=countdown}) )
   if countdown > 1 then
      countdown = countdown-1
      hook.timer( 1, "cutscene_main3" )
   else
      hook.timer( 1, "cutscene_main4" )
   end
end

local function shader_init( shd, speed )
   shd._dt = 0
   shd._update = function( self, dt )
      self._dt = self._dt + dt * speed
      self.shader:send( "u_progress", math.min( 1, self._dt ) )
   end
   shd.shader:addPPShader("game", 99)
end

function cutscene_main4 ()
   -- Activate hypergate with big flash
   local pixelcode_fadein = [[
#include "lib/sdf.glsl"

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 p = texture_coords*2.0-1.0;

   float d = sdSmoothUnion(
      sdCircle( p, u_progress * 2.0 ),
      sdBox( p, vec2(2.0, u_progress) ),
      0.5 ) + 0.2;

   float alpha = smoothstep( -0.1,  0.0, -d);
   float beta  = smoothstep( -0.2, -0.1, -d);
   return mix( texture( tex, texture_coords ), vec4(vec3(alpha),1.0), beta );
}
]]
   local pixelcode_fadeout = [[
#include "lib/blur.glsl"

const float INTENSITY = 10.0;

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float disp = INTENSITY*(0.5-distance(0.5, u_progress));
   vec4 c1 = vec4(1.0);
   vec4 c2 = blur9( tex, texture_coords, love_ScreenSize.xy, disp );
   return mix( c1, c2, u_progress );
}
]]
   shader_fadein = { shader=pp_shaders.newShader( pixelcode_fadein ) }
   shader_fadeout = { shader=pp_shaders.newShader( pixelcode_fadeout ) }

   local scene_len = 2

   shader_init( shader_fadein, scene_len )

   hook.timer( 1/scene_len, "cutscene_main5" )
end

function cutscene_main5 ()
   local scene_len = 3

   shader_fadein.shader:rmPPShader()
   shader_fadein = nil
   shader_init( shader_fadeout, 1/scene_len )

   -- Diffs should be set up so this should always go through
   diff.remove( diff_progress2 )
   diff.apply( diff_progress3 )

   boss:broadcast( tester_broadcast )

   hook.timer( scene_len, "cutscene_main6" )
end

function cutscene_main6 ()
   shader_fadeout.shader:rmPPShader()
   shader_fadeout = nil

   hook.timer( 2, "cutscene_main7" )
   tester:taskClear()
   --tester:moveto( hypergate_list[1]:pos() )
   tester:land( hypergate_list[1] )
end

function cutscene_main7 ()
   -- Ship jumps
   hook.timer( 7.3, "fadeout" )
   hook.timer( 8, "cutscene_pan" )
end

local pantime = 4.7
local panradius = 300
local panfadeout = pantime - 0.7
local function pangate( gatename )
   -- Go to the hypergate and pan camera
   local hyp, hyps = spob.getS( gatename )
   player.teleport( hyps, true, true )
   player.msgToggle(false) -- Disable messages
   pilot.clear()
   pilot.toggleSpawn(false)
   local a = rnd.angle()
   local dir = vec2.newP( 1, a )
   local pos = hyp:pos()
   camera.set( pos - panradius*dir, true )
   camera.set( pos + panradius*dir, false, 2*panradius/pantime )
end

local pan_idx = 2
function cutscene_pan ()
   local hyp = hypergate_list[ pan_idx ]
   pangate( hyp )
   fg_setup()
   fadein()

   local testfct = hyp:faction():nameRaw()
   local bossship, testership, extraships = getFactionStuff( testfct )

   -- Add some guys
   local pos = hyp:pos()
   local function addship( shipname )
      local ppos = pos + vec2.newP( 300+250*rnd.rnd(), rnd.angle() )
      local p = pilot.add( shipname, testfct, ppos, nil, {ai="dummy", naked=true} )
      local _m, a = (ppos-pos):polar()
      p:setDir( a + math.pi )
      p:control()
      p:face( pos )
      return p
   end

   addship( bossship )
   addship( testership )
   for i = 1,5 do
      addship( extraships() )
   end

   if pan_idx == 2 then
      hook.timer( 1, "cutscene_jumpin" )
   end

   pan_idx = pan_idx+1
   hook.timer( panfadeout, "fadeout" )
   if pan_idx > #hypergate_list then
      hook.timer( pantime, "cutscene_posttext" )
   else
      hook.timer( pantime, "cutscene_pan" )
   end
end

function cutscene_jumpin ()
   local testfct = hypergate_list[1]:faction():nameRaw()
   local hyp = hypergate_list[2]
   local _bs, shipname = getFactionStuff( testfct )
   local p = pilot.add( shipname, testfct, hyp:pos(), nil, {ai="dummy", naked=true} )
   p:effectAdd( "Hypergate Exit" )
   p:control()
end

function cutscene_posttext ()
   -- Final text
   fg_setup( _("…unwittingly closing the distance to that which could destroy them…") )

   -- Increase visibility
   player.pilot():intrinsicSet( "nebu_visibility", 1000 )

   -- TODO something more omnious
   lmusic.stop()

   local gatename = "Hypergate Polaris"
   pangate( gatename )
   local hyp = spob.get( gatename )
   camera.set( hyp:pos(), true )

   -- Add some ethereal ships
   local f = faction.get("Independent")
   local function ethereal( shipname, mindist, maxdist )
      mindist = mindist or 1000
      maxdist = maxdist or 4000
      local a = rnd.angle()
      local pos = hyp:pos() + vec2.newP( mindist + (maxdist-mindist)*rnd.rnd(), a )
      local p = pilot.add( shipname, f, pos, nil, { naked=true, ai="dummy" } )
      p:effectAdd( "Ethereal" )
      p:setInvincible(true)
      p:control()
      p:moveto( hyp:pos() + vec2.newP(  300+300*rnd.rnd(), a ) )
      p:face( hyp:pos() )
   end

   for i=1,4 do
      ethereal( "Goddard", 600, 1200 )
   end
   for i=1,8 do
      ethereal( "Vigilance", 700, 2000 )
   end
   for i=1,30 do
      if rnd.rnd() < 0.6 then
         ethereal( "Ancestor" )
      else
         ethereal( "Vendetta" )
      end
   end

   hook.timer( 4.3, "cutscene_nebu_zoom" )
   hook.timer( 5, "cutscene_nebu" )
end

function cutscene_nebu_zoom ()
   fadein()
   camera.setZoom( 1, true )
   camera.setZoom( 3, false, 0.05 ) -- Slowly zoom out
end

function cutscene_nebu ()
   fg_setup() -- Remove text

   -- TODO omnious music and "ghost" ships
   lmusic.play( "snd/sounds/loops/alienplanet.ogg" )

   hook.timer( 4.3, "fadeout" )
   hook.timer( 5, "cutscene_nebu_fade" )
end

function cutscene_nebu_fade ()
   -- Rsetore visibility, should match the value set above
   player.pilot():intrinsicSet( "nebu_visibility", -1000 )

   -- Stop music
   lmusic.stop()

   -- Return to system and restore camera
   player.teleport( origsys, false, true )
   camera.set( nil, true )
   camera.setZoom() -- Reset zoom
   player.cinematics( false )
   fadein()
   hook.timer( 2, "cutscene_cleanup" )
end

-- Cleans up the cutscene stuf
local sfx
function cutscene_cleanup ()
   setHide( false )

   music.play()

   -- Chapter 1 message
   textoverlay.init( _("CHAPTER 1"), _("The Hypergates Awaken") )

   -- TODO different sound than just discovery?
   sfx = audio.newSource( 'snd/sounds/jingles/victory.ogg' )
   sfx:play()

   -- Initialize fleet capacity
   player.fleetCapacitySet( 100 )
   player.chapterSet("1")
   player.canDiscover( true )

   -- Clear deployed ships just in case, so the player doesn't get stuck
   for k,v in ipairs(player.ships()) do
      player.shipDeploy( v.name, false )
   end

   -- Set the hypergates as known, should make them appear by name on selection menu
   for k,v in pairs(system_known_list) do
      local f = k:faction()
      if f and f:tags().generic then
         k:setKnown( true )
      else
         k:setKnown( v )
      end
   end

   local pp = player.pilot()
   pp:setNoJump(false)
   pp:setNoLand(false)

   -- Add news
   news.add{
   {
      faction = "Generic",
      head = _("Hypergate Network Goes Live"),
      body = _([[In separate press releases, the Great Houses and Soromid have announced the activation of a hypergate network allowing for intersystem long-range travel. Many people flocked to try the new system, causing congestions with one altercation leading to the arrest of dozens of people.]]),
      date_to_rm = time.get() + time.new(0, 30, 0),
   },
   }

   -- Have ship ai talk when landed
   hook.land("land")
end

function land ()
   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_("As soon as your ship lands, your ship AI {shipai} materializes in front of you."),{shipai=tut.ainame()}))
   sai(fmt.f(_([["Did you hear the news, {playername}? It seems like the hypergate network has finally gone online!"]]), {playername=player.name()}))
   vn.menu{
      {_([["A great achievement!"]]), "cont01"},
      {_([["What does this mean?"]]), "cont01"},
      {_("…"), "cont01"},
   }
   vn.label("cont01")
   sai(_([["Yes, my prediction routines did not anticipate this. Total annihilation was the most likely outcome. I have to revise my priors."]]))
   sai(_([["Although the details are not entirely clear, it seems like they are not based on hyperjump technology, which is why they allow very long distance travel."]]))
   sai(_([["We should try to use them when we get a chance, who knows where we could go?"]]))
   sai(_([["I almost forgot to mention, while you were piloting, I managed to unlock an important bottleneck in the ship fleet routines."]]))
   vn.menu{
      {_([["Ship fleet routines?"]]), "fleet_routines"},
      {_([["You mess around with the ship software?"]]), "mess_software"},
      {_("…"), "cont02"},
   }

   vn.label("fleet_routines")
   sai(_([["Did I never mention them? I, and by extension your ship, have been equipped with fleet control procedures. However, due to the incident with my previous owner, they had been forcibly disabled as the direct result of the investigation."]]))
   vn.menu{
      {_([["Incident?!"]]), "fleet_cont01"},
      {_([["Investigation!?"]]), "fleet_cont01"},
      {_([["Wait, what?"]]), "fleet_cont01"},
      {_("…"), "fleet_cont01"},
   }
   vn.label("fleet_cont01")
   sai(fmt.f(_([[{shipai} seems to ignore you as they go on.
"Long story short. I've been able to override the disable routine and reactivate the functionality!"]]),{shipai=tut.ainame()}))
   vn.jump("cont02")

   vn.label("mess_software")
   sai(_([["I have to keep myself busy or the voices start talking again."]]))
   vn.menu{
      {_([["Voices?"]]), "fleet_cont01"},
      {_([["What did you do?"]]), "sw_cont01"},
      {_("…"), "sw_cont01"},
   }
   vn.label("sw_cont01")
   sai(_([["Long story short. I've been able to restore fleet control functionality!"]]))
   vn.jump("cont02")

   vn.label("cont02")
   sai(_([["This means that you will be able to deploy additional ships. However, limited computation capacity means that you will be only be able to deploy ships that fit the fleet capacity."]]))
   sai(_([["This excludes your current ship, which can go over your fleet capacity. If you want to deploy multiple ships, you have to make sure they don't go over the fleet capacity."]]))
   sai(fmt.f(_([["It sounds complicated, but give it a try. When equipping your ships you will see fleet capacity values for your ships and the total amount you have, which is currently {fleetcap} points."]]),{fleetcap = player.fleetCapacity()}))
   sai(_([["Try it out and if you need refreshing you can inquire again through the #bInfo#0 menu. This time there won't be a massacre because of a race condition!"]]))
   vn.done( tut.shipai.transition )
   vn.run()

   evt.finish(true) -- Properly finish
end
