def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A000031(n)
  ary = [1]
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += phi(j) * 2 ** (i / j) if i % j == 0
    } 
    ary << s / i
  }
  ary
end

n = 3500
ary = A000031(n)
(1..n).each{|i|
  j = ary[i]
  j += 2 ** (i / 2 - 1) if i % 2 == 0
  j /= 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
