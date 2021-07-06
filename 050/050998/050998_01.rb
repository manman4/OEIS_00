def search(a, size, num)
  if num == 0
    @ary << a.clone
  else
    (2 * size - 1 - num).times{|i|
      if a[i] == 0 && a[i + num + 1] == 0
        a[i], a[i + num + 1] = num, num
        search(a, size, num - 1)
        a[i], a[i + num + 1] = 0, 0
      end
    }
  end
end
def A(n)
  a = [0] * n * 2
  @ary = []
  search(a, n, n)
  @ary.select{|i| i[0] < i[-1]}.map{|i| i.join.to_i}.sort
end
def A050998()
  A(3) + A(4) + A(7) + A(8)
end
ary = A050998()
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
