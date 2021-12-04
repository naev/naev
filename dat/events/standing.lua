--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Faction Standing">
 <trigger>load</trigger>
 <chance>100</chance>
</event>
--]]
--[[
   Handles the player's faction standing reputation caps
--]]

local factions = {
   require("factions.zalek"),
}

local cap_tags_list = {}
for k, f in ipairs( factions ) do
   for t, v in pairs( f.cap_tags ) do
      cap_tags_list[t] = { val=v.val, max=v.max, fct=f.fct, var=f.cap_misn_var }
   end
end

local function recalculate ()
   -- Initialize caps
   local caps = {}
   for k, f in ipairs( factions ) do
      caps[ f.cap_misn_var ] = f.cap_misn_def
   end

   -- Create a list of done tags
   local donetags = {}
   for k, m in ipairs(player.misnDoneList()) do
      for t, b in pairs( m.tags ) do
         local c = cap_tags_list[t]
         if c then
            table.insert( donetags, c )
         end
      end
   end

   -- Sort based on max (lower goes first)
   table.sort( donetags, function(a, b) return a.max < b.max end )

   -- Now run over and apply within limit
   for k, c in ipairs(donetags) do
      caps[ c.var ] = math.min( caps[ c.var ] + c.val, c.max )
   end

   -- Set the caps
   for k, v in pairs(caps) do
      var.push( k, v )
   end
end

function create ()
   recalculate()

   -- Done
   evt.finish()
end
