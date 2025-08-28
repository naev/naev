
with open("preset-list","r") as f:
   presetlist = f.read().strip()
   presetlist = presetlist.split("\n")

with open("main.c", "w") as f:
   f.write("""
#include "print.c"

int main (void)
{
""")

   for ps in presetlist:
      varname = ps.lower()
      f.write(f"const EFXEAXREVERBPROPERTIES {varname} = EFX_REVERB_PRESET_{ps};\n")
      f.write(f"print_params( \"{varname}\", &{varname} );\n")

   f.write("}")
