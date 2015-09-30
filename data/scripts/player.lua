function Player:new(pos)

   Sprite.new(self, type.player, bit.bor(type.wall, type.enemy), pos, 20)

   es.addResponder(self.entity)

   self:setTexture(texture.new("../../data/textures/player.dds"))
   self:setColor(color.toRGBFromHSV(color.new(0,1,1,1)))
   self:setMass(1)  
   self:setOffset(0,0)
   self.cooldown = 0
   self.bullets = {}
   self.speed = 400

   self.thruster = particle.new(thruster)
   self.thruster:play()

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
   
   es.addForce(self.entity, leftStick.x * self.speed, leftStick.y * self.speed)

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

   if self.cooldown <= 0 then
      
     if #rightStick > 0.3 then

	 local dir = rightStick.normalize	 
	 
	 local bullet = Bullet(self, player:getPosition() + dir * 50)

	 table.insert(self.bullets, bullet)
	 
	 es.addForce(bullet.entity, dir.x * 800, dir.y * 800)

	 self.cooldown = 0.1
      end

   else

      self.cooldown = self.cooldown - dt
   end

   for i = #self.bullets, 1, -1 do

      local bullet = self.bullets[i]
      
      bullet:update()
      
      local bpos = bullet:getPosition()

      local collidedEntities = es.getCollidedEntity(bullet.entity)
      for j = #collidedEntities, 1, -1 do
	 local entity = collidedEntities[j][1]
	 local pos = collidedEntities[j][2]
	 local obj = objects[entity]
	 if obj ~= nil then	 
	    local p = particle.new(explosion)
	    p:setPosition(vec2.new(pos.x, pos.y))
	    p:play()
	    obj:destroy()
	 end
      end

      if #collidedEntities > 0 then
	 table.remove(self.bullets, i)
	 particle.delete(bullet.effect)
	 bullet:destroy()
      end
      
   end

   local collidedEntities = es.getCollidedEntity(self.entity)
   for i,v in ipairs(collidedEntities) do
      local obj = objects[v[1]]
      if obj ~= nil then
	 local p = particle.new(explosion)
	 p:setPosition(vec2.new(v[2].x, v[2].y))
	 p:play()
	 obj:destroy()
      end
   end 

end
