def f(n)
  return 1 if n < 2
  return (1..n).inject(:*)
end

def A(k, n)
  a = [1]
  ary = [1] + [0] * n
  (1..n).each{|i|
    a << 0
    b = [0]
    (0..i - 1).each{|j|
      b[j + 1] = a[j] + (j + 1) * a[j + 1]
    }
    a = b
    (0..i).each{|j|
      if i + j <= n
        ary[i + j] += k ** j * a[j]
      end
    }
  }
  ary
end

n = 1000
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
