
--[[
# Tiny File System API
--]]


local tfs = {}

--[[
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
   if type(res) ~= 'table' then
      return nil
   else
      local t = {}
      for k, v in pairs(res) do
         if k~= '_parent_' then
            t[k] = tfs.readdir(v) or v
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

return tfs
