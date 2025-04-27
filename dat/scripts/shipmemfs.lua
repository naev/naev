
--[[
This module contains:
 1. smfs API
 2. smfs Console Interface
--]]


local smfs = {}

--[[
## 1. smfs API

The following functions can't alter the fs (read only and don't return fs mutables):
 - function smfs.listdir( ptr )
 - function smfs.readdir( p, path )
 - function smfs.readfile( p, path )

The following function may create dirs and will change the file value if any:
function smfs.writefile( p, path, value )

The following function may create dirs and will return the dir, allowing to modify it:
function smfs.checkdir( p, path )

--]]

-- Input:
--  - a dir ( a table in some shipMemory structure )
--
-- Returns:
--  - a table of the dir contents.
--
-- Side - effect:
--  - the returned table is a *copy* (therefore you can brake the original)
--  - the return table does not have '_parent_':.. entry contrary to the original table,
--    that may contain one.
function smfs.listdir( ptr )
   ptr = (type(ptr) == 'table' and ptr) or {}
   local t = {}
   for k,v in pairs(ptr) do
      if k~= '_parent_' then
         t[k] = v
      end
   end
   return t
end

-- Input:
--  - a player p
--  - a itable describing the path
--
-- Returns:
--  - the directory corresponding to path (a table).
--  - or nil is invalid shipmem
--
-- Side - effect:
--  - Any non-existing dir on the way is created.
--
function smfs.checkdir( p, path )
   local ptr = p and p:shipMemory()

   if ptr ~= nil then
      for i,k in ipairs(path) do
         local prv = k
         ptr[prv] = ptr[prv] or {}
         ptr = ptr[prv]
      end
   end
   return ptr
end

local function smfs_read( p, path )
   local ptr = p and p:shipMemory() or {}
   for _i,k in ipairs(path) do
      ptr = (ptr or {})[k]
   end
   return ptr
end

-- Input:
--  - a player p
--  - a itable describing the path
--
-- Returns:
--  - a table of the dir contents if it exists.
--  - or nil if not
--
function smfs.readdir( p, path )
   local res = smfs_read( p, path)
   if res == nil then
      return nil
   else
      return smfs.listdir( res )
   end
end

-- Input:
--  - a player p
--  - a itable describing the path to the file
--      ( a file is just a k,v pair in some dir *where v not a table* )
--      ( k is the file name, v is the file content )
function smfs.readfile( p, path )
   local res = smfs_read( p, path)
   if type(res) == 'table' then
      return nil  -- this is not a file
   else
      return res
   end
end

-- Input:
--  - a player p
--  - a itable describing the path to the file
--  - a function mapping crt file content to new file content
-- Returns:
--  - a boolean indicating whether something changed
--  - or nil if invalid shipmem or empty path (therefore no filename)
-- Side - effect
--  - Any missing dir is created on the way.
--  - The file is created if it does not exist.
function smfs.updatefile( p, path, f )
   local tmp = {}
   local fil = nil

   for i,v in ipairs(path) do
      if i < #path then
         tmp[i] = v
      else
         fil = v
      end
   end
   if fil == nil then
      return nil
   end

   local ptr = smfs.checkdir( p, tmp )

   if ptr == nil then
      return nil
   else
      local bef = ptr[fil]
      local value = f(bef)
      ptr[fil] = value
      return (bef ~= value)
   end
end

-- Input:
--  - a player p
--  - a itable describing the path to the file
--  - a file content (a non-table value)
-- Returns:
--  - a boolean indicating whether something changed
--  - or nil if invalid shipmem or empty path (therefore no filename)
-- Side - effect
--  - Any missing dir is created on the way.
--  - The file is created if it does not exist.
function smfs.writefile( p, path, value )
   local function f(_in)
      return value
   end
   return smfs.updatefile( p, path, f )
end

--[[

## 2. smfs Console Interface ( Example usage )

> fs = require("shipmemfs")
  Inited shipmemfs with pilot "aze"

> fs.ls()
  _ec/
  _ec_total/
  _ec_needs_refresh : false

> fs.find()
  _ec/2/engine_limit : 630
  _ec/2/accel : 203
  _ec/2/turn : 131
  _ec/2/part : 100
  _ec/2/speed : 222
  _ec/3/engine_limit : 475
  _ec/3/halted : true
  _ec/3/turn : 108
  _ec/3/accel : 109
  _ec/3/speed : 201
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

> fs.cd(fs.path._ec[3])
 /_ec/3/

> fs.cd(fs.root._ec[2])
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

   smfs.root = smfs_pilot:shipMemory()
   smfs.path = smfs.root

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
            ptr = smfs.root
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
         if v and (v['_parent_'] or v == smfs.root) then
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
      local res = cd_silent( v )
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

   local function markall ( )
      if type(smfs.path) == 'table' then
         for k,v in pairs(smfs.path) do
            if type(v) == 'table' and k ~= "_parent_" and cd_silent(k) then
               markall()
               cd_silent("_parent_")
            end
         end
      end
   end
   markall()

   function smfs.find( pref )
      local count = 0
      if pref == nil then
         pref = "  "
      end
      if type(smfs.path) == 'table' then
         for k,v in pairs(smfs.path) do
            if k ~= "_parent_" then
               count = count + 1
               if type(v) == 'table' then
                  local pref2 = pref .. tostring(k) .. "/"
                  if cd_silent(k) then
                     local res = smfs.find(pref2)
                     if res == 0 then
                        print(pref2)
                     end
                     cd_silent("_parent_")
                  end
               else
                  print(pref .. tostring(k) .. " : " .. tostring(v))
               end
            end
         end
      end
      return count
   end

end

if _G['inspect'] ~= nil then
   smfs.init(player.pilot():target() or player.pilot())
end

return smfs

