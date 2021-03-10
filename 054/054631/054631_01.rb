def A(n, k)
  (1..n).inject(0){|s, i| s + k ** (i.gcd(n))} / n
end

def A054631(n)
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      ary << A(i, j)
    }
  }
  ary
end
n = 140
ary = A054631(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
