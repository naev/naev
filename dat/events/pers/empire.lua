local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local pres = scur:presences()["Empire"] or 0
   if pres <= 0 then
      return nil -- Need at least some presence
   end

   -- Larger ships can be there
   if pres > 100 then
      local function spawn_executor( name, ad, taunt )
         return function ()
            local p = pilot.add("Empire Peacemaker", "Empire", nil, name, {naked=true, ai="pers_patrol"})
            equipopt.empire( p )
            local m = p:memory()
            m.ad = ad
            m.comm_greet = ad
            m.taunt = taunt
            m.bribe_no = _("I shall particularly enjoy your execution.")
            return p
         end
      end
      for k,v in ipairs{
         {
            spawn = spawn_executor( _("Executor Lee"),
               _([["Justice is swift and decisive."]]),
               _("It is time for your execution!") ),
            w = 0.5,
         }, {
            spawn = spawn_executor( _("Executor Bismuth"),
               _([["The justice of the Empire is swift and merciless."]]),
               _("You will be swiftly executed!") ),
            w = 0.5,
         }, {
            spawn = spawn_executor( _("Executor Jadiker"),
               _([["Nobody escapes the Emperor's judgement!"]]),
               _("Your time of judgement has come!") ),
            w = 0.5,
         }, {
            spawn = spawn_executor( _("Executor Spizza"),
               _([["We are all but vermin in the shadow of the Emperor."]]),
               _("You made a dire mistake, vermin!") ),
            w = 0.5,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
