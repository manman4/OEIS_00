def A000041(n)
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

n = 10000
ary = A000041(2 * n)
(0..n).each{|i|
  j = ary[2 * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
