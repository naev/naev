--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Faction Standing">
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Handles the player's faction standing reputation caps
--]]
local fmt = require "format"
local lf = require "love.filesystem"

local factions = {}
for k,v in ipairs(lf.getDirectoryItems("scripts/factions")) do
   local e = require( "factions."..string.gsub(v,".lua","") )
   table.insert( factions, e )
end

local cap_tags_list = {}
for k, f in ipairs( factions ) do
   for t, v in pairs( f.cap_tags ) do
      cap_tags_list[t] = { val=v.val, max=v.max, fct=f.fct, var=f.cap_misn_var }
   end
end

local function recalculate( domsg )
   local ocaps = {}
   if domsg then
      for k, f in ipairs( factions ) do
         --ocaps[ f.cap_misn_var ] = var.peek( f.cap_misn_var ) or f.cap_misn_def
         ocaps[ f.cap_misn_var ] = f.cap_misn_def
      end
   end

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
   local scaps = {}
   for k, v in pairs(caps) do
      scaps[k] = var.peek( k ) or ocaps[k]
      var.push( k, v )
   end

   -- Do message if increased
   if domsg then
      for k, n in pairs(caps) do
         local s = scaps[ k ]
         if s ~= n then
            local fct
            for i, f in ipairs(factions) do
               if f.cap_misn_var==k then
                  fct = f.fct
                  break
               end
            end
            player.msg(fmt.f("#g".._("Reputation limit with {fct} increased to {val}!").."#0",{fct=fct, val=n}))
         end
      end
   end
end

function eventmission_done( data )
   -- Only update if there's a tag we care about
   for t, b in pairs( data.tags ) do
      local c = cap_tags_list[t]
      if c then
         recalculate( true )
         return
      end
   end
end

function create ()
   recalculate( false )

   hook.mission_done( "eventmission_done" )
   hook.event_done( "eventmission_done" )
end
