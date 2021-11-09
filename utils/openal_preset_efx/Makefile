
reverb_preset.lua: main
	echo "local efx_preset = {}" > $@
	./main >> $@
	echo "return efx_preset" >> $@

main: main.c print.c
	$(CC) -lm `pkg-config openal --cflags` -W -Wall -o $@ $<

main.c: preset-list make_prog.py
	python3 make_prog.py

preset-list: /usr/include/AL/efx-presets.h
	grep EFX_REVERB_PRESET_ /usr/include/AL/efx-presets.h | sed "s/.*EFX_REVERB_PRESET_\(.*\) ./\1/" > $@

clean:
	$(RM) preset-list main.c main efx_preset.lua
