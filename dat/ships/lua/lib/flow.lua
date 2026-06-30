local srs = require "common.sirius"

local flow = {}

local flow_base = {
   -- Ships
   ["Sirius Divinity"]           = 500,
   ["Sirius Dogma"]              = 500,
   ["Pirate Kestrel Galaxy Soul"]  = 400, --Dreamer ships use Internal Groove Synthesizer.
   ["Sirius Providence"]         = 300,
   ["Starbridge Herald"]         = 200,
   ["Sirius Preacher"]           = 160,
   ["Sirius Shaman"]             = 80,
   ["Sirius Fidelity"]           = 50,
   -- Projections
   ["Astral Projection Greater"] = 320,
   ["Astral Projection Normal"]  = 160,
   ["Astral Projection Lesser"]  = 80,
   -- Outfits
   ["Large Flow Amplifier"]      = 400,
   ["Medium Flow Amplifier"]     = 200,
   ["Small Flow Amplifier"]      = 100,
   ["Large Meditation Chamber"]  = 280,
   ["Medium Meditation Chamber"] = 140,
   ["Small Meditation Chamber"]  = 70,
   ["Astral Flow Amplifier"]     = 100,
   ["Large Groove Synthesizer"]  = 300,
   ["Medium Groove Synthesizer"] = 150,
   ["Small Groove Synthesizer"]  = 75,
   ["Large Groove Pit"]          = 200,
   ["Medium Groove Pit"]         = 100,
   ["Small Groove Pit"]          = 50,
}
flow.list_base = flow_base

local flow_regen = {
   -- Ships
   ["Sirius Divinity"]           = 7,
   ["Sirius Dogma"]              = 7,
   ["Pirate Kestrel Galaxy Soul"]  = 6, --Dreamer ships use Internal Groove Synthesizer.
   ["Sirius Providence"]         = 5,
   ["Starbridge Herald"]         = 4,
   ["Sirius Preacher"]           = 3,
   ["Sirius Shaman"]             = 2,
   ["Sirius Fidelity"]           = 1.5,
   -- Projections
   ["Astral Projection Greater"] = 32,
   ["Astral Projection Normal"]  = 16,
   ["Astral Projection Lesser"]  = 8,
   -- Outfits
   ["Large Flow Resonator"]      = 6,
   ["Medium Flow Resonator"]     = 4,
   ["Small Flow Resonator"]      = 2,
   ["Astral Flow Amplifier"]     = 10,
   ["Large Groove Synthesizer"]  = 4,
   ["Medium Groove Synthesizer"] = 2,
   ["Small Groove Synthesizer"]  = 1,
}
flow.list_regen = flow_regen

local flow_mod = {
--[[
   -- Ships
   ["Sirius Divinity"]           = 1.3,
   ["Sirius Dogma"]              = 1.3,
   ["Sirius Providence"]         = 1.3,
   ["Sirius Preacher"]           = 1.3,
   ["Sirius Shama"]              = 1.3,
   ["Sirius Fidelity"]           = 1.3,
   -- Projections
   ["Astral Projection Greater"] = 1.3,
   ["Astral Projection Normal"]  = 1.3,
   ["Astral Projection Lesser"]  = 1.3,
   -- Outfits
   ["Astral Projection"]         = 1/1.3,
   ["Avatar of the Sirichana"]   = 1/1.3,
   ["Cleansing Flames"]          = 1/1.3,
   ["Feather Drive"]             = 1/1.3,
   ["House of Mirrors"]          = 1/1.3,
   ["Reality Rip"]               = 1/1.3,
   ["Seeking Chakra"]            = 1/1.3,
--]]
}
flow.list_mod = flow_mod

function flow.has( p )
   local sm = p:shipMemory()
   return sm._flow_mod~=nil
end

function flow.get( p, sm )
   sm = sm or p:shipMemory()
   return sm._flow or 0
end

function flow.max( p )
   local sm = p:shipMemory()
   return sm._flow_base or 0
end

function flow.regen( p )
   local sm = p:shipMemory()
   return sm._flow_regen or 0
end

function flow.activate( p )
   local sm = p:shipMemory()
   local fa = (sm._flow_active or 0)
   sm._flow_active = fa+1
end

function flow.deactivate( p )
   local sm = p:shipMemory()
   local fa = (sm._flow_active or 0)
   sm._flow_active = math.max(fa-1)
end

local function default_capacity( sm )
   return (sm._flow_dreamer and 0.1) or 0.5
end

function flow.reset( p )
   local sm = p:shipMemory()
   sm._flow = (sm._flow_base or 0) * default_capacity(sm)
   sm._flow_active = 0
end

function flow.inc( p, amount )
   local sm = p:shipMemory()
   local fb = sm._flow_base or 0
   local f = sm._flow or 0
   sm._flow = math.min( fb, f+amount )
end

function flow.dec( p, amount )
   local sm = p:shipMemory()
   local f = sm._flow or 0
   sm._flow = math.max( 0, f-amount )
end

function flow.update( p, dt )
   local sm = p:shipMemory()
   local fb = sm._flow_base or 0
   local f = sm._flow or 0
   local cap = default_capacity(sm)
   if sm._flow < cap*fb then
      local fa = sm._flow_active or 0
      if fa <= 0 then
         -- Regen when under cap and no active on
         local fr = sm._flow_regen or 0
         sm._flow = math.min( cap*fb, f + dt*fr )
      end
   else
      -- Lose 2% a second when over cap
      sm._flow = math.max( cap*fb, f - dt*0.02*fb )
   end
end

function flow.recalculate( p )
   local sm = p:shipMemory()
   local has_amplifier = false
   local is_dreamer = false
   local fm, fb, fr

   -- Try to find the amplifier if applicable
   local wplayer = p:withPlayer()
   local sn = p:ship():nameRaw()
   fm = flow_mod[ sn ] or 1
   fb = flow_base[ sn ] or 0
   fr = flow_regen[ sn ] or 0
   for k,v in ipairs(p:outfitsList("all")) do
      local vn = v:nameRaw()
      fm = fm * (flow_mod[ vn ] or 1)
      fb = fb + (flow_base[ vn ] or 0)
      fr = fr + (flow_regen[ vn ] or 0)
      local t = v:tags()
      if t.flow_amplifier and (not wplayer) or (wplayer and srs.playerIsPsychic()) or t.dreamer then
         has_amplifier = true
      end
      if t.dreamer then
         is_dreamer = true
      end
   end

   if has_amplifier then
      -- Get bonus from abilities to player
      if p==player.pilot() then
         local fam = 0
         -- Abilities are unique, so we can only check unequipped
         for k,o in ipairs(player.outfits(true)) do
            if o:tags().flow_ability then
               fam = fam + 1
            end
         end
         fm = fm + 0.05 * math.max( 0, fam-1 )
      end

      -- Base stats
      sm._flow_mod   = fm
      sm._flow_base  = fb * fm
      sm._flow_regen = fr

      -- Dreamers change behaiour a bit
      if is_dreamer then
         sm._flow_dreamer = true
      else
         sm._flow_dreamer = false
      end
   else
      sm._flow_mod = nil
      sm._flow_base = nil
      sm._flow_regen = nil
      sm._flow_dreamer = nil
   end

   -- Reset just in case
   flow.reset( p )
end

local amp_size = {
   ["Small Flow Amplifier"]   = 1,
   ["Medium Flow Amplifier"]  = 2,
   ["Large Flow Amplifier"]   = 3,
   ["Small Groove Synthesizer"]   = 1,
   ["Medium Groove Synthesizer"]  = 2,
   ["Large Groove Synthesizer"]   = 3,
}
function flow.size( p )
   -- Sirius ships are determined by their size
   for k,v in ipairs(p:outfitsList()) do
      if v:tags().flow_amplifier then
         return amp_size[ v:nameRaw() ]
      end
   end
   -- We default to ship size if not using an explicit amplifier
   local ps = p:ship()
   return math.floor( ps:size()*0.5 + 0.5 )
end

local prefix = {
   p_("flow prefix", "Lesser"),
   p_("flow prefix", "Normal"),
   p_("flow prefix", "Greater"),
}
function flow.prefix( s )
   return prefix[s]
end

return flow
