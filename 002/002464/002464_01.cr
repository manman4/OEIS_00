def b(ary, n)
  a = (1..n - 1).map{|i| ary[i] - ary[i - 1]}
  (0..n - 2).each{|i|
    return false if a[i] == 1
    return false if a[i] == -1
  }
  true
end

def a(n)
  return 1 if n < 2
  cnt = 0
  (1..n).to_a.each_permutation(n){|s|
    if b(s, n)
      cnt += 1
    end
  }
  cnt
end

p (0..11).map{|i| a(i)}
