
function GameInitialize()

   -- Maps Entities to Sprites.
   sprites = {}

   -- Load sprite definitions.
   dofile "sprite.lua"

   -- Load Concretes.
   Bullet = class(MovableCollidableSprite)
   Player = class(MovableCollidableSprite)
   Enemy = class(MovableCollidableSprite)
   Worm = class(MovableCollidableSprite)
   WormPart = class(CollidableSprite)

   Map = {}
   Map.enemies = {}

   -- Collision flags.
   type = {}
   type.player = bit.lshift(1, 0) 
   type.playerbullet = bit.lshift(1, 1)
   type.enemy = bit.lshift(1, 2)
   type.enemybullet = bit.lshift(1, 3)
   type.wall = bit.lshift(1, 4)

   dofile "effect.lua"
   dofile "debug.lua"
   dofile "map.lua"

   -- Preload the texture to be used.
   wallTexture = texture.new("pixel.dds")
   boulderTexture = texture.new("boulder.dds")
   wheelTexture = texture.new("wheel.dds")
   diamondTexture = texture.new("diamond.dds")

   -- Initialize all the entities in the map.
   Map.start();

   -- Non HDR values was 1.9, 1.0, 1.0
   voidstorm.setPostProcessParams(1.9, 1.0, 0.0)

end

function GameUpdateAndRender()

   debug_view()
   
   Map.update()

end
