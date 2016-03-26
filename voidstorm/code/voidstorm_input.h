#pragma once

#define VOIDSTORM_NUM_KEYS 512

// TODO (daniel): Fill in this mapping
enum VoidstormKeys
{
    VOIDSTORM_KEY_F11 = 122,
};

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
	    memset(keys, 0, sizeof(bool32) * VOIDSTORM_NUM_KEYS);
	}
    
    bool32 keys[VOIDSTORM_NUM_KEYS];
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
    // TODO (daniel): Add Mouse State
};

inline bool
isKeyDown(GameInput* gameInput, uint32_t key)
{
    if(gameInput->currentKeyboard->keys[key])
    {
	return true;
    }

    return false;
}

inline bool
isKeyReleasedToDown(GameInput* gameInput, uint32_t key)
{
    bool32 isDown = gameInput->currentKeyboard->keys[key];

    if(isDown)
    {
	if (!gameInput->previousKeyboard->keys[key])
	{
	    return true;
	}
    }

    return false;
}

inline bool
isKeyDownToReleased(GameInput* gameInput, uint32_t key)
{
    bool32 isDown = gameInput->currentKeyboard->keys[key];

    if(!isDown)
    {
	if (gameInput->previousKeyboard->keys[key])
	{
	    return true;
	}
    }

    return false;
}
