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

def B(k, n)
  ary = [1] + (1..k - 1).map{|i| i * 2}
  ps = Array.new(n + 1, 0)
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ([0] * (2 * k - 2) + ps)[0..n]
end

def C(k, n)
  ary0 = A(k, n)
  ary1 = B(k, n)
  (0..n).map{|i| ary0[i] - ary1[i]}
end

n = 10000
ary = C(4, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
