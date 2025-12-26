
--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Restricted Zone NGC-11718">
 <location>enter</location>
 <chance>100</chance>
 <system>NGC-11718</system>
</event>
--]]
local equipopt = require "equipopt"

local AURA = outfit.get("Executor Shield Aura")

function executor_death ()
   local k = var.peek("executors_killed") or 0
   var.push("executors_killed",k+1)
end

local function spawn_executor ()
   local p = pilot.add("Empire Peacemaker", "Empire", nil, _("Executor Archibald"), {naked=true, ai="pers_patrol"})
   p:outfitAddIntrinsic("Escape Pod")
   equipopt.empire( p, {
      outfits_add={"Executor Shield Aura"},
      prefer={["Executor Shield Aura"] = 100}} )
   local m = p:memory()
   m.capturable = true
   m.lootable_outfit = AURA
   m.bribe_no = _([["I shall particularly enjoy your execution."]])

   hook.pilot( p, "death", "executor_death" )
end

function create ()
   if player.outfitNum( AURA ) <= 0 or rnd.rnd() < 0.5 then
      spawn_executor()
   end
end
