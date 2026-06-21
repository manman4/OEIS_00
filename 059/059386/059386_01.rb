def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = Sum_{i=1..floor(n/2)} 1/(2^(2*i-1)*(2*i)!) * Sum_{j=1..2*i} (-1)^j * j^(2*n) * binomial(4*i,2*i-j).
def a(n)
  return 1 if n == 0
  sum = 0
  (1..n / 2).each{|i|
    s = 0
    (1..2 * i).each{|j|
      s += (-1) ** j * j ** (2 * n) * ncr(4 * i, 2 * i - j)
    }
    sum += s / (2 ** (2 * i - 1) * (1..(2 * i)).inject(:*))
  }
  sum
end

n = 250
(0..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
