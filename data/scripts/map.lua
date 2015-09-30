
dofile "particles.lua" -- Contains all particle descriptions.
dofile "player.lua" -- Contains the player sprite.
dofile "entity_types.lua" -- Contains bullet, enemy sprites etc.

Map = {}
Map.size = vec2.new(1000, 1000)
Map.wall_width = 20

function Map.start()

      -- Create player.
      player = Player(vec2.new(50, 50))

      -- Create wall entities.
      Map.lw = es.createEntity()
      Map.bw = es.createEntity()
      Map.rw = es.createEntity()
      Map.tw = es.createEntity()

      es.addPolygonShape(Map.lw, type.wall, 0,
			 4, 
			 vec2.new(0, 0),
			 vec2.new(-Map.wall_width, 0),
			 vec2.new(-Map.wall_width, Map.size.y),
			 vec2.new(0, Map.size.y)) 

      es.addPolygonShape(Map.bw, type.wall, 0,
			 4, 
			 vec2.new(0, Map.size.y),
			 vec2.new(0, Map.size.y + Map.wall_width),
			 vec2.new(Map.size.x, Map.size.y + Map.wall_width),
			 vec2.new(Map.size.x, Map.size.y))

      es.addPolygonShape(Map.rw, type.wall, 0,
			 4, 
			 vec2.new(Map.size.x, 0),
			 vec2.new(Map.size.x + Map.wall_width, 0),
			 vec2.new(Map.size.x + Map.wall_width, Map.size.y),
			 vec2.new(Map.size.x, Map.size.y)) 

      es.addPolygonShape(Map.tw, type.wall, 0,
			 4, 
			 vec2.new(Map.size.x, -Map.wall_width),
			 vec2.new(Map.size.x, 0),
			 vec2.new(0, 0),
			 vec2.new(0, -Map.wall_width))
	

      -- Create enemy entities.
      boulder = Enemy(boulderTexture, vec2.new(400, 300))
      wheel = Enemy(wheelTexture, vec2.new(500, 400))
      diamond = Enemy(diamondTexture, vec2.new(100, 200))
      
end

function Map.update()

   player:update()

end


