--[[

   Autoequipper for the player

--]]
local equipopt = require 'equipopt'
function autoequip( p )
   local s = p:ship()

   -- First check, make sure required slots are equipped
   local hasrequired = true
   for k,v in ipairs( s:getSlots() ) do
      if v.required and not p:outfitGet(k) then
         hasrequired = false
         break
      end
   end
   if not hasrequired then
      tk.msg(_("Missing Required Outfits"), _("Please equip all required outfit slots first!"))
      return
   end

   -- Make sure
   if not tk.yesno(_("Autoequip Confirmation"), _("This will re-equip your current pilot, are you sure you want to continue?")) then
      return
   end

   -- Remove all non-cores from the ship and add to inventory
   for k,v in ipairs( s:getSlots() ) do
      if not v.required and not v.locked then
         local o = p:outfitGet(k)
         if o then
            -- Store and remove old
            player.outfitAdd( o )
            p:outfitRm( o )
         end
      end
   end

   -- Get available outfits
   local poutfits_raw = player.outfits() -- Should include removed ones
   -- Get rid of cores
   local poutfits = {}
   for k,o in ipairs(poutfits_raw) do
      local _sn, _ss, _sd, req, _exc = o:slot()
      if not req then
         table.insert( poutfits, o )
      end
   end

   -- Set parameters, making sure we don't equip too many of the same
   local opt_params = equipopt.params.choose( p )
   for k,o in ipairs(poutfits) do
      opt_params.type_range[ o:nameRaw() ] = { max = player.outfitNum(o, true) }
   end

   -- Do equipopt
   local success = equipopt.optimize.optimize( p, nil, poutfits, opt_params )

   -- Remove equipped outfits from inventory
   for k,v in ipairs( s:getSlots() ) do
      if not v.required and not v.locked then
         local o = p:outfitGet(k)
         if o then
            player.outfitRm( o )
         end
      end
   end

   -- Report success or failure
   if not success then
      tk.msg(_("Autoequipper Failure!"),_("Autoequipper failed to equip! Please make sure that the equipped required outfits are adequate for this ship."))
   end
   return success
end
