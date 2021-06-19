def A(k, n)
  ary = [1]
  s = 1
  (1..n).each{|i|
    t = s - i - k + 1
    if t > 0 && !ary.include?(t)
      ary << t
    else
       t = s + i + k - 1
       ary << t
    end
    s = t
  }
  ary
end

def B(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

(0..7).each{|i|
  p A(i, 10)
}
n = 5
ary = B(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
