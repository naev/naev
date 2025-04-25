
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
function smfs.init(p)
Example usage

> fs = require("shipmemfs")
  Inited shipmemfs with pilot "aze"

> fs.ls()
  _ec/
  _ec_total/
  _ec_needs_refresh : false

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
  _ec/3/halted : true
  _ec/3/turn : 108
  _ec/3/accel : 109
  _ec/3/speed : 201
  _ec_total/
  _ec_total/engine_limit : 630
  _ec_total/turn : 131
  _ec_total/accel : 203
  _ec_total/speed : 222
  _ec_needs_refresh : false

> fs.cd('_ec')
 /_ec/

> fs.cd(2)
 /_ec/2/

> fs.ls()
  engine_limit : 630
  _parent_/
  accel : 203
  turn : 131
  part : 100
  speed : 222

> fs.cd('/_ec_total')
 /_ec_total/

> fs.cd('..')
 /

> fs.cd(fs.path._ec[2])
 /_ec/2/

> fs.ls()
  engine_limit : 630
  _parent_/
  accel : 203
  turn : 131
  part : 100
  speed : 222
--]]

function smfs.init(pil)
   local smfs_pilot = pil

   if smfs_pilot == nil then
      smfs_pilot = player.pilot()
   end

   print('  Inited shipmemfs with pilot "' .. smfs_pilot:name() .. '"')

   local function root()
      return smfs_pilot:shipMemory()
   end

   smfs.path = root()

   local function _pwd( t )
      local p = t._parent_
      if p == nil then
         return '/'
      else
         for v,k in pairs(p) do
            if k == t then
               return _pwd(p) .. tostring(v) .. '/'
            end
         end
         return '???/'
      end
   end

   function smfs.pwd( )
      print(" " .. _pwd(smfs.path))
   end

   local function _cd( v, create )
      local ptr = smfs.path
      if create then
         ptr = ptr or {}
      end

      for _i,k in ipairs(v) do
         if k == '/' then
            ptr = root()
         else
            local prv = ptr
            ptr = ptr[k]
            if create then
               ptr = ptr or {}
            end
            if ptr and k ~= "_parent_" then
               ptr["_parent_"] = prv
            end
         end
      end
      return ptr
   end

   local function strl(l)
      local acc = '{ '
      for _i,k in ipairs(l) do
         acc = acc .. tostring(k) .. ', '
      end
      return acc .. '}'
   end

   local function cd_silent( v )
      if type(v) == 'table' then
         if v and v['_parent_'] then
            smfs.path = v
            return true
         else
            print(' Not a valid dir.')
            return false
         end
      else
         local l = {}
         if type(v) == 'string' then
            local fst = true
            for str in string.gmatch(v, "([^/]*)") do
               if str == '' then
                  if fst then
                     table.insert(l, '/')
                  end
               elseif str == ".." then
                  table.insert(l, "_parent_")
               else
                  table.insert(l, str)
               end
               fst = false
            end
         else
            table.insert(l, v)
         end

         local res = _cd( l )
         if res then
            smfs.path = _cd( l, true)
            return true
         else
            print(' Cannot enter "' .. strl(l) .. '"')
            return false
         end
      end
   end

   function smfs.cd( v )
      local res=cd_silent( v )
      if res then
         smfs.pwd()
      end
      return res
   end

   function smfs.ls( )
      if type(smfs.path) == 'table' then
         for k,v in pairs(smfs.path) do
            if type(v) == 'table' then
               print("  " .. tostring(k) .. "/")
            else
               print("  " ..tostring(k) .. " : " .. tostring(v))
            end
         end
      end
   end

   function smfs.find( pref, hide )
      if pref == nil then
         pref = "  "
      end
      if type(smfs.path) == 'table' then
         for k,v in pairs(smfs.path) do
            if k ~= "_parent_" then
               if type(v) == 'table' then
                  local pref2 = pref .. tostring(k) .. "/"
                  if not hide then
                     print(pref2)
                  end
                  if cd_silent(k) then
                     smfs.find(pref2, hide)
                     cd_silent("_parent_")
                  end
               else
                  if not hide then
                     print(pref .. tostring(k) .. " : " .. tostring(v))
                  end
               end
            end
         end
      end
   end

   smfs.find(nil,true)
end

if inspect then
   smfs.init(player.pilot():target() or player.pilot())
end

return smfs

