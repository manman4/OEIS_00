require 'prime'

n = 10000

ary = Array.new(n + 1, 1)

a = [1]
(1..n).each{|i|
  i.prime_division.each{|j|
    ary[j[0]] += j[1]
  }
  a << ary.inject(:*)
  # ary[2] += 1
  # a << 2 * k - ary.inject(:*)
  # ary[2] -= 1
}
(0..n).each{|i|
  j = a[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
