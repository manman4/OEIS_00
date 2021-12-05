def A(n)
  ary = [1]
  (1..n).each{|i|
    s = i ** i
    a = [s]
    (1..i).each{|j|
      s += (-1) ** j * (i - j) ** i
      a << s
    }
    ary << a
  }
  ary.flatten
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
