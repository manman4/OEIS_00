def A030717(n)
  ary = [1]
  n.times{
    ary += ary.uniq.sort.map{|i| ary.count(i)}
  }
  ary
end

n = 110
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
    break if cnt == 100
  end
}