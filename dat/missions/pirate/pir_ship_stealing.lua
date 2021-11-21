--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Stealing ships">
 <avail>
  <priority>4</priority>
  <chance>10</chance>
  <location>Bar</location>
  <cond>faction.playerStanding("Pirate") &gt;= -20</cond>
  <faction>Wild Ones</faction>
  <faction>Black Lotus</faction>
  <faction>Raven Clan</faction>
  <faction>Dreamer Clan</faction>
  <faction>Pirate</faction>
 </avail>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   The player pays a fellow pirate to know where to steal a random ship.

   The player pays to get the position of a ship on a random planet of a random
   faction. When he gets there, the planet is guarded (which means he may have
   to fight his way through, which is the most probable option).

   When he lands on the target planet, he gets a nice message explaining what
   happens, he gets a new ship, is able to refit it, etc.

   Then, when the player wants to leave the planet, and that will eventually
   happen (at least, I hope…) he’ll be pursued by a few fighters.
--]]
local pir = require "common.pirate"
local swapship = require "swapship"
local fmt = require "format"
local portrait = require "portrait"
local lmisn = require "lmisn"
require "factions.equip.generic"

-- luacheck: globals enter land (Hook functions passed by name)
-- luacheck: globals equip_generic (From "factions.equip.generic")

local base_price = 100e3

local ships = {
   Dvaered = {
      fighter   = { "Dvaered Vendetta" },
      bomber    = { "Dvaered Ancestor" },
      corvette  = { "Dvaered Phalanx" },
      destroyer = { "Dvaered Vigilance" },
      carrier   = { "Dvaered Arsenal" },
      battleship= { "Dvaered Goddard" },
   },
   Empire = {
      interceptor={ "Empire Shark" },
      fighter   = { "Empire Lancelot" },
      corvette  = { "Empire Admonisher" },
      destroyer = { "Empire Pacifier" },
      cruiser   = { "Empire Hawking" },
      carrier   = { "Empire Peacemaker", "Empire Rainmaker" },
   },
   Frontier = {
      interceptor={ "Hyena" },
      fighter   = { "Lancelot", "Vendetta" },
      bomber    = { "Ancestor" },
      corvette  = { "Phalanx" },
      destroyer = { "Pacifier" },
   },
   Goddard = {
      fighter   = { "Lancelot" },
      battleship= { "Goddard" },
   },
   Independent = {
      interceptor={ "Hyena", "Shark" },
      fighter   = { "Lancelot", "Vendetta" },
      bomber    = { "Ancestor" },
      corvette  = { "Phalanx", "Admonisher" },
      destroyer = { "Vigilance", "Pacifier" },
      cruiser   = { "Kestrel", "Hawking" },
   },
   Sirius = {
      interceptor={ "Sirius Fidelity" },
      bomber    = { "Sirius Shaman" },
      corvette  = { "Sirius Preacher" },
      battleship= { "Sirius Dogma" },
      carrier   = { "Sirius Divinity" },
   },
   Soromid = {
      interceptor={ "Soromid Brigand" },
      fighter   = { "Soromid Reaver" },
      bomber    = { "Soromid Marauder" },
      corvette  = { "Soromid Odium" },
      destroyer = { "Soromid Nyx" },
      cruiser   = { "Soromid Ira" },
      battleship= { "Soromid Vox" },
      carrier   = { "Soromid Arx" },
   },
   ["Za'lek"] = {
      corvette  = { "Za'lek Sting" },
      destroyer = { "Za'lek Demon" },
      cruiser   = { "Za'lek Mephisto" },
      carrier   = { "Za'lek Diablo" },
   },
}

local classes = {}
for k,v in pairs(ships) do
   classes[k] = {}

   for k2,v2 in pairs(v) do
      classes[k][#classes[k]+1] = k2
   end
end

local function price(size)
   local modifier = 1.5^(size-3)
   modifier = math.floor(4*modifier) / 4 -- rounder numbers
   return modifier * base_price
end

local function random_class(faction)
   local m = #classes[faction]

   if m == 0 then
      return
   end

   local r = rnd.rnd(1, m)

   return classes[faction][r]
end

local function random_ship(faction, class)
   local m = #ships[faction][class]

   if m == 0 then
      return
   end

   local r = rnd.rnd(1, m)

   return ship.get(ships[faction][class][r])
end

local function random_planet()
   local planets = {}
   local maximum_distance = 6
   local minimum_distance = 0

   lmisn.getSysAtDistance(
      system.cur(),
      minimum_distance, maximum_distance,

      function(s)
         for i, v in ipairs(s:planets()) do
            local f = v:faction()
            if f and ships[f:nameRaw()] and v:services().shipyard then
               planets[#planets + 1] = v
            end
         end
         return false
      end, nil, true )

   if #planets > 0 then
      return planets[rnd.rnd(1,#planets)]
   else
      return
   end
end

--[[
local function improve_standing(size, faction_name)
   local enemies = faction.get(faction_name):enemies()
   local standing = math.max( size - 2, 0 )

   for i = 1,#enemies do
      local enemy = enemies[i]
      local current_standing = faction.playerStanding(enemy)
      if current_standing + standing > 5 then
         -- Never more than 5.
         standing = math.max(0, current_standing - standing)
      end
      faction.modPlayerSingle(enemy, standing)
   end
end
]]

local function damage_standing(size, faction_name)
   local base = 2^(size-2)
   local modifier = 1

   -- “Oh dude, that guy is capable! He managed to steal one of our own ships!”
   if faction_name == "Pirate" then
      return
   end

   if faction_name == "Independent" or faction_name == "Frontier" then
      modifier = 0.5
   end

   faction.modPlayerSingle(faction_name, -base * modifier)
end

function create ()
   mem.theship = {}
   mem.reward_faction = pir.systemClanP( system.cur() )
   mem.theship.planet  = random_planet()

   if not mem.theship.planet or mem.theship.planet:faction() == nil then
      -- If we’re here, it means we couldn’t get a planet close enough.
      misn.finish(false)
   end

   mem.theship.faction = mem.theship.planet:faction():nameRaw()
   mem.theship.class   = random_class(mem.theship.faction)

   if not mem.theship.class then
      -- If we’re here, it means we couldn’t get a ship of the right faction
      -- and of the right class.
      misn.finish(false)
   end

   -- We’re assuming ships[faction][class] is not empty, here…
   mem.theship.ship = random_ship(mem.theship.faction, mem.theship.class)
   mem.theship.size = mem.theship.ship:size()
   mem.theship.price   = price(mem.theship.size)

   mem.theship.system = mem.theship.planet:system()

   misn.setNPC( _("A Pirate informer"), portrait.get("Pirate"), _("A pirate informer is looking at you. Maybe they have some useful information to sell?") )
end

function accept()
   if tk.yesno( _("Ship to steal"), fmt.f(_([["Hi, pilot. I have the location and security codes of an unattended {fct} {class}. Maybe it interests you, who knows?
    "However, I'm going to sell that information only. It'd cost you {credits}, but the ship is probably worth much more if you can get to it."
    Do you want to pay to know where that ship is?]]), {
         fct=_(mem.theship.faction), class=_(mem.theship.class), credits=fmt.credits(mem.theship.price)} ) ) then
      if player.credits() >= mem.theship.price then
         tk.msg( _("Of course"), fmt.f(_([[You pay the informer, who tells you the ship in currently on {pnt}, in the {sys} system. He also gives you its security codes and warns you about patrols.
    The pile of information he gives you also contains a way to land on the planet and to dissimulate your ship there.]]), {
            pnt=mem.theship.planet, sys=mem.theship.system} ) )

         player.pay( -mem.theship.price )
         misn.accept()

	 local title = fmt.f(_("Stealing a {class}"), {class=_(mem.theship.class)} )
	 local description = fmt.f( _("Land on {pnt} in the {sys} system and escape with your new {class}"),
                                    {pnt=mem.theship.planet, sys=mem.theship.system, class=_(mem.theship.class)} )
         misn.setTitle( title )
         misn.setReward( fmt.f(_("A brand new {class}"), {class=_(mem.theship.class)} ) )
         misn.setDesc( description )

         -- Mission marker
         misn.markerAdd( mem.theship.system, "low" )

         -- OSD
         misn.osdCreate( title, {description})

         hook.land("land")
         hook.enter("enter")
      else
         tk.msg( _("Not Enough Money"), _([["Do you take me for a fool? Get out of here! Come back when you have enough money."]]) )
         misn.finish()
      end
   else
      -- Why would we care?
      misn.finish()
   end
end

function land()
   local landed = planet.cur()
   if landed == mem.theship.planet then
      -- Try to swap ships
      local tmp = pilot.add( mem.theship.ship, "Independent" )
      equip_generic( tmp )
      if not swapship.swap( tmp ) then
         -- Failed to swap ship!
         tk.msg( _("Ship left alone!"), _("Before you make it into the ship and take control of it, you realize you are not ready to deal with the logistics of moving your cargo over. You decide to leave the ship stealing business for later.") )
         tmp:rm() -- Get rid of the temporary pilot
         return
      end

      -- Oh yeah, we stole the ship. \o/
      tk.msg(
         _("Ship successfully stolen!"),
         fmt.f(
            _([[It took you a while, but you finally make it into the ship and take control of it with the access codes you were given. Hopefully, you will be able to sell this {ship}, or maybe even to use it.
    Enemy ships will probably be after you as soon as you leave the atmosphere, so you should get ready and use the time you have on this planet wisely.]]),
            {ship=mem.theship.ship}
         )
      )

      -- Hey, stealing a ship isn’t anything! (if you survive, that is)
      faction.modPlayerSingle( mem.reward_faction, rnd.rnd(3,5) )

      -- Let’s keep a counter. Just in case we want to know how many you
      -- stole in the future.
      local stolen_ships = var.peek("pir_stolen_ships") or 0
      var.push("pir_stolen_ships", stolen_ships + 1)

      -- Stealing a ship for the first time increases your maximum faction
      -- standing.
      if stolen_ships == 0 then
         var.push("_fcap_pirate", var.peek("_fcap_pirate") + 5)
      end

      -- If you stole a ship of some value, the faction will have something
      -- to say, even if they can only suspect you.
      damage_standing(mem.theship.size, mem.theship.faction)

      -- This is a success. The player stole his new ship, and everyone is
      -- happy with it. Getting out of the system alive is the player’s
      -- responsibility, now.
      misn.finish(true)
   end
end

function enter()
   -- A few faction ships guard the target planet.
   if system.cur() == mem.theship.system then
      -- We want the player to be able to land on the destination planet…
      mem.theship.planet:landOverride(true)
   end
end

