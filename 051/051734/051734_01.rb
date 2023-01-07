def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << (0..i - 1).inject(0){|s, j| s + (-1) ** j * [j + 1, i - j].min * ary[j]}
  }
  ary
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}