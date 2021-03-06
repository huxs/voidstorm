function WormPart.new(self, pos, index, parent)

   self.parent = parent
   self.index = index

   MovableCollidableSprite.new(self, pos, type.worm, bit.bor(type.wall, type.worm, type.player, type.boulder))

   self:setCircleShape(20)
   self:setTextureAndSize(diamondTexture)
   self:setColor(color.new(1,1,0,1))

end

function WormPart.destroy(self)

   local p = particle.new(explosion)
   p:setPosition(self:getPosition())
   p:play()

   local remainingParts = self.parent.partCount - self.index

   -- Construct a new worm
   if remainingParts > 0 then

      local newWorm = Worm(self.parent.parts[self.index + 1]:getPosition(), 0)
      newWorm.timer = self.parent.timer
      newWorm:setRotation(self.parent:getRotation())

      for i = 2, remainingParts do	 
	 local part = WormPart(self.parent.parts[self.index + i]:getPosition(), i, newWorm)
	 part:setRotation(self.parent.parts[self.index + i]:getRotation())
	 table.insert(newWorm.parts, part)
      end

      newWorm.partCount = remainingParts

      table.insert(enemies, newWorm)
   end

   -- Destroy parts of the old worm
   self.parent.partCount = self.index - 1

   for i = self.index, self.index + remainingParts do
      Sprite.destroy(self.parent.parts[i])
   end

end

function Worm.new(self, pos, partCount)

   MovableCollidableSprite.new(self, pos, type.worm, bit.bor(type.wall, type.worm, type.player, type.boulder, type.test))

   self:setCircleShape(20)
   self:setTextureAndSize(boulderTexture)
   self:setColor(color.new(1,0,0,1))
   self.timer = 0
   self.speed = 100
   self.amplitude = 2
   self.offset = math.max(diamondTexture:size().x, diamondTexture:size().y) - 2.0
   self.partCount = partCount + 1
   self.parts = {}
   self.isDead = 0

   self:randomDestination()

   self.ray = Ray(self:getPosition(), self.destination, type.worm)

   table.insert(self.parts, self)
   for i = 2, self.partCount do
      table.insert(self.parts, WormPart(pos + vec2.new(i * self.offset,  0), i, self))
   end

end

function Worm.randomDestination(self)

   self.destination = vec2.new(math.random() * size.x, math.random() * size.y);

end

function Worm.destroy(self)
   
   for i = 1, self.partCount do
      local p = particle.new(explosion)
      p:setPosition(self.parts[i]:getPosition())
      p:play()
      Sprite.destroy(self.parts[i])
   end

   self.ray:destroy()

   -- Flag this for removal by the enemy manager
   self.isDead = 1

end

function Worm.rayTest(self)

   local p = self:getPosition()
   local d = self.destination - p
   
   self.ray:setPosition(p)
   self.ray:setDirection(d)

   local collidedEntities = es.getCollidedEntity(self.ray.entity)
   for i = #collidedEntities, 1, -1 do      
      local entity = collidedEntities[i][1]
      local pos = collidedEntities[i][2]
      local sprite = sprites[entity]
      if entity ~= self.entity then	 
	 self:randomDestination()
      end
   end

end

function Worm.update(self)

   local s = sprites[self.entity]

   s.timer = s.timer + dt
   v = s.amplitude * math.sin(s.timer)

   s:rayTest()

   local d = s.destination - s:getPosition()
   local dir = d.normalize
   if #d < 15.0 then
      s:randomDestination()
   end
   
   local force = vec2.new(dir.x * s.speed, (dir.y + v) * s.speed)
   es.addForce(s.entity, force)

   local angle = math.atan2(dir.y, dir.x)
   s:setRotation(-angle + 1.5 * math.pi)

   for i = 2, s.partCount do

      local dirr = (s.parts[i]:getPosition() - s.parts[i-1]:getPosition()).normalize
      local tp = s.parts[i-1]:getPosition() + (dirr * s.offset)
      local p = s.parts[i]:getPosition()      
      p = p + (tp - p) * 0.3

      s.parts[i]:setPosition(p)

      local angle = math.atan2(dirr.y, dirr.x)
      s.parts[i]:setRotation(-angle + 1.5 * math.pi)

   end

end
