# Facade Decompilation
 A work in progress decompilation of the front-end Windows release of the 2005 interactive story game, Facade.

# Notice
This decompilation uses symbols found in the MAC OS X release of the game to fill in the gaps of prior assumptions, but everything else that isn't known, will be guessed in a logical sense. <br>
You will also need a copy of the original game's assets in order for this to work, an installer can be found on the Internet Archive [here](https://archive.org/details/facade-installer-1.1b).

# Building
Since this project is huge I don't expect you to have much luck building animEgineDLL with Visual Studio, so please use cmake <br.

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

You can then find the compiled animEngineDLL.dll in ./build/Release
