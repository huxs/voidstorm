function class(super)

   local klass = {}

   -- Copy members from super.
   if super then
      for k,v in pairs(super) do
	 klass[k] = v
      end
   end

   local meta = {}

   -- Emulate constructor syntax.
   -- Triggers when a value is called like a function.
   meta.__call = function(self, ...)

      local instance = setmetatable({}, self)
  
      if instance.new then instance:new(...) end
      return instance
   end

   -- Emulate classes using prototyping.
   setmetatable(klass, meta)
   klass.__index = klass

   return klass

end

