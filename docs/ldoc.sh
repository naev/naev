# Need to be in the docs/ directory.
cd "$(dirname $0)"

# Create docs/lua/ directory
test -d lua || mkdir lua

# Convert Doxygen comments to Luadoc comments
for F in ../src/nlua_*.c ../src/ai.c; do
   ./c2luadoc.sh $F lua/"$(basename $F)".luadoc
done

# Run Luadoc, put HTML files into html/ dir
cd lua
ldoc -d ../html .
error=$?
rm *.luadoc
exit $error
