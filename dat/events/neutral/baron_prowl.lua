--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Prowling baron">
 <location>enter</location>
 <chance>100</chance>
 <system>Ingot</system>
 <cond>player.misnActive("Baron")==false and player.misnActive("Prince")==false</cond>
 <notes>
  <campaign>Baron Sauterfeldt</campaign>
 </notes>
</event>
--]]
--[[
-- Prowl Event for the Baron mission string. Only used when NOT doing any Baron missions.
--]]

local pnt = spob.get("Ulios")

function create()
   -- TODO: Change this to the Krieger once the Baron has it. Needs "King" mission first.
   local baronship = pilot.add( "Proteron Gauss", "Independent", pnt:pos() + vec2.new(-400,-400), _("Pinnacle"), {ai="trader"} )
   baronship:setInvincible(true)
   baronship:setFriendly()
   baronship:control()
   baronship:moveto(pnt:pos() + vec2.new( 500, -500), false, false)
   hook.pilot(baronship, "idle", "idle")
end

function idle(baronship)
   baronship:moveto(pnt:pos() + vec2.new( 500,  500), false, false)
   baronship:moveto(pnt:pos() + vec2.new(-500,  500), false, false)
   baronship:moveto(pnt:pos() + vec2.new(-500, -500), false, false)
   baronship:moveto(pnt:pos() + vec2.new( 500, -500), false, false)
end
