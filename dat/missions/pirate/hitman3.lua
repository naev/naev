--[[

   Pirate Hitman 3

   A one time random pirate hitman mission that gets you pirate landing permissions

   Author: nloewen

--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "A young buisnessman. I wonder what he is doing here."

   -- Mission details
   misn_title  = "Kill %s"
   misn_reward = "Pirate Landing permision"
   misn_desc   = "There is a empire patrol known as %s who must be terminated. He was last seen near the %s system."

   -- Text
   title    = {}
   text     = {}
   title[1] = "Spaceport Bar"
   title[3] = "Mission Complete"
   text[1] = [[]]
   bargin_text = {}
   bargin_text[1]  = [[Hello, lets get right too it. I need to hire a pilot, and I've heard from a certain trader that your good. What do you charge for the simple removal of a convoy?]]
   bargin_text[2] = [[No, no, no. That is far too much. How about 10000.]]
   bargin_text[3] = [[I understand you are a skilled pilot, but that is too much. I am prepared to offer you 20000.]]
   bargin_text[4] = [[You are a stuborn one. I will go to 30000, no higher.]]
   bargin_text[5] = [[You ask too much. I will find another pilot.]]

   -- Messages
   msg      = {}
   msg[1]   = "MISSION SUCCESS! Return for payment."
end

function create ()
   targetsystem = system.get("Delta Pavonis") -- Find target system

   -- Spaceport bar stuff
   misn.setNPC( "Young Businessman",  "Businessman")
   misn.setDesc( bar_desc )
   
   -- hooks to let busninessman approach you
   hook.land( "businessman_timer", "bar" )
end

function businessman_timer ()
   hook.timer( 3000, "accept" )
end

--[[
Mission entry point.
--]]
function accept ()
   -- Create the target pirate
   pir_name, pir_ship, pir_outfits = pir_generate()

   -- Get target system
   near_sys = get_pir_system( system.get() )

   -- Get credits
   credits  = rnd.rnd(5,10) * 10000

   -- Mission details:
   bargin_attempt = 1
   do
   price = bargin()
   if price > 30000 then
      if bargin_attempt < 5 then
         if tk.yesno(title[1], bargin_text[bargin_attempt]) = true then
            price = 10000 * (bargin_attempt - 1)
         end
      else
         tk.msg(bargin_text[5]
         return
      end
   end
   while price > 30000
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
end

-- Gets a piratey system
function get_pir_system( sys )
   local adj_sys = sys:adjacentSystems()
  
   -- Only take into account system with pirates.
   local pir_sys = {}
   for k,v in pairs(adj_sys) do
      if k:hasPresence( "Pirate" ) then
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
   player.modFaction( "Empire", 5 )
   
   -- The goods
   diff.apply("heavy_combat_vessel_license")
   
   -- Finish mission
   misn.finish(true)
end


-- Entering a system
function sys_enter ()
   cur_sys = system.get()
   -- Check to see if reaching target system
   if cur_sys == near_sys then

      -- Create the badass enemy
      p     = pilot.add(pir_ship)
      pir   = p[1]
      pir:rename(pir_name)
      pir:setHostile()
      pir:rmOutfit("all") -- Start naked
      pilot_outfitAddSet( pir, pir_outfits )
      hook.pilot( pir, "death", "pir_dead" )
      hook.pilot( pir, "jump", "pir_jump" )
   end
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
   rating = player.getRating()
   if rating < 50 then
      pir_ship, pir_outfits = pir_easy()
   elseif rating < 150 then
      pir_ship, pir_outfits = pir_medium()
   else
      pir_ship, pir_outfits = pir_hard()
   end

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
function pir_medium ()
   if rnd.rnd() < 0.5 then
      return pirate_createAdmonisher(false)
   else
      return pir_easy()
   end
end
function pir_hard ()
   if rnd.rnd() < 0.5 then
      return pirate_createKestrel(false)
   else
      return pir_medium()
   end
end

