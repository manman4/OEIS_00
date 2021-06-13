def A(n)
  ary = [[1]]
  (1..n).each{|i|
    a = [0]
    (1..((Math.sqrt(i * 8 + 1) - 1) / 2).floor).each{|j|
      b = ary[i - j]
      s = 0
      s += b[j - 1].to_i + 2 * b[j].to_i + b[j + 1].to_i
      a << s
    }
    ary << a
  }
  ary
end

n = 3000
ary = A(n).map{|i| i[1]}
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
