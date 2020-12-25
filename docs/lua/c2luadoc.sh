#!/bin/sh

# Convert Doxygen comments to Luadoc comments
sed -n '
   1 i -- This file was generated automatically from C sources to feed LDoc.
# Convert Doxygen /** to Luadoc ---
   s|^ */\*\* *$|---|p
# Convert special tags to Lua expressions.
# Lines after @luafunc & @luamod will be ignored by Luadoc
# Doxygen comments that do not contain any of these tags have no impact on
# the Luadoc output.
# Rename some tags:
   s|^ *\* *@luafunc|-- @function|p
   s|^ *\* *@brief|--|p
   s|^ *\* *@luasee|-- @see|p
   s|^ *\* *@luaparam|-- @param|p
   s|^ *\* *@luatparam|-- @tparam|p
   s|^ *\* *@luareturn|-- @return|p #we accept both @luareturn & @return
   s|^ *\* *@luatreturn|-- @treturn|p
   s|^ *\* *@luamod|-- @module|p
   s|^ *\* *@luatype|-- @type|p
   s|^ *\* *@luaset|-- @set|p
# Keep tags Luadoc understands:
#   s|^ *\* *@param|-- @param|p # use luaparam, param reserved for C arguments
#   s|^ *\* *@see|-- @see|p # use luasee
#   s|^ *\* *@return|-- @return|p # use luareturn
   s|^ *\* *@usage|-- @usage|p
   s|^ *\* *@description|-- @description|p
   s|^ *\* *@name|-- @name|p
   s|^ *\* *@class|-- @class|p
   s|^ *\* *@field|-- @field|p
   s|^ *\* *@release|-- @release|p
# Custom tags:
   s|^ *\* *@code|-- <pre>|p
   s|^ *\* *@endcode|-- </pre>|p
# Remove other tags:
   \|^ *\* *@.*|d
# Insert newline between comments, replace */ with \n:
   \|^ *\*/|c
# Keep other comments, replace * with --
   s|^ *\*|--|p
# Keep blank lines
   s|^\s*$||p
# Delete everything else, just in case:
   d
   ' $1 | awk '
# Skips all comment blocks without an @ tag
   BEGIN {
      RS="\n\n"
   }
   /@/ {
      print $0, "\n"
   }' > $2