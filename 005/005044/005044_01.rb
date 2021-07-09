def A(k, n)
  ary = [k] + (1..k - 1).map{|i| i * (k - 1)}
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
ary = A(3, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
