def A030717(n)
  ary = [1]
  (1..n).each{|i|
    ary += ary.uniq.sort.map{|i| ary.count(i)}
    ary.flatten!
  }
  ary
end

n = 510
ary = A030717(n)
max = 0
cnt = 0
(1..ary.size).each{|i|
  j = ary[i - 1]
  if max < j
    max = j
    cnt += 1
    print cnt
    print ' '
    puts j
    break if cnt == 500
  end
}