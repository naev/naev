local var = {}

local function _v( tbl )
   if #tbl==1 then
      return function () return tbl[1].s end
   end

   table.sort( tbl, function ( a, b )
      return a.w > b.w
   end )
   local wmax = 0
   for k,v in ipairs(tbl) do
      wmax = wmax+v.w
      if not v.s then
         warn(_("Variant has s==nil!"))
      end
   end
   return function ()
      local r = rnd.rnd()*wmax
      local w = 0
      for k,v in ipairs(tbl) do
         w = w+v.w
         if r <= w then
            return v.s
         end
      end
   end
end

-- Yacht
var.llama = _v{
   { w=1,    s=ship.exists("Llama") },
   { w=0.05, s=ship.exists("Llama Voyager") },
}
var.gawain = _v{
   { w=1,    s=ship.exists("Gawain") },
   { w=0.05,    s=ship.exists("Gawain XY-37") },
}
-- Courier
var.koala = _v{
   { w=1,    s=ship.exists("Koala") },
   { w=0.05, s=ship.exists("Koala Armoured") },
}
var.quicksilver = _v{
   { w=1,    s=ship.exists("Quicksilver") },
   { w=0.05, s=ship.exists("Quicksilver Mercury") },
}
-- Freighter
var.mule = _v{
   { w=1,    s=ship.exists("Mule") },
   { w=0.05, s=ship.exists("Mule Hardhat") },
}
-- Armoured Transport
var.rhino = _v{
   { w=1,    s=ship.exists("Rhino") },
}
-- Scout
var.schroedinger = _v{
   { w=1,    s=ship.exists("Schroedinger") },
}
-- Interceptor
var.shark = _v{
   { w=1,    s=ship.exists("Shark") },
   { w=0.05, s=ship.exists("Shark ΨIIIa") },
}
var.pirate_shark = _v{
   { w=1,    s=ship.exists("Pirate Shark") },
   { w=0.05, s=ship.exists("Pirate Blue Shark") },
}
-- Fighter
var.vendetta = _v{
   { w=1,    s=ship.exists("Vendetta") },
   { w=0.05, s=ship.exists("Vendetta Whiplash") },
}
var.empire_lancelot = _v{
   { w=1,    s=ship.exists("Empire Lancelot") },
   { w=0.05, s=ship.exists("Empire Lancelot Golden Efreeti") },
}
-- Bomber
var.ancestor = _v{
   { w=1,    s=ship.exists("Ancestor") },
   { w=0.05, s=ship.exists("Ancestor HG Eagle-Eye") },
}
var.dvaered_ancestor = _v{
   { w=1,    s=ship.exists("Dvaered Ancestor") },
   { w=0.05, s=ship.exists("Dvaered Ancestor Calamity") },
}
-- Corvette
var.admonisher = _v{
   { w=1,    s=ship.exists("Admonisher") },
   { w=0.05, s=ship.exists("Admonisher ΩIIa") },
}
var.empire_admonisher = _v{
   { w=1,    s=ship.exists("Empire Admonisher") },
   { w=0.05, s=ship.exists("Empire Admonisher Longbow") },
}
var.zalek_sting= _v{
   { w=1,    s=ship.exists("Za'lek Sting") },
   { w=0.05, s=ship.exists("Za'lek Sting Type II") },
   { w=0.05, s=ship.exists("Za'lek Sting Type IV") },
}
-- Destroyer
var.starbridge = _v{
   { w=1,    s=ship.exists("Starbridge") },
   { w=0.05, s=ship.exists("Starbridge Sigma") },
}
var.zalek_demon = _v{
   { w=1,    s=ship.exists("Za'lek Demon") },
   { w=0.05, s=ship.exists("Za'lek Demon Type IV") },
}
var.empire_pacifier = _v{
   { w=1,    s=ship.exists("Empire Pacifier") },
   { w=0.05, s=ship.exists("Empire Pacifier Hoplite") },
}
-- Cruiser
var.kestrel = _v{
   { w=1,    s=ship.exists("Kestrel") },
   { w=0.05, s=ship.exists("Kestrel Sigma") },
}
var.pirate_kestrel = _v{
   { w=1,    s=ship.exists("Pirate Kestrel") },
   { w=0.05, s=ship.exists("Pirate Kestrel Yuri's Kiss") },
}
-- Battleship
var.goddard = _v{
   { w=1,    s=ship.exists("Goddard") },
}
var.zalek_mephisto = _v{
   { w=1,    s=ship.exists("Za'lek Mephisto") },
   { w=0.05, s=ship.exists("Za'lek Mephisto Type V") },
}

return var
