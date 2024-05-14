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

def A061791(n)
  (1..n).map{|i| A(i)}
end

p A061791(60) 


# n = 1000
# (1..n).each{|i|
#   j = A(i)
#   break if j.to_s.size > 1000
#   print i
#   print ' '
#   puts j
# }