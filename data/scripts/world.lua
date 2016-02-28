
function World.initialize()

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
   
   World.enemies = {}
   World.size = vec2.new(3000, 3000)
   World.wall_width = 50
   World.camera = World.size / 2
   
   dofile "ray.lua"
   dofile "effects.lua"
   dofile "player.lua"
   dofile "worm.lua"
   dofile "boulder.lua"
   
   dofile "test.lua"

end

function World.start()

   -- Simulate the starsystem for 2000 frames
   World.effect = particle.new(starsystem)
   World.effect:play(2000)

   player = Player(vec2.new(World.size.x / 2, World.size.y / 2))
   --player = Player(vec2.new(550, 350))

   math.randomseed(os.time())

   World.createWalls()
   World.spawnEnemies()

   Test.initialize()
   
end

function World.createWalls()

   World.lw = MovableCollidableSprite(vec2.new(-World.wall_width, 0), type.wall, 0)
  
   World.lw:setOrigin(vec2.new(0, 0))
   World.lw:setTexture(wallTexture)
   World.lw:setSize(vec2.new(World.wall_width, World.size.y))
   World.lw:setColor(color.toRGBFromHSV(color.new(0,1,1,1)))
   World.lw:setDisabled(true)
   World.lw:setRestitution(2.0)

   World.bw = MovableCollidableSprite(vec2.new(0, World.size.y), type.wall, 0)
   World.bw:setOrigin(vec2.new(0, 0))
   World.bw:setTexture(wallTexture)
   World.bw:setSize(vec2.new(World.size.x, World.wall_width))
   World.bw:setDisabled(true)
   World.bw:setRestitution(2.0)

   World.rw = MovableCollidableSprite(vec2.new(World.size.x, 0), type.wall, 0)
   World.rw:setOrigin(vec2.new(0, 0))
   World.rw:setTexture(wallTexture)
   World.rw:setSize(vec2.new(World.wall_width, World.size.y))
   World.rw:setDisabled(true)
   World.rw:setRestitution(2.0)

   World.tw = MovableCollidableSprite(vec2.new(0, -World.wall_width), type.wall, 0)
   World.tw:setOrigin(vec2.new(0, 0))
   World.tw:setTexture(wallTexture)
   World.tw:setSize(vec2.new(World.size.x, World.wall_width))
   World.tw:setDisabled(true)
   World.tw:setRestitution(2.0)
   
   es.setPolygonShape(World.lw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(World.wall_width, 0),
		      vec2.new(World.wall_width, World.size.y),
		      vec2.new(0, World.size.y)) 

   es.setPolygonShape(World.bw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(World.size.x, 0),
		      vec2.new(World.size.x, World.wall_width),
		      vec2.new(0, World.wall_width))

   es.setPolygonShape(World.rw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(World.wall_width, 0),
		      vec2.new(World.wall_width, World.size.y),
		      vec2.new(0, World.size.y)) 

   es.setPolygonShape(World.tw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(World.size.x, 0),
		      vec2.new(World.size.x, World.wall_width),
		      vec2.new(0, World.wall_width))
end

function World.spawnEnemies()

   for i = 0, 16 do
      table.insert(World.enemies, Worm(vec2.new(math.random() * World.size.x,
						math.random() * World.size.y), 16))
   end

   for i = 0, 32 do
      table.insert(World.enemies, Boulder(vec2.new(math.random() * World.size.x,
						   math.random() * World.size.y)))
   end

end

function World.pulsateWalls()

   local hsv = color.toHSVFromRGB(World.lw:getColor()) + color.new(60 * dt, 0, 0, 0)
   if hsv.h > 360 then
      hsv.h = 0
   end 

   local rgb = color.toRGBFromHSV(hsv)

   World.lw:setColor(rgb)
   World.bw:setColor(rgb)
   World.rw:setColor(rgb)
   World.tw:setColor(rgb)

end

function World.update()

   World.pulsateWalls()

   voidstorm.setCameraPosition(World.camera)
   
   -- Update player
   if player.isDead == 0 then

      World.camera = player:getPosition()

      player:update()
      
   end

   -- Update enemies in reverse in case of removal
   for i = #World.enemies, 1, -1 do
      
      local enemy = World.enemies[i]

      if enemy.isDead == 1 then
	 table.remove(World.enemies, i)
      else
	 enemy:update()
      end

   end

end


