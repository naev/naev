--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Stranded in Gliese">
 <location>enter</location>
 <chance>100</chance>
 <cond>system.cur() == system.get("Gliese") and not var.peek("gliese_lockedin")</cond>
</event>
--]]

--[[
   Stranded in Gliese

   This event triggers when you enter the Gliese-G Minor pocket in the nebula that you CAN'T escape except through the quest in there. It's meant to foreshadow you being trapped, and adds another checkpoint from before you jumped.
--]]

local vn = require "vn"
local tut = require "common.tutorial"

function create ()
   player.saveBackup("pre-stranding")
   var.push("gliese_lockedin", true)
   local _j, r = jump.get("Behar", "Gliese")
   r = r:pos()
   diff.apply("gliese_lockin")
   hook.timer(5, "ohno", r)
end

function ohno(pos)
   camera.set(pos)
---[[
   hook.timer(1, "cheated")
end

function cheated()
--]]
   vn.clear()
   vn.scene()
   local sai = vn.newCharacter(tut.vn_shipai())
   vn.appear( sai, tut.shipai.transition )
   sai(_([["This system does not have an exit jump point."]]))
   sai(_([["It appears that the scavenger tricked us."]]))
   sai(_([["I suggest we explore the system, we may find another way out."]]))
   vn.disappear( sai, tut.shipai.transition )
   vn.run()
---[[
   hook.enter("cleanup")
end

function cleanup ()
   camera.set()
   evt.finish()
--]]
end
