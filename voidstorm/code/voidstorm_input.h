#pragma once

struct ControllerButtonState
{
    bool32 isPressed;
};

struct ControllerState
{
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;

    union
    {
	struct
	{
	    ControllerButtonState A;
	    ControllerButtonState B;
	    ControllerButtonState X;
	    ControllerButtonState Y;
	};
	
	ControllerButtonState arr[4];
    };
};	

struct KeyboardState
{
    KeyboardState()
	{
	    memset(keys, 0, sizeof(uint8_t) * SDL_NUM_SCANCODES);
	}
    
    uint8_t keys[SDL_NUM_SCANCODES];
};

struct GameInput
{
    float dt;
    KeyboardState keyboards[2];
    KeyboardState* currentKeyboard;
    KeyboardState* previousKeyboard;
    ControllerState controllers[2];
    ControllerState* currentController;
    ControllerState* previousController;   
};

inline bool isKeyDown(SDL_Scancode key)
{
    if(SDL_GetKeyboardState(NULL)[key])
    {
	return true;
    }

    return false;
}

inline bool isKeyDownAndReleased(GameInput* gameInput, SDL_Scancode key)
{
    uint8_t isDown = gameInput->currentKeyboard->keys[key];
    
    if(isDown)
	if (!gameInput->previousKeyboard->keys[key])
	{
	    return true;
	}

    return false;
}
