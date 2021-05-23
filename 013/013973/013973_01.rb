def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A0(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 24 * s(1, i) - 12 * s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A1(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 12 * s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A2(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 12 * s(2, i) + 8 * s(4, i) - 8 * s(1, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A3(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 24 * s(4, i) - 12 * s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 100
ary0 = A0(n)
ary1 = [0] + A1(n)
ary2 = [0] * 2 + A2(n)
ary3 = [0] * 3 + A3(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary0[i] - 480 * ary1[i] - 16896 * ary2[i] + 8192 * ary3[i]
}

def A5(k, n)
  ary = [1]
  p a = [0] + (1..n).map{|i| (2 * s(1, i) - 1 * s(2, i) / 2)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * a[j] * ary[-j]} / i}
  ary
end
p A5(1, n)
