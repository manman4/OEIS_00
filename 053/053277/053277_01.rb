def A(k, n)
  ary = (k + 1..2 * k + 1).to_a
  ps = Array.new(n + 1){0}
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

def A053277(n)
  ary = Array.new(n + 1, 0)
  (0..n).each{|i|
    a = [0] * (i * (i + 1)) + A(i, n)
    (0..n).each{|j| ary[j] += a[j]}
  }
  ary
end

n = 1000
ary = A053277(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
