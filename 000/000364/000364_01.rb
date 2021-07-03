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

def A000364(n)
  p a = A000111(2 * n)
  (0..n).map{|i| a[2 * i]}
end

n = 16
ary = A000364(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
