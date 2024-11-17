--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Discovery">
 <location>enter</location>
 <chance>100</chance>
</event>
--]]
--[[
-- Shows the player fancy messages as they discover things. Meant to be flavourful.
--]]

local fmt = require 'format'
local audio = require 'love.audio'
local textoverlay = require "textoverlay"


-- These trigger at specific places
local system_events = {
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
      dist = 5e3,
      pos  = spob.get("Emperor's Wrath"):pos(),
      name = "disc_emperorswrath",
      title = _("Emperor's Wrath"),
      subtitle = _("Human Made Divine"),
   },
   ["Za'lek"] = {
      type = "distance",
      dist = 5e3,
      pos  = spob.get("House Za'lek Central"):pos(),
      name = "disc_zalekcentral",
      title = _("House Za'lek Central"),
      subtitle = _("Bastion of Knowledge"),
   },
   Ruadan = {
      type = "distance",
      dist = 5e3,
      pos  = spob.get("Ruadan Prime"):pos(),
      name = "disc_zalekruadan",
      title = _("Ruadan Prime"),
      subtitle = _("New Heart of the Za'lek"),
   },
   Dvaer = {
      type = "distance",
      dist = 5e3,
      pos  = spob.get("Dvaered High Command"):pos(),
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
      dist = 5e3,
      pos  = spob.get("Mutris"):pos(),
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
      --asset = spob.get("Minerva Station"),
      type = "distance",
      dist = 5e3,
      pos  = spob.get("Minerva Station"):pos(),
      name = "disc_minerva",
      title = _("Minerva Station"),
      subtitle = _("Gambler's Paradise"),
   },
   Beeklo = {
      type = "distance",
      dist = 5e3,
      pos  = spob.get("Totoran"):pos(),
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
      dist = 5e3,
      pos  = spob.get("New Haven"):pos(),
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
local faction_events = {
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
      func = function() faction.get("FLF"):setKnown( true ) end
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
--[[
local function test_systems( syslist )
   local n = system.cur():nameRaw()
   for k,v in ipairs(syslist) do
      if v==n then
         return true
      end
   end
   return false
end
--]]
local function pir_discovery( fname, disc, subtitle )
   return {
      test = function ()
         local p = system.cur():presences()[ fname ]
         return (p and p>0)
      end,
      type = "enter",
      name = disc,
      title = "#H"..fmt.f(_("{fname} Territory"),{fname=_(fname)}).."#0",
      subtitle = "#H"..subtitle.."#0",
      func = function()
         local fpir = faction.get(fname)
         fpir:setKnown( true )
         for k,p in ipairs( pilot.get( {fpir}, true ) ) do
            if p:name() == _("Unknown") then
               p:rename( p:ship():name() )
            end
         end
      end,
   }
end

local custom_events = {
   Nebula = {
      test = function ()
         return system.cur():tags().thenebula ~= nil
      end,
      type = "enter",
      name = "disc_nebula",
      title = _("The Nebula"),
      subtitle = _("Grim Reminder of Our Fragility"),
   },
   NorthWinds = {
      test = function ()
         return system.cur():tags().northstellarwind ~= nil
      end,
      type = "enter",
      name = "disc_northwinds",
      title = "#b".._("Northern Stellar Winds").."#0",
      --subtitle = _("None"),
   },
   SouthWinds = {
      test = function ()
         return system.cur():tags().southstellarwind ~= nil
      end,
      type = "enter",
      name = "disc_southwinds",
      title = "#b".._("Southern Stellar Winds").."#0",
      --subtitle = _("None"),
   },
   Haze = {
      test = function ()
         return system.cur():tags().haze ~= nil
      end,
      type = "enter",
      name = "disc_haze",
      title = "#r".._("The Haze").."#0",
      --subtitle = _("None"),
   },
   --[[PlasmaStorm = {
      test = function ()
         return system.cur():tags().plasmastorm ~= nil
      end,
      type = "enter",
      name = "disc_plasmastorm",
      title = _("Sirii Plasma Storm"),
      --subtitle = _("None"),
   },--]]
   BlackHole = {
      test = function ()
         return system.cur():background() == "blackhole"
      end,
      type = "enter",
      name = "disc_blackhole",
      title = _("Anubis Black Hole"),
      subtitle = _("Gaping Maw of the Abyss"),
   },
   WildOnes = pir_discovery( "Wild Ones", "disc_wildones", _("Uncontrolled and Raging Pirate Fury") ),
   RavenClan = pir_discovery( "Raven Clan", "disc_ravenclan", _("Dark Hand of the Black Market") ),
   BlackLotus = pir_discovery( "Black Lotus", "disc_blacklotus", _("Piracy has never been Snazzier") ),
   DreamerClan = pir_discovery( "Dreamer Clan", "disc_dreamerclan", _("Piracy to Rebel against Reality") ),
}

local sfx, discover_trigger -- function forward-declaration

local function sfxDiscovery()
   --sfx = audio.newSource( 'snd/sounds/jingles/success.ogg' )
   sfx = audio.newSource( 'snd/sounds/jingles/victory.ogg' )
   sfx:play()
end

local triggered = false
local function handle_event( event )
   -- Don't trigger if already done
   if var.peek( event.name ) then return false end

   -- Trigger
   if event.type=="enter" then
      if not triggered then
         discover_trigger( event )
         triggered = true
      end
   elseif event.type=="discover" then
      hook.discover( "discovered", event )
   elseif event.type=="distance" then
      hook.timer( 0.5, "heartbeat", event )
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
   event = sf and faction_events[ sf:nameRaw() ] or false
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
      hook.timer( 0.5, "heartbeat", event )
   end
end

function discover_trigger( event )
   -- Don't trigger stuff in cinematics (looking at your chapter 1)
   if player.cinematicsCheck() then
      return
   end

   local template = (event.subtitle and _("You found #o{title} - {subtitle}#0!")) or _("You found #o{title}#0!")
   local msg = fmt.f(template, event)
   -- Log and message
   player.msg( msg )
   shiplog.create( "discovery", _("Discovery"), _("Travel") )
   shiplog.append( "discovery", msg )

   -- Break autonav
   player.autonavReset( 3 )

   -- If custom function, run it
   if event.func then
      event.func()
   end

   -- Mark as done
   var.push( event.name, true )

   -- Play sound and show message
   sfxDiscovery()
   textoverlay.init( event.title, event.subtitle )
end
