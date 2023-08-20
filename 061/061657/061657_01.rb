def isok(n, k)
  a = n.to_s(k)
  b = (n * n).to_s(k)
  # おそらくb.size = a.size * 2 となりそうだが定義通りに計算
  (a.split('') * b.size).sort.join == (b.split('') * a.size).sort.join
end

cnt = 1
(2..1447088).each{|i|
  if isok(i, 3)
    print cnt
    print ' '
    puts i
    cnt += 1
  end
}
