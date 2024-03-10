--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Stealing ships">
 <priority>4</priority>
 <chance>10</chance>
 <location>Bar</location>
 <cond>faction.playerStanding("Pirate") &gt;= -20 and not player.misnActive("Stealing ships")</cond>
 <faction>Wild Ones</faction>
 <faction>Black Lotus</faction>
 <faction>Raven Clan</faction>
 <faction>Dreamer Clan</faction>
 <faction>Pirate</faction>
 <notes>
  <tier>2</tier>
 </notes>
 <tags>
  <tag>pir_cap_ch01_lrg</tag>
 </tags>
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
local lmisn = require "lmisn"
local equipopt = require "equipopt"
local vn = require "vn"
local vni = require "vnimage"
local vntk = require "vntk"

local base_price = 100e3

local faction_tags = {
   Dvaered = "dvaered",
   Empire = "empire",
   Frontier = "frontier",
   Goddard = "goddard",
   Independent = "standard",
   Sirius = "sirius",
   Soromid = "soromid",
   ["Za'lek"] = "zalek",
}
local ships = {}
for i, ship in ipairs(ship.getAll()) do
   local tags = ship:tags()
   if not tags["noplayer"] and not tags["nosteal"] then
      for fctname, tag in pairs(faction_tags) do
         if tags[tag] then
            ships[fctname] = ships[fctname] or {}
            table.insert( ships[fctname], ship )
         end
      end
   end
end

local function price(size)
   local modifier = 1.5^(size-3)
   modifier = math.floor(4*modifier) / 4 -- rounder numbers
   return modifier * base_price
end

local function random_ship(fct)
   local l = ships[fct:nameRaw()]
   if not l then
      return
   end
   return l[ rnd.rnd(1,#l) ]
end

local function random_planet()
   local planets = {}
   local maximum_distance = 6
   local minimum_distance = 0

   lmisn.getSysAtDistance(
      system.cur(),
      minimum_distance, maximum_distance,

      function(s)
         for i, v in ipairs(s:spobs()) do
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
local function improve_standing(size, fct)
   local enemies = fct:enemies()
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

local function damage_standing( size, fct )
   local base = 2^(size-2)
   local modifier = 1

   -- “Oh dude, that guy is capable! He managed to steal one of our own ships!”
   if pir.factionIsPirate( fct ) then
      return
   end

   if fct == faction.get("Independent") or fct == faction.get("Frontier") then
      modifier = 0.5
   end

   fct:modPlayerSingle( -base * modifier )
end

local pir_portrait, pir_image
function create ()
   mem.reward_faction = pir.systemClanP( system.cur() )
   mem.planet  = random_planet()
   mem.system = mem.planet:system()

   if not mem.planet or mem.planet:faction() == nil then
      -- If we’re here, it means we couldn’t get a planet close enough.
      misn.finish(false)
   end

   mem.faction = mem.planet:faction()
   mem.ship = random_ship(mem.faction)

   if not mem.ship then
      -- If we’re here, it means we couldn’t get a ship of the right faction.
      misn.finish(false)
   end

   mem.price = price(mem.ship:size())

   pir_image, pir_portrait = vni.pirate()
   misn.setNPC( _("A Pirate informer"), pir_portrait, _("A pirate informer is looking at you. Maybe they have some useful information to sell?") )
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local p = vn.newCharacter( _("Pirate Informer"), {image=pir_image} )
   vn.transition()

   p(fmt.f(_([["Hi, pilot. I have the location and security codes of an unattended {fct} {class}. Maybe it interests you, who knows?
"However, I'm going to sell that information only. It'd cost you {credits}, but the ship is probably worth much more if you can get to it."
Do you want to pay to know where that ship is?]]),
      {fct=mem.faction, class=_(mem.ship:class()), credits=fmt.credits(mem.price)}))
   vn.menu{
      {fmt.f(_([[Pay {credits}]]),{credits=fmt.credits(mem.price)}), "pay"},
      {_([[Leave]]), "leave"},
   }

   vn.label("leave")
   vn.done()

   vn.label("broke")
   p(_([["Do you take me for a fool? Get out of here! Come back when you have enough money."]]) )
   vn.done()

   vn.label("pay")
   vn.func( function ()
      if player.credits() < mem.price then
         vn.jump("broke")
         return
      end
      player.pay( -mem.price )
      accepted = true
   end )
   p(fmt.f(_([[You pay the informer, who tells you the ship in currently on {pnt}, in the {sys} system. He also gives you its security codes and warns you about patrols.
The pile of information he gives you also contains a way to land on the planet and to dissimulate your ship there.]]),
      {pnt=mem.planet, sys=mem.system}))

   vn.run()

   if not accepted then return end

   misn.accept()

   local title = fmt.f(_("Stealing a {class}"), {class=_(mem.ship:class())} )
   local description = fmt.f( _("Land on {pnt} in the {sys} system and escape with your new {class}"),
   {pnt=mem.planet, sys=mem.system, class=_(mem.ship:class())} )
   misn.setTitle( title )
   misn.setReward( fmt.f(_("A brand new {class}"), {class=_(mem.ship:class())} ) )
   misn.setDesc( description )

   -- Mission marker
   misn.markerAdd( mem.system, "low" )

   -- OSD
   misn.osdCreate( title, {description})

   hook.land("land")
   hook.enter("enter")
end

function land()
   local landed, landsys = spob.cur()
   if landed == mem.planet then
      -- Try to swap ships
      local tmp = pilot.add( mem.ship, "Independent", nil, nil, {naked=true} )
      equipopt.generic( tmp )
      if not swapship.swap( tmp, fmt.f(_("You acquired the ship through illegitimate means at {pnt} in the {sys} system. Yarr!"),{pnt=landed, sys=landsys}) ) then
         -- Failed to swap ship!
         vntk.msg( _("Ship left alone!"), _("Before you make it into the ship and take control of it, you realize you are not ready to deal with the logistics of moving your cargo over. You decide to leave the ship stealing business for later.") )
         tmp:rm() -- Get rid of the temporary pilot
         return
      end

      -- Oh yeah, we stole the ship. \o/
      vntk.msg(
         _("Ship successfully stolen!"),
         fmt.f(
            _([[It took you a while, but you finally make it into the ship and take control of it with the access codes you were given. Hopefully, you will be able to sell this {ship}, or maybe even to use it.
Enemy ships will probably be after you as soon as you leave the atmosphere, so you should get ready and use the time you have on this planet wisely.]]),
            {ship=mem.ship}
         )
      )

      -- Hey, stealing a ship isn’t anything! (if you survive, that is)
      faction.modPlayerSingle( mem.reward_faction, rnd.rnd(3,5) )

      -- Let’s keep a counter. Just in case we want to know how many you
      -- stole in the future.
      local stolen_ships = var.peek("pir_stolen_ships") or 0
      var.push("pir_stolen_ships", stolen_ships + 1)

      -- If you stole a ship of some value, the faction will have something
      -- to say, even if they can only suspect you.
      damage_standing(mem.ship:size(), mem.faction)

      -- This is a success. The player stole his new ship, and everyone is
      -- happy with it. Getting out of the system alive is the player’s
      -- responsibility, now.
      misn.finish(true)
   end
end

function enter()
   -- A few faction ships guard the target planet.
   if system.cur() == mem.system then
      -- We want the player to be able to land on the destination planet…
      mem.planet:landAllow(true)
   end
end
