# Need to be in the docs/ directory.
cd "$(dirname $0)"

# Create docs/lua/ directory
test -d lua || mkdir lua

# Convert Doxygen comments to Luadoc comments
for F in ../src/nlua_*.c; do
   sed -n '
         1 i -- This file was generated automatically from C sources to feed Luadoc.
# Convert Doxygen /** to Luadoc ---
         s|^ */\*\* *$|---|p
# Convert special tags to Lua expressions.
# Lines after @luafunc & @luamod will be ignored by Luadoc
# Doxygen comments that do not contain any of these tags have no impact on
# the Luadoc output.
         s|^ *\* *@luafunc|function|p
         s|^ *\* *@luamod *\(.*\)|module "\1"|p
# Rename some tags:
         s|^ *\* *@brief|-- @description|p
         s|^ *\* *@luasee|-- @see|p
         s|^ *\* *@luaparam|-- @param|p
         s|^ *\* *@luareturn|-- @return|p #we accept both @luareturn & @return
# Keep tags Luadoc understands:
#        s|^ *\* *@param|-- @param|p # use luaparam, param reserved for C arguments
#        s|^ *\* *@see|-- @see|p # use luasee
#        s|^ *\* *@return|-- @return|p # use luareturn
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
# Delete everything else, just in case:
         d
         ' $F > lua/"$(basename $F)".luadoc
done

# Run Luadoc, put HTML files into lua/ dir
(
   cd lua
   luadoc --nofiles --taglet "naev-taglet" -t templates/ *.luadoc
	rm *.luadoc
)
