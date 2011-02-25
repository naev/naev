install_name_tool -change /Library/Frameworks/FreeType.framework/Versions/2.3/FreeType @executable_path/../Frameworks/FreeType.framework/Versions/2.3/FreeType "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL_image.framework/Versions/A/SDL_image @executable_path/../Frameworks/SDL_image.framework/Versions/A/SDL_image "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer @executable_path/../Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "build/${CONFIGURATION}/naev.app/Contents/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer"

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "build/${CONFIGURATION}/naev.app/Contents/Frameworks/SDL_image.framework/Versions/A/SDL_image"

install_name_tool -change /Library/Frameworks/FreeType.framework/Versions/2.3/FreeType @executable_path/../Frameworks/FreeType.framework/Versions/2.3/FreeType "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL_image.framework/Versions/A/SDL_image @executable_path/../Frameworks/SDL_image.framework/Versions/A/SDL_image "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer @executable_path/../Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer "build/${CONFIGURATION}/naev.app/Contents/MacOS/naev"

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "build/${CONFIGURATION}/naev.app/Contents/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer"

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "build/${CONFIGURATION}/naev.app/Contents/Frameworks/SDL_image.framework/Versions/A/SDL_image"
