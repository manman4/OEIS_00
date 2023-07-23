# Number of distinct sums i^3 + j^3 for 1<=i<=j<=n.
def A(n)
  h = {}
  (1..n).each{|i|
    (i..n).each{|j|
      k = i * i * i + j * j * j
      if h.has_key?(k)
        h[k] += 1
      else
        h[k] = 1
      end
    }
  }
  h.size
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}