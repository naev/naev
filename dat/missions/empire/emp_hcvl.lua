--[[

   Empire Heavy Combat Vessel License Mision

   Mission that is basically a one-time pirate bounty mission.

   Author: bobbens
      minor edits by Infiltrator

--]]

-- Bar stuff
bar_desc    = _("You see an Empire Official.")

-- Mission details
misn_title  = _("Kill %s")
misn_reward = _("Authorization for Heavy Combat Vessel License")
misn_desc   = _("There is a pirate known as %s who must be terminated. He was last seen near the %s system.")

-- Text
title    = {}
text     = {}
title[1] = _("Spaceport Bar")
text[1]  = _([[You are greeted by an Empire official while at the bar. "Hello %s, Commander Soldner has spoken well of you. He said you're a problem solver.
    "Well, we have a problem with a pirate known as %s near the system %s. This mission would serve as your test for the Heavy Combat Vessel License. Would you be interested?"]])
text[2]  = _([["Good luck! The pirate has already killed his last contender, although I don't think he'll be a match for you."]])

-- Messages
msg      = {}
msg[1]   = _("MISSION SUCCESS! You are now authorized for the Heavy Combat Vessel License.")
msg[2]   = _("Pursue %s!")


-- Scripts we need
include("pilot/pirate.lua")
include("dat/missions/empire/common.lua")


function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( _("Official"), emp_getOfficialRandomPortrait() )
   misn.setDesc( bar_desc )
end


--[[
Mission entry point.
--]]
function accept ()
   -- Create the target pirate
   pir_name, pir_ship, pir_outfits = pir_generate()

   -- Get target system
   near_sys = get_pir_system( system.cur() )

   -- Get credits
   credits  = rnd.rnd(5,10) * 100000

   -- Mission details:
   if tk.yesno( title[1], string.format( text[1], player.name(),
         pir_name, near_sys:name() ) ) then
      misn.accept()

      -- Set mission details
      misn.setTitle( string.format( misn_title, pir_name) )
      misn.setReward( string.format( misn_reward, credits) )
      misn.setDesc( string.format( misn_desc, pir_name, near_sys:name() ) )
      misn.markerAdd( near_sys, "low" )

      -- Some flavour text
      tk.msg( title[1], text[2] )

      -- Set hooks
      hook.enter("sys_enter")
      last_sys = system.cur()
   end
end


-- Gets a piratey system
function get_pir_system( sys )
   local adj_sys = sys:adjacentSystems()
  
   -- Only take into account system with pirates.
   local pir_sys = {}
   for _,k in ipairs(adj_sys) do
      if k:presences()["Pirate"] then
         pir_sys[ #pir_sys+1 ] = k
      end
   end

   -- Make sure system has pirates
   if #pir_sys == nil then
      return sys
   else
      return pir_sys[ rnd.rnd(1,#pir_sys) ]
   end
end



-- Player won, gives rewards.
function give_rewards ()
   -- Give factions
   faction.modPlayerSingle( "Empire", 5 )
   
   -- The goods
   diff.apply("heavy_combat_vessel_license")
   
   -- Finish mission
   misn.finish(true)
end


-- Entering a system
function sys_enter ()
   cur_sys = system.cur()
   -- Check to see if reaching target system
   if cur_sys == near_sys then

      -- Choose position
      local pos
      if cur_sys == last_sys then
         pos = player.pilot():pos()
      else
         pos = jump.pos(cur_sys, last_sys)
      end
      local x,y = pos:get()
      local d = rnd.rnd( 1500, 2500 )
      local a = math.atan2( y, x ) + math.pi
      local offset = vec2.newP(d, a)
      pos = pos + offset

      -- Create the badass enemy
      p     = pilot.add( pir_ship, nil, pos )
      pir   = p[1]
      pir:rename(pir_name)
      pir:setHostile()
      pir:setVisplayer(true)
      pir:setHilight(true)
      pir:rmOutfit("all") -- Start naked
      pilot_outfitAddSet( pir, pir_outfits )
      hook.pilot( pir, "death", "pir_dead" )
      hook.pilot( pir, "jump", "pir_jump" )
   end
   last_sys = cur_sys
end


-- Pirate is dead
function pir_dead ()
   player.msg( msg[1] )
   give_rewards()
end


-- Pirate jumped away
function pir_jump ()
   player.msg( string.format(msg[2], pir_name) )

   -- Basically just swap the system
   near_sys = get_pir_system( near_sys )
end


--[[
Functions to create pirates based on difficulty more easily.
--]]
function pir_generate ()
   -- Get the pirate name
   pir_name = pirate_name()

   -- Get the pirate details
   pir_ship, pir_outfits = pir_easy()

   -- Make sure to save the outfits.
   pir_outfits["__save"] = true

   return pir_name, pir_ship, pir_outfits
end
function pir_easy ()
   if rnd.rnd() < 0.5 then
      return pirate_createAncestor(false)
   else
      return pirate_createVendetta(false)
   end
end

