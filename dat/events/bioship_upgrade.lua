--[[

   Bioship Upgrade Event

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--

   This event runs in the background to upgrade any bioship the player
   may be flying. Bioships upgrade whenever the player gets paid;
   getting paid, if flying a bioship, triggers an increase in
   "experience" points, stored in the "_bioship_exp" variable. Note: all
   bioships share the same experience points; this is done because
   there's no really reliable way to divide them up and players can
   easily game the system anyway by swapping out ships at the right
   moment for many missions (e.g. doing a bounty mission, then swapping
   ships before landing on the faction's planet to get paid).

--]]


-- A table of all part types. Each entry is another table with
-- two entries. The first entry in each inner table is the base name of
-- the part containing %s, which will be replaced by a number indicating
-- the stage of the part, or "X" for the final stage. The second entry
-- in each inner table is the number of normal "stages", excluding the
-- final stage (Stage X).
bioship_parts = {
   -- Brains (Core System analog)
   { "Ultralight Bioship Brain Stage %s", 2 },
   { "Light Bioship Brain Stage %s", 3 },
   { "Medium Bioship Brain Stage %s", 4 },
   { "Medium-Heavy Bioship Brain Stage %s", 5 },
   { "Heavy Bioship Brain Stage %s", 6 },
   { "Superheavy Bioship Brain Stage %s", 7 },

   -- Shells (Hull analog)
   { "Ultralight Bioship Shell Stage %s", 2 },
   { "Light Bioship Shell Stage %s", 3 },
   { "Medium Bioship Shell Stage %s", 4 },
   { "Medium-Heavy Bioship Shell Stage %s", 5 },
   { "Heavy Bioship Shell Stage %s", 6 },
   { "Superheavy Bioship Shell Stage %s", 7 },

   -- Fins (Engine analog)
   { "Ultralight Bioship Fast Fin Stage %s", 2 },
   { "Light Bioship Fast Fin Stage %s", 3 },
   { "Medium Bioship Fast Fin Stage %s", 4 },
   { "Medium-Heavy Bioship Fast Fin Stage %s", 5 },
   { "Heavy Bioship Fast Fin Stage %s", 6 },
   { "Superheavy Bioship Fast Fin Stage %s", 7 },
   { "Ultralight Bioship Strong Fin Stage %s", 2 },
   { "Light Bioship Strong Fin Stage %s", 3 },
   { "Medium Bioship Strong Fin Stage %s", 4 },
   { "Medium-Heavy Bioship Strong Fin Stage %s", 5 },
   { "Heavy Bioship Strong Fin Stage %s", 6 },
   { "Superheavy Bioship Strong Fin Stage %s", 7 },

   -- Weapons
   { "BioPlasma Organ Stage %s", 3 },
}


function create ()
   hook.pay( "pay" )
end


function is_bioship_part( s )
   for i, p in ipairs( bioship_parts ) do
      if string.match( s, p[1]:format( ".*" ) ) then
         return true
      end
   end
   return false
end


-- Returns the index of bioship_parts the given outfit is a part of if a
-- valid undeveloped part, or nil otherwise.
function undeveloped_bioship_part_index( s )
   for i, p in ipairs( bioship_parts ) do
      if string.match( s, p[1]:format( "%d" ) ) then
         return i
      end
   end
   return nil
end


function has_bioship ()
   for i, o in ipairs( player.pilot():outfits() ) do
      if is_bioship_part( o:name() ) then
         return true
      end
   end
   return false
end


-- Returns a table of inner tables for each undeveloped bioship part on
-- the player; each inner table contains, respectively:
--    * The outfit name
--    * The corresponding index in bioship_parts
function get_bioship_parts ()
   local parts = {}
   for i, o in ipairs( player.pilot():outfits() ) do
      local index = undeveloped_bioship_part_index( o:name() )
      if index ~= nil then
         parts[ #parts + 1 ] = { o:name(), index }
      end
   end
   return parts
end


function pay( amount )
   local exp_gain = math.floor(amount / 10000)
   if amount > 0 and has_bioship() then
      local exp = var.peek( "_bioship_exp" ) or 0
      exp = exp + exp_gain
      while exp >= 100 do
         exp = exp - 100
         local parts = get_bioship_parts()
         if #parts > 0 then
            local part_t = parts[ rnd.rnd( 1, #parts ) ]
            local part = part_t[1]
            local index = part_t[2]
            local current_level = 0
            local max_level = bioship_parts[index][2]

            for i=1,max_level do
               if part == bioship_parts[index][1]:format( string.format( "%d", i ) ) then
                  current_level = i
                  break
               end
            end

            local new_level = current_level + 1

            player.pilot():rmOutfit( part )
            local new_part
            if new_level > max_level then
               new_part = bioship_parts[index][1]:format( "X" ) 
            else
               local sn = string.format( "%d", new_level )
               new_part = bioship_parts[index][1]:format( sn )
            end
            player.pilot():addOutfit( new_part )
         end
      end 
   end
end
