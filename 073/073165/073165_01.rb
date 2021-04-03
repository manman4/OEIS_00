def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def S(n, k)
  (0..k).inject(1){|s, i|
    s * (1..i).inject(1){|t, j|
      t * (n - k + i + j - 1) / (i + j - 1r)
    }
  }.to_i
end

def A(n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << S(i, j)
    }
  }
  ary
end

n = 139
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}