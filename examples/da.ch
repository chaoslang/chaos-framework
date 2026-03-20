struct Vector2 = {
  items: *float,
  count: u64,
  capacity: u64
}

fn Vector2.init(self: *Vector2): void {
  self.capacity = 16;
  self.count = 0;
  self.items = @alloc(float, self.capacity);
}

fn Vector2.append(self: Vector2, v: float): void {
  if (self.count + 1 >= self.capacity){
    self.capacity *= 2;
    var items: *float = @alloc(float, self.capacity);
    @memcpy(items, self.items, self.count * @sizeof(float));
    @free(self.items);
    self.items = items;
  }
  self.items[self.count] = v;
  self.count += 1;
}

fn Vector2.print(self: Vector2): void {
  var i: u64 = 0;

  while (i < self.count){
    print(self.items[i]);
    i++;
  }
}

fn main(): int {
  var v: Vector2;

  v.init();
  
  v.append(3);
  v.append(4);

  v.print();
  return 0;
}
