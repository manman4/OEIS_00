def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + j[1] * s(j[0], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

def B(k, n)
  A([[2, 5 * k], [1, -2 * k], [4, -2 * k]], n)
end

n = 10000
ary = B(29, 2 * n)
(0..n).each{|i|
  j = ary[2 * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
