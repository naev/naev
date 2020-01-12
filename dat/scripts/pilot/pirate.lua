--[[
-- @brief Generates pilot names
--]]
function pirate_name ()
   -- TODO this needs some work to be translatable...
   local articles = {
      _("The"),
   }
   local descriptors = {
      _("Lustful"),
      _("Bloody"),
      _("Morbid"),
      _("Horrible"),
      _("Terrible"),
      _("Very Bad"),
      _("No Good"),
      _("Dark"),
      _("Evil"),
      _("Murderous"),
      _("Fearsome"),
      _("Defiant"),
      _("Unsightly"),
      _("Pirate's"),
      _("Night's"),
      _("Space"),
      _("Astro"),
      _("Delicious"),
      _("Fearless"),
      _("Eternal"),
      _("Mighty")
   }
   local colours = {
      _("Red"),
      _("Green"),
      _("Blue"),
      _("Cyan"),
      _("Black"),
      _("Brown"),
      _("Mauve"),
      _("Crimson"),
      _("Yellow"),
      _("Purple")
   }
   local actors = {
      _("Beard"),
      _("Moustache"),
      _("Neckbeard"),
      _("Demon"),
      _("Vengeance"),
      _("Corsair"),
      _("Pride"),
      _("Insanity"),
      _("Peril"),
      _("Death"),
      _("Doom"),
      _("Raider"),
      _("Devil"),
      _("Serpent"),
      _("Bulk"),
      _("Killer"),
      _("Thunder"),
      _("Tyrant"),
      _("Lance"),
      _("Destroyer"),
      _("Horror"),
      _("Dread"),
      _("Blargh"),
      _("Terror")
   }
   local actorspecials = {
      _("Angle Grinder"),
      _("Belt Sander"),
      _("Chainsaw"),
      _("Impact Wrench"),
      _("Band Saw"),
      _("Table Saw"),
      _("Drill Press"),
      _("Jigsaw"),
      _("Turret Lathe"),
      _("Claw Hammer"),
      _("Rubber Mallet"),
      _("Squeegee"),
      _("Pipe Wrench"),
      _("Bolt Cutter"),
      _("Staple Gun"),
      _("Crowbar"),
      _("Pickaxe"),
      _("Bench Grinder"),
      _("Scythe")
   }
   local article = articles[ rnd.rnd(1,#articles) ]
   local descriptor = descriptors[ rnd.rnd(1,#descriptors) ]
   local colour = colours[ rnd.rnd(1,#colours) ]
   local actor = actors[ rnd.rnd(1,#actors) ]
   local actorspecial = actorspecials[ rnd.rnd(1,#actorspecials) ]

   if rnd.rnd() < 0.25 then
      return article .. " " .. actorspecial
   else
      local r = rnd.rnd()
      if r < 0.166 then
         return article .. " " .. actor
      elseif r < 0.333 then
         return colour .. " " .. actor
      elseif r < 0.50 then
         return descriptor .. " " .. actor
      elseif r < 0.666 then
         return article .. " " .. descriptor .. " " .. actor
      elseif r < 0.833 then
         return article .. " " .. colour .. " " .. actor
      else
         return article .. " " .. descriptor .. " " .. colour .. " " .. actor
      end
   end
end
