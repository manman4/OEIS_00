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
  a = []
  (1..k).each{|i| a << i if k % i == 0}
  ary = []
  a.each{|i|
    ary << [2 * i, 5]
    ary << [i, -2]
    ary << [4 * i, -2]
  }
  A(ary, n)
end

n = 10000
ary = B(32, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
