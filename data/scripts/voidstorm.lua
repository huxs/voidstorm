
function GameInitialize()  
    
   dofile "sprite.lua"
   dofile "particles.lua"
   dofile "debug.lua"

   math.randomseed(os.time())

   -- globals state
   sprites = {}

   -- Load Concretes
   Bullet = class(MovableCollidableSprite)
   Player = class(MovableCollidableSprite)
   Enemy = class(MovableCollidableSprite)
   Worm = class(MovableCollidableSprite)
   WormPart = class(CollidableSprite)
   Boulder = class(MovableCollidableSprite)
   TinyBoulder = class(MovableCollidableSprite)

   -- Collision flags
   type = {}
   type.player = bit.lshift(1, 0) 
   type.playerbullet = bit.lshift(1, 1)
   type.worm = bit.lshift(1, 2)
   type.boulder = bit.lshift(1, 3)
   type.enemybullet = bit.lshift(1, 4)
   type.wall = bit.lshift(1, 5)
   type.ray = bit.lshift(1, 6)

   type.test = bit.lshift(1, 7)

   -- Preload the texture to be used
   wallTexture = texture.new("pixel.dds")
   boulderTexture = texture.new("boulder.dds")
   wheelTexture = texture.new("wheel.dds")
   diamondTexture = texture.new("diamond.dds")

   -- global state
   enemies = {}

   size = vec2.new(3000, 3000)
   wall_width = 50
   camera = size / 2

   dofile "ray.lua"
   dofile "effects.lua"
   dofile "player.lua"
   dofile "worm.lua"
   dofile "boulder.lua"   
   dofile "test.lua"

   -- Simulate the starsystem for 2000 frames
   effect = particle.new(starsystem)
   effect:play(2000)

   createPlayer()
   createWalls()
   createEnemies()

   voidstorm.setPostProcessParams(3.0, 1.0, 0.0)

   collectgarbage()

end

function createPlayer()

   player = Player(vec2.new(size.x / 2, size.y / 2))

end

function createWalls()

   lw = MovableCollidableSprite(vec2.new(-wall_width, 0), type.wall, 0)  
   lw:setOrigin(vec2.new(0, 0))
   lw:setTexture(wallTexture)
   lw:setSize(vec2.new(wall_width, size.y))
   lw:setColor(color.toRGBFromHSV(color.new(0,1,1,1)))
   lw:setDisabled(true)
   lw:setRestitution(2.0)

   bw = MovableCollidableSprite(vec2.new(0, size.y), type.wall, 0)
   bw:setOrigin(vec2.new(0, 0))
   bw:setTexture(wallTexture)
   bw:setSize(vec2.new(size.x, wall_width))
   bw:setDisabled(true)
   bw:setRestitution(2.0)

   rw = MovableCollidableSprite(vec2.new(size.x, 0), type.wall, 0)
   rw:setOrigin(vec2.new(0, 0))
   rw:setTexture(wallTexture)
   rw:setSize(vec2.new(wall_width, size.y))
   rw:setDisabled(true)
   rw:setRestitution(2.0)

   tw = MovableCollidableSprite(vec2.new(0, -wall_width), type.wall, 0)
   tw:setOrigin(vec2.new(0, 0))
   tw:setTexture(wallTexture)
   tw:setSize(vec2.new(size.x, wall_width))
   tw:setDisabled(true)
   tw:setRestitution(2.0)
   
   es.setPolygonShape(lw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(wall_width, 0),
		      vec2.new(wall_width, size.y),
		      vec2.new(0, size.y)) 

   es.setPolygonShape(bw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(size.x, 0),
		      vec2.new(size.x, wall_width),
		      vec2.new(0, wall_width))

   es.setPolygonShape(rw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(wall_width, 0),
		      vec2.new(wall_width, size.y),
		      vec2.new(0, size.y)) 

   es.setPolygonShape(tw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(size.x, 0),
		      vec2.new(size.x, wall_width),
		      vec2.new(0, wall_width))

end

function createEnemies()

   for i = 0, 16 do
      table.insert(enemies, Worm(vec2.new(math.random() * size.x,
					  math.random() * size.y), 16))
   end

   for i = 0, 32 do
      table.insert(enemies, Boulder(vec2.new(math.random() * size.x,
					     math.random() * size.y)))
   end

end

function pulsateWalls()

   local hsv = color.toHSVFromRGB(lw:getColor()) + color.new(60 * dt, 0, 0, 0)
   if hsv.h > 360 then
      hsv.h = 0
   end 

   local rgb = color.toRGBFromHSV(hsv)

   lw:setColor(rgb)
   bw:setColor(rgb)
   rw:setColor(rgb)
   tw:setColor(rgb)

end

function GameUpdateAndRender()

   debug_view()

   voidstorm.setCameraPosition(camera)

   pulsateWalls()
   
   -- Update player
   if player.isDead == 0 then

      camera = player:getPosition()

      player:update()
      
   end

   -- Update enemies in reverse in case of removal
   for i = #enemies, 1, -1 do
      
      local enemy = enemies[i]

      if enemy.isDead == 1 then
	 table.remove(enemies, i)
      else
	 enemy:update()
      end

   end

end
