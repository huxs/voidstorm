
function Bullet.new(self, parent, dir)

   self.parent = parent

   local scale = 0.5

   MovableCollidableSprite.new(self, parent:getPosition() + dir * 50, type.playerbullet, bit.bor(type.worm, type.boulder, type.wall))
   self:setCircleShape((boulderTexture:size().x / 2) * scale)
   self:addResponder()
   self:setTextureAndSize(boulderTexture)
   self:setColor(player:getColor())
   self:setMass(0.1)   
   self:setScale(scale)

   bullet_trail.emitters[1].colorMin = parent:getColor()
   bullet_trail.emitters[1].colorMax = parent:getColor()   

   self.effect = particle.new(bullet_trail)
   self.effect:play()

end

function Bullet:destroy()

   particle.delete(self.effect)

   Sprite.destroy(self)

end

function Bullet:update(index)

   -- Update the particle emitter to follow the bullet
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

   -- If we hit something we remove the bullet from our parents bullet list
   if #collidedEntities > 0 then
      table.remove(self.parent.bullets, index)
      self:destroy()
   end

end

function Player.new(self, pos)

   MovableCollidableSprite.new(self, pos, type.player, bit.bor(type.wall, type.worm, type.boulder))
   self:setCircleShape(20)
   self:addResponder()
   self:setTextureAndSize(texture.new("../../data/textures/player.dds"))
   self:setColor(color.toRGBFromHSV(color.new(0,1,1,1)))
   self:setMass(1)  

   self.cooldown = 0
   self.bullets = {}
   self.speed = 400
   self.isDead = 0

   self.thruster = particle.new(thruster)
   self.thruster:play()

end

function Player.destroy(self)

   for i = #self.bullets, 1, -1 do
      local bullet = self.bullets[i]      
      bullet:destroy(i)
   end

   particle.delete(self.thruster)

   Sprite.destroy(self)

end

function Player.updateThruster(self, ls)
   
   self.thruster:setPosition(player:getPosition())
   self.thruster:setRotation(player:getRotation())
   self.thruster:setDepth(player:getDepth())

   thruster.emitters[1].colorMin = player:getColor()
   thruster.emitters[2].colorMin = player:getColor()
   thruster.emitters[3].colorMin = player:getColor()
   thruster.emitters[4].colorMin = player:getColor()
   thruster.emitters[1].colorMax = player:getColor()
   thruster.emitters[2].colorMax = player:getColor()
   thruster.emitters[3].colorMax = player:getColor()
   thruster.emitters[4].colorMax = player:getColor()

   if ls > 0.3 then
      thruster.emitters[1].particlesPerEmit = 1
      thruster.emitters[2].particlesPerEmit = 1
      thruster.emitters[1].spawntime = 0.9 - (1 * ls)
      thruster.emitters[2].spawntime = 0.9 - (1 * ls)
      thruster.emitters[1].force = vec2.new(ls * 600, ls * 600)
      thruster.emitters[2].force = vec2.new(ls * 600, ls * 600)
   else
      thruster.emitters[1].particlesPerEmit = 0
      thruster.emitters[2].particlesPerEmit = 0      
   end

   thruster:store()
end

function Player.update(self)

   local leftStick = input.getLeftStick()
   local rightStick = input.getRightStick()

   -- Move the player
   self:addForce(leftStick * self.speed)

   -- Rotate the player
   local ls = #leftStick  
   if ls > 0.3 then
      if ls > 0.96 then
	 ls = 0.96
      end

      angle = math.atan2(leftStick.y, leftStick.x)
      self:setRotation(-angle + 1.5 * math.pi)
   end

   self:animateHue()
   self:updateThruster(ls)

   -- Shoot bullets
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

   -- Update bullets in reverse in case of removal
   for i = #self.bullets, 1, -1 do
      local bullet = self.bullets[i]      
      bullet:update(i)
   end

   -- Destroy sprites we collide with
   local collidedEntities = es.getCollidedEntity(self.entity)
   for i = #collidedEntities, 1, -1 do

      local entity = collidedEntities[i][1]
      local pos = collidedEntities[i][2]
      local sprite = sprites[entity]
      if sprite ~= nil then
	 if sprite:getType() ~= type.wall then
	    local p = particle.new(explosion)
	    p:setPosition(pos)
	    p:play()
	    self:destroy()
	    self.isDead = 1
	 end
      end
   end 

end
