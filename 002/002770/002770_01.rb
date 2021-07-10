require 'prime'

def c(n, r)
  return 1 if r == 0
  return c(n, n - r) if r > n - r
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# 4, 8, 12�Ԗ�
def h(n)
  a = [1r / 10]
  (2..n).each{|i| a << 3 * (1..i - 1).inject(0){|s, j| s + (4 * j - 1) * (4 * i - 4 * j - 1) * c(4 * i, 4 * j) * a[j - 1] * a[i - j - 1]} / ((2 * i - 3) * (4 * i - 1) * (4 * i + 1))}
  a
end

def f(n)
  i = 1
  flag = true
  while flag
    i += 1
    j = n - i * i
    flag = false if Math.sqrt(j).to_i ** 2 == j
  end
  j = Math.sqrt(n - i * i).to_i
  i, j = j, i if i % 2 == 0
  return i if (i - j - 1) % 4 == 0
  -i
end

def A(n)
  ary = []
  p_ary = Prime.each(4 * n + 1).select{|i| i % 4 == 1}
  h_ary = h(n)
  4.step(4 * n, 4){|i|
    j = h_ary[i / 4 - 1] - 1r / 2
    p_ary.each{|k|
      break if k - 1 > i
      j -= (2r * f(k)) ** (i / (k - 1)) / k if i % (k - 1) == 0
    }
    ary << j.numerator
  }
  ary
end

n = 14
ary = A(n)[1..-1]
(2..n).each{|i|
  j = ary[i - 2]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
