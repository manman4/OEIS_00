def A010816(n)
  ary = Array.new(n + 1, 0)
  # ヤコビの公式の必要なところだけ取り出す
  i = 0
  j, k = 2 * i + 1, i * (i + 1) / 2
  while k <= n
    i & 1 == 1? ary[k] = -j : ary[k] = j
    i += 1
    j, k = 2 * i + 1, i * (i + 1) / 2
  end
  ary
end

n = 10000
ary = A010816(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
