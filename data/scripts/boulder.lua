function TinyBoulder.new(self, pos, dir)

   MovableCollidableSprite.new(self, pos, type.boulder, bit.bor(type.wall, type.worm, type.player, type.boulder))

   self:setCircleShape(10)
   self:setMass(0.3)
   self:setTexture(boulderTexture)
   self:setSize(boulderTexture:size() / 2)
   self:setColor(color.new(0,1,0,1))
   self:addResponder()

   self:setVelocity(dir)

end

function TinyBoulder.destroy(self)

   Sprite.destroy(self)
   self.isDead = 1;

end

function TinyBoulder.update(self)

   self:addForce(self:getVelocity().normalize * 200)

end

function Boulder.new(self, pos)

   MovableCollidableSprite.new(self, pos, type.boulder, bit.bor(type.wall, type.boulder, type.player, type.worm))

   self:setCircleShape(20)
   self:setTextureAndSize(boulderTexture)
   self:setColor(color.new(0,1,0,1))
   self:addResponder()

   self:setVelocity(vec2.new(math.random() * 2 - 1, math.random() * 2 -1))

end

function Boulder.destroy(self)

   for i = 0, 7 do
      local angle = ((2 * math.pi) / 8) * i;
      local dir = vec2.new(math.cos(angle), math.sin(angle));

      table.insert(World.enemies, TinyBoulder(self:getPosition() + dir * 30, dir))
   end

   Sprite.destroy(self)
   self.isDead = 1;
end

function Boulder.update(self)

   self:addForce(self:getVelocity().normalize * 200)

end
