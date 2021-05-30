def A(k, n)
  ary = (1..k).to_a
  ps = Array.new(n + 1, 0)
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ([0] * k + ps)[0..n]
end

n = 10000
ary = A(7, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
