function Bullet.new(self, parent, dir)

   self.parent = parent

   local scale = 0.5

   MovableCollidableSprite.new(self, parent:getPosition() + dir * 50, type.playerbullet, bit.bor(type.enemy, type.wall))
   self:setCircleShape((boulderTexture:size().x / 2) * scale)
   self:addResponder()
   self:setTextureAndSize(boulderTexture)
   self:setColor(player:getColor())
   self:setMass(0.1)   
   self:setScale(scale)

   testeffect.emitters[1].startColor = parent:getColor()
   
   self.effect = particle.new(testeffect)
   self.effect:play()

end

function Bullet:destroy()

   particle.delete(self.effect)

   Sprite.destroy(self)

end

function Bullet:update(index)

   -- Update the particle emitter to follow the bullet.
   self.effect:setPosition(self:getPosition())

   -- Returns a 2D-table with the layout: table = { { entity0, pos0 }, { entity1, pos1 }, ... }
   local collidedEntities = es.getCollidedEntity(self.entity)
   for i = #collidedEntities, 1, -1 do

      local entity = collidedEntities[i][1]
      local pos = collidedEntities[i][2]
      local sprite = sprites[entity]
      if sprite ~= nil then	 
	 
	 local p = particle.new(explosion)
	 p:setPosition(pos)
	 p:play()

	 if sprite:getType() ~= type.wall then
	    sprite:destroy()
	 end

      end
   end

   -- If we hit something we remove the bullet from our parents bullet list.
   if #collidedEntities > 0 then
      table.remove(self.parent.bullets, index)
      self:destroy()
   end

end

function Player.new(self, pos)

   MovableCollidableSprite.new(self, pos, type.player, bit.bor(type.wall, type.enemy))
   self:setCircleShape(20)
   self:addResponder()
   self:setTextureAndSize(texture.new("../../data/textures/player.dds"))
   self:setColor(color.toRGBFromHSV(color.new(0,1,1,1)))
   self:setMass(1)  

   self.cooldown = 0
   self.bullets = {}
   self.speed = 400

   self.thruster = particle.new(thruster)
   self.thruster:play()

end

function Player.destroy(self)

   print("Hello?")

   Sprite.destroy(self)

end

function Player.update(self)

   self:animateHue()

   voidstorm.setCameraPosition(self:getPosition())
   
   self.thruster:setPosition(player:getPosition())
   self.thruster:setRotation(player:getRotation())
   self.thruster:setDepth(player:getDepth())

   -- Override thruster description.
   thruster.emitters[1].startColor = player:getColor()
   thruster.emitters[2].startColor = player:getColor()
   thruster.emitters[3].startColor = player:getColor()
   thruster.emitters[4].startColor = player:getColor()

   local leftStick = input.getLeftStick()
   local rightStick = input.getRightStick()

   -- Move the player
   self:addForce(leftStick * self.speed)

   local ls = #leftStick  
   if ls > 0.3 then

      if ls > 0.96 then
	 ls = 0.96
      end
      
      thruster.emitters[1].particlesPerEmit = 1
      thruster.emitters[2].particlesPerEmit = 1
      thruster.emitters[1].spawnTime = 0.975 - (1 * ls)
      thruster.emitters[2].spawnTime = 0.975 - (1 * ls)
      thruster.emitters[1].force = vec2.new(ls * 600, ls * 600)
      thruster.emitters[2].force = vec2.new(ls * 600, ls * 600)
      
      angle = math.atan2(leftStick.y, leftStick.x)

      self:setRotation(-angle + 1.5 * math.pi)
   else
      thruster.emitters[1].particlesPerEmit = 0
      thruster.emitters[2].particlesPerEmit = 0      
   end

   thruster:store()

   -- Shoot bullets.
   if self.cooldown <= 0 then
     if #rightStick > 0.3 then
	 local dir = rightStick.normalize	 	 
	 local bullet = Bullet(self, dir)
	 bullet:addForce(dir * 800)
	 table.insert(self.bullets, bullet)
	 self.cooldown = 0.1
      end
   else
      self.cooldown = self.cooldown - dt
   end

   -- Update bullets in reverse in case of removal.
   for i = #self.bullets, 1, -1 do
      local bullet = self.bullets[i]      
      bullet:update(i)
   end

   -- Destroy sprites we collide with.
   local collidedEntities = es.getCollidedEntity(self.entity)
   for i = #collidedEntities, 1, -1 do

      local entity = collidedEntities[i][1]
      local pos = collidedEntities[i][2]
      local sprite = sprites[entity]
      if sprite ~= nil then

	 local p = particle.new(explosion)
	 p:setPosition(pos)
	 p:play()

	 if sprite:getType() ~= type.wall then
	    sprite:destroy()
	 end
      end
   end 

end
