#pragma once

// TODO (daniel): Put this into the resource manager
static dcfx::ShaderHandle
loadShader(dcfx::Context *renderCtx,
	   const char *filename,
	   dcfx::ShaderType type)
{
    tinystl::string assetsString(VOIDSTORM_SHADER_DIRECTORY);
    tinystl::string nameString(filename);
    assetsString.append(nameString.c_str(), nameString.c_str() + nameString.size());
    PRINT("%s\n", assetsString.c_str());

    File file = readEntireFile(assetsString.c_str());
    if (file.data == NULL)
    {
	PRINT("Failed to open file. %s\n", assetsString.c_str());
	return { dcfx::InvalidHandle };
    }

    dcfx::ShaderHandle shader = renderCtx->createShader(file.data, file.size, type);

    return shader;
}
