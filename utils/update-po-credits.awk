# Loop over lines, and when we get plain text (not a "[...]" line),
/^[^[[]/ {
   # Escape any quotation marks...
   gsub(/"/, "\\\"");
   # And print a msgid/msgstr pair with the filename and line number.
   print "#: " FILENAME ":" NR "\nmsgid \"" $0 "\"\nmsgstr \"\"\n"
}
