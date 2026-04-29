# T(0,0) = 1; T(0,k) = 0 if k>0 or if k<0; T(n,k) = k*T(n-1,k-1) + (k+1)*T(n-1,k+1).
# a(n) = T(n,2) for even n, and a(n) = T(n,3) for odd n.
t = Hash.new(0)
t[0] = 1
(1..500).each{|n|
  next_t = Hash.new(0)
  t.each do |k, v|
    next_t[k + 1] += (k + 1) * v
    next_t[k - 1] += k * v
  end
  t = next_t
  j = n.even? ? t[2] : t[3]
  break if j.to_s.size > 1000
  print n
  print " "
  puts j
}
