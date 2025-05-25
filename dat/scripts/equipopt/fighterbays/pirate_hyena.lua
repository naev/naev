local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Pirate Hyena"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {}
         o.core_systems = outfit.get("Unicorp PT-16 Core System")
         o.core_engines = outfit.get("Unicorp Hawk 160 Engine")
         o.core_hull = outfit.get("Unicorp D-2 Light Plating")
         o.weap1 = outfit.get("Plasma Blaster MK1")
         o.weap2 = outfit.get("Laser Cannon MK1")
         o.weap3 = outfit.get("Ion Cannon")
         o.stru1 = outfit.get("Reactor Class I")
      end
      -- Cores
      p:outfitAddSlot( o.core_systems, "systems" )
      p:outfitAddSlot( o.core_engines, "engines" )
      p:outfitAddSlot( o.core_hull, "hull" )
      p:outfitAdd( o.weap1 )
      p:outfitAdd( o.weap2 )
      p:outfitAdd( o.weap3 )
      p:outfitAdd( o.stru1 )
      return false
   end,
}
