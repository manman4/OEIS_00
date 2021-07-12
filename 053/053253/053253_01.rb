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

def A053253(n)
  ary = Array.new(n + 1, 0)
  (0..n).each{|i|
    a = [0] * i + A(i, n - i)
    (0..n).each{|j| ary[j] += a[j]}
  }
  ary
end

n = 49
ary = A053253(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
