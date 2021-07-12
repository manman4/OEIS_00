def A(k, n)
  ary = []
  1.step(2 * k + 2, 2){|i| ary << i}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

def A053282(n)
  ary = Array.new(n + 1, 0)
  (0..n).each{|i|
    a = [0] * ((i + 1) * (i + 2) / 2) + A(i, n)
    (0..n).each{|j| ary[j] += a[j]}
  }
  ary
end

n = 1000
ary = A053282(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
