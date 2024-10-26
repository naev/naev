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

local rep_max_tags_list = {}
for k, f in ipairs( factions ) do
   for t, v in pairs( f.rep_max_tags ) do
      rep_max_tags_list[t] = { val=v.val, max=v.max, fct=f.fct, var=f.rep_max_var }
   end
end

local function recalculate( domsg )
   local omax = {}
   if domsg then
      for k, f in ipairs( factions ) do
         --omax[ f.rep_max_var ] = var.peek( f.rep_max_var ) or f.cap_misn_def
         omax[ f.rep_max_var ] = f.rep_max
      end
   end

   -- Initialize reputation maximums
   local max = {}
   for k, f in ipairs( factions ) do
      max[ f.rep_max_var ] = f.rep_max
   end

   -- Create a list of done tags
   local donetags = {}
   for k, m in ipairs(player.misnDoneList()) do
      for t, b in pairs( m.tags ) do
         local c = rep_max_tags_list[t]
         if c then
            table.insert( donetags, c )
         end
      end
   end

   -- Sort based on max (lower goes first)
   table.sort( donetags, function(a, b) return a.max < b.max end )

   -- Now run over and apply within limit
   for k, c in ipairs(donetags) do
      max[ c.var ] = math.min( max[ c.var ] + c.val, c.max )
   end

   -- Set the max
   local smax = {}
   for k, v in pairs(max) do
      smax[k] = var.peek( k ) or omax[k]
      var.push( k, v )
   end

   -- Do message if increased
   if domsg then
      for k, n in pairs(max) do
         local s = smax[ k ]
         if s ~= n then
            local fct
            for i, f in ipairs(factions) do
               if f.rep_max_var==k then
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
      local c = rep_max_tags_list[t]
      if c then
         recalculate( true )
         return
      end
   end
end

local fpir = faction.get("Pirate")
local fmar = faction.get("Marauder")
local htimer
local fctchanged = {}
function standing_change( fct, sys, mod, _source, _secondary, _primary_fct )
   -- Messages are disabled
   if var.peek("factionhit_hide") then return end

   -- Ignore marauder / pirate changes, since they're fake changes tied with the clans
   if fct==fpir or fct==fmar then return end

   if not fct:known() then return end

   if fct:invisible() or fct:static() then return end

   --if secondary ~= 0 and var.peek("factionhit_hide_secondary") then return end

   local n = fct:nameRaw()
   local v = fctchanged[n]
   if not v then
      v = 0
   else
      v = v.mod
   end
   -- We actually overwrite system here, so a local + global hit will be interpreted as the last one...
   -- Might make more sense to separate them.
   fctchanged[n] = { fct=fct, mod=v+mod, sys=sys }

   -- Start timer if it hasn't yet
   if htimer == nil then
      -- Show changed factions with a delay, this basically ends up "accumulating" during the time
      htimer = hook.timer( 1, "standing_change_hook" )
   end
end

function standing_change_hook ()
   local fctlist = {}
   for k,v in pairs(fctchanged) do
      local mod = math.floor( v.mod+0.5 )
      if math.abs(mod) > 0 then
         v.mod = mod
         table.insert( fctlist, v )
      end
   end
   table.sort( fctlist, function( a, b )
      if a.mod < b.mod then
         return true
      elseif b.mod < a.mod then
         return false
      end
      return a.fct:name() < b.fct:name()
   end )
   for k,v in ipairs(fctlist) do
      local mod = v.mod
      local reptype
      if v.sys then
         reptype = p_("reputation","local")
      else
         reptype = p_("reputation","global")
      end
      local fctstr = v.fct:name()
      if v.fct:areAllies( faction.player(), system.cur() ) then
         fctstr = "#F"..fctstr.."#0"
      elseif v.fct:areEnemies( faction.player(), system.cur() ) then
         fctstr = "#H"..fctstr.."#0"
      else
         fctstr = "#N"..fctstr.."#0"
      end

      if mod < 0 then
         player.msg(fmt.f("#r".._("Lost {amount} {reptype} reputation with {fct}."),{amount=math.abs(mod), fct=fctstr, reptype=reptype}))
      else
         player.msg(fmt.f("#g".._("Gained {amount} {reptype} reputation with {fct}."),{amount=mod, fct=fctstr, reptype=reptype}))
      end
   end
   fctchanged = {} -- Clear changes
   htimer = nil -- Timer is done
end

function create ()
   recalculate( false )

   hook.mission_done( "eventmission_done" )
   hook.event_done( "eventmission_done" )
   hook.standing( "standing_change" )
end
