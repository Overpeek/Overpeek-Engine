# overpeek-engine
Engine project

### Cloning
```
git clone --recursive https://gitlab.com/Overpeek/overpeek-engine.git
```

### CMake
```
mkdir build
cd build
cmake -DBUILD_TESTS=TRUE ..
```

#### Depends:
- Freetype
- GLFW3
- OpenGL
- OpenAL

### Features
- Window creation (glfw) 
- Audio (OpenAL) 
- Rendering (OpenGL)
    - Text rendering (Freetype) (basic) 
    - GUI (basic)
- Networking (basic)
- General utility tools (that I find useful) 


### TODO
- [ ] Vulkan
- [ ] Texture packer - rework textures and renderer
- [ ] Freetype -> stb_truetype
- [ ] Optimized text rendering
- [ ] ImGUI 
- [ ] Easy to use mode
