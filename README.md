```
   ____      _       ____     _      
U | __")uU  /"\  uU /"___|u  |"|     
 \|  _ \/ \/ _ \/ \| |  _ /U | | u   
  | |_) | / ___ \  | |_| |  \| |/__  
  |____/ /_/   \_\  \____|   |_____| 
 _|| \\_  \\    >>  _)(|_    //  \\  
(__) (__)(__)  (__)(__)__)  (_")("_) 

```
# Overview

Bagl is a small 3D graphics libary written in C. As an abstraction of OpenGL, it is designed to be as user-friendly as possible, while still offering fine-grained control where it matters!

# Features

- Easy to use API
- Minimal dependencies
- Render scenes with built-in pipelines, or design your own!
- First-class support for post-processing and multi-pass rendering

# Building from Source

### Requirements
- CMake v3.50
- Your favorite C compiler

From the project's root directory, run the following commands:
```
mkdir build
cmake .
cmake --build build --config Release
```

# Dependencies

- [glfw](https://www.glfw.org/) - Window/input management
- [glad](https://glad.dav1d.de/) - OpenGL function loader
- [stb_image](https://github.com/nothings/stb/) - Image loader
- [assimp](https://github.com/assimp/assimp) - Model loader

---

```
    ( ( (        |                           _     _    
  '. ___ .'      |.===.        __MMM__     o' \,=./ `o  
 '  (> <) '      {}o o{}        (o o)         (o o)     
ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-
```