local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Empire"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   -- Larger ships can be there
   if presence > 200 then
      local function executor_spawn( name, ad, taunt )
         return function ()
            local p = pilot.add("Empire Peacemaker", "Empire", nil, name, {naked=true, ai="pers_patrol"})
            p:outfitAddIntrinsic("Escape Pod")
            equipopt.empire( p, {
               outfits_add={"Executor Shield Aura"},
               prefer={["Executor Shield Aura"] = 100}} )
            local m = p:memory()
            m.capturable = true
            m.lootable_outfit = outfit.get("Executor Shield Aura")
            m.ad = ad
            m.comm_greet = ad
            m.taunt = taunt
            m.bribe_no = _([["I shall particularly enjoy your execution."]])
            return p
         end
      end
      local function executor_killed ()
         local k = var.peek("executors_killed") or 0
         var.push("executors_killed",k+1)
      end
      for k,v in ipairs{
         {
            spawn = executor_spawn( _("Executor Lee"),
               _([[Justice is swift and decisive.]]),
               _("It is time for your execution!") ),
            ondeath = executor_killed,
            w = 0.5,
         }, {
            spawn = executor_spawn( _("Executor Bismuth"),
               _([[The justice of the Empire is swift and merciless.]]),
               _("You will be swiftly executed!") ),
            ondeath = executor_killed,
            w = 0.5,
         }, {
            spawn = executor_spawn( _("Executor Jadiker"),
               _([[Nobody escapes the Emperor's judgement!]]),
               _("Your time of judgement has come!") ),
            ondeath = executor_killed,
            w = 0.5,
         }, {
            spawn = executor_spawn( _("Executor Spizza"),
               _([[We are all but vermin in the shadow of the Emperor.]]),
               _("You made a dire mistake, vermin!") ),
            ondeath = executor_killed,
            w = 0.5,
         },
      } do
         table.insert( pers, v )
      end
   end

   -- Medium ships here
   if presence > 100 then
      for k,v in ipairs{
         {
            spawn = function ()
               -- ECB stands for Empire Combat Bureaucrat
               local p = pilot.add("Empire Pacifier Hoplite", "Empire", nil, _("ECB Bolten"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet( "fwd_damage", 10 )
               p:intrinsicSet( "shield_mod", 25 )
               equipopt.empire( p, {
                  turret=0, beam=0, fighterbay=0,
                  outfits_add = {"Unicorp Storm Launcher",}
               } )
               local m = p:memory()
               m.capturable = true
               m.ad = _("Empire is recruiting new Combat Bureaucrats. Inquire at your nearest Bureau.")
               m.comm_greet = _([["You wouldn't be interested in becoming an Empire Combat Bureaucrat, would you?"]])
               m.taunt = _("You just signed your own death warrant, and here comes the certificate!")
               m.bribe_no = _([["You didn't fill in the EB-2781 request for bribe form!"]])
               local pos = p:pos()
               local vel = p:vel()
               for i=1,3 do
                  local e = pilot.add("Empire Lancelot", "Empire", pos, _("ECB Intern") )
                  local em = p:memory()
                  em.capturable = true
                  em.comm_no = _([["I don't have the authority to talk to you."]])
                  e:setVel(vel)
                  e:setLeader(p)
               end
               return p
            end,
         },
         {
            spawn = function ()
               -- ECB stands for Empire Combat Bureaucrat
               local p = pilot.add("Empire Pacifier Hoplite", "Empire", nil, _("ECB Payne"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet( "tur_damage", 25 )
               equipopt.empire( p, {
                  forward=0, beam=0,
               } )
               local m = p:memory()
               m.capturable = true
               m.ad = _("Want to be a Combat Bureaucrat? Apply to your nearest Bureau! Some terms may apply.")
               m.comm_greet = _([["I sleep well at night on my pile of finished paperwork."]])
               m.taunt = _("I'll happly fill our your death form!")
               m.bribe_no = _([["You need to fill in the EB-2781 request for bribing."]])
               local pos = p:pos()
               local vel = p:vel()
               for i=1,3 do
                  local e = pilot.add("Empire Shark", "Empire", pos, _("ECB Intern") )
                  local em = p:memory()
                  em.capturable = true
                  em.comm_no = _([["I don't have the authority to talk to you."]])
                  e:setVel(vel)
                  e:setLeader(p)
               end
               return p
            end,
         },
         {
            spawn = function ()
               -- ECB stands for Empire Combat Bureaucrat
               local p = pilot.add("Empire Pacifier Hoplite", "Empire", nil, _("ECB O'Connor"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet( "fbay_damage", 25 )
               equipopt.empire( p, {
                  beam=0, fighterbay=5,
               } )
               local m = p:memory()
               m.capturable = true
               m.comm_no = _([["Please file an appointment with my secretary."]])
               m.taunt = _("I'll have my Secretary begin the paperwork for your defunction!")
               local pos = p:pos()
               local vel = p:vel()
               for i=1,1 do
                  local e = pilot.add("Empire Admonisher", "Empire", pos, _("ECB Secretary") )
                  local em = p:memory()
                  em.capturable = true
                  em.comm_no = _([["You'll need to schedule an appointment first."]])
                  e:setVel(vel)
                  e:setLeader(p)
               end
               return p
            end,
         },
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
