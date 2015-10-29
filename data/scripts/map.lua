dofile "particles.lua"
dofile "player.lua"
dofile "enemy.lua"

Map.size = vec2.new(3000, 3000)
Map.wall_width = 50

starsystem = ParticleEffect("starsystem")

local starsystem_emitters_x = 20;
local starsystem_emitters_y = 20;

local starsystem_margin_x = (Map.size.x / starsystem_emitters_x) * 2.0
local starsystem_margin_y = (Map.size.y / starsystem_emitters_y) * 2.0

for y = 0, starsystem_emitters_y, 1 do
   for x = 0, starsystem_emitters_x, 1 do

      local emitter = starsystem:addEmitter()
      emitter.texture = texture.new("test.dds")
      emitter.colorMin = color.new(0,0,0,1)
      emitter.colorMax = color.new(1,1,1,1)
      emitter.particlesPerEmit = 5
      emitter.lifetime = 0 -- infinite
      emitter.spawntime = 0 -- only initial spawn
      emitter.lifetimeMin = 0 -- live forever
      emitter.lifetimeMax = 0
      emitter.sizeMin = 1
      emitter.sizeMax = 1
      emitter.depthMin = -2
      emitter.depthMax = 2
      emitter.force = vec2.new(0.01, 0.01)
      emitter.relative = false
      emitter.spread = 360
      emitter.rotationMin = 0
      emitter.rotationMax = 360
      emitter.position = vec2.new(starsystem_margin_x * x,
				  starsystem_margin_y * y)
   end
end

starsystem:store() 

Map.effect = particle.new(starsystem)

function Map.start()

   -- Simulate the starsystem for 2000 frames
   Map.effect:play(2000)

   -- Create player.
   player = Player(vec2.new(Map.size.x / 2, Map.size.y / 2))

   -- Create wall entities.
   Map.lw = CollidableSprite(vec2.new(-Map.wall_width, 0), type.wall, 0)
   Map.lw:setOrigin(vec2.new(0, 0))
   Map.lw:setTexture(wallTexture)
   Map.lw:setSize(vec2.new(Map.wall_width, Map.size.y))

   Map.bw = CollidableSprite(vec2.new(-Map.wall_width, Map.size.y), type.wall, 0)
   Map.bw:setOrigin(vec2.new(0, 0))
   Map.bw:setTexture(wallTexture)
   Map.bw:setSize(vec2.new(Map.size.x + 2 * Map.wall_width, Map.wall_width))

   Map.rw = CollidableSprite(vec2.new(Map.size.x, -Map.wall_width), type.wall, 0)
   Map.rw:setOrigin(vec2.new(0, 0))
   Map.rw:setTexture(wallTexture)
   Map.rw:setSize(vec2.new(Map.wall_width, Map.size.y + 2 * Map.wall_width))

   Map.tw = CollidableSprite(vec2.new(-Map.wall_width, -Map.wall_width), type.wall, 0)
   Map.tw:setOrigin(vec2.new(0, 0))
   Map.tw:setTexture(wallTexture)
   Map.tw:setSize(vec2.new(Map.size.x + 2 * Map.wall_width, Map.wall_width))
   
   es.setPolygonShape(Map.lw.entity,
		      4, 
		      vec2.new(0, 0),
		      vec2.new(-Map.wall_width, 0),
		      vec2.new(-Map.wall_width, Map.size.y),
		      vec2.new(0, Map.size.y)) 

   es.setPolygonShape(Map.bw.entity,
		      4, 
		      vec2.new(0, Map.size.y),
		      vec2.new(0, Map.size.y + Map.wall_width),
		      vec2.new(Map.size.x, Map.size.y + Map.wall_width),
		      vec2.new(Map.size.x, Map.size.y))

   es.setPolygonShape(Map.rw.entity,
		      4, 
		      vec2.new(Map.size.x, 0),
		      vec2.new(Map.size.x + Map.wall_width, 0),
		      vec2.new(Map.size.x + Map.wall_width, Map.size.y),
		      vec2.new(Map.size.x, Map.size.y)) 

   es.setPolygonShape(Map.tw.entity,
		      4, 
		      vec2.new(Map.size.x, -Map.wall_width),
		      vec2.new(Map.size.x, 0),
		      vec2.new(0, 0),
		      vec2.new(0, -Map.wall_width))

   -- Spawn enemies
   table.insert(Map.enemies, Worm(vec2.new(600, 300), 32))
   table.insert(Map.enemies, Worm(vec2.new(2900, 100), 16))
   table.insert(Map.enemies, Worm(vec2.new(2800, 1780), 32))
   table.insert(Map.enemies, Worm(vec2.new(500, 1670), 16))
   table.insert(Map.enemies, Worm(vec2.new(1400, 2590), 28))

end

function Map.update()

   -- Disco walls
   local hsv = color.toHSVFromRGB(player:getColor()) + color.new(60 * dt, 0, 0, 0)
   if hsv.h > 360 then
      hsv.h = 0
   end 

   local rgb = color.toRGBFromHSV(hsv)

   Map.lw:setColor(rgb)
   Map.bw:setColor(rgb)
   Map.rw:setColor(rgb)
   Map.tw:setColor(rgb)

   -- Update player logic
   player:update()

   -- Update enemy logic
   for i = #Map.enemies, 1, -1 do
      
      local enemy = Map.enemies[i]

      if enemy.isDead == 1 then
	 table.remove(Map.enemies, i)
      else
	 enemy:update()
      end

   end

end


