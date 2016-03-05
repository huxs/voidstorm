#pragma once

// TODO (daniel): Put this into the resource manager
static dcfx::ShaderHandle
loadShader(dcfx::Context* renderCtx,
	   const char* filepath,
	   dcfx::ShaderType type)
{
#ifndef ANDROID    
    tinystl::string assets(VOIDSTORM_SHADER_DIRECTORY);
    tinystl::string file(filepath);
    assets.append(file.c_str(), file.c_str() + file.size());
    PRINT("%s\n", assets.c_str());
#else
    tinystl::string assets(filepath);
#endif    
    
    SDL_RWops* handle = SDL_RWFromFile(assets.c_str(), "r");
    if (handle == nullptr)
    {
	PRINT("Failed to open file. %s\n", assets.c_str());
	return{ dcfx::InvalidHandle };
    }

    // Get the size of the file in bytes.
    SDL_RWseek(handle, 0, SEEK_END);
    size_t size = (unsigned int)SDL_RWtell(handle);
    SDL_RWseek(handle, 0, SEEK_SET);

    // Allocate space for storing the content of the file.
    char* mem = (char*)renderCtx->frameAlloc(sizeof(char) * size + 1);

    size_t bytes = SDL_RWread(handle, mem, size, 1);
    if (bytes != 1)
    {
	PRINT("Error reading from file %s.\n", filepath);
    }

    mem[size] = '\0';

    dcfx::ShaderHandle shader = renderCtx->createShader(mem, size, type);

    return shader;
}
