#run this from docs directory
#create docs/lua directory
if [ ! -d lua ]; then mkdir lua; fi
#convert doxygen comments to luadoc comments
for F in ../src/nlua_*.c
do
sed -n '
	1 i -- This is file was generated automatically from C sources to feed luadoc
#converts doxygens /** to luadocs ---
	s|^ */\*\*|---|p 
#convert special tags to lua expressions.
#notice lines after @luafunc & @luamod will be ignored by luadoc
#also doxygen comments that do not contain a any of this tags have no
#influence on luadoc output
	s|^ *\* *@luafunc|function|p
	s|^ *\* *@luamod *\(.*\)|module "\1"|p
#rename some tags:
	s|^ *\* *@brief|-- @description|p
#keep tags luadoc understands:
	s|^ *\* *@param|-- @param|p
	s|^ *\* *@see|-- @see|p
	s|^ *\* *@return|-- @return|p
	s|^ *\* *@usage|-- @usage|p
	s|^ *\* *@description|-- @description|p
	s|^ *\* *@name|-- @name|p
	s|^ *\* *@class|-- @class|p
	s|^ *\* *@field|-- @field|p
	s|^ *\* *@release|-- @release|p
#remove other tags:
	\|^ *\* *@.*|d
#insert newline between comments, replace */ by \n
	\|^ *\*/|c
#keep other comments, replace * by --
	s|^ *\*|--|p
#delete everything else, just in case
	d
' $F > lua/`basename $F`.luadoc
done
#run luadoc, put html files into lua dir
(
	cd lua
	luadoc --nofiles *.luadoc
	rm *.luadoc
)
