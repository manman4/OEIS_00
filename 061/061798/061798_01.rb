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
  h.to_a.select{|i| i[1] > 1}.size
end

def A061798(n)
  (1..n).map{|i| A(i)}
end

p A061798(80)

n = 436
ary = A061798(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}