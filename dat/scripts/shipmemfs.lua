
local smfs = {}

function smfs.read( p, path )
   local ptr = p and p:shipMemory() or {}
   for _i,k in ipairs(path) do
      ptr = (ptr or {})[k]
   end
   return ptr
end

function smfs.write( p, path, value )
   if p and p:shipMemory() == nil then
      return
   end

   local ptr = p:shipMemory()
   local prv
   for i,k in ipairs(path) do
      if i > 1 then
         ptr[prv] = ptr[prv] or {}
         ptr = ptr[prv]
      end
      prv = k
   end
   local bef = ptr[prv]
   ptr[prv] = value
   return (bef ~= value)
end


--[[
Example usage

> fs = require("shipmemfs")
> fs.init()
> fs.ls()
_ec/
_ec_total /
_ec_needs_refresh:false
> fs.cd()
> fs.ls()
2/
3/
../
> fs.cd()
> fs.ls()
_ec/
_ec_total /
_ec_needs_refresh:false
> fs.find()
_ec/
_ec/2/
_ec/2/engine_limit : 630
_ec/2/accel : 203
_ec/2/turn : 131
_ec/2/part : 100
_ec/2/speed : 222
_ec/3/
_ec/3/engine_limit : 475
_ec/3/status : true
_ec/3/turn : 108
_ec/3/accel : 109
_ec/3/speed : 201
_ec_total/
_ec_total/engine_limit : 630
_ec_total/turn : 131
_ec_total/accel : 203
_ec_total/speed : 222
_ec_needs_refresh : false
--]]

function smfs.init(p)
   local smfs_pilot = p

   if smfs_pilot == nil then
      smfs_pilot = player.pilot()
   end

   path = smfs_pilot:shipMemory()

   local function cd1(ptr, val)
      if val == '/' then
         return smfs_pilot:shipMemory()
      else
         return ptr[val]
      end
   end

   local function _cd( v, create )
      local ptr = path
      if create then
         ptr = ptr or {}
      end

      if type(v) ~= 'table' then
         v = {v}
      end

      for _i,k in ipairs(v) do
         ptr = cd1( ptr, k )
         if create then
            ptr = ptr or {}
         end
      end
      return ptr
   end

   function smfs.cd( v )
      local res = _cd( v )
      if res then
         if v~=".." then
            res['..'] = path
         end
         path = _cd( v, true)
         return true
      else
         print "Can't do it."
         return false
      end
   end

   function smfs.ls( )
      if type(path) == 'table' then
         for k,v in pairs(path) do
            if type(v) == 'table' then
               print(tostring(k) .. "/")
            else
               print(tostring(k) .. " : " .. tostring(v))
            end
         end
      end
   end

   function smfs.find( pref )
      if pref == nil then
         pref = ""
      end
      if type(path) == 'table' then
         for k,v in pairs(path) do
            if k ~= '..' then
               if type(v) == 'table' then
                  local pref2 = pref .. tostring(k) .. "/"
                  print(pref2)
                  if k ~= '..' and smfs.cd(k) then
                     smfs.find(pref2)
                     smfs.cd('..')
                  end
               else
                  print(pref .. tostring(k) .. " : " .. tostring(v))
               end
            end
         end
      end
   end
end

if inspect ~= nil then
   smfs.init(player.pilot():target() or player.pilot())
end

return smfs

