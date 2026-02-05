# a(2*n) = a(n) and a(2*n+1) = Sum_{k=0..n} a(k) * a(n-k) with a(0)=1.
def a(n)
  ary = [1]
  (1..n).each{|i|
    if i.even?
      ary << ary[i / 2]
    else
      m = i / 2
      ary << (0..m).inject(0){|s, k| s + ary[k] * ary[m - k]}
    end
  }
  ary
end

n = 10000
ary = a(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}