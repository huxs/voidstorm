
Test = {}
Test.width = 80
Test.height = 40

function Test.create(pos)
   
   local o = MovableCollidableSprite(pos, type.test, bit.bor(type.test, type.wall))
   o:setSize(vec2.new(Test.width, Test.height))
   o:setOrigin(vec2.new(0.5, 0.5))
   o:setTexture(wallTexture)
   o:setMass(2.0)
   o:setRestitution(5.0)
   o:setRotation(3.14)
   --o:setDisabled(true)

   return o;

end

function Test.initialize()

   Test.sprites = {}

   local s = Test.create(vec2.new(200, 200));   
   es.setPolygonShape(s.entity,
		      4, 
		      vec2.new(-Test.width / 2, -Test.height / 2), 
		      vec2.new(Test.width / 2, -Test.height / 2), 
		      vec2.new(Test.width / 2, Test.height / 2),
		      vec2.new(-Test.width / 2, Test.height / 2))

   table.insert(Test.sprites, s);

   local s = Test.create(vec2.new(Test.width + 200, 200));   
   es.setPolygonShape(s.entity,
		      4, 
		      vec2.new(-Test.width / 2, -Test.height / 2), 
		      vec2.new(Test.width / 2, -Test.height / 2), 
		      vec2.new(Test.width / 2, Test.height / 2),
		      vec2.new(-Test.width / 2, Test.height / 2))

   table.insert(Test.sprites, s);

   -- Radius 40!

   --[[
   local s = Test.create(vec2.new(100, 100 + Test.height));   
   es.setPolygonShape(s.entity,
		      4, 
		      vec2.new(0, 0), 
		      vec2.new(Test.width, 0), 
		      vec2.new(Test.width / 2, Test.height),
		      vec2.new(0, Test.height))

   table.insert(Test.sprites, s);

   local s = Test.create(vec2.new(Test.width / 2 + 100, 100 + Test.height));   
   es.setPolygonShape(s.entity,
		      4, 
		      vec2.new(Test.width / 2, 0), 
		      vec2.new(Test.width, 0), 
		      vec2.new(Test.width, Test.height),
		      vec2.new(0, Test.height))

   table.insert(Test.sprites, s);
   ]]

end

function Test.update()

   local r = Test.sprites[1]:getRotation()

   Test.sprites[1]:setRotation(r + dt * 1.0)

end


