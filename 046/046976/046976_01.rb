def f(n)
  return 1 if n == 0
  (1..n).inject(:*)
end

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
  a = A000111(2 * n)
  (0..n).map{|i| a[2 * i]}
end

n = 30
p ary = A000364(n)
(0..n).each{|i|
  j = (ary[i] / f(2 * i).to_r).numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
