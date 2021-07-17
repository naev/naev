--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Discovery">
 <trigger>enter</trigger>
 <chance>100</chance>
</event>
--]]
--[[
-- Shows the player fancy messages as they discover things. Meant to be flavourful.
--]]

local love = require 'love'
local lg = require 'love.graphics'
local audio = require 'love.audio'
local love_math = require 'love.math'
local love_shaders = require 'love_shaders'
local transitions = require 'vn.transitions'

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
love.x = 0
love.y = 0
love.w = nw
love.h = nh
lg.origin()

-- These trigger at specific places
system_events = {
   --[[
   -- Unique / Interesting systems
   --]]
   Sol = {
      type = "enter",
      name = "disc_sol",
      title = _("Sol"),
      subtitle = _("Home"),
   },
   ["Gamma Polaris"] = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("Emperor's Wrath"):pos(),
      name = "disc_emperorswrath",
      title = _("Emperor's Wrath"),
      subtitle = _("Human Made Divine"),
   },
   ["Za'lek"] = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("House Za'lek Central Station"):pos(),
      name = "disc_zalekcentral",
      title = _("House Za'lek Central Station"),
      subtitle = _("Bastion of Knowledge"),
   },
   Ruadan = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("Ruadan Prime"):pos(),
      name = "disc_zalekruadan",
      title = _("Ruadan Prime"),
      subtitle = _("New Heart of the Za'lek"),
   },
   Dvaer = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("Dvaered High Command"):pos(),
      name = "disc_dvaeredhigh",
      title = _("Dvaered High Command"),
      subtitle = _("Convening of the Warlords"),
   },
   Feye = {
      type = "enter",
      name = "disc_kataka",
      title = _("Feye"),
      subtitle = _("Remembering Sorom"),
   },
   Aesir = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("Mutris"):pos(),
      name = "disc_mutris",
      title = _("Crater City"),
      subtitle = _("Touching the Universe"),
   },
   Taiomi = {
      type = "enter",
      name = "disc_taiomi",
      title = _("Taiomi"),
      subtitle = _("Ship Graveyard"),
   },
   Limbo = {
      -- Discover will not work if the planet is found through maps
      --type = "discover",
      --asset = planet.get("Minerva Station"),
      type = "distance",
      dist = 5000,
      pos  = planet.get("Minerva Station"):pos(),
      name = "disc_minerva",
      title = _("Minerva Station"),
      subtitle = _("Gambler's Paradise"),
   },
   Beeklo = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("Totoran"):pos(),
      name = "disc_totoran",
      title = _("Totoran"),
      subtitle = _("Brave your Fate in the #rCrimson Gauntlet#0"),
   },
   Haven = {
      type = "enter",
      name = "disc_haven",
      title = _("The Devastation of Haven"),
      subtitle = _("The Old Wound That Never Heals"),
   },
   --[[
   -- Pirate Strongholds
   -- ]]
   ["New Haven"] = {
      type = "distance",
      dist = 5000,
      pos  = planet.get("New Haven"):pos(),
      name = "disc_newhaven",
      title = _("New Haven"),
      subtitle = _("They Will Never Destroy Us"),
   },
   Kretogg = {
      type = "enter",
      name = "disc_kretogg",
      title = _("Kretogg"),
      subtitle = _("Any Business is Good Business"),
   },
}
-- These trigger for specific factions controlled systems
faction_events = {
   ["Za'lek"] = {
      type = "enter",
      name = "disc_zalek",
      title = _("The Za'lek Territories"),
      subtitle = _("Knowledge at All Costs"),
   },
   Dvaered = {
      type = "enter",
      name = "disc_dvaered",
      title = _("The Dvaered Territories"),
      subtitle = _("The Warlords are Eager for Blood"),
   },
   Soromid = {
      type = "enter",
      name = "disc_soromid",
      title = _("The Soromid Territories"),
      subtitle = _("Human is Just an Ephemeral Condition"),
   },
   Sirius = {
      type = "enter",
      name = "disc_sirius",
      title = _("The Sirius Territories"),
      subtitle = _("Sirichana Will Lead the Way"),
   },
   Proteron = {
      type = "enter",
      name = "disc_proteron",
      title = _("The Proteron Territories"),
      subtitle = _("United through Sacrifice"),
      func = function() faction.get("Proteron"):setKnown( true ) end
   },
   Thurion = {
      type = "enter",
      name = "disc_thurion",
      title = _("The Thurion Space"),
      subtitle = _("We Shall All Become One"),
   },
   Frontier = {
      type = "enter",
      name = "disc_frontier",
      title = _("The Frontier"),
      subtitle = _("Leading to a New Future"),
   },
   Collective = {
      type = "enter",
      name = "disc_collective",
      title = _("The Collective"),
      subtitle = _("Do Robots Dream of Electric Sheep?"),
      func = function() faction.get("Collective"):setKnown( true ) end
   },
}
-- Custom events can handle custom triggers such as nebula systems
local function test_systems( syslist )
   local n = system.cur():nameRaw()
   for k,v in ipairs(syslist) do
      if v==n then
         return true
      end
   end
   return false
end
custom_events = {
   Nebula = {
      test = function ()
         -- These are currently the only systems from which the player can
         -- enter the nebula
         local nsys = {
            "Thirty Stars",
            "Raelid",
            "Toaxis",
            "Myad",
            "Tormulex",
         }
         return test_systems( nsys )
      end,
      type = "enter",
      name = "disc_nebula",
      title = _("The Nebula"),
      subtitle = _("Grim Reminder of Our Fragility"),
   },
   NorthWinds = {
      test = function ()
         local nsys = {
            "Defa",
            "Vedalus",
            "Titus",
            "New Haven",
            "Daled",
            "Mason",
         }
         return test_systems( nsys )
      end,
      type = "enter",
      name = "disc_northwinds",
      title = _("Northern Solar Winds"),
      --subtitle = _("None"),
   },
   SouthWinds = {
      test = function ()
         local nsys = {
            "Kretogg",
            "Unicorn",
            "Volus",
            "Sheffield",
            "Gold",
            "Fried",
         }
         return test_systems( nsys )
      end,
      type = "enter",
      name = "disc_southwinds",
      title = _("Southern Solar Winds"),
      --subtitle = _("None"),
   },
}

function sfxDiscovery()
   --sfx = audio.newSource( 'snd/sounds/jingles/success.ogg' )
   sfx = audio.newSource( 'snd/sounds/jingles/victory.ogg' )
   sfx:play()
end

function handle_event( event )
   -- Don't trigger if already done
   if var.peek( event.name ) then return false end

   -- Trigger
   if event.type=="enter" then
      discover_trigger( event )
   elseif event.type=="discover" then
      hook.discover( "discovered", event )
   elseif event.type=="distance" then
      hook.timer( 500, "heartbeat", event )
   end
   return true
end

function create()
   local sc = system.cur()
   local event = system_events[ sc:nameRaw() ]
   local hasevent = false
   if event then
      hasevent = hasevent or handle_event( event )
   end
   local sf = sc:faction()
   local event = sf and faction_events[ sf:nameRaw() ] or false
   if event then
      hasevent = hasevent or handle_event( event )
   end
   for k,v in pairs(custom_events) do
      if not var.peek( v.name ) and v.test() then
         hasevent = hasevent or handle_event( v )
      end
   end

   -- Nothing triggered
   if not hasevent then
      endevent()
   end

   -- Ends when player lands or leaves either way
   hook.enter("endevent")
   hook.land("endevent")
end
function endevent () evt.finish() end
function discovered( type, discovery, event )
   if event.asset and type=="asset" and discovery==event.asset then
      discover_trigger( event )
   end
end
function heartbeat( event )
   local dist = player.pilot():pos():dist( event.pos )
   if dist < event.dist then
      discover_trigger( event )
   else
      hook.timer( 500, "heartbeat", event )
   end
end


function discover_trigger( event )
   local msg  = string.format(_("You found #o%s!"),event.title)
   -- Log and message
   player.msg( msg )
   local logid = shiplog.create( "discovery", _("Travel Log"), _("Discovery") )
   shiplog.append( "discovery", msg )

   -- Break autonav
   player.autonavReset( 5 )

   -- If custom function, run it
   if event.func then
      event.func()
   end

   -- Mark as done
   var.push( event.name, true )

   -- Play sound and show message
   sfxDiscovery()
   textinit( event.title, event.subtitle )
end

text_fadein = 1.5
text_fadeout = 1.5
text_length = 8.0
function textinit( titletext, subtitletext )
   local fontname = _("fonts/CormorantUnicase-Medium.ttf")
   -- Title
   title = { text=titletext, h=48 }
   title.font = lg.newFont( title.h )
   --title.font = lg.newFont( fontname, title.h )
   title.font:setOutline(3)
   title.w = title.font:getWidth( title.text )
   -- Subtitle
   if subtitletext then
      subtitle = { text=subtitletext, h=32 }
      subtitle.font = lg.newFont( subtitle.h )
      --subtitle.font = lg.newFont( fontname, subtitle.h )
      subtitle.font:setOutline(2)
      subtitle.w = subtitle.font:getWidth( subtitle.text )
   end

   local oldcanvas = lg.getCanvas()
   local emptycanvas = lg.newCanvas()
   lg.setCanvas( emptycanvas )
   lg.clear( 0, 0, 0, 0 )
   lg.setCanvas( oldcanvas )

   -- TODO probably rewrite the shader as this is being computed with the full
   -- screen resolution, breaks with all transitions that use love_ScreenSize...
   textshader  = transitions.get( "perlin" )
   textshader:send( "texprev", emptycanvas )
   textshader._emptycanvas = emptycanvas
   texttimer   = 0

   -- Render to canvas
   local pixelcode = string.format([[
precision highp float;

#include "lib/simplex.glsl"

const float u_r = %f;
const float u_sharp = %f;

float vignette( vec2 uv )
{
   uv *= 1.0 - uv.yx;
   float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
   vig = pow(vig, 0.5); // change pow for modifying the extend of the  vignette
   return vig;
}

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolor = color * texture( tex, uv );

   float n = 0.0;
   for (float i=1.0; i<8.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( px * u_sharp * 0.003 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolor *= 0.4*n+0.8;
   texcolor.a *= vignette( uv );
   texcolor.rgb *= 0.0;

   return texcolor;
}
]], love_math.random(), 3 )
   local shader = lg.newShader( pixelcode, love_shaders.vertexcode )
   local w, h
   if subtitle then
      w = math.max( title.w, subtitle.w )*1.5
      h = (title.h * 1.5 + subtitle.h)*2
   else
      w = title.w*1.5
      h = title.h*2
   end
   textcanvas = love_shaders.shader2canvas( shader, w, h )

   lg.setCanvas( textcanvas )
   title.x = (w-title.w)/2
   title.y = h*0.2
   lg.print( title.text, title.font, title.x, title.y )
   if subtitle then
      subtitle.x = (w-subtitle.w)/2
      subtitle.y = title.y + title.h*1.5
      lg.print( subtitle.text, subtitle.font, subtitle.x, subtitle.y )
   end
   lg.setCanvas()

   hook.renderfg( "textfg" )
   hook.update( "textupdate" )
   --hook.timer( text_length*1000, "endevent")
end
function textfg ()
   local progress
   if texttimer < text_fadein then
      progress = texttimer / text_fadein
   elseif texttimer > text_length-text_fadeout then
      progress = (text_length-texttimer) / text_fadeout
   end

   if progress then
      lg.setShader( textshader )
      textshader:send( "progress", progress )
   end

   lg.setColor( 1, 1, 1, 1 )

   local x = (love.w-textcanvas.w)*0.5
   local y = (love.h-textcanvas.h)*0.3
   x = math.floor(x)
   y = math.floor(y)
   lg.draw( textcanvas, x, y )
   if progress then
      lg.setShader()
   end

   -- Horrible hack, but since the canvas is being scaled by Naev's scale stuff,
   -- we actually draw pretty text ontop using the signed distance transform shader
   -- when it is fully drawn
   if not progress then
      lg.print( title.text, title.font, x+title.x, y+title.y )
      if subtitle then
         lg.print( subtitle.text, subtitle.font, x+subtitle.x, y+subtitle.y )
      end
   end
end
function textupdate( dt, real_dt )
   -- We want to show it regardless of the time compression and such
   texttimer = texttimer + real_dt
end
