
--[[
# Tiny File System

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
function tfs.checkdir( ptr, path )
   if ptr ~= nil then
      for i, k in ipairs(path) do
         ptr[k] = ptr[k] or {}
         ptr = ptr[k]
      end
   end
   return ptr
end

local function tfs_read( ptr, path )
   for _i, k in ipairs(path or {}) do
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
function tfs.readdir( ptr, path )
   local res = tfs_read(ptr, path)
   if res == nil then
      return nil
   else
      ptr = (type(res) == 'table' and res) or {}
      local t = {}
      for k, v in pairs(ptr) do
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
--      ( a file is just a k, v pair in some dir *where v not a table* )
--      ( k is the file name, v is the file content )
function tfs.readfile( ptr, path )
   local res = tfs_read(ptr, path)
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
-- Side - effects:
--  - Any missing dir is created on the way.
--  - The file is created if it does not exist.
function tfs.updatefile( ptr, path, f )
   local tmp = {}
   local fil = nil

   for i, v in ipairs(path) do
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
-- Side - effects:
--  - Any missing dir is created on the way.
--  - The file is created if it does not exist.
function tfs.writefile( ptr, path, value )
   return tfs.updatefile(ptr, path, function ( _in )
         return value
      end)
end

function tfs.append( ptr, path, value )
   local res = tfs.checkdir(ptr, path)
   if res then
      res[(#res) + 1] = value
   end
end

--[[
## 2. tfs Console Interface ( Example usage )

> fs = require 'tfs'
 mounted table:_0x4194ca30 at /shipmem
 mounted table:_0x417ccda0 at /mengine
 /pilot_My_Dogma/

> fs.ls()
  [0] ../
  [1] shipmem/
  [2] mengine/

> fs.cd(2)
 /pilot_My_Dogma/mengine/

> fs.ls()
  [0] ../
      needs_refresh : false
  [1] engines/
  [2] total/
      _dev_ : table:_0x417ccda0

> fs.cd('engines')
 /pilot_My_Dogma/mengine/engines/

> fs.fd()
  1/engine_limit : 2700
  ...
  2/speed : 73

> fs.cd(player.pilot():shipMemory())
 /pilot_My_Dogma/shipmem/

> fs.cd('/')
 /

> fs.cd(player.pilot():outfits())
 Not a valid dir.

> fs.mnt(player.pilot():outfits(), "outfits")
 mounted table:_0x40abbb00 at /outfits

> fs.cd('outfits')
 /outfits/

> fs.ls()
  [0] ../
      1 : Tricon Typhoon Engine
      2 : Tricon Typhoon Engine
      3 : S&K War Plating
  ...
--]]

function tfs.init()
   tfs.root = {}
   tfs.path = tfs.root
   tfs.shorthand = {}

   local function _cd( v, create )
      local ptr = tfs.path or (create and {}) or nil

      for _i, k in ipairs(v) do
         if k == '/' then
            ptr = tfs.root
         else
            local prv = ptr
            ptr = ptr[k] or (create and {})
            if ptr and k ~= "_parent_" and ptr['_parent_'] == nil then
               ptr["_parent_"] = prv
            end
         end
      end
      return ptr
   end

   local function _cd_silent( v )
      local res
      if type(v) == 'table' then
         res = (v and (v['_parent_'] or (v == tfs.root))) and v
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
         res = _cd(l) --and _cd(l, true)
      end
      if res then
         tfs.path = res
      end
      return res
   end

   local function _markall( )
      for k, v in pairs(tfs.path) do
         if type(v) == 'table' and v['_parent_'] == nil and _cd_silent(k) then
            _markall()
            _cd_silent("_parent_")
         end
      end
   end

   local function _pwd( t )
      local p = t._parent_ or {}
      for v, k in pairs(p) do
         if k == t then
            return _pwd(p) .. '/' .. tostring(v)
         end
      end
      return ''
   end

   function tfs.pwd( )
      print(" " .. _pwd(tfs.path) .. '/')
   end

   function tfs.mnt( dat, name )
      if type(dat) == 'table' then
         if dat._parent_ ~= nil then
            print(" " .. tostring(dat) .. ' already mounted at "' .. _pwd(dat) .. '"')
         else
            name = name or (#(tfs.path) + 1)
            tfs.path[name] = dat
            _cd_silent(name)
            _markall()
            dat['_dev_'] = string.gsub(tostring(dat), ": ", "@")
            _cd_silent('_parent_')
            print(' mounted ' .. dat['_dev_'] .. ' at /' .. name )
            return name
         end
      end
   end

   function tfs.automount_pil( p )
      local pil = p or player.pilot()

      if type(pil) == type(player.pilot()) then
         local name = 'pilot_' .. string.gsub(pil:name(), " ", "_")
         print(" automount pilot: " .. name)
         tfs.path[name] = {}
         _cd_silent(name)
         tfs.mnt(pil:shipMemory(), 'shipmem')
         if pil:outfitHasSlot('engines_secondary') then
            tfs.mnt(pil:outfitMessageSlot('engines', 'wtf?'), 'mengine')
         end
         _cd_silent('_parent_')
         return name
      end
   end

   function tfs.cd( v )
      v = tfs.shorthand[v] or v
      local res = _cd_silent(v)
      if res then
         tfs.pwd()
         tfs.shorthand = {}
      else
         print(' Not a valid dir.')
      end
      return res
   end

   local function tostr( k )
      if type(k) == 'function' then
         return string.gsub(tostring(k), "function: ", "function@")
      else
         return tostring(k)
      end
   end

   function tfs.ls( )
      if tfs.path._parent_ then
         tfs.shorthand[0] = tfs.path._parent_
         print("  [0] ../")
      end
      for k, v in ipairs(tfs.path) do
         if type(v) == 'table' then
            print("  [.] " .. tostring(k) .. "/")
         else
            print("      " .. tostring(k) .. " : " .. tostr(v))
         end
      end
      local count = #(tfs.path)
      for k, v in pairs(tfs.path) do
         if not (type(k) == type(1) and k>=1 and k<= #(tfs.path)) and k ~= "_parent_" then
            if type(v) == 'table' then
               count = count + 1
               tfs.shorthand[count] = k
               print("  [" .. tostring(count) .. "] " .. tostring(k) .. "/")
            else
               print("      " .. tostring(k) ..  " : " .. tostr(v))
            end
         end
      end
   end

   function tfs.fd( pref )
      pref = pref or "  "
      local count = 0

      for k, v in pairs(tfs.path) do
         if k ~= "_parent_" and k~= "_dev_" then
            if type(v) == 'table' then
               local pref2 = pref .. (v['_dev_'] == nil and "" or "*") .. tostring(k) .. "/"
               if _cd_silent(k) then
                  if tfs.fd(pref2) == 0 then
                     print(pref2)
                  end
                  _cd_silent("_parent_")
               end
            else
               print(pref .. tostring(k) .. " : " .. tostr(v))
            end
            count = count + 1
         end
      end
      return count
   end
end

if _G['inspect'] ~= nil then
   tfs.init()
   local tgt=player.pilot():target()
   if tgt then
      tfs.automount_pil(tgt)
   end
   tfs.cd(tfs.automount_pil())
end

return tfs
