def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def B(a)
  s = f(a.inject(:+))
  while a.size > 0
    t = a.size - 1
    b = []
    (0..t).each{|i|
      s /= t - i + a[i]
      b << a[i] - 1 if a[i] > 1
    }
    a = b
  end
  s
end

def A(n)
  a = Array.new(n + 1, 0)
  partition(n, 1, n).each{|ary|
    a[ary.size] += B(ary)
  }
  b = [0]
  (1..n).each{|i| b << b[-1] + a[i]}
  b[1..-1]
end

n = 70
ary = (1..n).map{|i| A(i)}.flatten!
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
