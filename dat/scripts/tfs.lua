
--[[
Tiny File System

This module contains:
 1. tfs API
 2. tfs Console Interface
--]]


local tfs = {}

--[[
## 1. tfs API

The following functions can't alter the fs (read only and don't return fs mutables):
 - function tfs.readdir( ptr, path )
 - function tfs.readfile( ppr, path )

The following function may create dirs and will change the file value if any:
function tfs.writefile( p, path, value )

The following function may create dirs and will return the dir, allowing to modify it:
function tfs.checkdir( p, path )

--]]

-- Input:
--  - a root
--  - a itable describing the path
--
-- Returns:
--  - the directory corresponding to path (a table).
--  - or nil is invalid shipmem
--
-- Side - effect:
--  - Any non-existing dir on the way is created.
--
function tfs.checkdir( ptr, path )
   if ptr ~= nil then
      for i,k in ipairs(path) do
         ptr[k] = ptr[k] or {}
         ptr = ptr[k]
      end
   end
   return ptr
end

local function tfs_read( ptr, path )
   for _i,k in ipairs(path or {}) do
      ptr = (ptr or {})[k]
   end
   return ptr
end

-- Input:
--  - a pointer ptr
--  - a itable describing the path
--
-- Returns:
--  - a table of the dir contents if it exists.
--  - or nil if not
--
function tfs.readdir( ptr, path )
   local res = tfs_read(ptr, path)
   if res == nil then
      return nil
   else
      ptr = (type(res) == 'table' and res) or {}
      local t = {}
      for k,v in pairs(ptr) do
         if k~= '_parent_' then
            t[k] = v
         end
      end
      return t
   end
end

-- Input:
--  - a ptr
--  - a itable describing the path to the file
--      ( a file is just a k,v pair in some dir *where v not a table* )
--      ( k is the file name, v is the file content )
function tfs.readfile( ptr, path )
   local res = tfs_read( ptr, path)
   if type(res) == 'table' then
      return nil  -- this is not a file
   else
      return res
   end
end

-- Input:
--  - a ptr
--  - a itable describing the path to the file
--  - a function mapping crt file content to new file content
-- Returns:
--  - a boolean indicating whether something changed
--  - or nil if invalid shipmem or empty path (therefore no filename)
-- Side - effect
--  - Any missing dir is created on the way.
--  - The file is created if it does not exist.
function tfs.updatefile( ptr, path, f )
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

   ptr = tfs.checkdir(ptr, tmp)

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
--  - a ptr
--  - a itable describing the path to the file
--  - a file content (a non-table value)
-- Returns:
--  - a boolean indicating whether something changed
--  - or nil if invalid shipmem or empty path (therefore no filename)
-- Side - effect
--  - Any missing dir is created on the way.
--  - The file is created if it does not exist.
function tfs.writefile( ptr, path, value )
   return tfs.updatefile(ptr, path, function ( _in )
         return value
      end)
end

--[[

## 2. tfs Console Interface ( Example usage )

> fs = require("tfs")
  Inited tfs with pilot "aze"'s main engine mem.

> fs.ls()
  _ec/
  _ec_total/
  _ec_needs_refresh : false

> fs.fd()
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

function tfs.init(pil)
   if pil == nil then
      pil = player.pilot():target() or player.pilot()
   end

   if type(pil) == type(player.pilot()) then
      if pil:outfitHasSlot('engines_secondary') then
         print('  Inited tfs with pilot "' .. pil:name() .. '"'.."'s main engine mem.")
         tfs.root = pil:outfitMessageSlot('engines','wtf?')
      else
         print('  Did not Init tfs with pilot main engine mem (don\'t have secondary)')
      end
   else
      print('  Inited tfs with custom ptr "' .. tostring(pil) .. '"')
      tfs.root = pil
   end

   tfs.path = tfs.root

   local function _pwd( t )
      local p = t._parent_
      if p == nil then
         return '@' .. tostring(tfs.root) .. ':/'
      else
         for v,k in pairs(p) do
            if k == t then
               return _pwd(p) .. tostring(v) .. '/'
            end
         end
         return '???/'
      end
   end

   function tfs.pwd( )
      print(" " .. _pwd(tfs.path))
   end

   local function _cd( v, create )
      local ptr = tfs.path
      if create then
         ptr = ptr or {}
      end

      for _i,k in ipairs(v) do
         if k == '/' then
            ptr = tfs.root
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

   local function cd_silent( v, force )
      if type(v) == 'table' then
         if v then
            if (v['_parent_'] or v == tfs.root) then
               tfs.path = v
               return true
            elseif force then
               tfs.root = v
               tfs.path = v
               return true
            end
         end
         print(' Not a valid dir.')
         return false
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

         local res = _cd(l)
         if res then
            tfs.path = _cd( l, true)
            return true
         else
            print(' Cannot enter "' .. strl(l) .. '"')
            return false
         end
      end
   end

   function tfs.cd( v, force )
      local res = cd_silent(v, force)
      if res then
         tfs.pwd()
      end
      return res
   end
   function tfs.ls( )
      if type(tfs.path) == 'table' then
         for k,v in pairs(tfs.path) do
            if type(v) == 'table' then
               print("  " .. tostring(k) .. "/")
            else
               print("  " ..tostring(k) .. " : " .. tostring(v))
            end
         end
      end
   end

   local function markall ( )
      if type(tfs.path) == 'table' then
         for k,v in pairs(tfs.path) do
            if type(v) == 'table' and k ~= "_parent_" and cd_silent(k) then
               markall()
               cd_silent("_parent_")
            end
         end
      end
   end
   markall()

   function tfs.fd( pref )
      local count = 0
      if pref == nil then
         pref = "  "
      end
      if type(tfs.path) == 'table' then
         for k,v in pairs(tfs.path) do
            if k ~= "_parent_" then
               count = count + 1
               if type(v) == 'table' then
                  local pref2 = pref .. tostring(k) .. "/"
                  if cd_silent(k) then
                     local res = tfs.fd(pref2)
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
   tfs.init()
end

return tfs
