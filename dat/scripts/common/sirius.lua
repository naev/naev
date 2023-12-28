--[[

   Sirius Common Functions

--]]
local pp_shaders = require "pp_shaders"
local lf = require "love.filesystem"
local audio = require 'love.audio'
local luaspfx = require "luaspfx"

local srs = {}

srs.prefix = "#y".._("SIRIUS: ").."#0"

function srs.playerIsPsychic ()
   return (var.peek("sirius_psychic")==true)
end

function srs.addAcHackLog( text )
   shiplog.create( "achack", _("Academy Hack"), _("Sirius") )
   shiplog.append( "achack", text )
end

function srs.addHereticLog( text )
   shiplog.create( "heretic", _("Heretic"), _("Sirius") )
   shiplog.append( "heretic", text )
end

local sfxGong
function srs.sfxGong()
   if not sfxGong then
      sfxGong = audio.newSource( 'snd/sounds/gamelan_gong.ogg' )
   end
   luaspfx.sfx( false, nil, sfxGong )
end

function srs.weapsets( outfits )
   local pp = player.pilot()
   pp:weapsetCleanup()
   pp:weapsetSetInrange(nil,false)
   for k,o in ipairs(outfits) do
      pp:weapsetType( k, "hold" )
      pp:weapsetAdd( k, o )
   end
   local n = #outfits+1
   pp:weapsetType( n, "switch" )
   pp:weapsetAddType( n, "Bolt Weapon" )
   pp:weapsetSetActive( n )
end

local ssys, sysr, obelisk, spos, sdir, hook_limits
function srs.obeliskEnter( oblk )
   obelisk = oblk
   ssys = system.cur()
   sysr = ssys:radius()
   local pp = player.pilot()
   spos = pp:pos()
   sdir = pp:dir()

   -- Hide rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   ssys:setHidden(false)

   -- Clean up the escorts
   for k,p in ipairs(pp:followers()) do
      if p:flags("carried") then
         p:rm()
      else
         p:setHide( true ) -- Don't remove or it'll mess cargo
      end
   end

   -- Stop and play different music
   music.stop()
   -- TODO sound

   hook_limits = hook.update( "_srs_update_limits" )
end

function srs.obeliskExit ()
   ssys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end

   -- Restore the escorts
   for k,p in ipairs(player.pilot():followers()) do
      p:setHide( false ) -- Don't remove or it'll mess cargo
   end
end

local pixelcode_enter = lf.read( "glsl/love/obelisk_enter.frag" )
local pixelcode_exit = lf.read( "glsl/love/obelisk_exit.frag" )
local shader, endfunc, finalfunc
function srs.obeliskCleanup( func, cleanup )
   endfunc = func
   finalfunc = cleanup
   -- Played backwards from entering
   shader = pp_shaders.newShader( pixelcode_exit )
   shader.addPPShader( shader, "gui" )
   hook.update( "_srs_obelisk_end" )
end

local end_timer = 2.0
local jumped = false
function _srs_obelisk_end( _dt, real_dt )
   end_timer = end_timer - real_dt
   shader:send( "u_progress", end_timer/2.0 )
   if end_timer < 0 then
      if not jumped then
         jumped = true
         end_timer = 2.0
         hook.safe( "_srs_return_obelisk" )
      else
         shader.rmPPShader( shader )
         srs.obeliskExit()
         hook.safe( "_srs_obelisk_cleanup" )
      end
   end
end

function _srs_obelisk_cleanup ()
   if finalfunc then
      finalfunc()
   end
end

function _srs_return_obelisk ()
   local _spb,sys = spob.getS( obelisk )
   hook.rm( hook_limits )
   player.teleport( sys, true, true )
   shader.rmPPShader( shader )
   shader = pp_shaders.newShader( pixelcode_enter )
   shader.addPPShader( shader, "gui" )
   shader:send( "u_progress", 1.0 )
   if endfunc then
      endfunc()
   end
   local pp = player.pilot()
   pp:setDir( sdir )
   pp:setPos( spos )
   pp:setVel( vec2.new() )
   pp:fillAmmo() -- They lose fighters because ship swapping, so at least give ammo back
   srs.sfxGong()
   music.stop()
end

-- Forces the player (and other ships) to stay in the radius of the system
function _srs_update_limits ()
   for k,p in ipairs(pilot.get()) do
      local pos = p:pos()
      local d = pos:dist()
      if d > sysr then
         local _m, dir = pos:polar()
         p:setPos( vec2.newP( sysr, dir ) )
      end
   end
end

return srs
