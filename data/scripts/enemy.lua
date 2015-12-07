function WormPart.new(self, pos, index, parent)

   self.parent = parent
   self.index = index

   CollidableSprite.new(self, pos, type.enemy, type.wall)

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

      table.insert(World.enemies, newWorm)
   end

   -- Destroy parts of the old worm
   self.parent.partCount = self.index - 1

   for i = self.index, self.index + remainingParts do
      Sprite.destroy(self.parent.parts[i])
   end

end

function Worm.new(self, pos, partCount)

   MovableCollidableSprite.new(self, pos, type.enemy, type.wall)

   self:setCircleShape(20)
   self:setTextureAndSize(boulderTexture)
   self:setColor(color.new(1,0,0,1))
   self.timer = 0
   self.speed = 100
   self.amplitude = 2
   self.offset = math.min(diamondTexture:size().x, diamondTexture:size().y)
   self.partCount = partCount + 1
   self.parts = {}
   self.isDead = 0

   table.insert(self.parts, self)
   for i = 2, self.partCount do
      table.insert(self.parts, WormPart(pos + vec2.new(i * self.offset, 0), i, self))
   end

end

function Worm.destroy(self)
   
   for i = 1, self.partCount do
      local p = particle.new(explosion)
      p:setPosition(self.parts[i]:getPosition())
      p:play()
      Sprite.destroy(self.parts[i])
   end

   -- Flag this for removal by the enemy manager
   self.isDead = 1

end

function Worm.update(self)

   self.timer = self.timer + dt
   v = self.amplitude * math.sin(self.timer)

   local dir = (player:getPosition() - self:getPosition()).normalize
   local force = vec2.new(dir.x * self.speed, (dir.y + v) * self.speed)
   es.addForce(self.entity, force)

   local angle = math.atan2(dir.y, dir.x)
   self:setRotation(-angle + 1.5 * math.pi)

   for i = 2, self.partCount do

      local dirr = (self.parts[i]:getPosition() - self.parts[i-1]:getPosition()).normalize
      local tp = self.parts[i-1]:getPosition() + (dirr * self.offset)
      local p = self.parts[i]:getPosition()      
      p = p + (tp - p) * 0.3

      self.parts[i]:setPosition(p)

      local angle = math.atan2(dirr.y, dirr.x)
      self.parts[i]:setRotation(-angle + 1.5 * math.pi)

   end

end


