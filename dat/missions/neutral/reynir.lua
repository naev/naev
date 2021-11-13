--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Hot dogs from space">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>1</priority>
  <chance>2</chance>
  <location>Bar</location>
  <cond>(function ()
           local count = 0
           for i, p in pairs(system.cur():planets()) do
              if p:services()["inhabited"] then
                 count=count+1
              end
           end
           return count &gt; 1
        end)()</cond>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

   MISSION: Hot dogs from space
   DESCRIPTION: An old man who owns a hot dog factory wants to go to space

   The old man has elevated pressure in his cochlea so he can't go to space.
   He's getting old and wants to go to space before he dies. He owns a hot dog
   factory and will pay you in hot dogs (food). Because of his illness, you
   can't land on any planet outside the system (the player doesn't know this).

   NOTE: This mission is best suited in systems with 2 or more planets, but can
   be used in any system with a planet.

--]]

local fmt = require "format"
local neu = require "common.neutral"


function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( _("Reynir"), "neutral/unique/reynir.webp", _("You see an old man with a cap on, on which the letters R-E-Y-N-I-R are imprinted.") )

   -- Mission variables
   mem.misn_base, mem.misn_base_sys = planet.cur()
   mem.misn_bleeding = false
end


function accept ()
   -- make sure there are at least 2 inhabited planets
   if (function ()
            local count = 0
            for i, p in ipairs (system.cur():planets()) do
               if p:services()["inhabited"] then
                  count=count+1
               end
            end
            return count > 1
         end) ()
      and tk.yesno( _("Spaceport Bar"), _([["Do you like money?"]]) ) and tk.yesno( _("Spaceport Bar"), _([["Ever since I was a kid I've wanted to go to space. However, my doctor says I can't go to space because I have an elevated pressure in my cochlea, a common disease around here.
    "I am getting old now, as you can see. Before I die I want to travel to space, and I want you to fly me there! I own a hot dog factory, so I can reward you richly! Will you do it?"]]) ) then

      misn.accept()  -- For missions from the Bar only.

      misn.setTitle( _("Rich reward from space!") )
      misn.setReward( _("Lots of cash") )
      misn.setDesc( _("Reynir wants to travel to space and will reward you richly.") )
      hook.land( "landed" )

      tk.msg( _("Reynir"), fmt.f(_([["Thank you so much! Just fly me around in the system, preferably near {pnt}."]]), {pnt=mem.misn_base}) )
      misn.osdCreate(_("Rich reward from space!"), {fmt.f(_("Fly around in the system, preferably near {pnt}"), {pnt=mem.misn_base})})
      local c = misn.cargoNew( N_("Reynir"), N_("A old man who wants to see space.") )
      mem.cargoID = misn.cargoAdd( c, 0 )
   end

end


function landed()
   -- If landed on mem.misn_base then give reward
   if planet.cur() == mem.misn_base then
      misn.cargoRm( mem.cargoID )
      local reward_text, log_text
      if mem.misn_bleeding then
         mem.reward = math.min(1, player.pilot():cargoFree())
         reward_text = _([[Reynir doesn't look happy when you meet him outside the ship.
    "I lost my hearing out there! Damn you!! I made a promise, though, so I'd better keep it. Here's your reward, {tonnes} of hot dogs..."]])
         log_text = _([[You took an old man named Reynir on a ride in outer space, but he was made very angry because the distance you traveled led to him getting injured and losing his hearing. Still, he begrudgingly paid you in the form of {tonnes} of hot dogs.]])
      else
         mem.reward = player.pilot():cargoFree()
         reward_text = _([["Thank you so much! Here's {tonnes} of hot dogs. They're worth more than their weight in gold, aren't they?"]])
         log_text = _([[You took an old man named Reynir on a ride in outer space. He was happy and paid you in the form of {tonnes} of hot dogs.]])
      end
      tk.msg( _("Reynir"), fmt.f( reward_text, {tonnes=fmt.tonnes(mem.reward)} ) )
      player.pilot():cargoAdd( "Food", mem.reward )
      neu.addMiscLog( fmt.f( log_text, {tonnes=fmt.tonnes(mem.reward)} ) )
      misn.finish(true)
   -- If we're in mem.misn_base_sys but not on mem.misn_base then...
   elseif system.cur() == mem.misn_base_sys then
      tk.msg( _("Reynir"), fmt.f(_([[Reynir walks out of the ship, amazed by the view. "So this is how {pnt_cur} looks like! I've always wondered... I want to go back to {pnt} now, please."]]), {pnt_cur=planet.cur(), pnt=mem.misn_base}) )
      misn.osdCreate(_("Rich reward from space!"), {fmt.f(_("Take Reynir home to {pnt}"), {pnt=mem.misn_base})})
   -- If we're in another system then make Reynir bleed out his ears ;)
   else
      tk.msg( _("Reynir"), fmt.f(_([[Reynir walks out of the ship. You notice that he's bleeding out of both ears. "Where have you taken me?! Get me back to {pnt} right now!!"]]), {pnt=mem.misn_base}) )
      misn.osdCreate(_("Rich reward from space!"), {fmt.f(_("Take Reynir home to {pnt}"), {pnt=mem.misn_base})})
      mem.misn_bleeding = true
   end
end

-- TODO: There should probably be more here
function abort ()
   -- Remove the passenger.
   misn.cargoRm( mem.cargoID )
   misn.finish(false)
end
