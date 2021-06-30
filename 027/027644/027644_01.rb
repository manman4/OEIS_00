n = 29
@ary = [[1], [0, 1]]
a = [0, 1]
(2..n).each{|i|
  b = Array.new(i + 1, 0)
  (1..i - 1).each{|j|
    b[j] = a[j - 1] + j * a[j]
  }
  b[i] = 1
  a = b
  @ary << a
}

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def b(n, k)
  (-1) ** (n % 2) * (0..n).inject(0){|s, i| s + (-1) ** (i % 2) * f(i) * @ary[n][i] / (i + 1r) ** k}
end

(0..n).each{|i|
  j = b(i, 2).denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
