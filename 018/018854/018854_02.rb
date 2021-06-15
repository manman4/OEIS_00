# 10^d - 1‚Ü‚Å
def A(d)
  ary = Array.new(10 ** d, -1)
  ary[1] = 1
  cnt = 1
  f = 1
  s = 1
  while cnt < 10 ** d - 1
    f *= s
    str = f.to_s
    (0..d - 1).each{|i|
      j = str[0..i].to_i
      if ary[j] == -1
        ary[j] = f
        cnt += 1
      end
    }
    s += 1
  end
  ary
end

ary = A(2)
(1..100).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
