def A000111(n)
  e_ary = [1]
  ary = [1]
  n.times{|i|
    new_ary = ary.unshift(0)
    (1..i + 1).each{|i| new_ary[i] += new_ary[i - 1]}
    ary = new_ary.reverse
    e_ary << ary[0]
  }
  e_ary
end

def A000182(n)
  a = A000111(2 * n - 1)
  (1..n).map{|i| a[2 * i - 1]}
end

n = 300
ary = A000182(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
