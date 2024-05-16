def A064951(n)
 ary = [1]
 sum = 1
 (2..n).each{|i|
    sum += 2 * (1..i).inject(0){|s, j| s + i.lcm(j)} - i
    ary << sum
  }
  ary
end

n = 10000
ary = A064951(n)
(1..n).each{|i|
  j = ary[i - 1]
  print i
  print ' '
  puts j
}

