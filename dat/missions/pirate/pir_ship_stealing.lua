--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Stealing ships">
  <avail>
   <priority>4</priority>
   <chance>10</chance>
   <location>Bar</location>
   <cond>faction.playerStanding("Pirate") &gt;= 0 or planet.cur():faction() == faction.get("Pirate")</cond>
  </avail>
  <notes>
   <tier>2</tier>
  </notes>
 </mission>
 --]]
--[[

-- The player pays a fellow pirate to know where to steal a random ship.

   The player pays to get the position of a ship on a random planet of a random
   faction. When he gets there, the planet is guarded (which means he may have
   to fight his way through, which is the most probable option).

   When he lands on the target planet, he gets a nice message explaining what
   happens, he gets a new ship, is able to refit it, etc.

   Then, when the player wants to leave the planet, and that will eventually
   happen (at least, I hope…) he’ll be pursued by a few fighters.
--]]

require "swapship.lua"
require "jumpdist.lua"
require "numstring.lua"
require "portrait.lua"
require "factions/equip/generic.lua"


title = _("Stealing a %s")
reward = _("A brand new %s")
description = _("Land on %s in the %s system and escape with your new %s")

-- localization stuff, translators would work here
informer = {
   description = _("A pirate informer is looking at you. Maybe they have some useful information to sell?"),
   title = _("Ship to steal"),
   message = _([["Hi, pilot. I have the location and security codes of an unattended %s %s. Maybe it interests you, who knows?
    "However, I'm going to sell that information only. It'd cost you %s, but the ship is probably worth much more if you can get to it."
    Do you want to pay to know where that ship is?]])
}

approval = {
   title = _("Of course"),
   message = _([[You pay the informer, who tells you the ship in currently on %s, in the %s system. He also gives you its security codes and warns you about patrols.
    The pile of information he gives you also contains a way to land on the planet and to dissimulate your ship there.]])
}

not_enough_title = _("Not Enough Money")
not_enough_msg = _([["Do you take me for a fool? Get out of here! Come back when you have enough money."]])

success = {
   title = _("Ship successfully stolen!"),
   message = _([[It took you a while, but you finally make it into the ship and take control of it with the access codes you were given. Hopefully, you will be able to sell this %s, or maybe even to use it.
    Enemy ships will probably be after you as soon as you leave the atmosphere, so you should get ready and use the time you have on this planet wisely.]])
}

unableto = {
   title = _("Ship left alone!"),
   message = _("Before you make it into the ship and take control of it, you realize you are not ready to deal with the logistics of moving your cargo over. You decide to leave the ship stealing business for later.")
}

base_price = 100000

ships = {
   Dvaered = {
      fighter   = { "Dvaered Vendetta" },
      bomber    = { "Dvaered Ancestor" },
      corvette  = { "Dvaered Phalanx" },
      destroyer = { "Dvaered Vigilance" },
      cruiser   = { "Dvaered Goddard" },
   },
   Empire = {
      fighter   = { "Empire Shark", "Empire Lancelot" },
      corvette  = { "Empire Admonisher" },
      destroyer = { "Empire Pacifier" },
      cruiser   = { "Empire Hawking" },
      carrier   = { "Empire Peacemaker" },
   },
   Frontier = {
      fighter   = { "Lancelot", "Vendetta", "Hyena" },
      bomber    = { "Ancestor" },
      corvette  = { "Phalanx" },
      destroyer = { "Pacifier" },
   },
   Goddard = {
      fighter   = { "Lancelot" },
      cruiser   = { "Goddard" },
   },
   Independent = {
      fighter   = { "Lancelot", "Vendetta", "Hyena", "Shark" },
      bomber    = { "Ancestor" },
      corvette  = { "Phalanx", "Admonisher" },
      destroyer = { "Vigilance", "Pacifier" },
      cruiser   = { "Kestrel", "Hawking" },
   },
   Sirius = {
      fighter   = { "Sirius Fidelity" },
      bomber    = { "Sirius Shaman" },
      corvette  = { "Sirius Preacher" },
      cruiser   = { "Sirius Dogma" },
      carrier   = { "Sirius Divinity" },
   },
   Soromid = {
      fighter   = { "Soromid Brigand", "Soromid Reaver" },
      bomber    = { "Soromid Marauder" },
      corvette  = { "Soromid Odium" },
      destroyer = { "Soromid Nyx" },
      cruiser   = { "Soromid Ira", "Soromid Vox" },
      carrier   = { "Soromid Arx" },
   },
   ["Za'lek"] = {
      corvette  = { "Za'lek Imp", "Za'lek Sting" },
      destroyer = { "Za'lek Demon" },
      cruiser   = { "Za'lek Mephisto", "Za'lek Prototype" },
      carrier   = { "Za'lek Diablo", "Za'lek Hephaestus" },
   },
}

classes = {}

for k,v in pairs(ships) do
   classes[k] = {}

   for k2,v2 in pairs(v) do
      classes[k][#classes[k]+1] = k2
   end
end

function price(class)
   local modifier = 1
   if class == "fighter" then
      modifier = 0.5
   elseif class == "bomber" then
      modifier = 0.75
   elseif class == "destroyer" then
      modifier = 1.5
   elseif class == "cruiser" then
      modifier = 2
   elseif class == "carrier" then
      modifier = 3
   end

   return modifier * base_price
end

function random_class(faction)
   local m = #classes[faction]

   if m == 0 then
      return
   end

   local r = rnd.rnd(1, m)

   return classes[faction][r]
end

function random_ship(faction, class)
   local m = #ships[faction][class]

   if m == 0 then
      return
   end

   local r = rnd.rnd(1, m)

   return ships[faction][class][r]
end

function random_planet()
   local planets = {}
   local maximum_distance = 6
   local minimum_distance = 0

   getsysatdistance(
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

function improve_standing(class, faction_name)
   local enemies = faction.get(faction_name):enemies()
   local standing = 0

   if class == "corvette" then
      standing = 1
   elseif class == "destroyer" then
      standing = 2
   elseif class =="cruiser" then
      standing = 3
   elseif class == "carrier" then
      standing = 4
   end

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

function damage_standing(class, faction_name)
   local modifier = 1

   -- “Oh dude, that guy is capable! He managed to steal one of our own ships!”
   if faction == "Pirate" then
      return
   end

   if faction == "Independent" or faction == "Frontier" then
      modifier = 0.5
   end

   if class == "corvette" then
      faction.modPlayerSingle(faction_name, -2 * modifier)
   elseif class == "destroyer" then
      faction.modPlayerSingle(faction_name, -4 * modifier)
   elseif class == "cruiser" then
      faction.modPlayerSingle(faction_name, -8 * modifier)
   elseif class == "carrier" then
      -- Hey, who do you think you are to steal a carrier?
      faction.modPlayerSingle(faction_name, -16 * modifier)
   end
end

function create ()
   theship = { __save = true }

   theship.planet  = random_planet()

   if not theship.planet or theship.planet:faction() == nil then
      -- If we’re here, it means we couldn’t get a planet close enough.
      misn.finish(false)
   end

   theship.faction = theship.planet:faction():nameRaw()
   theship.class   = random_class(theship.faction)

   if not theship.class then
      -- If we’re here, it means we couldn’t get a ship of the right faction
      -- and of the right class.
      misn.finish(false)
   end

   -- We’re assuming ships[faction][class] is not empty, here…
   theship.exact_class = random_ship(theship.faction, theship.class)
   theship.price   = price(theship.class)

   theship.system = theship.planet:system()

   misn.setNPC( _("A Pirate informer"), getPortrait("Pirate") )
   misn.setDesc( informer.description )
end

function accept()
   if tk.yesno( informer.title, informer.message:format(
         _(theship.faction), _(theship.class), creditstring(theship.price) ) ) then
      if player.credits() >= theship.price then
         tk.msg( approval.title, approval.message:format(
            theship.planet:name(), theship.system:name() ) )

         player.pay( -theship.price )
         misn.accept()

         -- Mission title, reward, description
         misn.setTitle( title:format( _(theship.class) ) )
         misn.setReward( reward:format( _(theship.class) ) )
         misn.setDesc( description:format(
            theship.planet:name(), theship.system:name(), _(theship.class) ) )

         -- Mission marker
         misn.markerAdd( theship.system, "low" )

         -- OSD
         misn.osdCreate(
            string.format(
               title,
               _(theship.class)
            ), {
               string.format(
                  description,
                  theship.planet:name(),
                  theship.system:name(),
                  _(theship.class)
               )
            }
         )

         hook.land("land")
         hook.enter("enter")
      else
         tk.msg( not_enough_title, not_enough_text )
         misn.finish()
      end
   else
      -- Why would we care?
      misn.finish()
   end
end

function land()
   local landed = planet.cur()
   if landed == theship.planet then
      -- Try to swap ships
      local tmp = pilot.addRaw( theship.exact_class, nil, nil, 'Independent' )
      equip_generic( tmp )
      if not swapship( tmp ) then
         -- Failed to swap ship!
         tk.msg( unableto.title, unableto.message )
         tmp:rm() -- Get rid of the temporary pilot
         return
      end

      -- Oh yeah, we stole the ship. \o/
      tk.msg(
         success.title,
         string.format(
            success.message,
            theship.exact_class
         )
      )

      -- Hey, stealing a ship isn’t anything! (if you survive, that is)
      faction.modPlayerSingle("Pirate", rnd.rnd(3,5))

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
      damage_standing(theship.class, theship.faction)

      -- This is a success. The player stole his new ship, and everyone is
      -- happy with it. Getting out of the system alive is the player’s 
      -- responsibility, now.
      misn.finish(true)
   end
end

function enter()
   -- A few faction ships guard the target planet.
   if system.cur() == theship.system then
      -- We want the player to be able to land on the destination planet…
      theship.planet:landOverride(true)
   end
end

