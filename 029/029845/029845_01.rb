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
  b = A([[2, 3 * k], [1, -k], [4, -2 * k]], n)
end

n = 46
ary = B(8, n + 1)
(-1..n).each{|i|
  j = ary[i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
