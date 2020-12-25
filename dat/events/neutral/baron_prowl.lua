--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Prowling baron">
  <trigger>enter</trigger>
  <chance>100</chance>
  <cond>player.misnActive("Baron") == false and player.misnActive("Prince") == false and system.cur() == system.get("Ingot")</cond>
  <flags>
  </flags>
  <notes>
   <campaign>Baron Sauterfeldt</campaign>
  </notes>
 </event>
 --]]
--[[
-- Prowl Event for the Baron mission string. Only used when NOT doing any Baron missions.
--]]

function create()
    -- TODO: Change this to the Krieger once the Baron has it. Needs "King" mission first.
    shipname = "Pinnacle"
    baronship = pilot.add("Proteron Kahan", "trader", planet.get("Ulios"):pos() + vec2.new(-400,-400))[1]
    baronship:setFaction("Civilian")
    baronship:rename(shipname)
    baronship:setInvincible(true)
    baronship:setFriendly()
    baronship:control()
    baronship:moveto(planet.get("Ulios"):pos() + vec2.new( 500, -500), false, false)
    hook.pilot(baronship, "idle", "idle")
end

function idle()
    baronship:moveto(planet.get("Ulios"):pos() + vec2.new( 500,  500), false, false)
    baronship:moveto(planet.get("Ulios"):pos() + vec2.new(-500,  500), false, false)
    baronship:moveto(planet.get("Ulios"):pos() + vec2.new(-500, -500), false, false)
    baronship:moveto(planet.get("Ulios"):pos() + vec2.new( 500, -500), false, false)
end
