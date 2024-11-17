--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Explosion Benchmark">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
--[[
--]]
local pilots = {}
local function test( ships, fct, y )
   table.sort( ships, function( a, b )
      local sa = ship.get(a)
      local sb = ship.get(b)
      return sa:size() < sb:size()
   end )

   for k,v in ipairs( ships ) do
      local p = pilot.add( v, fct, player.pos() + vec2.new( -800+150*k, y+75 ) )
      p:control()
      table.insert( pilots, p )
   end
end

function create ()
   player.pilot():setPos( vec2.new(50e6, 0) )
   player.cinematics( true )
   player.pilot():setInvincible(true)
   player.pilot():setHide(true)

   test({
      "Soromid Arx",
      "Soromid Brigand",
      "Soromid Copia",
      "Soromid Ira",
      "Soromid Marauder",
      "Soromid Nyx",
      "Soromid Odium",
      "Soromid Reaver",
      "Soromid Vox",
   }, "Soromid", 300 )

   test({
      "Sirius Divinity",
      "Sirius Dogma",
      "Sirius Fidelity",
      "Sirius Preacher",
      "Sirius Providence",
      "Sirius Shaman",
   }, "Sirius", 150 )

   test({
      "Za'lek Demon",
      "Za'lek Diablo",
      "Za'lek Hephaestus",
      "Za'lek Mammon",
      "Za'lek Mephisto",
      "Za'lek Sting",
      "Za'lek Heavy Drone",
      "Za'lek Bomber Drone",
      "Za'lek Light Drone",
      "Za'lek Scout Drone",
   }, "Za'lek", 0 )

   test({
      "Dvaered Ancestor",
      "Dvaered Arsenal",
      "Dvaered Goddard",
      "Dvaered Phalanx",
      "Dvaered Retribution",
      "Dvaered Vendetta",
      "Dvaered Vigilance",
   }, "Za'lek", -150 )

   test({
      "Empire Admonisher",
      "Empire Hawking",
      "Empire Lancelot",
      "Empire Pacifier",
      "Empire Peacemaker",
      "Empire Rainmaker",
      "Empire Shark",
   }, "Empire", -300 )

   --[[
   test({
      "Thurion Apprehension",
      "Thurion Certitude",
      "Thurion Ingenuity",
      "Thurion Perspicacity",
      "Thurion Scintillation",
      "Thurion Taciturnity",
      "Thurion Virtuosity",
   }, "Thurion", -450 )
   --]]

   hook.timer( 5, "start" )
end

function start ()
   for k,p in ipairs(pilots) do
      p:setHealth(-1,-1)
   end
end
