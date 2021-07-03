def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A000013(n)
  ary = [1]
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += phi(2 * j) * 2 ** (i / j) if i % j == 0
    } 
    ary << s / (2 * i)
  }
  ary
end

n = 3500
ary = A000013(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
