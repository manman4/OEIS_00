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

n = 500
ary = A000111(n)
(0..n).each{|i|
  j = (ary[i] / f(i).to_r).numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
