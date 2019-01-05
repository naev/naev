--[[
   This is a reputation sanity event that is run on load and just recreates
   all the reputation changes to make sure it's sane even if we change
   reputation caps.
--]]

reputations = {
   { name="Empire Shipping 3",      var="_fcap_empire",  inc=5 },
   { name="Operation Cold Metal",   var="_fcap_empire",  inc=10 },
   { name="Destroy the FLF base!",  var="_fcap_dvaered", inc=5 }
}

function create ()
   -- Create updates
   local vars = {}
   for _,v in ipairs(reputations) do
      if player.misnDone( v.name ) then
         local n = vars[ v.var ] or 0
         vars[ v.var ] = n + v.inc
      end
   end

   -- Propagate updates
   for k,v in pairs(vars) do
      local new = 30+v
      local cur = var.peek( k )
      if new ~= cur then
         print( string.format( _("Inconsistent reputation cap detected - updating '%s'"), k ) )
         var.push( k, new )
      end
   end

   -- Done
   evt.finish()
end


