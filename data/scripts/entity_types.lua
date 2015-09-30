function debugSprite(sprite)

   font.write("EntityID: " .. tostring(sprite.entity), sprite:getPosition() + vec2.new(-30, -40), true)
   
end

function Enemy:new(texture, pos)

   Sprite.new(self, type.enemy, 0, pos, 20)
   
   self:setTexture(texture)
   self:setColor(color.new(1,1,0,1))
   self:setMass(1)
   self:setOffset(0,0)
   
end

function Bullet:new(player, pos)

   Sprite.new(self, type.playerbullet, bit.bor(type.enemy, type.wall), pos, 20)

   es.addResponder(self.entity)

   self:setTexture(boulderTexture)
   self:setColor(player:getColor())
   self:setMass(0.1)   
   self:setOffset(0, 0)
   self:setScale(0.5)

   testeffect.emitters[1].startColor = player:getColor()
   
   self.effect = particle.new(testeffect)
   self.effect:play()

end

function Bullet:update()

   self.effect:setPosition(self:getPosition())

end
