sprites = {}
objects = {}

dofile "class.lua"

-- Base sprite class.
Sprite = class(nil)

dofile "sprite.lua"
dofile "effect.lua"

-- Entity types.
Enemy = class(Sprite)
Bullet = class(Sprite)
Player = class(Sprite)

-- Collision flags.
type = {}
type.player = bit.lshift(1, 0) 
type.playerbullet = bit.lshift(1, 1)
type.enemy = bit.lshift(1, 2)
type.enemybullet = bit.lshift(1, 3)
type.wall = bit.lshift(1, 4)

