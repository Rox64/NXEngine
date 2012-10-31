This is iOS port of the complete open-source clone/rewrite of Doukutsu Monogatari (also known as Cave Story) originally created by rogueeve (kty@lavabit.com).

Demonstration: http://youtu.be/jH2xsjXx28U

Game has been tested on jailbroken iPhone 4S (iOS 5.1.1) and non-jailbroken iPad 2 (iOS 5.1.1).

# Biggest changes:
- Original engine has been transfered to SDL2.
- Render system has been changed to use hardware accelerated SLD2 API.
- Graphics resolution will be automatically selected in runtime. In theory, it must correctly run on all iPad's and iPhones version >= 4.
- Original engine has been changed to support separate file locations for game resources (read-only), save files (read-write, persistent) and cache files (read-write, temporary).
- Primitive virtual joypad has been implemented.

# Requiremetns:
1. SDL2
2. SDL_ttf 2
3. Freetype 2

# How to build:

1. Check out game source:
```
git clone git://github.com/PIlin/NXEngine-iOS.git
```
2. Downaload dependencies into iOS/deps folder. Use script:
```
cd NXEngin-iOS/iOS/deps
./deploy.sh
```

3. Xcode porject is located in NXEngine-iOS/iOS/CaveStory.xcodeproj

# How to run on jailbroken device:

1. Configure your environment: http://iphonedevwiki.net/index.php/Xcode#Developing_without_Provisioning_Profile
2. Don't forget to set -gta flag in project properties.

# Links:

Original NXEngine project: http://nxengine.sourceforge.net/

Liberation Mono font: https://fedorahosted.org/liberation-fonts/

