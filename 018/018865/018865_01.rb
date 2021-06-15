# 10^d - 1‚Ü‚Å‚Åsize‚ªlenˆÈ‰º
def A(n, d, len)
  ary = Array.new(10 ** d, -1)
  ary[1] = 1
  cnt = 1
  f = 1
  while cnt < 10 ** d - 1
    f *= n
    str = f.to_s
    break if str.size > len
    (0..d - 1).each{|i|
      j = str[0..i].to_i
      if ary[j] == -1
        ary[j] = f
        cnt += 1
      end
    }
  end
  # ary[0] = -1
  i = ary[1..-1].index(-1) + 1
  return ary if i == nil
  ary[0..i - 1]
end

d = 4
ary = A(7, d, 1000)
(1..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
