def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

def B(n)
  p ary1 = [0] +

  [1,1,1,1,2,2,2,2,3,4,4,4,5,6,6,6,8,9,10,10,12,13,
  14,14,16,19,20,21,23,26,27,28,31,34,37,38,43,46,
  49,50,55,60,63,66,71,78,81,84,90,98,104,107,116,
  124,132,135,144,154,163,169,178,192,201,209,220,
  235,247,256]

  ary1 = (0..n).map{|i| ary1[i]}
  ary4 = Array.new(n + 1, 1)
  A(ary1, ary4, n)
end

n = 45
p ary = B(n)
(0..10).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
