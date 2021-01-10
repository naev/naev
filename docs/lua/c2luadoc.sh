#!/bin/sh

# Convert Doxygen comments to Luadoc comments
# CAUTION: We need to support BSD/macOS sed in addition to GNU sed.
#          If you aren't suffering, you've probably introduced a bug. :P
sed -n                                                                         \
 -e '1i\
-- This file was generated automatically from C sources to feed LDoc.'         \
`# Convert Doxygen /** to Luadoc ---`                                          \
 -e 's|^ */\*\* *$|---|p'                                                      \
`# Convert special tags to Lua expressions.`                                   \
`# Lines after @luafunc & @luamod will be ignored by Luadoc`                   \
`# Doxygen comments that do not contain any of these tags have no impact on`   \
`# the Luadoc output.`                                                         \
`# Rename some tags:`                                                          \
 -e 's|^ *\* *@luafunc|-- @function|p'                                         \
 -e 's|^ *\* *@brief|--|p'                                                     \
 -e 's|^ *\* *@luasee|-- @see|p'                                               \
 -e 's|^ *\* *@luaparam|-- @param|p'                                           \
 -e 's|^ *\* *@luatparam|-- @tparam|p'                                         \
 -e 's|^ *\* *@luareturn|-- @return|p' `#we accept both @luareturn & @return`  \
 -e 's|^ *\* *@luatreturn|-- @treturn|p'                                       \
 -e 's|^ *\* *@luamod|-- @module|p'                                            \
 -e 's|^ *\* *@luatype|-- @type|p'                                             \
 -e 's|^ *\* *@luaset|-- @set|p'                                               \
`# Keep tags Luadoc understands:`                                              \
`#   s|^ *\* *@param|-- @param|p # use luaparam; param is for C arguments`     \
`#   s|^ *\* *@see|-- @see|p # use luasee`                                     \
`#   s|^ *\* *@return|-- @return|p # use luareturn`                            \
 -e 's|^ *\* *@usage|-- @usage|p'                                              \
 -e 's|^ *\* *@description|-- @description|p'                                  \
 -e 's|^ *\* *@name|-- @name|p'                                                \
 -e 's|^ *\* *@class|-- @class|p'                                              \
 -e 's|^ *\* *@field|-- @field|p'                                              \
 -e 's|^ *\* *@release|-- @release|p'                                          \
`# HACK: ldoc supports annotations: fixme, todo, warning.`                     \
`# They eat your message. Let's do our own thing here.`                        \
 -e 's|^ *\* *@fixme|-- <em>FIXME</em>|p'                                      \
 -e 's|^ *\* *@todo|-- <em>TODO</em>|p'                                        \
 -e 's|^ *\* *@TODO|-- <em>TODO</em>|p'                                        \
 -e 's|^ *\* *@warning|-- <em>Warning</em>|p'                                  \
 -e 's|^ *\* *@note|-- <em>Note</em>|p'                                        \
`# Custom tags:`                                                               \
 -e 's|^ *\* *@code|-- <pre>|p'                                                \
 -e 's|^ *\* *@endcode|-- </pre>|p'                                            \
`# Remove other tags:`                                                         \
 -e '\|^ *\* *@.*|d'                                                           \
`# Insert newline between comments, replace */ with \n:`                       \
 -e '\|^ *\*/|c\
'                                                                              \
`# Keep other comments, replace * with --`                                     \
 -e 's|^ *\*|--|p'                                                             \
`# Keep blank lines`                                                           \
 -e 's|^\s*$||p'                                                               \
`# Delete everything else, just in case:`                                      \
 -e 'd'                                                                        \
   $1 | awk '
# Skips all comment blocks without an @ tag
   BEGIN {
      RS="\n\n"
   }
   /@/ {
      print $0, "\n"
   }' > $2
