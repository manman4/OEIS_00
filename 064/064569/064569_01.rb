a = 0
ary = [0]
b = []
(1..589932).each{|i|
  if a - i > 0 && !ary.include?(a - i)
    a -= i
  else
    a += i
  end
  ary << a
  b << [i, a, a / i] if a % i == 0
}
p b
