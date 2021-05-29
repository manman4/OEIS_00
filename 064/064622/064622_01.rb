a = 0
ary = [0]
b = []
c = 0
(1..10000).each{|i|
  if a - i > 0 && !ary.include?(a - i)
    a -= i
  else
    a += i
  end
  ary << a
  d = a / i.to_r
  if c < d
    b << i
    c = d
  end
}
p b
