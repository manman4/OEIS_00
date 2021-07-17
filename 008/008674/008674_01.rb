def T(n, k)
  ary = []
  1.step(2 * k, 2){|i| ary << i}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps[n]
end

def A(k, n)
  (0..n).map{|i| T(i, k)}
end

n = 10000
ary = A(5, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
