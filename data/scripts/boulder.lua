function TinyBoulder.new(self, pos, dir)

   MovableCollidableSprite.new(self, pos, type.enemy, type.wall)

   self:setCircleShape(10)
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

   MovableCollidableSprite.new(self, pos, type.enemy, type.wall)

   self:setCircleShape(20)
   self:setTextureAndSize(boulderTexture)
   self:setColor(color.new(0,1,0,1))
   self:addResponder()

   self:setVelocity(vec2.new(math.random() * 2 - 1, math.random() * 2 -1))

end

function Boulder.destroy(self)

   for i = 0, 7 do
      local angle = ((2 * 3.14) / 8) * i;
      local dir = vec2.new(math.cos(angle), math.sin(angle));

      table.insert(World.enemies, TinyBoulder(self:getPosition(), dir))
   end

   Sprite.destroy(self)
   self.isDead = 1;
end

function Boulder.update(self)

   self:addForce(self:getVelocity().normalize * 200)

end
