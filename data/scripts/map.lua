dofile "particles.lua"
dofile "player.lua"
dofile "enemy.lua"

function Map.start()

   Map.size = vec2.new(3000, 3000)
   Map.wall_width = 50

   -- Create player.
   player = Player(vec2.new(50, 300))

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

   table.insert(Map.enemies, Worm(vec2.new(600, 300), 32))
   table.insert(Map.enemies, Worm(vec2.new(2900, 100), 16))
   table.insert(Map.enemies, Worm(vec2.new(2800, 1780), 32))
   table.insert(Map.enemies, Worm(vec2.new(500, 1670), 16))
   table.insert(Map.enemies, Worm(vec2.new(1400, 2590), 28))

end

function Map.update()

   local hsv = color.toHSVFromRGB(player:getColor()) + color.new(60 * dt, 0, 0, 0)
   if hsv.h > 360 then
      hsv.h = 0
   end 

   local rgb = color.toRGBFromHSV(hsv)

   Map.lw:setColor(rgb)
   Map.bw:setColor(rgb)
   Map.rw:setColor(rgb)
   Map.tw:setColor(rgb)

   player:update()

   for i = #Map.enemies, 1, -1 do
      
      local enemy = Map.enemies[i]

      if enemy.isDead == 1 then
	 table.remove(Map.enemies, i)
      else
	 enemy:update()
      end

   end

end


