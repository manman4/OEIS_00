def f(k, m)
  return 1 if k * m == 0
  s =(1..k * m).inject(:*)
  (1..k).each{|i|
    (1..m).each{|j|
      s /= i + j - 1
    }
  }
  s
end

n = 500
ary = (0..n).map{|i| f(4, i)}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
