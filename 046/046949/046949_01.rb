def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % 2 == 1}
  s
end

def A(n)
  ary = [1]
  s = 1
  (1..n).each{|i|
    s += 24 * s(i)
    ary << s
  }
  ary
end

n = 10000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
