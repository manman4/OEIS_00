# a(n) = 4^(n-1) - Sum_{k=1..floor(n/2)} 4^(n-2*k) * a(k).
def A(n)
  ary = [0]
  (1..n).each{|i|
    s = 4 ** (i - 1)
    (1..(i / 2)).each{|k|
      s -= 4 ** (i - 2 * k) * ary[k]
    }
    ary << s
  }
  ary
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
