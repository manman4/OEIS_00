# Also the number of ways to stack n triangles in a valley 
# (pointing upwards or downwards depending on row parity). 

def A006950(n)
  return 1 if n < 3
  a = (1..n - 1).map{|i| [i]}
  ary = [[n]]
  while a.size != 0
    b = []
    a.each{|i|
      x = i[-1].to_i
      s = n - i.inject(:+)
      t = x % 2
      (x + t..s - 1).each{|j| b << i.clone + [j]}
      ary << i.clone + [s] if s >= x + t
    }
    a = b
  end
  p ary
  ary.size
end

p (0..10).map{|i| A006950(i)}
