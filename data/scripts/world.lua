
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
   type.enemy = bit.lshift(1, 2)
   type.enemybullet = bit.lshift(1, 3)
   type.wall = bit.lshift(1, 4)
   type.ray = bit.lshift(1, 5)
  
   -- Preload the texture to be used
   wallTexture = texture.new("pixel.dds")
   boulderTexture = texture.new("boulder.dds")
   wheelTexture = texture.new("wheel.dds")
   diamondTexture = texture.new("diamond.dds")
   
   World.enemies = {}
   World.size = vec2.new(3000, 3000)
   World.wall_width = 50
   World.camera = World.size / 2

   dofile "effects.lua"
   dofile "player.lua"
   dofile "worm.lua"
   dofile "boulder.lua"

end

function World.start()

   -- Simulate the starsystem for 2000 frames
   World.effect = particle.new(starsystem)
   World.effect:play(2000)

   -- Create player
   player = Player(vec2.new(World.size.x / 2, World.size.y / 2))

   -- Create wall entities
   World.lw = CollidableSprite(vec2.new(-World.wall_width, 0), type.wall, 0)
   World.lw:setOrigin(vec2.new(0, 0))
   World.lw:setTexture(wallTexture)
   World.lw:setSize(vec2.new(World.wall_width, World.size.y))
   World.lw:setColor(color.toRGBFromHSV(color.new(0,1,1,1)))

   World.bw = CollidableSprite(vec2.new(-World.wall_width, World.size.y), type.wall, 0)
   World.bw:setOrigin(vec2.new(0, 0))
   World.bw:setTexture(wallTexture)
   World.bw:setSize(vec2.new(World.size.x + 2 * World.wall_width, World.wall_width))

   World.rw = CollidableSprite(vec2.new(World.size.x, -World.wall_width), type.wall, 0)
   World.rw:setOrigin(vec2.new(0, 0))
   World.rw:setTexture(wallTexture)
   World.rw:setSize(vec2.new(World.wall_width, World.size.y + 2 * World.wall_width))

   World.tw = CollidableSprite(vec2.new(-World.wall_width, -World.wall_width), type.wall, 0)
   World.tw:setOrigin(vec2.new(0, 0))
   World.tw:setTexture(wallTexture)
   World.tw:setSize(vec2.new(World.size.x + 2 * World.wall_width, World.wall_width))
   
   es.setPolygonShape(World.lw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(-World.wall_width, 0),
		      vec2.new(-World.wall_width, World.size.y),
		      vec2.new(0, World.size.y)) 

   es.setPolygonShape(World.bw.entity,
		      4, 
		      vec2.new(0, World.size.y),
		      vec2.new(0, World.size.y + World.wall_width),
		      vec2.new(World.size.x, World.size.y + World.wall_width),
		      vec2.new(World.size.x, World.size.y))

   es.setPolygonShape(World.rw.entity,
		      4, 
		      vec2.new(World.size.x, 0),
		      vec2.new(World.size.x + World.wall_width, 0),
		      vec2.new(World.size.x + World.wall_width, World.size.y),
		      vec2.new(World.size.x, World.size.y)) 

   es.setPolygonShape(World.tw.entity,
		      4, 
		      vec2.new(World.size.x, -World.wall_width),
		      vec2.new(World.size.x, 0),
		      vec2.new(0, 0),
		      vec2.new(0, -World.wall_width))

   -- Spawn enemies
   math.randomseed(os.time())

   for i = 0, 2 do
      table.insert(World.enemies, Worm(vec2.new(math.random() * World.size.x,
						math.random() * World.size.y), 16))
   end

   for i = 0, 12 do
      table.insert(World.enemies, Boulder(vec2.new(math.random() * World.size.x,
						   math.random() * World.size.y)))
   end

   World.time = 0

   -- Ray entity
   Ray = {}
   Ray.entity = es.createEntity()
   Ray.dir = vec2.new(1,0)
   es.setPosition(Ray.entity, vec2.new(1000, 1000))
   es.addCollision(Ray.entity, type.ray, type.player) -- Example: Is the ray blocked by an enemy
   es.setRayShape(Ray.entity, Ray.dir)
   es.addResponder(Ray.entity)

end

function World.update()

   World.time = World.time + dt * 1.0 * math.pi

   -- Animate ray
   Ray.dir = vec2.new(math.cos(-World.time), math.sin(-World.time))
   es.setDirection(Ray.entity, Ray.dir)

   local collidedEntities = es.getCollidedEntity(Ray.entity)
   for i = #collidedEntities, 1, -1 do
      
      local entity = collidedEntities[i][1]
      local pos = collidedEntities[i][2]
      local sprite = sprites[entity]

      if sprite ~= nil then	 
	 print("Sound the alarm")
      end
   end

   -- Disco walls
   local hsv = color.toHSVFromRGB(World.lw:getColor()) + color.new(60 * dt, 0, 0, 0)
   if hsv.h > 360 then
      hsv.h = 0
   end 

   local rgb = color.toRGBFromHSV(hsv)

   World.lw:setColor(rgb)
   World.bw:setColor(rgb)
   World.rw:setColor(rgb)
   World.tw:setColor(rgb)

   -- Center camera on player
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


