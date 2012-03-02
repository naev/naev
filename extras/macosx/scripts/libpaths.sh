binarypath="${TARGET_BUILD_DIR}/naev.app/Contents/MacOS/naev"
install_name_tool -change /Library/Frameworks/FreeType.framework/Versions/2.3/FreeType @executable_path/../Frameworks/FreeType.framework/Versions/2.3/FreeType $binarypath
install_name_tool -change /Library/Frameworks/Vorbis.framework/Versions/A/Vorbis @executable_path/../Frameworks/Vorbis.framework/Versions/A/Vorbis $binarypath
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL $binarypath
install_name_tool -change /Library/Frameworks/SDL_image.framework/Versions/A/SDL_image @executable_path/../Frameworks/SDL_image.framework/Versions/A/SDL_image $binarypath
install_name_tool -change /Library/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer @executable_path/../Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer $binarypath
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "${TARGET_BUILD_DIR}/naev.app/Contents/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer"
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "${TARGET_BUILD_DIR}/naev.app/Contents/Frameworks/SDL_image.framework/Versions/A/SDL_image"
install_name_tool -change /Library/Frameworks/FreeType.framework/Versions/2.3/FreeType @executable_path/../Frameworks/FreeType.framework/Versions/2.3/FreeType $binarypath
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL $binarypath
install_name_tool -change /Library/Frameworks/SDL_image.framework/Versions/A/SDL_image @executable_path/../Frameworks/SDL_image.framework/Versions/A/SDL_image $binarypath
install_name_tool -change /Library/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer @executable_path/../Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer $binarypath
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "${TARGET_BUILD_DIR}/naev.app/Contents/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer"
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL "${TARGET_BUILD_DIR}/naev.app/Contents/Frameworks/SDL_image.framework/Versions/A/SDL_image"

#otool -L 
