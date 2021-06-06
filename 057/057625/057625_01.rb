def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A057625(n)
  s = 0
  (1..n).each{|i|
    if n % i == 0
      s += 1r / f(i)
    end
   }
   (s * f(n)).to_i
end

n = 500
(1..n).each{|i|
  j = A057625(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
