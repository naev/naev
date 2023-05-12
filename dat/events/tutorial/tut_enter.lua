--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Enter Tutorial Event">
 <location>enter</location>
 <chance>100</chance>
</event>
--]]
--[[

   Enter Tutorial Event

--]]
local fmt = require "format"
local tut = require "common.tutorial"
local vn  = require "vn"

function create ()
   if tut.isDisabled() then evt.finish() end

   local enter_delay = 5

   hook.land( "evt_done" )

   -- TODO we should probably allow looking at any faction not just empire
   if not var.peek("tut_illegal") and player.pilot():hasIllegal("Empire") then
      hook.timer( enter_delay, "tut_illegal" )
      return
   end

   local sys = system.cur()
   local _nebu_dens, nebu_volat = sys:nebula()
   if not var.peek( "tut_nebvol" ) and nebu_volat > 0 then
      hook.timer( enter_delay, "tut_volatility" )
      return
   end
end

function evt_done ()
   evt.finish()
end

function tut_illegal ()
   local pp = player.pilot()
   local badstuff = {}
   for k,o in ipairs(pp:outfitsList()) do
      if #o:illegality() > 0 then
         table.insert( badstuff, o )
      end
   end
   for k,v in ipairs(pp:cargoList()) do
      local c = commodity.get(v.name)
      if #c:illegality() > 0 then
         table.insert( badstuff, c )
      end
   end
   if #badstuff<=0 then
      warn("tut_illegal called, but can't find badstuff!")
      return
   end

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(fmt.f(_([[Just as after taking off, {ainame} materializes in front of you.
   "I don't know how to say this to you, but it seems like you have acquired some illegal items. Your {item} is on the rather dubious side of the law, and you will run into trouble if you are scanned by patrols. Sometimes, right after you are discovered, you may get away with paying a small fine in the form of a bribe, but that may not always be the case."]]),
      {item=badstuff[ rnd.rnd(1,#badstuff) ], ainame=tut.ainame()} ))
   sai(fmt.f(_([["If you have to deal with illegal goods, I would recommend you to try to maximize the stealth functionality of your ship, that you can enable with {stealthkey}. As ship detection in general is tied to ship mass, you will most likely have best result using small and agile ships to avoid detection. Remember to stay off patrol routes and avoid crowded systems for highest chance of success."]]),
      {stealthkey=tut.getKey("stealth")} ))
   sai(fmt.f(_([["You can check to see if your commodities or outfits are illegal from the #bInfo Menu#0 which you can open with {infokey}, and then looking at your ship outfits or commodities. Note that illegality is determined on a per-faction basis instead of globally."]]),
      {infokey=tut.getKey("info")}))
   vn.na(fmt.f(_([[{ainame} vanishes and you are left wondering why they are so knowledgable about illegal activities.]]),{ainame=tut.ainame()}))
   vn.done( tut.shipai.transition )
   vn.run()

   var.push( "tut_illegal", true )
   evt.finish()
end

function tut_volatility ()
   local sys = system.cur()
   local _nebden, nebvol = sys:nebula()

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[As you jump the system you notice a small alarm lights up in the control panel:
#rWARNING - Volatile nebula detected in {sys}! Taking {nebvol:.1f} MW damage!#0]]),{sys=sys, nebvol=nebvol}))
   sai(fmt.f(_([[{ainame} materializes in front of you.
"It looks like we entered part of the volatile nebula. The instability here causes heavy damage to any ships that enter. If our shield regeneration surpasses the volatility damage, we should be fine. However, if the volatility gets any stronger, it could be fatal to the {ship}. Going deeper into the nebula could prove to be a very risky endeavour."]]), {ship=player.ship(), ainame=tut.ainame()} ) )
   vn.done( tut.shipai.transition )
   vn.run()

   var.push( "tut_nebvol", true )
   evt.finish()
end
