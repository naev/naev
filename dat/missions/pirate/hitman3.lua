--[[

   Pirate Hitman 3

   A one time random pirate hitman mission that gets you pirate landing permissions

   Author: nloewen

--]]

-- Bar information
bar_desc = _("A well-dressed young businessman. He looks out of place, contrasting sharply with most of the bar's clientele")

-- Mission details
misn_title  = _("Pirate Hitman 3")
misn_reward = _("More easy money.")
misn_desc   = _("The Empire patrol vessel known as %s must be terminated. It was last seen near the %s system.")
misn_desc2  = _("Return to %s to get your rewards.")

-- Text
title    = {}
text     = {}
title[1] = _("Spaceport Bar")
title[2] = _("Mission Complete")
text[1]  = _([[As you approach, the man merely glances at you before pushing out a chair for you. "Hello. A certain trader associate of mine has recommended your services." You nod knowingly, and he continues, "A certain Empire pilot has been... consistent in refusing our bribes. We'd like to be rid of him as soon as possible. Are you up for it?]])
text[2]  = _([["Excellent. If you're successful in removing him, you will of course be rewarded." His demeanour shifts slightly before he continues, "Of course, we are not the forgiving type. If you rat us out, we will find you. If you fail, well, I suppose you'll be sent to one of the Empire's penal colonies. That said, you've performed admirably for my associate, so I trust I'll see you again soon."]])
text[3] = _([[The businessman is waiting for you. "Ah, you've returned. I've already received the good news from my associates who monitor the Empire communications band. Here's your pay. There's always work for a competent pilot; I look forward to working with you again." With that, the man walks away, disappearing into a crowd. You wonder how much "business" this supposed businessman is involved in.]])

msg = {}
msg[1] = _("Target destroyed. Mission objective updated")
msg[2] = _("Target has jumped. Persue %s!")


include("dat/missions/pirate/common.lua")


-- Scripts we need
include("pilot/empire.lua")
include("dat/scripts/jumpdist.lua")


function create ()
   -- Note: this mission does not make any system claims. 
   -- Create the target pirate
   emp_name, emp_ship, emp_outfits = emp_generate()
   
   -- Get target system
   near_sys = get_emp_system( system.cur() )

   -- Spaceport bar stuff
   misn.setNPC( _("Young Businessman"), "neutral/unique/youngbusinessman")
   misn.setDesc( bar_desc )

   --some other stuff
   misn_base, misn_base_sys = planet.cur()
end

function businessman_timer ()
   hook.timer( 5000, "accept" )
end

--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( title[1], string.format( text[1],
         emp_name, credits, near_sys:name() ) ) then
      misn.finish()
   end
   misn.accept()

   -- Set mission details
   misn.setTitle( string.format( misn_title, near_sys:name()) )
   misn.setReward( string.format( misn_reward, credits) )
   misn.setDesc( string.format( misn_desc, emp_name, near_sys:name() ) )
   misn_marker = misn.markerAdd( near_sys, "low" )
   misn.osdCreate(misn_title, {misn_desc:format(emp_name, near_sys:name())})
   -- Some flavour text
   tk.msg( title[1], text[2] )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Gets a empireish system
function get_emp_system( sys )
   local s = { }
   local dist = 1
   local target = {}
   while #target == 0 do
      target = getsysatdistance( sys, dist, dist+1, emp_systems_filter, s )
      dist = dist + 2
   end
   return target[rnd.rnd(1,#target)]
end

function emp_systems_filter( sys, data )
   -- Must have Empire
   if not sys:presences()["Empire"] then
      return false
   end

   -- Must not be safe
   if sys:presence("friendly") > 3.*sys:presence("hostile") then
      return false
   end

   -- Must not already be in list
   local found = false
   for k,v in ipairs(data) do
      if sys == v then
         return false
      end
   end

   return true
end

function misn_finished()
   player.msg( msg[1] )
   misn.setDesc( string.format( misn_desc2, misn_base:name(), misn_base_sys:name() ) )
   misn.markerRm( misn_marker )
   misn_marker = misn.markerAdd( misn_base_sys, "low" )
   misn.osdCreate(misn_title, {misn_desc2:format(misn_base:name())})

   hook.land("landed")
end

-- Player won, gives rewards.
function landed ()
   if planet.cur() == misn_base then
      tk.msg(title[2], text[3])

      -- Give rewards
      faction.modPlayerSingle( "Pirate", 5 )
      player.pay( 100000 ) -- 100k
      pir_modDecayFloor( 5 )

      -- Finish mission
      misn.finish(true)
   end
end


-- Entering a system
function sys_enter ()
   cur_sys = system.cur()
   -- Check to see if reaching target system
   if cur_sys == near_sys then

      -- Create the badass enemy
      p     = pilot.add(emp_ship)
      emp   = p[1]
      emp:rename(emp_name)
      emp:setHostile()
      emp:rmOutfit("all") -- Start naked
      pilot_outfitAddSet( emp, emp_outfits )
      hook.pilot( emp, "death", "misn_finished" )
      hook.pilot( emp, "jump", "emp_jump" )
   end
end


-- Empire patrol jumped away
function emp_jump ()
   player.msg( string.format(msg[2], emp_name) )

   -- Basically just swap the system
   near_sys = get_emp_system( near_sys )
end


--[[
Functions to create Empire patrols based on difficulty more easily.
--]]
function emp_generate ()
   -- Get the Empire ships's name
   emp_name = empire_name()

   -- Get the Empire patrol's details
   rating = player.getRating()
   if rating < 100 then
      emp_ship, emp_outfits = emp_easy()
   elseif rating < 200 then
      emp_ship, emp_outfits = emp_medium()
   else
      emp_ship, emp_outfits = emp_hard()
   end

   -- Make sure to save the outfits.
   emp_outfits["__save"] = true

   return emp_name, emp_ship, emp_outfits
end
function emp_easy ()
   if rnd.rnd() < 0.5 then
      return empire_createShark(false)
   else
      return empire_createLancelot(false)
   end
end
function emp_medium ()
   if rnd.rnd() < 0.5 then
      return empire_createAdmonisher(false)
   else
      return empire_createPacifier(false)
   end
end
function emp_hard ()
   if rnd.rnd() < 0.5 then
      return empire_createHawking(false)
   else
      return empire_createPeacemaker(false)
   end
end

